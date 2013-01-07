#include "logger.h"
#include <stdarg.h>

static
PCHAR _DbgLevel[] = {
	NULL,      // 0
	"[MIR]ERROR:",   // 1
	"[MIR]WARNING:", // 2
	NULL,      // 3
	"[MIR]INFO:"     // 4
};


VOID
_DbgPrintEx(
	_In_ PCSTR FileName,
	_In_ PCSTR FunctionName,
	_In_ ULONG LineNumber,
	_In_ DbgLevel Level,
	_In_z_ _Printf_format_string_ PCSTR Format,
	...
)
{
	va_list ArgList;

	UNREFERENCED_PARAMETER(FileName);
	UNREFERENCED_PARAMETER(FunctionName);
	UNREFERENCED_PARAMETER(LineNumber);

	va_start(ArgList, Format);
	_DbgPrintExV(NULL, NULL, 0, Level, Format, ArgList);
	va_end(ArgList);
}

VOID
_DbgPrintExV(
	_In_ PCSTR FileName,
	_In_ PCSTR FunctionName,
	_In_ ULONG LineNumber,
	_In_ DbgLevel Level,
	_In_z_ _Printf_format_string_ PCSTR Format,
	_In_ va_list ArgList
)
{
	UNREFERENCED_PARAMETER(FileName);
	UNREFERENCED_PARAMETER(FunctionName);
	UNREFERENCED_PARAMETER(LineNumber);

	vDbgPrintExWithPrefix(_DbgLevel[Level], DPFLTR_IHVDRIVER_ID, Level, Format, ArgList);
}

VOID
_DbgPrint(
	_In_ DbgLevel Level,
	_In_z_ _Printf_format_string_ PCSTR Format,
	...
)
{
	va_list ArgList;
	va_start(ArgList, Format);
	_DbgPrintExV(NULL, NULL, 0, Level, Format, ArgList);
	va_end(ArgList);
}

#define SHOW_FLAG(_fs, _f) \
	if (FlagOn(_fs, _f))  { \
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, MirInfo, " %s", #_f); \
	}

#define SHOW_OPT(_c, _e) \
	if (_c == _e) {      \
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, MirInfo, " %s", #_e); \
	}

