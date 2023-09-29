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
TerminateProcessByPid(
    HANDLE pid
);

const PCWSTR SensitiveDirectories[] = {
    L"\\"
};
#define SENSITIVEDIRECTORIES_COUNT (sizeof(SensitiveDirectories) / sizeof(PCWSTR))

const PCWSTR CanaryFiles[] = {
    L"a-sota.txt",
    L"DoNotTouch.txt"
};
#define CANARYFILES_COUNT (sizeof(CanaryFiles) / sizeof(PCWSTR))

const PCWSTR RansomwareFiles[] = {
    L"ed01ebfbc9eb5bbea545af4d01bf5f1071661840480439c6e5babe8e080e41aa.exe"
};
#define RANSOMWAREFILES_COUNT (sizeof(RansomwareFiles) / sizeof(PCWSTR))

const PCWSTR SensitiveExtensions[] = {
    L"der", 
    L"key", 
    L"crt", 
    L"pdf", 
    L"pem",
    L"sqlite3", 
    L"sqlitedb", 
    L"sql", 
    L"mp3", 
    L"wav", 
    L"wmv", 
    L"mpg", 
    L"mp4", 
    L"jpeg", 
    L"png",
    L"docx", 
    L"doc", 
    L"xlsm", 
    L"xlsx", 
    L"xls", 
    L"pptx",
    L"ppt",
    L"txt"
};
#define SENSITIVEEXTENSIONS_COUNT (sizeof(SensitiveExtensions) / sizeof(PCWSTR))

PCWSTR SafeOrigins1[] = {
    //L"\\Device\\HarddiskVolume1\\Windows",
    //L"\\Device\\HarddiskVolume2\\Windows",
    //L"\\Device\\HarddiskVolume3\\Windows",
    L"\\Device\\HarddiskVolume4\\Program Files(x86)\\",
    L"\\Device\\HarddiskVolume4\\Program Files (x86)\\",
    L"\\Device\\HarddiskVolume4\\Windows\\",
    L"\\Users\\WDKRemoteUser\\AppData\\Local\\Programs\\Microsoft VS Code\\"
};
#define SAFEORIGINS_COUNT1 (sizeof(SafeOrigins1) / sizeof(PCWSTR))

BOOLEAN
StringStartsWithAnySubstring(
    const PUNICODE_STRING target,
    const PCWSTR* PrefixList,
    const int count
);

BOOLEAN
StringEndsWithAnySubstring(
    const PUNICODE_STRING target,
    const PCWSTR* PrefixList,
    const int count
);

NTSTATUS CreateCanaryFile(PFLT_FILTER Filter, PUNICODE_STRING FilePath);

VOID SOTAProcessCreateCallback(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);

#define MAX_PATH_LENGTH 260  // Maximum path length, adjust as needed

#endif
