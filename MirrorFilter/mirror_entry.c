#include "mirror_entry.h"

#include "logger.h"
#include "allocate.h"
#include <ntstrsafe.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGED, MirrorEntryNew)
#pragma alloc_text(PAGED, MirrorEntryRelease)
#pragma alloc_text(PAGED, _FreeMirrorList)
#endif

_Ret_maybenull_
_Success_(return != NULL)
PMIRROR_ENTRY
MirrorEntryInitEmptyEntry()
/*++
Routine Description
	returns a new instance of empty MirrorEntry.
Return Value
	returns NULL if there's not enough memory for the MirrorEntry;
--*/
{
	PMIRROR_ENTRY newMirrorEntry = NULL;
	newMirrorEntry = (PMIRROR_ENTRY)MirAllocatePagedPool(sizeof(MIRROR_ENTRY));
	if ( newMirrorEntry == NULL ) {
		DBG_ERROR_ALLOC_FAIL(newMirrorEntry, sizeof(MIRROR_ENTRY));
	}
	else {
		RtlInitUnicodeString(&newMirrorEntry->MirrorPath, NULL);
		RtlInitUnicodeString(&newMirrorEntry->NewPath, NULL);
		newMirrorEntry->MirrorVolume = NULL;
		newMirrorEntry->NewVolume = NULL;
	}
	return newMirrorEntry;
}

#define IS_ROOT_PATH(Path) \
	(Path[0] == L'\\' && Path[1] == L'?' && Path[2] == L'?' && Path[3] == L'\\' && \
	((Path[4] >= L'A' && Path[4] <= L'Z')  || (Path[0] >= L'a' && Path[4] <= L'z'))  && \
	Path[5] == L':' && Path[6] == L'\\')

#define PATH_HAS_TRIAL_BACKSLASH(Path, Count) \
	(Path[Count - 1] == L'\\')

#define PATH_HAS_TRIAL_SPACE(Path, Count) \
	(Path[Count - 1] == L' ')

static
NTSTATUS
_MirrorEntrySetPath(
	_In_ PUNICODE_STRING OutputPath,
	_In_ PUNICODE_STRING Path)
/*++
Routine Description
	This is MirrorEntry's private funcion for assigning a path
Arguments
	OutputPath - the variable that is set to
	Path       - the input null-terminated path string, 
Remarks
	The Input Path must be full path in Win32 Name space, in other words, they should be 
	Dos style path prefix \??, e.g. \??\C:\MyDir.
--*/
{
	NTSTATUS status = STATUS_SUCCESS;
	USHORT length /* in characters */, bytes;

	if (!IS_ROOT_PATH(Path->Buffer)) {
		DBG_ERROR("Path '%s' is invalid, not an absolute path\n", Path);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	length = Path->Length / sizeof(WCHAR);
	// FIXME: fix this magic number, not sure why WDK doesn't have this defined.
	if (length > 260/*MAX_PATH*/) {
		DBG_ERROR("Path '%s' is invalid, path too long\n", Path);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	if (PATH_HAS_TRIAL_SPACE(Path->Buffer, length)) {
		DBG_ERROR("Path '%s' is invalid, has trail spaces\n", Path);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	if (PATH_HAS_TRIAL_BACKSLASH(Path->Buffer, length)) {
		DBG_ERROR("Path '%s' has trail slash, please remove it\n", Path);
		goto Exit;
	}

	// because length is smaller than 260, so the conversion should be safe.
	bytes = (USHORT)((length + 1) * sizeof(WCHAR));
	status = MirAllocateUnicodeString(OutputPath, bytes);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(OutputPath, bytes);
		goto Exit;
	}
	RtlUnicodeStringCbCopyN(OutputPath, Path, bytes);
Exit:
	return status;
}

NTSTATUS
MirrorEntryNew(
	_Outptr_ PMIRROR_ENTRY *MirrorEntry,
	_In_z_	 PUNICODE_STRING MirrorPath,
	_In_z_   PUNICODE_STRING NewPath,
	_In_     PFLT_FILTER     Filter
)
/*++
Routine Description
	This function creates a mirror entry with given original and the new paths.
Arguments
	MirrorEntry		 - the mirror entry to set the paths to.
	MirrorPath		 - the original path for the mirror
	NewPath			 - the new path for the mirror
Remarks
	The MirrorPath and NewPath must be null-terminated string. 
	
	The path must be full dos paths to directory, for example "C:\Dir1".
	
	The path should not have trailing backslashes, if it is ended with backslashes
	function returns STATUS_INVALID_PARAMETER.

	This function only assigns the paths, it doesn't do further path validations,
	like if the NewPath and MirrorPath exists and if they point to same dir, etc. 
	please refer to MirrorEntryParsePaths function for validation.

	Return Value:
	STATUS_SUCCESS  for operation succeed.
	STATUS_INSUFFICIENT_RESOURCES not enough memory to set paths.
--*/
{
	PMIRROR_ENTRY mirrorEntry = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	if (MirrorEntry == NULL) {
		DBG_ERROR("MirrorEntry is null\n");
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	mirrorEntry = MirrorEntryInitEmptyEntry();
	if (mirrorEntry == NULL) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	status = _MirrorEntrySetPath(&mirrorEntry->MirrorPath, MirrorPath);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR("Error set MirrorPath");
		goto Exit;
	}
	status = _MirrorEntrySetPath(&mirrorEntry->NewPath, NewPath);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR("Error set NewPath");
		goto Exit;
	}
	status = MirrorEntryParsePaths(mirrorEntry, Filter, TRUE /*Case insensitive */);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR("Error parsing paths");
		goto Exit;
	}

Exit:
	if (!NT_SUCCESS(status) && mirrorEntry != NULL) {
		MirrorEntryRelease(mirrorEntry);
		mirrorEntry = NULL;
	}
	*MirrorEntry = mirrorEntry;
	return status;
}

