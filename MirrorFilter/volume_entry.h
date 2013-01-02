#pragma once

#include <fltKernel.h>
#include "mirror_entry.h"

typedef struct _VOLUME_ENTRY {
	UNICODE_STRING DeviceName;
	UNICODE_STRING DosName;
	MIRROR_LIST MirrorList;
	LIST_ENTRY VolumeListEntry;
} VOLUME_ENTRY, *PVOLUME_ENTRY;

typedef struct _VOLUME_LIST {
	LIST_ENTRY ListHead;
	KSPIN_LOCK Lock;
} VOLUME_LIST, *PVOLUME_LIST;


NTSTATUS
_NewVolumeEntry(
	_Out_ PVOLUME_ENTRY *VolumeEntry,
	_In_ PCUNICODE_STRING DosName,
	_In_ PCUNICODE_STRING DeviceName
);
 
VOID
_FreeVolumeEntry(
	_In_ PVOLUME_ENTRY VolumeEntry
);

 
VOID
_FreeVolumeList(
	_In_ PVOLUME_LIST VolumeList
);


BOOLEAN
_VolumeEntryShouldAttach(
	_In_ PVOLUME_ENTRY VolumeEntry,
	_In_ PCUNICODE_STRING VolumeName
);