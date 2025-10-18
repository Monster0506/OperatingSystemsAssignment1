# Operating Systems Assignment 1

## Installation and Running Instructions

### 1. Clone the repository
```sh
git clone https://github.com/monster0506/OperatingSystemsAssignment1
cd OperatingSystemsAssignment1
```

### 2. Compile the Producer and Consumer

```sh
make producer
make consumer
# --or--
make
```

### 3. Run the Producer and Consumer

```sh
make run
# --or--
./producer & ./consumer &
```

#### 3a. Capture Output to a File

```sh
( ./producer & ./consumer ) > log.txt 2>&1 &
```

#### 3b. Read From the Logfile

```sh
tail -f log.txt
```


### 4. Stop the Running Processes

In a separate terminal window or `tmux` pane

```sh
pkill producer
pkill consumer
```
### 5. Clean Up the Binaries

```sh
make clean
# --or--
rm -f ./producer ./consumer
```

#### 5a. Clean Up Logfiles

If you logged to a file, make sure to remove that file

```sh
rm -f log.txt
```


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

Both will need to access a buffer of size `2`, and hold a count of how many items are currently in the stack. 

Both will also need semaphores for locking so they can edit the data without encountering race conditions (`mutex`). This asserts mutual exclusion. 

Both will also need to check and set flags indicating if they have added or removed an item, or if there is an item or an empty slot (`full`, `empty`).

## Producer Process

```c
void* produce(void* arg) {
    SharedData* shm = static_cast<SharedData*>(arg);
    int item = 0;

    while (true) {
        item++;
        sem_wait(&shm->empty);
        sem_wait(&shm->mutex);

        shm->buffer[shm->count] = item;
        shm->count += 1;

        std::cout << "[Producer] Added item: " << item
                  << " | Count: " << shm->count << std::endl;

        sem_post(&shm->mutex);
        sem_post(&shm->full);
    }

    return nullptr;
}

```

The above is the code for creating the producer process

The process takes the shared memory block containing the common struct

The process starts by converting the memory block to the struct, so that its members my be accessed.

Then, it sets the value of `item` to zero. 
This is what will be pushed to the stack when there is an open spot. It will be incremented each time to track how many items the producer has generated.

The process will continuously increment item, then wait until the consumer indicates there is an empty slot, and gives the lock to the producer.

We then use the shared count to determine what spot we should add an item to in the buffer, and place our produced item at that index.

We increment count to indicate we are moving to the next available spot, or outside of the buffer.

We then return the mutex lock, and signal that we have added an item and at least one slot is full


## Consumer Process

```c

void* consume(void* arg) {
    SharedData* shm = static_cast<SharedData*>(arg);

    while (true) {
        sem_wait(&shm->full);
        sem_wait(&shm->mutex);

        int item =
            shm->buffer[shm->count - 1];
        shm->count -= 1;

        std::cout << "[Consumer] Consumed item: " << item
                  << " | Count: " << shm->count << std::endl;
        sem_post(&shm->mutex);
        sem_post(&shm->empty);
    }

    return nullptr;
}
```

This process also takes the shared memory location, and casts it to our `SharedData` struct

It waits until the producer indicates that an item has been added (`full`) and it gets the `mutex`. 

It then pops the item from the stack, and decrements count to indicate that an item has been removed.

Finally, it releases the `mutex` and changes the empty flag to indicate that there is now an empty slot.

## Setup of Producer Process


```c
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

// start producer process
produce(shm);

munmap(addr, sizeof(SharedData));
close(fd);
shm_unlink(NAME);

```

### Shared Memory Handling
We first open a block of shared memory at the `NAME` "/Shared". 

If this fails, we log and return early.

We then try to truncate the shared memory to the size of our `SharedData` block to ensure we are only using the necessary amount of memory, and that we cannot write out of bounds.

If this fails, we log and return early (safely closing the shared memory).

We then allocate real memory to the location of our shared memory with the correct size, and read and write privileges.

If this fails, we log and return early (safely closing the shared memory).

Then, we cast the block of shared memory to our `SharedData` struct, so we can access members. 

### Semaphore Initializations

We setup our `mutex` semaphore, initialize it to 1, and indicate that it is shared across multiple processes

We setup our `empty` semaphore, initialize it to 2, and indicate that it is shared across multiple processes

We setup our `full` semaphore, initialize it to 0, and indicate that it is shared across multiple processes

We setup `count` to indicate that the first slot is empty.

### Process Initialization

We run the `produce` function.

Once we finish, we handle unmapping memory and closing the file descriptor.

## Setup of Consumer Process

```c
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
```

### Shared Memory Handling
We first open a block of shared memory at the `NAME` "/Shared". 

If this fails, we log and return early.

We then try to truncate the shared memory to the size of our `SharedData` block to ensure we are only using the necessary amount of memory, and that we cannot write out of bounds.

If this fails, we log and return early (safely closing the shared memory).

We then allocate real memory to the location of our shared memory with the correct size, and read and write privileges.

If this fails, we log and return early (safely closing the shared memory).

Then, we cast the block of shared memory to our `SharedData` struct, so we can access members. 

### Process Initialization

We run the `consume` function

Once we finish, we handle unmapping memory and closing the file descriptor.


## Example Image

![example.png](example.png)



## Changelog

5868b9b Add image
3bced5b Remove thread references, replace with processes
72e75dd Make them not threads, I guess? Not sure why that requirement is there
711b5c0 Fix typos
a76fdde Add details on consumer thread creation
781d077 Fix typos
3cdeba2 Fix typo
af6ce44 Fix formatting
767e354 Clean up minor flaws
689e96b Begin documenting thread creation for producer
3684539 Update README with description of consumer thread
3f10966 Update README with description of producer thread
6396c0e Include DETAILED usage instructions in the README
659f5e6 Begin documentation in the README for common information
54c8b67 Fix consumer fd flags
49f5931 Fix placement of semaphore mutex free
06f30b4 Properly initialize semaphore flags
53e570d Fix failing to set empty from consumer thread
c3a5d2d Safely join and close consumer thread
11d10f5 Create producer thread
31848b9 Create consumer thread
feb4333 Return mutex to consumer in producer thread
03a5180 Add logs for producer thread
1f7714e Add an item to the buffer in the producer thread
ea6d713 Wait on locks for producer
0b4b394 Set up the producer thread with shared memory and thing to generate
904f230 Unlock
39e729e Add logs for consumer to get item
db078d5 Consumer get last item in queue
d325778 Set up waiting on lock
1977c14 Begin work on consumer thread
c9403fd Set up all of our shared memory
938accc Allocate memory to our shared memory and check for errors
57402dc Whoops, we should probably handle failures nicely instead of just trying to continue
f119871 Try to fit shared memory block to correct size of data
b1dc990 Open a block of shared memory for both producer and consumer
f6b34bd Add shared name
86b5aec Add main functions for producer and consumer
6fac085 Add gitignore
45fa1a9 Add makefile
17edf26 Setup files and create basic share state
6a206fe Add README
