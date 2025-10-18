#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>

#include "shared.hpp"
void* produce(void* arg) {
    SharedData* shm = static_cast<SharedData*>(arg);
    int item = 0;

    while (true) {
        item++;
        sem_wait(&shm->empty);  // wait until there is an empty slot
        sem_wait(&shm->mutex);  // wait until we have the lock

        shm->buffer[shm->count] = item;  // add an item
        shm->count += 1;

        std::cout << "[Producer] Added item: " << item
                  << " | Count: " << shm->count << std::endl;

        sem_post(&shm->mutex);  // unlock
        sem_post(&shm->full);   // signal that we added an item
    }

    return nullptr;
}

int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        std::cout << "Error: failed to open shared memory (" << NAME << ")"
                  << std::endl;
        return 1;
    }

    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        std::cout << "Error: failed to fit shared memory block to size of "
                     "shared data ("
                  << NAME << ")" << std::endl;
        close(fd);
        return 1;
    }

    void* addr = mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        std::cout << "Error: mmap failed" << std::endl;
        close(fd);
        return 1;
    }

    auto* shm = static_cast<SharedData*>(addr);

    // Initialize semaphores
    sem_init(&shm->mutex, 1, 1);  // binary semaphore for mutual exclusion
    sem_init(&shm->empty, 1, MAX_ITEMS);  // count of empty slots
    sem_init(&shm->full, 1, 0);           // count of full slots
    shm->count = 0;

    // start producer thread
    produce(shm);

    munmap(addr, sizeof(SharedData));
    close(fd);

    return 0;
}
