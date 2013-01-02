#include "allocate.h"

#define MIRROR_TAG  'tMiR'

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MirAllocatePagedPool)
#pragma alloc_text(PAGE, MirFreePool)
#endif

PVOID MirAllocatePagedPool(ULONG Size) {
	return ExAllocatePoolWithTag(PagedPool, Size, MIRROR_TAG);
}

VOID MirFreePool(PVOID Block) {
	ExFreePoolWithTag(Block, MIRROR_TAG);
}