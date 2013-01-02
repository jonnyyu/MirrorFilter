#include "mirror.h"

#include "allocate.h"
#include "logger.h"
#include "volume.h"


static
NTSTATUS
_LoadMountPoints(_In_ PMOUNTMGR_MOUNT_POINTS *MountPoints);

static 
VOID
_FreeMountPoints(_In_ PMOUNTMGR_MOUNT_POINTS *MountPoints);

static
NTSTATUS
_CreateVolumeList(_In_ PVOLUME_LIST VolumeList, _In_ PMIRROR_LIST MirrorList, _In_ PMOUNTMGR_MOUNT_POINTS MountPoints);


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGED, MirrorInitialize)
#pragma alloc_text(PAGED, MirrorUninitialize)
#pragma alloc_text(PAGED, MirrorShouldAttachVolume)
#pragma alloc_text(PAGED, _LoadMountPoints)
#pragma alloc_text(PAGED, _FreeMountPoints)
#pragma alloc_text(PAGED, _CreateVolumeList)
#endif

static MIRROR_GLOBAL gMirror;

NTSTATUS 
MirrorInitialize() 
{
	NTSTATUS status = STATUS_SUCCESS;
	MIRROR_LIST MirrorList = {NULL, NULL};
	PMOUNTMGR_MOUNT_POINTS MountPoints = NULL;
	
	DBG_INFO_FUNC_ENTER();

	DBG_INFO("Loading MirrorList\n");
	
	status = _LoadMirrorList(&MirrorList);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(_LoadMirrorList, status);
		goto Exit;
	}

	DBG_INFO("Loading MountPoints\n");
	status = _LoadMountPoints(&MountPoints);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(_LoadingMountPoints, status);
		_FreeMirrorList(&MirrorList);
		goto Exit;
	}

	DBG_INFO("Creating VolumeList from MountPoints and MirrorList\n");
	status = _CreateVolumeList(&gMirror.VolumeList, &MirrorList, MountPoints);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(_CreateVolumeList, status);	
		_FreeMountPoints(&MountPoints);
		_FreeMirrorList(&MirrorList);
		goto Exit;
	}

	_FreeVolumeList(&gMirror.VolumeList);
	_FreeMountPoints(&MountPoints);
	_FreeMirrorList(&MirrorList);

Exit:
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
NTSTATUS
_CreateVolumeList(
	_Out_ PVOLUME_LIST VolumeList, 
	_Out_ PMIRROR_LIST MirrorList, 
	_Out_ PMOUNTMGR_MOUNT_POINTS MountPoints
) 
{
	UNICODE_STRING fakeDosName, fakeDeviceName;
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

	RtlInitUnicodeString(&fakeDosName, L"\\??\\C:");
	RtlInitUnicodeString(&fakeDeviceName, L"\\Device\\HarddiskVolume2");
	status = _NewVolumeEntry(&volumeEntry, &fakeDosName, &fakeDeviceName);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(_NewVolumeEntry, status);
		goto Exit;
	}
	KeAcquireSpinLock(&VolumeList->Lock, &oldIrql);
	InsertTailList(&VolumeList->ListHead, &volumeEntry->VolumeListEntry);
	KeReleaseSpinLock(&VolumeList->Lock, oldIrql);
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



NTSTATUS
MirrorShouldAttachVolume(
	_In_ PCUNICODE_STRING VolumeName
)
{
	KIRQL oldIrql = 0;
	BOOLEAN shouldAttach = FALSE;
	PLIST_ENTRY entry = NULL;
	PVOLUME_ENTRY volumeEntry = NULL;
	DBG_INFO_FUNC_ENTER();
	
	KeAcquireSpinLock(&gMirror.VolumeList.Lock, &oldIrql);

	for(entry = gMirror.VolumeList.ListHead.Flink; 
		entry != &gMirror.VolumeList.ListHead;
		entry = entry->Flink) 
	{
		volumeEntry = CONTAINING_RECORD(entry, VOLUME_ENTRY, VolumeListEntry);
		shouldAttach = _VolumeEntryShouldAttach(volumeEntry, VolumeName);
		if (shouldAttach)
			break;
	}	

	KeReleaseSpinLock(&gMirror.VolumeList.Lock, oldIrql);
	
	DBG_INFO_FUNC_LEAVE();
	return shouldAttach ? STATUS_SUCCESS : STATUS_FLT_DO_NOT_ATTACH;
}