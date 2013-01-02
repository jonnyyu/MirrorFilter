#pragma once

#include <fltKernel.h>
#include "volume_entry.h"

typedef struct _MIRROR_GLOBAL {
	VOLUME_LIST VolumeList; 
} MIRROR_GLOBAL;

NTSTATUS 
MirrorInitialize();

VOID 
MirrorUninitialize();

NTSTATUS
MirrorShouldAttachVolume(
	_In_ PCUNICODE_STRING VolumeName
);