#include "mirror_entry.h"

#include "logger.h"
#include "allocate.h"
#include <ntstrsafe.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGED, _NewMirrorEntry)
#pragma alloc_text(PAGED, _FreeMirrorEntry)
#pragma alloc_text(PAGED, _LoadMirrorList)
#pragma alloc_text(PAGED, _FreeMirrorList)
#endif


NTSTATUS 
_NewMirrorEntry(
	_Out_ PMIRROR_ENTRY *MirrorEntry,
	_In_ PWSTR VolumePath,
	_In_ ULONG VolumePathLength    /* Size in bytes, not including the trail null */
) 
{
	PMIRROR_ENTRY newEntry = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	if (MirrorEntry == NULL) {
		DBG_ERROR("MirrorEntry cannot be NULL\n");
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	if (VolumePathLength >= MAXUSHORT) {
		DBG_ERROR("VolumePathLength %ul exceeds MAXUSHORT\n", VolumePathLength);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	newEntry = (PMIRROR_ENTRY)MirAllocatePagedPool(sizeof(MIRROR_ENTRY));
	if (newEntry == NULL) {
		DBG_ERROR_ALLOC_FAIL(MirrorEntry, sizeof(MirrorEntry));
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	status = MirAllocateUnicodeString(&newEntry->MirrorPath, (USHORT)VolumePathLength);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(newEntry->MirrorPath, (USHORT)VolumePathLength);
		MirFreePool(newEntry);
		goto Exit;
	}

	status = RtlUnicodeStringCbCopyStringN(&newEntry->MirrorPath, VolumePath, VolumePathLength);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(RtlUnicodeStringCbCopyStringN, status);
		MirFreeUnicodeString(&newEntry->MirrorPath);
		MirFreePool(newEntry);
		goto Exit;
	}

	*MirrorEntry = newEntry;
Exit:
	return status;
}

VOID 
_FreeMirrorEntry(
	_In_ PMIRROR_ENTRY MirrorEntry
) 
{
	MirFreeUnicodeString(&MirrorEntry->MirrorPath);
	MirFreePool(MirrorEntry);
}


NTSTATUS
_LoadMirrorList(PMIRROR_LIST MirrorList) 
{
	KIRQL oldIrql = 0;
	NTSTATUS status = STATUS_SUCCESS;
	PMIRROR_ENTRY mirrorEntry = NULL;

	KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);
	InitializeListHead(&MirrorList->ListHead);
	KeReleaseSpinLock(&MirrorList->Lock, oldIrql);

	status = _NewMirrorEntry(&mirrorEntry, L"c:\\DriverTest", 13*sizeof(WCHAR));
	if (NT_SUCCESS(status)) {
		KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);
		InsertTailList(&MirrorList->ListHead, &mirrorEntry->ListEntry);
		KeReleaseSpinLock(&MirrorList->Lock, oldIrql);
	}

	return status;
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
		_FreeMirrorEntry(mirrorEntry);
	}
}