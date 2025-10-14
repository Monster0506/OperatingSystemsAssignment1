#include <sys/mman.h>
#include <unistd.h>

#include <iostream>

#include "shared.hpp"
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

    return 0;
}
