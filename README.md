# Operating Systems Assignment 1


## Shared Data

shared.hpp
```c
#pragma once
#include <semaphore.h>

const int MAX_ITEMS = 2;
const char* NAME = "/Shared";

struct SharedData {
    int buffer[MAX_ITEMS];
    int count;
    sem_t mutex;
    sem_t empty;
    sem_t full;
};
```


This is a common construct to both the producer and consumer. They will both need to access shared data and handle locks, so these struct members will be loaded into the shared memory block.

Both will need to access a buffer of size `2`, and hold a count of how many items are currently in the queue. 

Both will also need semaphores for locking so they can edit the data without encountering race conditions (`mutex`)

Both will also need to check and set flags indicating if they have added or removed an item, or if there is an item or an empty slot (`full`, `empty`)



