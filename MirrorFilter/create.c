#include "Ops.h"

#include "logger.h"


FLT_PREOP_CALLBACK_STATUS
MirrorFilterPreCreate (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    )
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The context for the completion routine for this
        operation.

Return Value:

    The return value is the status of the operation.

--*/
{
	//NTSTATUS status;

    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

	DBG_INFO_FUNC_ENTER();

	DBG_INFO_FLAG(Data->Flags, FLTFL_CALLBACK_DATA_IRP_OPERATION);
	DBG_INFO_FLAG(Data->Flags, FLTFL_CALLBACK_DATA_FAST_IO_OPERATION);
	DBG_INFO_FLAG(Data->Flags, FLTFL_CALLBACK_DATA_FS_FILTER_OPERATION);

    ////
    ////  See if this is an operation we would like the operation status
    ////  for.  If so request it.
    ////
    ////  NOTE: most filters do NOT need to do this.  You only need to make
    ////        this call if, for example, you need to know if the oplock was
    ////        actually granted.
    ////

    //if (MirrorFilterDoRequestOperationStatus( Data )) {

    //    status = FltRequestOperationStatusCallback( Data,
    //                                                MirrorFilterOperationStatusCallback,
    //                                                (PVOID)(++OperationStatusCtx) );
    //    if (!NT_SUCCESS(status)) {

    //        DBG_ERROR("MirrorFilterPreOperation: FltRequestOperationStatusCallback Failed, status=%08x\n", status);
    //    }
    //}

    // This template code does not do anything with the callbackData, but
    // rather returns FLT_PREOP_SUCCESS_WITH_CALLBACK.
    // This passes the request down to the next miniFilter in the chain.

	DBG_INFO_FUNC_LEAVE();
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}