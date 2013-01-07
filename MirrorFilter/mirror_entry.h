#pragma once

#include <fltKernel.h>

typedef struct _MIRROR_ENTRY {
	UNICODE_STRING MirrorPath;
	UNICODE_STRING NewPath;
	PFLT_VOLUME MirrorVolume;
	PFLT_VOLUME NewVolume;
	LIST_ENTRY ListEntry;
} MIRROR_ENTRY, *PMIRROR_ENTRY;

typedef struct _MIRROR_LIST {
	LIST_ENTRY ListHead;
	KSPIN_LOCK Lock;
} MIRROR_LIST, *PMIRROR_LIST;

 
NTSTATUS 
MirrorEntryNew(
	_Out_ PMIRROR_ENTRY *MirrorEntry,
	_In_ PUNICODE_STRING MirrorPath,
	_In_ PUNICODE_STRING NewPath,
	_In_ PFLT_FILTER Filter
);

VOID 
MirrorEntryRelease(
	_In_ PMIRROR_ENTRY MirrorEntry
);

NTSTATUS
MirrorEntryParsePaths(
	_In_ PMIRROR_ENTRY MirrorEntry,
	_In_ PFLT_FILTER Filter,
	_In_ BOOLEAN CaseInsenitive
);


NTSTATUS
_MirrorListInitialize(
	_In_ PMIRROR_LIST MirrorList
);

VOID
_FreeMirrorList(
	_In_ PMIRROR_LIST MirrorList
);

VOID
MirrorListInsertTail(
	_In_ PMIRROR_LIST MirrorList,
	_In_ PMIRROR_ENTRY MirrorEntry
);

VOID
MirrorListRemove(
	_In_ PMIRROR_LIST MirrorList,
	_In_ PMIRROR_ENTRY MirrorEntry
);