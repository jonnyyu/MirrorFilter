#pragma once

#include <fltKernel.h>

typedef struct _MIRROR_ENTRY {
	UNICODE_STRING MirrorPath;
	LIST_ENTRY ListEntry;
} MIRROR_ENTRY, *PMIRROR_ENTRY;

typedef struct _MIRROR_LIST {
	LIST_ENTRY ListHead;
	KSPIN_LOCK Lock;
} MIRROR_LIST, *PMIRROR_LIST;

 
NTSTATUS 
_NewMirrorEntry(
	_Out_ PMIRROR_ENTRY *MirrorEntry,
	_In_ PWSTR VolumePath,
	_In_ ULONG VolumePathLength    /* Size in bytes, not including the trail null */
);

VOID 
_FreeMirrorEntry(
	_In_ PMIRROR_ENTRY MirrorEntry
);

NTSTATUS
_LoadMirrorList(
	_In_ PMIRROR_LIST MirrorList
);

VOID
_FreeMirrorList(
	_In_ PMIRROR_LIST MirrorList
);