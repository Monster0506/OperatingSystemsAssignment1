#pragma once
#include <semaphore.h>

const int MAX_ITEMS = 2;

struct SharedData {
    int buffer[MAX_ITEMS];
    int count;
    sem_t mutex;
    sem_t empty;
    sem_t full;
};
