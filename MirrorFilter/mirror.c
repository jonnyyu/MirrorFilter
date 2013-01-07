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
#pragma alloc_text(PAGED, MirrorAttachInstance)
#pragma alloc_text(PAGED, _LoadMountPoints)
#pragma alloc_text(PAGED, _FreeMountPoints)
#pragma alloc_text(PAGED, _CreateVolumeList)
#endif

static MIRROR_GLOBAL gMirror;

typedef BOOLEAN (*PMIRRORENTRYCALLBACK)(
	_In_ PUNICODE_STRING MirrorPath,
	_In_ PUNICODE_STRING NewPath,
	_In_ PVOID Data);

static
BOOLEAN
_MirrorEntryCallback(
	_In_ PUNICODE_STRING MirrorPath,
	_In_ PUNICODE_STRING NewPath,
	_In_ PVOID Data)
{
	PMIRROR_LIST mirrorList = (PMIRROR_LIST)Data;
	PMIRROR_ENTRY entry;
	NTSTATUS status = STATUS_SUCCESS;

	status = MirrorEntryNew(&entry, MirrorPath, NewPath, gMirror.Filter);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(MirrorEntryNew, status);
		goto Exit;
	}
	

	MirrorListInsertTail(mirrorList, entry);
Exit:
	return TRUE;
}

static
NTSTATUS
_EnumerateMirrorConfigurations(
	_In_ PUNICODE_STRING RegistryPath,
	_In_ PMIRRORENTRYCALLBACK Callback,
	_In_ PVOID Data)
{
	UNICODE_STRING mirrorPath, newPath;

	UNREFERENCED_PARAMETER(RegistryPath);

	RtlInitUnicodeString(&mirrorPath, L"\\??\\E:\\base");
	RtlInitUnicodeString(&newPath, L"\\??\\E:\\mirror");

	Callback(&mirrorPath, &newPath, Data);
	return STATUS_SUCCESS;
}


NTSTATUS 
MirrorInitialize(
	_In_ PUNICODE_STRING RegistryPath,
	_In_ PFLT_FILTER Filter
)
{
	NTSTATUS status = STATUS_SUCCESS;
	MIRROR_LIST mirrorList = {NULL, NULL};
	
	DBG_INFO_FUNC_ENTER();
	DBG_INFO("Loading MirrorList\n");

	gMirror.Filter = Filter;
	status = _EnumerateMirrorConfigurations(RegistryPath, _MirrorEntryCallback, &mirrorList);
	if (!NT_SUCCESS(status)) {
		goto Exit;
	}
	gMirror.MirrorList = mirrorList;

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

	RtlInitUnicodeString(&fakeDosName, L"\\??\\E:");
	RtlInitUnicodeString(&fakeDeviceName, L"\\Device\\HarddiskVolume3");
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
MirrorAttachInstance(
	_In_ PCFLT_RELATED_OBJECTS FltObjects
)
/*++
Routine Description:

	This function creates the instance context and attach to given instance.

Arguments:
	
	FltObjects - The Filter callback objects that is passed to us.

	MirrorList - The available mirror entries list.

Remarks:
	The function attaches to instance when this instance's volume has mirror entry belong to it.
	If the instance has at least one mirror entry, the function will create an instance context on it.
	When creating the instance context, the function will move the matched mirror entry from MirrorList to instance context's
	MirrorList. This requires acquire SpinLock of the passed in MirrorList. 

	Also the ownership of the moved mirror entries will be managed by the instance context.

Return Value:
	STATUS_SUCCESS           - If the instance should be attached and the instance context has been created successfully.
	STATUS_FLT_DO_NOT_ATTACH - If the instance doesn't have any mirror entries belong to it. 
							   Caller should not attach this instance in this situation.
	otherwise are error.
--*/
{
	OBJECT_ATTRIBUTES attrs;
	UNICODE_STRING path, volumeName;
	HANDLE fileHandle;
	IO_STATUS_BLOCK ioStatus;
	NTSTATUS status = STATUS_SUCCESS;
	PFILE_OBJECT fileObject = NULL;
	PFLT_VOLUME fltVolume = NULL;
	

	UNREFERENCED_PARAMETER(FltObjects);
	
	DBG_INFO_FUNC_ENTER();

	RtlInitUnicodeString(&path, L"\\??\\C:\\Data");
	InitializeObjectAttributes(&attrs, &path, OBJ_KERNEL_HANDLE, NULL, NULL);
	
	status = ZwCreateFile(&fileHandle,				// FileHandle
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
	
	status = ObReferenceObjectByHandle(fileHandle,
									  GENERIC_READ,
									  *IoFileObjectType,
									  KernelMode,
									  &fileObject,
									  NULL);
	

	status = FltGetVolumeFromFileObject(FltObjects->Filter, fileObject, &fltVolume);
	MirGetFltVolumeName(fltVolume, &volumeName);

	DBG_INFO("VolumeName :%wZ\n", &volumeName);
	
	FltObjectDereference(fltVolume);
	ObDereferenceObject(fileObject);
	ZwClose(fileHandle);

	//KIRQL oldIrql = 0;
	//BOOLEAN shouldAttach = FALSE;
	//PLIST_ENTRY entry = NULL;
	//PVOLUME_ENTRY volumeEntry = NULL;
	//KeAcquireSpinLock(&gMirror.VolumeList.Lock, &oldIrql);

	//for(entry = gMirror.VolumeList.ListHead.Flink; 
	//	entry != &gMirror.VolumeList.ListHead;
	//	entry = entry->Flink) 
	//{
	//	volumeEntry = CONTAINING_RECORD(entry, VOLUME_ENTRY, VolumeListEntry);
	//	shouldAttach = _VolumeEntryShouldAttach(volumeEntry, VolumeName);
	//	if (shouldAttach)
	//		break;
	//}	

	//KeReleaseSpinLock(&gMirror.VolumeList.Lock, oldIrql);
	
	DBG_INFO_FUNC_LEAVE();
	return STATUS_FLT_DO_NOT_ATTACH;
	//return shouldAttach ? STATUS_SUCCESS : STATUS_FLT_DO_NOT_ATTACH;
}