VOID
_DbgInfoPrintMajorFunction(
	_In_ PCSTR VariableName,
	_In_ UCHAR MajorFunction
)
{
	_DbgPrint(MirInfo, "%s:%u ", VariableName, MajorFunction);
	SHOW_OPT(MajorFunction, IRP_MJ_CREATE);                
	SHOW_OPT(MajorFunction, IRP_MJ_CREATE_NAMED_PIPE);       
	SHOW_OPT(MajorFunction, IRP_MJ_CLOSE);                   
	SHOW_OPT(MajorFunction, IRP_MJ_READ);                    
	SHOW_OPT(MajorFunction, IRP_MJ_WRITE);                   
	SHOW_OPT(MajorFunction, IRP_MJ_QUERY_INFORMATION);       
	SHOW_OPT(MajorFunction, IRP_MJ_SET_INFORMATION);         
	SHOW_OPT(MajorFunction, IRP_MJ_QUERY_EA);                
	SHOW_OPT(MajorFunction, IRP_MJ_SET_EA);                  
	SHOW_OPT(MajorFunction, IRP_MJ_FLUSH_BUFFERS);           
	SHOW_OPT(MajorFunction, IRP_MJ_QUERY_VOLUME_INFORMATION);
	SHOW_OPT(MajorFunction, IRP_MJ_SET_VOLUME_INFORMATION);  
	SHOW_OPT(MajorFunction, IRP_MJ_DIRECTORY_CONTROL);       
	SHOW_OPT(MajorFunction, IRP_MJ_FILE_SYSTEM_CONTROL);     
	SHOW_OPT(MajorFunction, IRP_MJ_DEVICE_CONTROL);          
	SHOW_OPT(MajorFunction, IRP_MJ_INTERNAL_DEVICE_CONTROL); 
	SHOW_OPT(MajorFunction, IRP_MJ_SHUTDOWN);                
	SHOW_OPT(MajorFunction, IRP_MJ_LOCK_CONTROL);            
	SHOW_OPT(MajorFunction, IRP_MJ_CLEANUP);                 
	SHOW_OPT(MajorFunction, IRP_MJ_CREATE_MAILSLOT);         
	SHOW_OPT(MajorFunction, IRP_MJ_QUERY_SECURITY);          
	SHOW_OPT(MajorFunction, IRP_MJ_SET_SECURITY);            
	SHOW_OPT(MajorFunction, IRP_MJ_POWER);                   
	SHOW_OPT(MajorFunction, IRP_MJ_SYSTEM_CONTROL);          
	SHOW_OPT(MajorFunction, IRP_MJ_DEVICE_CHANGE);           
	SHOW_OPT(MajorFunction, IRP_MJ_QUERY_QUOTA);             
	SHOW_OPT(MajorFunction, IRP_MJ_SET_QUOTA);               
	SHOW_OPT(MajorFunction, IRP_MJ_PNP);                     
	SHOW_OPT(MajorFunction, IRP_MJ_PNP_POWER);               
	SHOW_OPT(MajorFunction, IRP_MJ_MAXIMUM_FUNCTION);


	//
	//  Along with the existing IRP_MJ_xxxx);
	//  this defines all of the operation IDs that can be sent to a mini-filter.
	//
	SHOW_OPT(MajorFunction, IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION);
	SHOW_OPT(MajorFunction, IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION);
	SHOW_OPT(MajorFunction, IRP_MJ_ACQUIRE_FOR_MOD_WRITE);
	SHOW_OPT(MajorFunction, IRP_MJ_RELEASE_FOR_MOD_WRITE);
	SHOW_OPT(MajorFunction, IRP_MJ_ACQUIRE_FOR_CC_FLUSH);
	SHOW_OPT(MajorFunction, IRP_MJ_RELEASE_FOR_CC_FLUSH);


	//
	//  Leave space for additional FS_FILTER codes here
	//
	SHOW_OPT(MajorFunction, IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE);
	SHOW_OPT(MajorFunction, IRP_MJ_NETWORK_QUERY_OPEN);
	SHOW_OPT(MajorFunction, IRP_MJ_MDL_READ);
	SHOW_OPT(MajorFunction, IRP_MJ_MDL_READ_COMPLETE);
	SHOW_OPT(MajorFunction, IRP_MJ_PREPARE_MDL_WRITE);
	SHOW_OPT(MajorFunction, IRP_MJ_MDL_WRITE_COMPLETE);
	SHOW_OPT(MajorFunction, IRP_MJ_VOLUME_MOUNT);
	SHOW_OPT(MajorFunction, IRP_MJ_VOLUME_DISMOUNT);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, MirInfo, "\n");
}

