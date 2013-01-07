#include <fltKernel.h>
#include "mirror_entry.h"

// FLT_INSTANCE specific information
// Because Each instance is mapped to one volume in the system.
// So I maintain a list of mirror entries on that volume in this context.
// The mirror entry list takes ownership of these mirror entries
// so the mirror entry list will be freed when instance context is cleaned up.
typedef struct _MIRROR_INSTANCE_CONTEXT {
	UNICODE_STRING VolumeName;
	MIRROR_LIST MirrorList;
} MIRROR_INSTANCE_CONTEXT, *PMIRROR_INSTANCE_CONTEXT;

#define MIRROR_INSTANCE_CONTEXT_SIZE         sizeof( MIRROR_INSTANCE_CONTEXT )
#define MIRROR_INSTANCE_CONTEXT_TAG			 'mIxC'

NTSTATUS
MirrorInstanceContextSetup (
	_In_ PCFLT_RELATED_OBJECTS FltObjects
);

VOID
MirrorContextCleanup (
    _In_ PFLT_CONTEXT Context,
    _In_ FLT_CONTEXT_TYPE ContextType
);
