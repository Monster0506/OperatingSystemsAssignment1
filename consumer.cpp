#include <sys/mman.h>

#include <iostream>

#include "shared.hpp"
int main() {
    int fd = shm_open(NAME, 02, 0666);
    if (fd == -1) {
        std::cout << "Error: failed to open shared memory (" << NAME << ")"
                  << std::endl;
        return 1;
    }

    return 0;
}
