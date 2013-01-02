#include <wdm.h>
#include <mountmgr.h>

#include "mirror.h"

typedef 
VOID (*PMOUNTPOINT_ENUMCALLBACK)(
	_In_ PCUNICODE_STRING SymbolicLinkName, 
	_In_ PCUNICODE_STRING DeviceName, 
	_In_ PMIRROR_LIST MirrorList,
	_In_ PVOLUME_LIST VolumeList);

NTSTATUS
MirEnumMountPoints(
	_In_ PMOUNTMGR_MOUNT_POINTS MountPoints, 
	_In_ PMOUNTPOINT_ENUMCALLBACK CallBack, 
	_In_ PMIRROR_LIST MirrorList, 
	_In_ PVOLUME_LIST VolumeList);


NTSTATUS MirGetMountPoints(
	_In_ PMOUNTMGR_MOUNT_POINTS MountPoints, 
	_In_ ULONG Size);