VOID
_DbgInfoPrintMinorFunction(
	_In_ PCSTR VariableName,
	_In_ UCHAR MajorFunction,
	_In_ UCHAR MinorFunction
)
{
	_DbgPrint(MirInfo, "%s:%u ", VariableName, MinorFunction);
	if (MajorFunction == IRP_MJ_PNP) {
		SHOW_OPT(MinorFunction, IRP_MN_START_DEVICE);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_REMOVE_DEVICE);
		SHOW_OPT(MinorFunction, IRP_MN_REMOVE_DEVICE);
		SHOW_OPT(MinorFunction, IRP_MN_CANCEL_REMOVE_DEVICE);
		SHOW_OPT(MinorFunction, IRP_MN_STOP_DEVICE);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_STOP_DEVICE);
		SHOW_OPT(MinorFunction, IRP_MN_CANCEL_STOP_DEVICE);

		SHOW_OPT(MinorFunction, IRP_MN_QUERY_DEVICE_RELATIONS);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_INTERFACE);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_CAPABILITIES);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_RESOURCES);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_RESOURCE_REQUIREMENTS);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_DEVICE_TEXT);
		SHOW_OPT(MinorFunction, IRP_MN_FILTER_RESOURCE_REQUIREMENTS);

		SHOW_OPT(MinorFunction, IRP_MN_READ_CONFIG);
		SHOW_OPT(MinorFunction, IRP_MN_WRITE_CONFIG);
		SHOW_OPT(MinorFunction, IRP_MN_EJECT);
		SHOW_OPT(MinorFunction, IRP_MN_SET_LOCK);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_ID);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_PNP_DEVICE_STATE);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_BUS_INFORMATION);
		SHOW_OPT(MinorFunction, IRP_MN_DEVICE_USAGE_NOTIFICATION);
		SHOW_OPT(MinorFunction, IRP_MN_SURPRISE_REMOVAL);

		#if (NTDDI_VERSION >= NTDDI_WIN7)
		SHOW_OPT(MinorFunction, IRP_MN_DEVICE_ENUMERATED);
		#endif
	}

	//
	// POWER minor function codes
	//
	if (MajorFunction == IRP_MJ_PNP_POWER) {
		SHOW_OPT(MinorFunction, IRP_MN_WAIT_WAKE);
		SHOW_OPT(MinorFunction, IRP_MN_POWER_SEQUENCE);
		SHOW_OPT(MinorFunction, IRP_MN_SET_POWER);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_POWER);
	}


	//
	// WMI minor function codes under IRP_MJ_SYSTEM_CONTROL
	//
	if (MajorFunction == IRP_MJ_SYSTEM_CONTROL) {
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_ALL_DATA);
		SHOW_OPT(MinorFunction, IRP_MN_QUERY_SINGLE_INSTANCE);
		SHOW_OPT(MinorFunction, IRP_MN_CHANGE_SINGLE_INSTANCE);
		SHOW_OPT(MinorFunction, IRP_MN_CHANGE_SINGLE_ITEM);
		SHOW_OPT(MinorFunction, IRP_MN_ENABLE_EVENTS);
		SHOW_OPT(MinorFunction, IRP_MN_DISABLE_EVENTS);
		SHOW_OPT(MinorFunction, IRP_MN_ENABLE_COLLECTION);
		SHOW_OPT(MinorFunction, IRP_MN_DISABLE_COLLECTION);
		SHOW_OPT(MinorFunction, IRP_MN_REGINFO);
		SHOW_OPT(MinorFunction, IRP_MN_EXECUTE_METHOD);
		SHOW_OPT(MinorFunction, IRP_MN_REGINFO_EX);
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, MirInfo, "\n");
}

VOID
_DbgInfoPrintIrpFlags(
	_In_ PCSTR VariableName,
	_In_ ULONG IrpFlags)
{
	_DbgPrint(MirInfo, "%s:%u ", VariableName, IrpFlags);
	SHOW_FLAG(IrpFlags, IRP_NOCACHE);               
	SHOW_FLAG(IrpFlags, IRP_PAGING_IO);             
	SHOW_FLAG(IrpFlags, IRP_MOUNT_COMPLETION);      
	SHOW_FLAG(IrpFlags, IRP_SYNCHRONOUS_API);       
	SHOW_FLAG(IrpFlags, IRP_ASSOCIATED_IRP);        
	SHOW_FLAG(IrpFlags, IRP_BUFFERED_IO);           
	SHOW_FLAG(IrpFlags, IRP_DEALLOCATE_BUFFER);     
	SHOW_FLAG(IrpFlags, IRP_INPUT_OPERATION);       
	SHOW_FLAG(IrpFlags, IRP_SYNCHRONOUS_PAGING_IO); 
	SHOW_FLAG(IrpFlags, IRP_CREATE_OPERATION);      
	SHOW_FLAG(IrpFlags, IRP_READ_OPERATION);        
	SHOW_FLAG(IrpFlags, IRP_WRITE_OPERATION);       
	SHOW_FLAG(IrpFlags, IRP_CLOSE_OPERATION);       
	SHOW_FLAG(IrpFlags, IRP_DEFER_IO_COMPLETION);   
	SHOW_FLAG(IrpFlags, IRP_OB_QUERY_NAME);         
	SHOW_FLAG(IrpFlags, IRP_HOLD_DEVICE_QUEUE);     
	SHOW_FLAG(IrpFlags, IRP_UM_DRIVER_INITIATED_IO);
	SHOW_FLAG(IrpFlags, IRP_QUOTA_CHARGED);         
	SHOW_FLAG(IrpFlags, IRP_ALLOCATED_MUST_SUCCEED);
	SHOW_FLAG(IrpFlags, IRP_ALLOCATED_FIXED_SIZE);  
	SHOW_FLAG(IrpFlags, IRP_LOOKASIDE_ALLOCATION);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, MirInfo, "\n");
}