static
NTSTATUS
_MirrorEntryOpenDirectory(
	_In_ PUNICODE_STRING Path,
	_Outptr_opt_result_maybenull_ PFILE_OBJECT *FileObject)
{
	OBJECT_ATTRIBUTES attrs;
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK ioStatus;
	HANDLE dirHandle = NULL;

	InitializeObjectAttributes(&attrs, Path, OBJ_KERNEL_HANDLE, NULL, NULL);

	status = ZwCreateFile(&dirHandle,				// FileHandle
						 FILE_LIST_DIRECTORY|		// desiredAccess, we are opening a directory
						 SYNCHRONIZE,				// synchronize access so it won't return STATUS_PENDING.
						 &attrs,				    // specify the path
						 &ioStatus,					// iostatus result
						 NULL,						// allocationSize we don't need it.
						 FILE_ATTRIBUTE_NORMAL,		// normal attributes
						 FILE_SHARE_READ,			// we are read-only operation, so sharable with other reads
						 FILE_OPEN,					// open if exist else fail.
						 FILE_DIRECTORY_FILE,       // open a directory
						 NULL,						// EaBuffer not needed
						 0);						// 0 required by WDK
	if (!NT_SUCCESS(status)) {
		goto Exit;
	}

	if (FileObject != NULL) {
		status = ObReferenceObjectByHandle(dirHandle,
										  GENERIC_READ,
										  *IoFileObjectType,
										  KernelMode,
										  FileObject,
										  NULL);
	}
	ZwClose(dirHandle);
Exit:
	return status;
}

