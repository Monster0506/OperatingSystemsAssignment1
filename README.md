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


