#pragma once

#include "uthash.h"
#include "utlist.h"

struct SotaProcess {
    int Pid;
    int PFid;
    PUNICODE_STRING Pname;
    UT_hash_handle hh; // hash table handler
};

void add_process(int pid, int pfid, PUNICODE_STRING pname);
//void add_process(int pid, PUNICODE_STRING pname);
struct SotaProcess* find_process(int pid);
void delete_process(struct SotaProcess* process);
void clean_pidtable();

typedef struct PidList{
    int Pid;
    struct PidList* next, * prev;
} PidList;

struct SotaPFamily {
    int PFid;
    PidList* Pids;
    UT_hash_handle hh; // hash table handler
};

void add_pfamily(int pfid, int pid);
struct SotaPFamily* find_pfamily(int pfid);
void delete_pfamily(struct SotaPFamily* pfamily, int pid);
void clean_pfidtable();

