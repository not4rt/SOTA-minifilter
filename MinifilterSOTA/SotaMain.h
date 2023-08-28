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

PWCHAR SensitiveFolders[] = {
    L"\\Users\\User\\Desktop",
    L"\\Users\\User\\Documents",
    L"\\Users\\User\\Downloads",
    L"\\Users\\User\\OneDrive",
    L"\\Users\\User\\Pictures",
};
#define SENSITIVEFOLDERS_COUNT (sizeof(SensitiveFolders) / sizeof(PWCHAR))

PWCHAR CanaryFiles[] = {
    L"\\Users\\User\\Documents\\test_file_2.txt"
};
#define CANARYFILES_COUNT (sizeof(CanaryFiles) / sizeof(PWCHAR))

BOOLEAN
StringStartsWithAnyPrefix(
    PUNICODE_STRING target,
    PWCHAR* PrefixList,
    int count
);

#endif
