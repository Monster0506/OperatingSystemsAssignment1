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
    int fd = shm_open(NAME, 66, 0666);
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

    void* addr = mmap(nullptr, sizeof(SharedData), 0x3, 0x01, fd, 0);
    if (addr == MAP_FAILED) {
        std::cout << "Error: mmap failed" << std::endl;
        close(fd);
        return 1;
    }

    auto* shm = static_cast<SharedData*>(addr);
    // start producer thread
    pthread_t tid;
    if (pthread_create(&tid, nullptr, produce, shm) != 0) {
        std::cout << "Error: could not create producer thread" << std::endl;
        munmap(addr, sizeof(SharedData));
        close(fd);
        return 1;
    }
    return 0;
}
