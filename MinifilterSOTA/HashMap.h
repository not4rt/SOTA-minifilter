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
    int key;
    int value;
    struct Entry* next;
} Entry;

struct HashMap {
    Entry** buckets;
    size_t size;
    size_t count;
};


typedef struct HashMap HashMap;

HashMap* initialize_map(void);
void free_map(HashMap* map);
int insert(HashMap* map, int key, int value);
int remove_entry(HashMap* map, int key);
int find_by_key(HashMap* map, int key);
int find_by_value(HashMap* map, int value);



// This structure will store a dynamic list of integers (keys).
typedef struct {
    int* keys;
    size_t count;
    size_t capacity;
} KeyList;

#endif // HASHMAP_H

