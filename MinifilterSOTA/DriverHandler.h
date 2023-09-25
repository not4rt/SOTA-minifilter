#pragma once

#ifndef __DRIVERHANDLER_H__
#define __DRIVERHANDLER_H__

#include <fltKernel.h>
#include <wdm.h>
#include "SotaHash.h"

//
//  The global variable
//

typedef struct _SOTA_DRIVER_DATA {

    BOOLEAN         IsRunning;       // Status of the minifilter
    PFLT_FILTER     Filter;          // Global FLT_FILTER pointer. Apparentely many APIs need this
    PDRIVER_OBJECT  DriverObject;    // Driver Object for the minifilter
    ULONG           UsermodePid;     // PID of the SOTA Usermode application, set by communication


    //
    //  PFID (Process Family ID) attributes
    //
    //PidHashMap* PidTable;      // Pid -> Pfid dictionary
    //PFidHashMap* PFidTable;      // Pfid -> Pids dictionary

    //
    // Communication variables
    //
    PFLT_PORT KernelPort;
    PFLT_PORT UserPort;

#if DBG
    //
    // Field to control nature of debug output
    //
    ULONG DebugLevel;
#endif
} SOTA_DRIVER_DATA, *PSOTA_DRIVER_DATA;

SOTA_DRIVER_DATA Globals;


NTSTATUS mapProcess(
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