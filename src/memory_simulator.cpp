#include "./../include/memory_simulator.h"
#include <iostream>
#include <vector>

#define initial_value 0 


using namespace std;

class MemorySimulator{

public:
    MemorySimulator::MemorySimulator() {

    }

    MemorySimulator::~MemorySimulator() {
        // Destructor
    }

    void MemorySimulator::initialize() {
        // for (auto& row : mem_vec) {
        //     fill(row.begin(), row.end(), initial_value);
        // }
        // TODO: Initialize DRAM subsystem
        // TODO: Initialize Flash memory (if hybrid mode)
        // TODO: Set up queues and buffers
    }

    void MemorySimulator::run() {
        // TODO: Main simulation loop
        // TODO: Process memory requests
        // TODO: Handle DRAM row-buffer hits/misses
        // TODO: Handle refresh cycles
    }

    void MemorySimulator::printStats() {
        // TODO: Output latency statistics
        // TODO: Output throughput metrics
        // TODO: Output hit/miss ratios
    }
};

// void clear_memory(vector<int>& arr){

//     for (auto& row : mem_vec) {
//         fill(row.begin(), row.end(), initial_value);
//     }

// }