VOID
_DbgInfoPrintFltFlags(
	_In_ PCSTR VariableName,
	_In_ FLT_CALLBACK_DATA_FLAGS Flags)
{
	_DbgPrint(MirInfo, "%s:%u ", VariableName, Flags);
	//SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_REISSUE_MASK);
    //
    //  The below 3 flags are mutually exclusive.
    //  i.e. only ONE and exacly one hould be set for the callback data.
    //  Once set they should never change
    //
    SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_IRP_OPERATION); // Set for Irp operations
    SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_FAST_IO_OPERATION); // Set for Fast Io operations
    SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_FS_FILTER_OPERATION); // Set for Fs Filter operations
    //
    //  In principle this flag can be set for any operation. Once set it shouldn't change
    //
    SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_SYSTEM_BUFFER); // Set if the buffer passed in for the i/o was a system buffer

    //
    //  Below flags are relevant only for IRP-based i/o - i.e. only
    //  if FLTFL_CALLBACK_DATA_IRP_OPERATION); was set. If the i/o was reissued
    //  both flags will necessarily be set
    //
	if (FlagOn(Flags, FLTFL_CALLBACK_DATA_IRP_OPERATION)) {
		SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_GENERATED_IO); // Set if this is I/O generated by a mini-filter
		SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_REISSUED_IO); // Set if this I/O was reissued
	}

    //
    //  Below 2 flags are set only for post-callbacks.
    //
    SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_DRAINING_IO); // set if this operation is being drained. If set,
    SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_POST_OPERATION); // Set if this is a POST operation

    //
    //  This flag can only be set by Filter Manager, only for an IRP based operation
    //  and only for a post callback. When set, it specifies that a lower level driver
    //  allocated a buffer for AssociatedIrp.SystemBuffer in which the data for
    //  the operation will be returned to the IO manager. Filters need to know this
    //  because when they were called in the PRE operation AssociatedIrp.SystemBuffer
    //  was null and as such their buffer is set to UserBuffer and they have no way of
    //  getting the real data from SystemBuffer. Check the IRP_DEALLOCATE_BUFFER flag for
    //  more details on how this is used by file systems.
    //

    SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_NEW_SYSTEM_BUFFER);

    //
    //  Flags set by mini-filters: these are set by the minifilters and may be reset
    //  by filter manager.
    //
    SHOW_FLAG(Flags, FLTFL_CALLBACK_DATA_DIRTY); // Set by caller if parameters were changed
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, MirInfo, "\n");
}

