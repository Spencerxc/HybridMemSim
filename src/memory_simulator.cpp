#include "./../include/memory_simulator.h"
#include <iostream>
#include <vector>
#include <iomanip>

#define initial_value 0 


using namespace std;

//class MemorySimulator{

//public:
    MemorySimulator::MemorySimulator(config config) {
        banks.reserve(config.bank_count);
        for (int i = 0; i < config.bank_count; i++) {
            banks.emplace_back(config.bank_size, 1024);
        }

        total_accesses = 0;
        row_hits = 0;
        row_misses = 0;
        total_latency = 0;
        refresh_cycles = 0;


    }

    MemorySimulator::~MemorySimulator() {
        // Destructor
    }

    void MemorySimulator::initialize() {
      cout << "Initializing DRAM subsystem..." << endl;
    
    // Initialize all banks
        for (size_t i = 0; i < banks.size(); i++) {
        // Reset all rows to initial value (0)
            for (auto& row : banks[i].rows) {
                fill(row.begin(), row.end(), 0);
            }
        // Set no row as active initially
        banks[i].active_row = -1;
        
        cout << "  Bank " << i << ": " << banks[i].rows.size() 
             << " rows x " << banks[i].rows[0].size() << " columns initialized" << endl;
        }
    
        // Initialize statistics
        total_accesses = 0;
        row_hits = 0;
        row_misses = 0;
        total_latency = 0;
        refresh_cycles = 0;
    
        cout << "DRAM initialization complete!" << endl;
        cout << "Total capacity: " << banks.size() << " banks x " 
            << banks[0].rows.size() << " rows x " 
            << banks[0].rows[0].size() << " bytes = "
            << (banks.size() * banks[0].rows.size() * banks[0].rows[0].size()) 
            << " bytes" << endl;
        // TODO: Initialize DRAM subsystem
        // TODO: Initialize Flash memory (if hybrid mode)
        // TODO: Set up queues and buffers
    }

    void MemorySimulator::clear() {
        for (auto& bank : banks) {
        // Reset all rows to zero
            for (auto& row : bank.rows) {
                fill(row.begin(), row.end(), 0);
            }
        // Reset active row indicator
        bank.active_row = -1;
        }


        total_accesses = 0;
        row_hits = 0;
        row_misses = 0;
        total_latency = 0;
        refresh_cycles = 0;

    }


    void MemorySimulator::run() {
        // TODO: Main simulation loop
        // TODO: Process memory requests
        // TODO: Handle DRAM row-buffer hits/misses
        // TODO: Handle refresh cycles
    }

    void MemorySimulator::printStats() {
        cout << "\n=== DRAM Memory Statistics ===" << endl;
        cout << "-----------------------------------" << endl;
        
        // Basic access statistics
        cout << "Total Memory Accesses:     " << total_accesses << endl;
        cout << "Row Buffer Hits:           " << row_hits << endl;
        cout << "Row Buffer Misses:         " << row_misses << endl;
        cout << "Refresh Cycles:            " << refresh_cycles << endl;
        
        cout << "\n--- Performance Metrics ---" << endl;
        
        // Hit ratio
        if (total_accesses > 0) {
            double hit_ratio = (double)row_hits / total_accesses * 100.0;
            double miss_ratio = (double)row_misses / total_accesses * 100.0;
            cout << "Row Buffer Hit Ratio:      " << fixed << setprecision(2) 
                << hit_ratio << "%" << endl;
            cout << "Row Buffer Miss Ratio:     " << fixed << setprecision(2) 
                << miss_ratio << "%" << endl;
        } else {
            cout << "Row Buffer Hit Ratio:      N/A (no accesses)" << endl;
            cout << "Row Buffer Miss Ratio:     N/A (no accesses)" << endl;
        }
        
        // Average latency
        if (total_accesses > 0) {
            double avg_latency = (double)total_latency / total_accesses;
            cout << "Average Access Latency:    " << fixed << setprecision(2) 
                << avg_latency << " cycles" << endl;
        } else {
            cout << "Average Access Latency:    N/A (no accesses)" << endl;
        }
        
        // Total latency
        cout << "Total Latency:             " << total_latency << " cycles" << endl;
        
        // Throughput (accesses per cycle)
        if (total_latency > 0) {
            double throughput = (double)total_accesses / total_latency;
            cout << "Throughput:                " << fixed << setprecision(4) 
                << throughput << " accesses/cycle" << endl;
        } else {
            cout << "Throughput:                N/A" << endl;
        }
        
        cout << "\n--- Bank Status ---" << endl;
        cout << "Total Banks:               " << banks.size() << endl;
        cout << "Rows per Bank:             " << (banks.empty() ? 0 : banks[0].rows.size()) << endl;
        cout << "Columns per Row:           " << (banks.empty() || banks[0].rows.empty() ? 0 : banks[0].rows[0].size()) << endl;
        
        cout << "-----------------------------------" << endl;
    }