static
NTSTATUS
_GetParentPath(
	_In_ PUNICODE_STRING ParentPath,
	_In_ PUNICODE_STRING Path)
{
	USHORT charIndex, count;
	NTSTATUS status = STATUS_SUCCESS;

	count = Path->Length / sizeof(WCHAR);

	NT_ASSERT(IS_ROOT_PATH(Path->Buffer));
	NT_ASSERT(!PATH_HAS_TRIAL_BACKSLASH(Path->Buffer, count));
	NT_ASSERT(!PATH_HAS_TRIAL_SPACE(Path->Buffer, count));

	charIndex = count - 1;
	while (charIndex >= 0) {
		if (Path->Buffer[charIndex] == L'\\')
			break;
		charIndex--;
	}

	if (charIndex < 0) {
		// slash not found in path. invalid path
		DBG_ERROR("path is invalid, '%wZ'\n", Path); 
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	if (charIndex == 6) {
		// because the path starts with \??\c:\, so if we found
		// index == 6, means we found the slash of root path.
		DBG_ERROR("path cannot be a root path, '%wZ'\n", Path);
		goto Exit;
	}

	// create a parent path string, not include the slash
	status = MirAllocateUnicodeString(ParentPath, charIndex * sizeof(WCHAR) );
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(ParentPath, charIndex * sizeof(WCHAR) );
		goto Exit;
	}

	status = RtlUnicodeStringCbCopyN(ParentPath, Path, charIndex * sizeof(WCHAR));
	if (!NT_SUCCESS(status)) {
		DBG_ERROR("Error copy Path to ParentPath\n");
		MirFreeUnicodeString(ParentPath);
		goto Exit;
	}

Exit:
	return status;
}

NTSTATUS
MirrorEntryParsePaths(
	_In_ PMIRROR_ENTRY MirrorEntry,
	_In_ PFLT_FILTER Filter,
	_In_ BOOLEAN CaseInsenitive)
/*++
Routine Description
	This function parses and validates the mirror and new paths
Arguments
	MirrorEntry - MirrorEntry to parse paths
Remarks
	the mirror path must exist and must be a directory.
	the new path's parent path must exist. but the new path must not exist.
	the new path should not be same or child of mirror path.

Return Value
	STATUS_SUCCESS the paths are parsed and validated.
	STATUS_INVALID_PARAMETER
--*/
{
	NTSTATUS status = STATUS_SUCCESS;
	PFILE_OBJECT mirrorFileObject = NULL, parentNewPathFileObject = NULL;
	PFLT_VOLUME mirrorVolume = NULL, newPathVolume = NULL;
	UNICODE_STRING parentOfNewPath;

	RtlInitUnicodeString(&parentOfNewPath, NULL);

	status = _MirrorEntryOpenDirectory(&MirrorEntry->MirrorPath, &mirrorFileObject);
	if (!NT_SUCCESS(status)) {
		switch (status) {
		case STATUS_NOT_FOUND:
			DBG_ERROR("Mirror Path doesn't exist\n");
			break;
		case STATUS_NOT_A_DIRECTORY:
			DBG_ERROR("Mirror Path is not a directory\n");
			break;
		default:
			DBG_ERROR_CALL_FAIL(_MirrorEntryOpenDirectory, status);
		}
		goto Exit;
	}
	DBG_INFO("Open '%wZ' as directory successfully\n", &MirrorEntry->MirrorPath);

	status = _GetParentPath(&parentOfNewPath, &MirrorEntry->NewPath);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR("Cannot get parent path of '%wZ'\n", &MirrorEntry->NewPath);
		goto Exit;
	}
	DBG_INFO("Get parent path of '%wZ' is '%wZ'\n", &MirrorEntry->NewPath, 
													&parentOfNewPath);
	status = _MirrorEntryOpenDirectory(&parentOfNewPath, &parentNewPathFileObject);
	if (!NT_SUCCESS(status)) {
		switch (status) {
		case STATUS_NOT_FOUND:
			DBG_ERROR("NewPath parent directory doesn't exist\n");
			break;
		case STATUS_NOT_A_DIRECTORY:
			DBG_ERROR("NewPath parent directory is not a directory\n");
			break;
		default:
			DBG_ERROR_CALL_FAIL(_MirrorEntryOpenDirectory, status);
		}
		goto Exit;
	}
	DBG_INFO("NewPath parent directory exists\n");

	status = _MirrorEntryOpenDirectory(&MirrorEntry->NewPath, NULL);
	if (status != STATUS_NOT_FOUND) {
		switch(status) {
		case STATUS_SUCCESS:
		case STATUS_NOT_A_DIRECTORY:
			DBG_ERROR("New already exists\n");
			break;
		default:
			DBG_ERROR_CALL_FAIL(_MirrorEntryOpenDirectory, status);
		}
		goto Exit;
	}
	DBG_INFO("NewPath doesn't exist, this is expected\n");

	status = FltGetVolumeFromFileObject(Filter, mirrorFileObject, &mirrorVolume);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(FltGetVolumeFromFileObject, status);
		goto Exit;
	}
	DBG_INFO("Get volume of mirrorPath successfully\n");
	
	status = FltGetVolumeFromFileObject(Filter, parentNewPathFileObject, &newPathVolume);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(FltGetVolumeFromFileObject, status);
		FltObjectDereference(mirrorVolume);
		goto Exit;
	}
	DBG_INFO("Get volume of newPath successfully\n");

	if (newPathVolume == mirrorVolume) {
		// if they are on the same volume, and newPath is child dir of mirrorPath.
		if (RtlPrefixUnicodeString(&mirrorFileObject->FileName, 
							       &parentNewPathFileObject->FileName, 
								   CaseInsenitive)) {
			DBG_ERROR("newPath should not be a child directory of mirrorPath\n");
			goto Exit;
		}
	}

	//all checks done, update mirror entry
	MirrorEntry->MirrorVolume = mirrorVolume;
	mirrorVolume = NULL;
	MirrorEntry->NewVolume = newPathVolume;
	newPathVolume = NULL;

	MirCopyUnicodeString(&MirrorEntry->MirrorPath, &mirrorFileObject->FileName);
	MirCopyUnicodeString(&MirrorEntry->NewPath, &parentNewPathFileObject->FileName);

