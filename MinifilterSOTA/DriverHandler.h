#pragma once

#ifndef __DRIVERHANDLER_H__
#define __DRIVERHANDLER_H__

#include "SotaHash.h"

//
//  The global variable
//

typedef struct _SOTA_DRIVER_DATA {

    BOOLEAN         IsRunning;       // Status of the minifilter
    PFLT_FILTER     Filter;          // Global FLT_FILTER pointer. Apparentely many APIs need this
    PDRIVER_OBJECT  DriverObject;    // Driver Object for the minifilter
    ULONG           UserModePid;     // PID of the SOTA Usermode application, set by communication


    //
    //  PFID (Process Family ID) attributes
    //
    //PidHashMap* PidTable;      // Pid -> Pfid dictionary
    //PFidHashMap* PFidTable;      // Pfid -> Pids dictionary


    //sn99
    ULONG irpOpsSize;      // number of irp ops waiting in entry_list
    LIST_ENTRY irpOps;     // list entry bidirectional list of irp ops
    KSPIN_LOCK irpOpsLock; // lock for irp list ops

    //
    // Communication variables
    //
    PCWSTR CommPortName;
    PFLT_PORT CommUserPort;
    PFLT_PORT CommKernelPort;

    BOOLEAN isCommOpen;


#if DBG
    //
    // Field to control nature of debug output
    //
    ULONG DebugLevel;
#endif
} SOTA_DRIVER_DATA, *PSOTA_DRIVER_DATA;

SOTA_DRIVER_DATA Globals;

void mapProcessToItself(_In_ HANDLE Process);

NTSTATUS mapProcessToParent(
    _In_ HANDLE Parent,
    _In_ HANDLE Process
);

NTSTATUS unmapProcess(
    _In_ HANDLE Process
);

VOID clean_tables();

NTSTATUS
GetProcessNameByPid(
    HANDLE pid,
    PUNICODE_STRING* ProcessName
);

NTSTATUS kill_pfamily(int pid);


#endif