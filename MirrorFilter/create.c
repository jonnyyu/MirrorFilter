#include "Ops.h"

#include "logger.h"

#include "Ops.h"
#include <fltKernel.h>
#include "mirror_entry.h"
#include "context.h"

static
NTSTATUS
_GetFileNameInformation(
	_In_ PFLT_CALLBACK_DATA Data,
	_Out_ PFLT_FILE_NAME_INFORMATION *FileNameInfo
);

FLT_PREOP_CALLBACK_STATUS
MirrorPreCreate (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
/*++
Routine Description:
	
	This function implements the folder mirroring logic in PreCreate callback context.

Arguments:

	Data - Pointer to the filter callback data that is passed to us.

	FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

	CompletionContext - The context for the completion routine for this
        operation.

Remarks:

	This function updates the File Object's Name from mirrored path to its original path.
	For paths not mirrored, the function will not change File Object and return FLT_PREOP_SUCCESS_WITH_CALLBACK.

Return Value:

	The return value is the status of the operation.
--*/
{
	UNICODE_STRING newFileName;
	PFLT_FILE_NAME_INFORMATION fileNameInfo = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	PMIRROR_INSTANCE_CONTEXT instanceContext = NULL;

	UNREFERENCED_PARAMETER(CompletionContext);

	// Get the instance context so that we can get the mirror path on this instance
	// if no context found means this instance doesn't have any mirrored path.
	status = FltGetInstanceContext(FltObjects->Instance, &instanceContext);
	if ( status == STATUS_NOT_FOUND ) {
		goto Exit;
	}

	RtlInitUnicodeString(&newFileName, NULL);
	
	// Get current file name from the File Object.
	status = _GetFileNameInformation(Data, &fileNameInfo);

	DBG_INFO("Name: %wZ, Volume: %wZ, ParentDir: %wZ, Share: %wZ, FinalComponent: %wZ\n", 
					fileNameInfo->Name,
					fileNameInfo->Volume,
					fileNameInfo->ParentDir,
					fileNameInfo->Share,
					fileNameInfo->FinalComponent);





Exit:
	if (fileNameInfo != NULL) {
		FltReleaseFileNameInformation(fileNameInfo);
	}
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}


static
NTSTATUS
_GetFileNameInformation(
	_In_ PFLT_CALLBACK_DATA Data,
	_Out_ PFLT_FILE_NAME_INFORMATION *FileNameInfo
)
/*++
Routine Description:

    Get parsed file name information from filter callback data.

Arguments:

    Data - Pointer to the filter callback Data that is passed to us.

    FileNameInfo - Pointer to pass out the parsed file name information.

Return Value:

    The return value is the status of the operation.

--*/
{
	PFLT_FILE_NAME_INFORMATION nameInfo;
	NTSTATUS status = STATUS_SUCCESS;

	DBG_INFO_FUNC_ENTER();
	//
    //  The SL_OPEN_TARGET_DIRECTORY flag indicates the caller is attempting
    //  to open the target of a rename or hard link creation operation. We
    //  must clear this flag when asking fltmgr for the name or the result
    //  will not include the final component. We need the full path in order
    //  to compare the name to our mapping.
    //
	if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY)) {

		ClearFlag(Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY);
	
		//
        //  Get the filename as it appears below this filter. Note that we use 
        //  FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY when querying the filename
        //  so that the filename as it appears below this filter does not end up
        //  in filter manager's name cache.
        //

        status = FltGetFileNameInformation( Data,
                                            FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY,
                                            &nameInfo );
		//
        //  Restore the SL_OPEN_TARGET_DIRECTORY flag so the create will proceed 
        //  for the target. The file systems depend on this flag being set in 
        //  the target create in order for the subsequent SET_INFORMATION 
        //  operation to proceed correctly.
        //
		SetFlag(Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY);
	}
	else 
	{
		//
        //  Note that we use FLT_FILE_NAME_QUERY_DEFAULT when querying the 
        //  filename. In the precreate the filename should not be in filter
        //  manager's name cache so there is no point looking there.
        //
		status = FltGetFileNameInformation( Data,
											FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_DEFAULT,
											&nameInfo);
	}

	if ( !NT_SUCCESS(status) ) {
		DBG_ERROR_CALL_FAIL(FltGetFileNameInformation, status);
		goto Exit;
	}

	status = FltParseFileNameInformation(nameInfo);
	
	if ( !NT_SUCCESS(status) ) {
		DBG_ERROR_CALL_FAIL(FltGetFileNameInformation, status);
		goto Exit;
	}

	*FileNameInfo = nameInfo;
Exit:
	DBG_INFO_FUNC_LEAVE();
	return status;
}