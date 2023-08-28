#pragma once

#ifndef __DRIVERHANDLER_H__
#define __DRIVERHANDLER_H__

#include <fltKernel.h>
#include <wdm.h>
#include <HashMap.h>

//
//  The global variable
//

typedef struct _SOTA_DRIVER_DATA {

    BOOLEAN         IsRunning;       // Status of the minifilter
    PFLT_FILTER     Filter;          // Global FLT_FILTER pointer. Apparentely many APIs need this
    PDRIVER_OBJECT  DriverObject;    // Driver Object for the minifilter
    ULONG           UsermodePid;     // PID of the SOTA Usermode application, set by communication

    LIST_ENTRY      IrpOps;          // List of IRP Operations
    ULONG           IrpOpsSize;      // Number of IRP Operations waiting in list_entry
    KSPIN_LOCK      IrpOpsLock;      // Lock of the IrpOps list

    LIST_ENTRY      RootDirs;        // List of protected directories
    ULONG           RootDirsSize;    // Number of protected directories in list_entry
    KSPIN_LOCK      RootDirsLock;    // Lock of RootDirectories list


    //
    //  PFID (Process Family ID) attributes
    //
    HashMap* PidTable;      // Pid -> Pfid dictionary


#if DBG
    //
    // Field to control nature of debug output
    //
    ULONG DebugLevel;
#endif
} SOTA_DRIVER_DATA, *PSOTA_DRIVER_DATA;

SOTA_DRIVER_DATA Globals;



FORCEINLINE
VOID
SotaCancelFileOpen(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ NTSTATUS Status
)
/*++

Routine Description:

    This function cancel the file open. This is supposed to be called at post create if
    the I/O is cancelled.

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    Status - The status code to be returned for this IRP.

Return Value:

    None.

--*/

{
    FltCancelFileOpen(FltObjects->Instance, FltObjects->FileObject);
    Data->IoStatus.Status = Status;
    Data->IoStatus.Information = 0;
}


#endif