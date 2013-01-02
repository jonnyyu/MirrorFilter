#include "mirror.h"

#include "allocate.h"
#include "logger.h"
#include <mountmgr.h>
#include "string.h"
#include "volume.h"
#include <ntstrsafe.h>



static MIRROR_GLOBAL gMirror;


static 
NTSTATUS 
_AllocateMirrorEntry(
	_Out_ PMIRROR_ENTRY MirrorEntry
);

static
VOID 
_FreeMirrorEntry(
	_In_ PMIRROR_ENTRY MirrorEntry
);

static
NTSTATUS
_LoadMirrorList(
	_In_ PMIRROR_LIST MirrorList);

static
VOID
_FreeMirrorList(
	_In_ PMIRROR_LIST MirrorList);

static
NTSTATUS
_LoadMountPoints(_In_ PMOUNTMGR_MOUNT_POINTS *MountPoints);

static 
VOID
_FreeMountPoints(_In_ PMOUNTMGR_MOUNT_POINTS *MountPoints);

static
NTSTATUS
_CreateVolumeList(_In_ PVOLUME_LIST VolumeList, _In_ PMIRROR_LIST MirrorList, _In_ PMOUNTMGR_MOUNT_POINTS MountPoints);

static 
VOID
_FreeVolumeList(_In_ PVOLUME_LIST VolumeList);

static
PVOLUME_ENTRY
_NewVolumeEntry(
	_In_ PCWSTR SymbolicLinkName,
	_In_ USHORT SymbolicLinkNameLength,
	_In_ PCWSTR DeviceName,
	_In_ USHORT DeviceNameLength
);

static 
VOID
_FreeVolumeEntry(
	_In_ PVOLUME_ENTRY VolumeEntry
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGED, MirrorInitialize)
#pragma alloc_text(PAGED, MirrorUninitialize)
#pragma alloc_text(PAGED, _AllocateMirrorEntry)
#pragma alloc_text(PAGED, _FreeMirrorEntry)
#pragma alloc_text(PAGED, _LoadMirrorList)
#pragma alloc_text(PAGED, _FreeMirrorList)
#pragma alloc_text(PAGED, _LoadMountPoints)
#pragma alloc_text(PAGED, _FreeMountPoints)
#pragma alloc_text(PAGED, _CreateVolumeList)
#pragma alloc_text(PAGED, _FreeVolumeList)
#pragma alloc_text(PAGED, _NewVolumeEntry)
#pragma alloc_text(PAGED, _FreeVolumeEntry)
#endif


NTSTATUS 
MirrorInitialize() 
{
	NTSTATUS status = STATUS_SUCCESS;
	MIRROR_LIST MirrorList = {NULL, NULL};
	PMOUNTMGR_MOUNT_POINTS MountPoints;
	
	DBG_INFO_FUNC_ENTER();

	DBG_INFO("Loading MirrorList\n");
	
	status = _LoadMirrorList(&MirrorList);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(_LoadMirrorList, status);
	}
	else {
		DBG_INFO("Loading MountPoints\n");
		status = _LoadMountPoints(&MountPoints);
		if (!NT_SUCCESS(status)) {
			DBG_ERROR_CALL_FAIL(_LoadingMountPoints, status);
		}
		else
		{
			DBG_INFO("Creating VolumeList from MountPoints and MirrorList\n");
			status = _CreateVolumeList(&gMirror.VolumeList, &MirrorList, MountPoints);
			if (!NT_SUCCESS(status)) {
				
			}

			_FreeVolumeList(&gMirror.VolumeList);
			_FreeMountPoints(&MountPoints);
		}
		_FreeMirrorList(&MirrorList);
	}
	
	DBG_INFO_FUNC_LEAVE();
	return STATUS_SUCCESS;
}

VOID 
MirrorUninitialize() {
	return;
}


//
// Private Implementations
//
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

	newEntry = (PMIRROR_ENTRY)MirAllocatePagedPool(sizeof(MirrorEntry));
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


static
NTSTATUS
_LoadMirrorList(PMIRROR_LIST MirrorList) 
{
	KIRQL oldIrql = 0;
	NTSTATUS status = STATUS_SUCCESS;
	PMIRROR_ENTRY mirrorEntry = NULL;

	KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);
	InitializeListHead(&MirrorList->ListHead);
	KeReleaseSpinLock(&MirrorList->Lock, oldIrql);

	status = _NewMirrorEntry(&mirrorEntry, L"c:\\DriverTest", 13);
	if (NT_SUCCESS(status)) {
		KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);
		InsertTailList(&MirrorList->ListHead, &mirrorEntry->ListEntry);
		KeReleaseSpinLock(&MirrorList->Lock, oldIrql);
	}

	return status;
}

