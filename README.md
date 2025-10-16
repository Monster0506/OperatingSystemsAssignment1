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

Both will need to access a buffer of size `2`, and hold a count of how many items are currently in the queue. 

Both will also need semaphores for locking so they can edit the data without encountering race conditions (`mutex`). This asserts mutual exclusion. 

Both will also need to check and set flags indicating if they have added or removed an item, or if there is an item or an empty slot (`full`, `empty`).

## Producer Thread

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

The above is the code for creating the producer thread

The thread takes the shared memory blocking containing the common struct

The thread starts by converting the memory block to the struct, so that its members my be accessed.

Then, it sets the value of `item` to zero. 
This is what will be pushed to the queue when there is an open spot. It will be incremented each time to track how many items the producer has generated.

The thread will continuously increment item, then wait until the consumer indicates there is an empty slot, and gives the lock to the producer.

We then use the shared count to determine what spot we should add an item to in the buffer, and place our produced item at that index.

We increment count to indicate we are moving to the next available spot, or outside of the buffer.

We then return the mutex lock, and signal that we have added an item and at least one slot is full


## Consumer Thread

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

This thread also takes the shared memory location, and casts it to our `SharedData` struct

It waits until the producer indicates that an item has been added (`full`) and it gets the `mutex`. 

It then pops the item from the queue, and decrements count to indicate that an item has been removed.

Finally, it releases the `mutex` and changes the empty flag to indicate that there is now an empty slot.

