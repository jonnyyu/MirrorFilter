#include "context.h"

#include "logger.h"
#include "mirror_entry.h"

static
VOID
_InstanceContextCleanup (
	_In_ PMIRROR_INSTANCE_CONTEXT Context
);


VOID
MirrorContextCleanup (
    _In_ PFLT_CONTEXT Context,
    _In_ FLT_CONTEXT_TYPE ContextType
    )
{
	switch (ContextType) {
	case FLT_INSTANCE_CONTEXT:
		_InstanceContextCleanup((PMIRROR_INSTANCE_CONTEXT)Context);
		break;
	default:
		DBG_ERROR("Unknown type of Context\n");
	}
}

NTSTATUS
MirrorInstanceContextSetup (
	_In_ PCFLT_RELATED_OBJECTS FltObjects
)
{
	PFLT_CONTEXT context = NULL;
	PMIRROR_INSTANCE_CONTEXT instanceContext = NULL;
	NTSTATUS status = STATUS_SUCCESS;
#if DBG
	status = FltGetInstanceContext(FltObjects->Instance, &context);
	NT_ASSERT(status == STATUS_NOT_FOUND);
#endif
	status = FltAllocateContext(FltObjects->Filter, 
								FLT_INSTANCE_CONTEXT, 
								MIRROR_INSTANCE_CONTEXT_SIZE,
								NonPagedPool,
								&context);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(instanceContext, MIRROR_INSTANCE_CONTEXT_SIZE);
		goto Exit;
	}

	instanceContext = (PMIRROR_INSTANCE_CONTEXT)instanceContext;
	status = _MirrorListInitialize(&instanceContext->MirrorList);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(_MirrorListInitialize, status);
		FltReleaseContext(context);
		goto Exit;
	}
Exit:
	return status;
}


VOID
_InstanceContextCleanup (
	_In_ PMIRROR_INSTANCE_CONTEXT InstanceContext
)
{
	DBG_INFO("Cleaning up MIRROR_INSTANCE_CONTEXT for volume: %wZ\n", &InstanceContext->VolumeName);
	MirFreeUnicodeString(&InstanceContext->VolumeName);
	_FreeMirrorList(&InstanceContext->MirrorList);
}