static
VOID
_FreeMirrorList(PMIRROR_LIST MirrorList)
{
	PLIST_ENTRY entry = NULL;
	PMIRROR_ENTRY mirrorEntry = NULL;
	KIRQL oldIrql = 0;

	KeAcquireSpinLock(&MirrorList->Lock, &oldIrql);

	while(!IsListEmpty(&MirrorList->ListHead)) {
		entry = RemoveTailList(&MirrorList->ListHead);
		mirrorEntry = CONTAINING_RECORD(entry, MIRROR_ENTRY, ListEntry);
		_FreeMirrorEntry(mirrorEntry);
	}

	KeReleaseSpinLock(&MirrorList->Lock, oldIrql);
}

static
NTSTATUS
_LoadMountPoints(PMOUNTMGR_MOUNT_POINTS *MountPoints)
{
	UNREFERENCED_PARAMETER(MountPoints);
	return STATUS_SUCCESS;
}

static 
VOID
_FreeMountPoints(PMOUNTMGR_MOUNT_POINTS *MountPoints)
{
	UNREFERENCED_PARAMETER(MountPoints);
}

VOID _MountPointEnumCallback(
	_In_ PCUNICODE_STRING SymbolicLinkName, 
	_In_ PCUNICODE_STRING DeviceName, 
	_In_ PMIRROR_LIST MirrorList,
	_In_ PVOLUME_LIST VolumeList) 
{
	UNREFERENCED_PARAMETER(SymbolicLinkName);
	UNREFERENCED_PARAMETER(DeviceName);
	UNREFERENCED_PARAMETER(MirrorList);
	UNREFERENCED_PARAMETER(VolumeList);

	// process dos drive only
	if (!MOUNTMGR_IS_DRIVE_LETTER(SymbolicLinkName))
		return;

}

static
PVOLUME_ENTRY
_NewVolumeEntry(
	_In_ PCWSTR DosName,
	_In_ USHORT DosNameLength,
	_In_ PCWSTR DeviceName,
	_In_ USHORT DeviceNameLength
)
{
	PVOLUME_ENTRY newEntry = NULL;
	NTSTATUS      status   = STATUS_SUCCESS;
	
	newEntry = (PVOLUME_ENTRY)MirAllocatePagedPool(sizeof(VOLUME_ENTRY));
	if (newEntry == NULL) {
		DBG_ERROR_ALLOC_FAIL(newEntry, sizeof(VOLUME_ENTRY));
		goto Exit;
	}

	InitializeListHead(&newEntry->MirrorList.ListHead);
	
	status = MirAllocateUnicodeString(&newEntry->DosName, DosNameLength);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(newEntry->DosName, DosNameLength);
		MirFreePool(newEntry);
		goto Exit;
	}
	status = MirAllocateUnicodeString(&newEntry->DeviceName, DeviceNameLength);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(newEntry->DeviceName, DeviceNameLength);
		MirFreeUnicodeString(&newEntry->DosName);
		MirFreePool(newEntry);
		goto Exit;
	}

	RtlUnicodeStringCbCopyStringN(&newEntry->DosName, DosName, DosNameLength);
	RtlUnicodeStringCbCopyStringN(&newEntry->DeviceName, DeviceName, DeviceNameLength);

Exit:
	return newEntry;
}

static 
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

static
NTSTATUS
_CreateVolumeList(
	_Out_ PVOLUME_LIST VolumeList, 
	_Out_ PMIRROR_LIST MirrorList, 
	_Out_ PMOUNTMGR_MOUNT_POINTS MountPoints
) 
{
	KIRQL oldIrql = 0;
	NTSTATUS status = STATUS_SUCCESS;
	PVOLUME_ENTRY volumeEntry = NULL;
	
	//Fake solution
	UNREFERENCED_PARAMETER(MirrorList);
	UNREFERENCED_PARAMETER(MountPoints);

	DBG_INFO("Initialize VolumeList ListHead\n");
	KeAcquireSpinLock(&VolumeList->Lock, &oldIrql);
	InitializeListHead(&VolumeList->ListHead);
	KeReleaseSpinLock(&VolumeList->Lock, oldIrql);

	volumeEntry = _NewVolumeEntry(L"\\.\\C:", 5, L"\\.\\Harddisk0\\Partition0", 23);
	if (volumeEntry == NULL) {
		DBG_ERROR_CALL_FAIL(_NewVolumeEntry, STATUS_INSUFFICIENT_RESOURCES);
		goto Exit;
	}
Exit:
	return status;

	//KIRQL oldIrql = 0;
	//NTSTATUS status = STATUS_SUCCESS;

	//DBG_INFO("Initialize VolumeList ListHead\n");
	//KeAcquireSpinLock(&VolumeList->Lock, &oldIrql);
	//InitializeListHead(&VolumeList->ListHead);
	//KeReleaseSpinLock(&VolumeList->Lock, oldIrql);

	//DBG_INFO("Enumerating MountPoints to create VolumeList\n");
	//status = MirEnumMountPoints(MountPoints, _MountPointEnumCallback, MirrorList, VolumeList);
	//if (!NT_SUCCESS(status)) {
	//	DBG_ERROR_CALL_FAIL(MirEnumMountPoints, status);
	//}
//Exit:
//	return status;
}

static 
VOID
_FreeVolumeList(PVOLUME_LIST VolumeList) {
	UNREFERENCED_PARAMETER(VolumeList);
}