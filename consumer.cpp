#include <sys/mman.h>
#include <unistd.h>

#include <iostream>

#include "shared.hpp"

void* consume(void* arg) {
    SharedData* shm = static_cast<SharedData*>(arg);

    return nullptr;
}

int main() {
    int fd = shm_open(NAME, 02, 0666);
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
    // start consumer thread
    return 0;
}
