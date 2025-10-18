#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>

#include "shared.hpp"

void* consume(void* arg) {
    SharedData* shm = static_cast<SharedData*>(arg);

    while (true) {
        sem_wait(&shm->full);   // wait until we have an item
        sem_wait(&shm->mutex);  // wait on lock

        int item =
            shm->buffer[shm->count - 1];  // get the last item from the buffer
        shm->count -= 1;

        std::cout << "[Consumer] Consumed item: " << item
                  << " | Count: " << shm->count << std::endl;
        sem_post(&shm->mutex);  // unlock
        sem_post(&shm->empty);  // signal that we removed an item
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

    void* addr = mmap(nullptr, sizeof(SharedData), 0x3, 0x01, fd, 0);

    if (addr == MAP_FAILED) {
        std::cout << "Error: mmap failed" << std::endl;
        close(fd);
        return 1;
    }

    auto* shm = static_cast<SharedData*>(addr);
    // start consumer process
    consume(shm);

    munmap(addr, sizeof(SharedData));
    close(fd);
    return 0;
}
