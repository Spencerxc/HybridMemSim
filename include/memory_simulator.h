#ifndef MEMORY_SIMULATOR_H
#define MEMORY_SIMULATOR_H

#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

typedef struct config{

    int bank_count;
    int bank_size;
    int initial_value;

} Config;

struct Bank {
    std::vector<std::vector<uint8_t>> rows;
    uint32_t active_row;
    
    Bank(uint32_t num_rows, uint32_t cols) : active_row(-1) {
        rows.resize(num_rows, std::vector<uint8_t>(cols, 0));
    }
};

class MemorySimulator {
public:
    MemorySimulator(Config config);

    ~MemorySimulator();

    void initialize();
    void clear();
    void run();
    void printStats();

private:

std::vector<Bank> banks;
    // TODO: Add member variables for DRAM, Flash, queues, etc.
};

#endif // MEMORY_SIMULATOR_H
