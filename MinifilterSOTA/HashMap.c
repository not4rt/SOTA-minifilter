#include "HashMap.h"
#include <ntddk.h>

// Simple hash function.
static unsigned int hash(int key) {
    return key % INITIAL_SIZE;
}

HashMap* initialize_map() {
    HashMap* map = (HashMap*)ExAllocatePool2(NonPagedPool, sizeof(HashMap), 'map1');
    if (!map) return NULL;

    map->size = INITIAL_SIZE;
    map->count = 0;
    map->buckets = (Entry**)ExAllocatePool2(NonPagedPool, sizeof(Entry*) * map->size, 'bckt');
    if (!map->buckets) {
        ExFreePoolWithTag(map, 'map1');
        return NULL;
    }

    for (size_t i = 0; i < map->size; i++) {
        map->buckets[i] = NULL;
    }

    return map;
}

void free_map(HashMap* map) {
    for (size_t i = 0; i < map->size; i++) {
        Entry* current = map->buckets[i];
        while (current) {
            Entry* temp = current;
            current = current->next;
            ExFreePoolWithTag(temp, 'ent1');
        }
    }
    ExFreePoolWithTag(map->buckets, 'bckt');
    ExFreePoolWithTag(map, 'map1');
}

int insert(HashMap* map, int key, int value) {
    unsigned int idx = hash(key);
    Entry* new_entry = (Entry*)ExAllocatePool2(NonPagedPool, sizeof(Entry), 'ent1');
    if (!new_entry) return -1;

    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = NULL;

    if (!map->buckets[idx]) {
        map->buckets[idx] = new_entry;
    }
    else {
        Entry* current = map->buckets[idx];
        while (current->next) {
            current = current->next;
        }
        current->next = new_entry;
    }

    map->count++;

    //// Resize logic
    //if ((double)map->count / map->size > LOAD_FACTOR) {
    //    size_t new_size = map->size * 2;
    //    Entry** new_buckets = (Entry**)ExAllocatePool2(NonPagedPool, sizeof(Entry*) * new_size, 'bckt');
    //    if (!new_buckets) return -1;

    //    for (size_t i = 0; i < new_size; i++) {
    //        new_buckets[i] = NULL;
    //    }

    //    for (size_t i = 0; i < map->size; i++) {
    //        Entry* current = map->buckets[i];
    //        while (current) {
    //            unsigned int new_idx = current->key % new_size;
    //            Entry* next = current->next;
    //            current->next = new_buckets[new_idx];
    //            new_buckets[new_idx] = current;
    //            current = next;
    //        }
    //    }

    //    ExFreePoolWithTag(map->buckets, 'bckt');
    //    map->buckets = new_buckets;
    //    map->size = new_size;
    //}

    return 0;
}

int remove(HashMap* map, int key) {
    unsigned int idx = hash(key);
    Entry* current = map->buckets[idx];
    Entry* prev = NULL;
    while (current) {
        if (current->key == key) {
            if (prev == NULL) {
                map->buckets[idx] = current->next;
            }
            else {
                prev->next = current->next;
            }
            ExFreePoolWithTag(current, 'ent1');
            map->count--;
            return 0; // Removed
        }
        prev = current;
        current = current->next;
    }
    return -1;  // Not found
}

int find_by_key(HashMap* map, int key) {
    unsigned int idx = hash(key);
    Entry* current = map->buckets[idx];
    while (current) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return -1;  // Not found
}

int find_by_value(HashMap* map, int value) {
    for (size_t i = 0; i < map->size; i++) {
        Entry* current = map->buckets[i];
        while (current) {
            if (current->value == value) {
                return current->key;
            }
            current = current->next;
        }
    }
    return -1;  // Not found
}


//
// Keylist functions
//

// Initialize a dynamic list to store keys.
KeyList* initialize_keylist() {
    KeyList* list = (KeyList*)ExAllocatePool2(NonPagedPool, sizeof(KeyList), 'klst');
    if (!list) return NULL;

    list->count = 0;
    list->capacity = 10;  // Initial capacity
    list->keys = (int*)ExAllocatePool2(NonPagedPool, sizeof(int) * list->capacity, 'karr');

    if (!list->keys) {
        ExFreePoolWithTag(list, 'klst');
        return NULL;
    }

    return list;
}

// Append key to the list.
int append_to_keylist(KeyList* list, int key) {
    if (list->count == list->capacity) {
        size_t new_capacity = list->capacity * 2;
        int* new_keys = (int*)ExAllocatePool2(NonPagedPool, sizeof(int) * new_capacity, 'karr');
        if (!new_keys) return -1;

        for (size_t i = 0; i < list->count; i++) {
            new_keys[i] = list->keys[i];
        }
        ExFreePoolWithTag(list->keys, 'karr');
        list->keys = new_keys;
        list->capacity = new_capacity;
    }

    list->keys[list->count++] = key;
    return 0;
}

// Free the dynamic list.
void free_keylist(KeyList* list) {
    if (list) {
        if (list->keys) {
            ExFreePoolWithTag(list->keys, 'karr');
        }
        ExFreePoolWithTag(list, 'klst');
    }
}

// Find all keys associated with a particular value.
KeyList* find_keys_by_value(HashMap* map, int value) {
    KeyList* list = initialize_keylist();
    if (!list) return NULL;

    for (size_t i = 0; i < map->size; i++) {
        Entry* current = map->buckets[i];
        while (current) {
            if (current->value == value) {
                if (append_to_keylist(list, current->key) != 0) {
                    free_keylist(list);
                    return NULL;  // Memory allocation failed
                }
            }
            current = current->next;
        }
    }

    return list;
}