VOID
_DbgInfoPrintOperationFlags(
	_In_ PCSTR VariableName,
	_In_ UCHAR MajorFunction,
	_In_ UCHAR MinorFunction,
	_In_ ULONG OperationFlags)
{
	UNREFERENCED_PARAMETER(MinorFunction);

	_DbgPrint(MirInfo, "%s:%u ", VariableName, OperationFlags);

	if (MajorFunction == IRP_MJ_CREATE || MajorFunction == IRP_MJ_CREATE_NAMED_PIPE) {
		//
		// Create / Create Named Pipe (IRP_MJ_CREATE/IRP_MJ_CREATE_NAMED_PIPE)
		//
		// The following flags must exactly match those in the IoCreateFile call's
		// options.  The case sensitive flag is added in later, by the parse routine,
		// and is not an actual option to open.  Rather, it is part of the object
		// manager's attributes structure.
		//

		SHOW_FLAG(OperationFlags, SL_FORCE_ACCESS_CHECK);
		SHOW_FLAG(OperationFlags, SL_OPEN_PAGING_FILE);
		SHOW_FLAG(OperationFlags, SL_OPEN_TARGET_DIRECTORY);
		SHOW_FLAG(OperationFlags, SL_STOP_ON_SYMLINK);

		SHOW_FLAG(OperationFlags, SL_CASE_SENSITIVE);
	}

	if (MajorFunction == IRP_MJ_READ || MajorFunction == IRP_MJ_WRITE) {
		//
		// Read / Write (IRP_MJ_READ/IRP_MJ_WRITE)
		//

		SHOW_FLAG(OperationFlags, SL_KEY_SPECIFIED);
		SHOW_FLAG(OperationFlags, SL_OVERRIDE_VERIFY_VOLUME);
		SHOW_FLAG(OperationFlags, SL_WRITE_THROUGH);
		SHOW_FLAG(OperationFlags, SL_FT_SEQUENTIAL_WRITE);
		SHOW_FLAG(OperationFlags, SL_FORCE_DIRECT_WRITE);
		SHOW_FLAG(OperationFlags, SL_REALTIME_STREAM);
	}

	if (MajorFunction == IRP_MJ_DEVICE_CONTROL) {
		//
		// Device I/O Control
		//
		//
		// Same SL_OVERRIDE_VERIFY_VOLUME);as for read/write above.
		//

		SHOW_FLAG(OperationFlags, SL_READ_ACCESS_GRANTED);
		SHOW_FLAG(OperationFlags, SL_WRITE_ACCESS_GRANTED);    // Gap for SL_OVERRIDE_VERIFY_VOLUME
	}

	if (MajorFunction == IRP_MJ_LOCK_CONTROL) {
		//
		// Lock (IRP_MJ_LOCK_CONTROL)
		//

		SHOW_FLAG(OperationFlags, SL_FAIL_IMMEDIATELY);
		SHOW_FLAG(OperationFlags, SL_EXCLUSIVE_LOCK);
	}

	if (MajorFunction == IRP_MJ_DIRECTORY_CONTROL || MajorFunction == IRP_MJ_QUERY_EA ||
		MajorFunction == IRP_MJ_QUERY_QUOTA) {
		//
		// QueryDirectory / QueryEa / QueryQuota (IRP_MJ_DIRECTORY_CONTROL/IRP_MJ_QUERY_EA/IRP_MJ_QUERY_QUOTA))
		//

		SHOW_FLAG(OperationFlags, SL_RESTART_SCAN);
		SHOW_FLAG(OperationFlags, SL_RETURN_SINGLE_ENTRY);
		SHOW_FLAG(OperationFlags, SL_INDEX_SPECIFIED);
	}

	if (MajorFunction == IRP_MJ_DIRECTORY_CONTROL) {
		//
		// NotifyDirectory (IRP_MJ_DIRECTORY_CONTROL)
		//

		SHOW_FLAG(OperationFlags, SL_WATCH_TREE);
	}

	if (MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) {
		//
		// FileSystemControl (IRP_MJ_FILE_SYSTEM_CONTROL)
		//
		//    minor: mount/verify volume
		//

		SHOW_FLAG(OperationFlags, SL_ALLOW_RAW_MOUNT);
	}

	if (MajorFunction == IRP_MJ_SET_INFORMATION) {
		//
		//  SetInformationFile (IRP_MJ_SET_INFORMATION)
		//
		//      Rename/Link Information
		//

		SHOW_FLAG(OperationFlags, SL_BYPASS_ACCESS_CHECK);
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, MirInfo, "\n");
}