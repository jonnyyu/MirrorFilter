#pragma once

#include <fltKernel.h>
#include "mirror_entry.h"

typedef struct _MIRROR_GLOBAL {
	MIRROR_LIST MirrorList; 
	PFLT_FILTER Filter;
} MIRROR_GLOBAL;

NTSTATUS 
MirrorInitialize(
	_In_ PUNICODE_STRING RegistryPath,
	_In_ PFLT_FILTER Filter
);

VOID 
MirrorUninitialize();

NTSTATUS
MirrorAttachInstance(
	_In_ PCFLT_RELATED_OBJECTS FltObjects
);