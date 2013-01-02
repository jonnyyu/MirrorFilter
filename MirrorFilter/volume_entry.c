#include "volume_entry.h"

#include "logger.h"
#include "allocate.h"
#include "string.h"
#include <ntstrsafe.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGED, _NewVolumeEntry)
#pragma alloc_text(PAGED, _FreeVolumeEntry)
#pragma alloc_text(PAGED, _FreeVolumeList)
#pragma alloc_text(PAGED, _VolumeEntryShouldAttach)
#endif


NTSTATUS
_NewVolumeEntry(
	_Out_ PVOLUME_ENTRY *VolumeEntry,
	_In_ PCUNICODE_STRING DosName,
	_In_ PCUNICODE_STRING DeviceName
)
{
	PVOLUME_ENTRY newEntry = NULL;
	NTSTATUS      status   = STATUS_SUCCESS;

	if (VolumeEntry == NULL) {
		DBG_ERROR("VolumeEntry cannot be null.\n");
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}
	
	newEntry = (PVOLUME_ENTRY)MirAllocatePagedPool(sizeof(VOLUME_ENTRY));
	if (newEntry == NULL) {
		DBG_ERROR_ALLOC_FAIL(newEntry, sizeof(VOLUME_ENTRY));
		goto Exit;
	}

	InitializeListHead(&newEntry->MirrorList.ListHead);
	
	status = MirAllocateUnicodeString(&newEntry->DosName, DosName->Length);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(newEntry->DosName, DosName->Length);
		MirFreePool(newEntry);
		goto Exit;
	}
	status = MirAllocateUnicodeString(&newEntry->DeviceName, DeviceName->Length);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(newEntry->DeviceName, DeviceName->Length);
		MirFreeUnicodeString(&newEntry->DosName);
		MirFreePool(newEntry);
		goto Exit;
	}

	RtlCopyUnicodeString(&newEntry->DosName, DosName);
	RtlCopyUnicodeString(&newEntry->DeviceName, DeviceName);

	*VolumeEntry = newEntry;

Exit:
	return status;
}


VOID
_FreeVolumeEntry(
	_In_ PVOLUME_ENTRY VolumeEntry
)
{
	DBG_INFO_FUNC_ENTER();
	_FreeMirrorList(&VolumeEntry->MirrorList);
	MirFreeUnicodeString(&VolumeEntry->DeviceName);
	MirFreeUnicodeString(&VolumeEntry->DosName);
	MirFreePool(VolumeEntry);
	DBG_INFO_FUNC_LEAVE();
}

 
VOID
_FreeVolumeList(PVOLUME_LIST VolumeList) {
	UNREFERENCED_PARAMETER(VolumeList);
}

BOOLEAN
_VolumeEntryShouldAttach(
	_In_ PVOLUME_ENTRY VolumeEntry,
	_In_ PCUNICODE_STRING VolumeName)
{
	return RtlEqualUnicodeString(&VolumeEntry->DeviceName, VolumeName, FALSE);
}