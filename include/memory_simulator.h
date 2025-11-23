#ifndef MEMORY_SIMULATOR_H
#define MEMORY_SIMULATOR_H

#include <cstdint>

typedef struct config{

    int bank_count;
    int bank_size;
    int initial_value;

} Config;

class MemorySimulator {
public:
    MemorySimulator(config config_struct);
    ~MemorySimulator();

    void initialize();
    void run();
    void printStats();

private:
    // TODO: Add member variables for DRAM, Flash, queues, etc.
};

#endif // MEMORY_SIMULATOR_H
