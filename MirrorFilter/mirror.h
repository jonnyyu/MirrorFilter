#pragma once

#include <wdm.h>

typedef struct _MIRROR_ENTRY {
	UNICODE_STRING MirrorPath;
	LIST_ENTRY ListEntry;
} MIRROR_ENTRY, *PMIRROR_ENTRY;

typedef struct _MIRROR_LIST {
	LIST_ENTRY ListHead;
	KSPIN_LOCK Lock;
} MIRROR_LIST, *PMIRROR_LIST;


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

typedef struct _MIRROR_GLOBAL {
	VOLUME_LIST VolumeList; 
} MIRROR_GLOBAL;

NTSTATUS MirrorInitialize();

VOID MirrorUninitialize();