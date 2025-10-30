#ifndef MEMORY_SIMULATOR_H
#define MEMORY_SIMULATOR_H

#include <cstdint>

class MemorySimulator {
public:
    MemorySimulator();
    ~MemorySimulator();

    void initialize();
    void run();
    void printStats();

private:
    // TODO: Add member variables for DRAM, Flash, queues, etc.
};

#endif // MEMORY_SIMULATOR_H
