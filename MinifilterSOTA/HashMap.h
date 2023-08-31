#pragma once

/*++

Module Name:

	HashMap.h

Abstract:

	Header file which contains the type definitions and global variables 
	of the HashMap used by the Fpid-Pid map. Mainly used by SotaMain module.

Environment:

	Kernel mode

--*/

#ifndef HASHMAP_H
#define HASHMAP_H

#include <ntddk.h>

#define INITIAL_SIZE 100

typedef struct Entry {
    ULONG key;
    ULONG value;
    struct Entry* next;
} Entry;

struct HashMap {
    Entry** buckets;
    ULONGLONG idcount; // how many ids were generated
    size_t size; // total size of the map
    size_t count; // actual size of the map
    KSPIN_LOCK lock;
};


typedef struct HashMap HashMap;

HashMap* initialize_map(void);
void free_map(HashMap* map);
NTSTATUS insert(HashMap* map, ULONG rawkey, ULONG value);
NTSTATUS remove(HashMap* map, ULONG rawkey);
ULONG find_by_key(HashMap* map, ULONG rawkey);
ULONG find_by_value(HashMap* map, ULONG value);



// This structure will store a dynamic list of integers (keys).
typedef struct {
    int* keys;
    size_t count;
    size_t capacity;
} KeyList;

#endif // HASHMAP_H