Exit:
	if (mirrorFileObject) {
		ObDereferenceObject(mirrorFileObject);
		mirrorFileObject = NULL;
	}
	if (parentNewPathFileObject) {
		ObDereferenceObject(parentNewPathFileObject);
		parentNewPathFileObject = NULL;
	}
	MirFreeUnicodeString(&parentOfNewPath);
	return status;
}


VOID 
MirrorEntryRelease(
	_In_ PMIRROR_ENTRY MirrorEntry
) 
{
	MirFreeUnicodeString(&MirrorEntry->MirrorPath);
	MirFreeUnicodeString(&MirrorEntry->NewPath);
	if (MirrorEntry->MirrorVolume != NULL) {
		FltObjectDereference(&MirrorEntry->MirrorVolume);
		MirrorEntry->MirrorVolume = NULL;
	}
	if (MirrorEntry->NewVolume != NULL) {
		FltObjectDereference(&MirrorEntry->NewVolume);
		MirrorEntry->NewVolume = NULL;
	}
	MirFreePool(MirrorEntry);
}


NTSTATUS
_MirrorListInitialize(
	_In_ PMIRROR_LIST MirrorList
)
{
	KIRQL oldIrql = 0;
	
	KeInitializeSpinLock(&MirrorList->Lock);
	KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);
	InitializeListHead(&MirrorList->ListHead);
	KeReleaseSpinLock(&MirrorList->Lock, oldIrql);
	
	return STATUS_SUCCESS;
}


VOID
_FreeMirrorList(PMIRROR_LIST MirrorList)
{
	PLIST_ENTRY entry = NULL;
	PMIRROR_ENTRY mirrorEntry = NULL;
	KIRQL oldIrql = 0;
	BOOLEAN stop = FALSE;

	while(!stop) {

		KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);
		entry = RemoveTailList(&MirrorList->ListHead);
		stop = entry == &MirrorList->ListHead;
		KeReleaseSpinLock(&MirrorList->Lock, oldIrql);

		if (stop) 
			break;

		mirrorEntry = CONTAINING_RECORD(entry, MIRROR_ENTRY, ListEntry);
		MirrorEntryRelease(mirrorEntry);
	}
}

VOID
MirrorListInsertTail(
	_In_ PMIRROR_LIST MirrorList,
	_In_ PMIRROR_ENTRY MirrorEntry
)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);
	InsertTailList(&MirrorList->ListHead, &MirrorEntry->ListEntry);
	KeReleaseSpinLock(&MirrorList->Lock, oldIrql);
}

VOID
MirrorListRemove(
	_In_ PMIRROR_LIST MirrorList,
	_In_ PMIRROR_ENTRY MirrorEntry
)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);
	RemoveEntryList(&MirrorEntry->ListEntry);
	KeReleaseSpinLock(&MirrorList->Lock, oldIrql);
}