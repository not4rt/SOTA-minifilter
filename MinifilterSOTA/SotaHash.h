#pragma once

#include <fltKernel.h>
#ifndef __SOTAHASH_H__
#define __SOTAHASH_H__

#include "uthash.h"
#include "utlist.h"

struct SotaProcess {
    int Pid;
    int PFid;
    PUNICODE_STRING Pname;
    UT_hash_handle hh; // hash table handler
};

void add_process(int pid, int pfid, PUNICODE_STRING pname);
struct SotaProcess* find_process(int pid);
void delete_process(struct SotaProcess* process);
void clean_pidtable();

typedef struct PidList{
    int Pid;
    struct PidList* next, * prev;
} PidList;

typedef struct UniqueDirectoriesAccessed {
    PUNICODE_STRING UniqueDirectory;
    UT_hash_handle hh; // hash table handler
} UniqueDirectoriesAccessed;

struct SotaPFamily {
    int PFid;
    PidList* Pids;
    ULONG COUNT_IRP_MJ_CREATE;
    UniqueDirectoriesAccessed* UniqueDirsCreate;
    ULONG COUNT_IRP_MJ_READ;
    UniqueDirectoriesAccessed* UniqueDirsREAD;
    ULONG COUNT_IRP_MJ_WRITE;
    UniqueDirectoriesAccessed* UniqueDirsWRITE;
    ULONG COUNT_IRP_MJ_SET_INFORMATION;
    UniqueDirectoriesAccessed* UniqueDirsSET_INFORMATION;
    UT_hash_handle hh; // hash table handler
};


void add_pfamily(int pfid, int pid);
struct SotaPFamily* find_pfamily(int pfid);
void delete_pfamily(struct SotaPFamily* pfamily, int pid);
void clean_pfidtable();
void countIRP_pfamily(struct SotaPFamily* pfamily, PUNICODE_STRING file, ULONG irp_type);


#endif