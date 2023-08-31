/*++

Module Name:

	sotamain.h

Abstract:

	Header file which contains the structures, type definitions,
    constants, global variables and function prototypes that are
    only visible within the kernel. Mainly used by sotamain module.

Environment:

	Kernel mode

--*/

#pragma once

#ifndef __SOTAMAIN_H__
#define __SOTAMAIN_H__

#include <dontuse.h>
#include <fltKernel.h>
#include <wdm.h>
#include <ntstrsafe.h>

#include <DriverHandler.h>
//#include <HashMap.h>


/*************************************************************************
    Local Function Prototypes
*************************************************************************/

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

NTSTATUS
SotaUnload(
    _Unreferenced_parameter_ FLT_FILTER_UNLOAD_FLAGS Flags
);

NTSTATUS
SotaInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
);

//VOID
//SotaInstanceTeardownStart(
//    _In_ PCFLT_RELATED_OBJECTS FltObjects,
//    _Unreferenced_parameter_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
//);
//
//VOID
//SotaInstanceTeardownComplete(
//    _In_ PCFLT_RELATED_OBJECTS FltObjects,
//    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
//);
//
NTSTATUS
SotaInstanceQueryTeardown(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
SotaPreOperationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);

FLT_POSTOP_CALLBACK_STATUS
SotaPostOperationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

NTSTATUS
GetProcessNameByPid(
    HANDLE pid,
    PUNICODE_STRING* ProcessName
);

NTSTATUS
TerminateProcessByPid(
    HANDLE pid
);

PWCHAR SafeDirectories[] = {
    L"\\Device\\HarddiskVolume1",
    L"\\Device\\HarddiskVolume2",
    L"\\Device\\HarddiskVolume3",
    L"\\Device\\HarddiskVolume4"
};
#define SAFEDIRECTORIES_COUNT (sizeof(SafeDirectories) / sizeof(PWCHAR))

PWCHAR SensitiveDirectories[] = {
    L"\\Users\\User\\"
};
#define SENSITIVEDIRECTORIES_COUNT (sizeof(SensitiveDirectories) / sizeof(PWCHAR))

PWCHAR CanaryFiles[] = {
    L"\\Users\\User\\a-sota.sql",
    L"\\Users\\User\\sota.sql",
    L"\\Users\\User\\Desktop\\a-sota.database",
    L"\\Users\\User\\Desktop\\sota.database",
    L"\\Users\\User\\Documents\\a-sota.txt",
    L"\\Users\\User\\Documents\\sota.txt",
    L"\\Users\\User\\Downloads\\a-sota.txt",
    L"\\Users\\User\\Downloads\\sota.txt"
};
#define CANARYFILES_COUNT (sizeof(CanaryFiles) / sizeof(PWCHAR))

BOOLEAN
StringStartsWithAnyPrefix(
    PUNICODE_STRING target,
    PWCHAR* PrefixList,
    int count
);

VOID SOTAProcessCreateCallback(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);


#endif
