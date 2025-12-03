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
        cout << "Starting memory simulation..." << endl;
        
        // Prompt for trace file
        string trace_file;
        cout << "Enter path to trace file (or press Enter for default test): ";
        getline(cin, trace_file);
        
        // If no file provided, run a simple test pattern
        if (trace_file.empty()) {
            cout << "Running built-in test pattern..." << endl;
            
            // Generate test accesses
            vector<pair<char, uint64_t>> test_accesses = {
                {'R', 0x0000}, // Bank 0, Row 0
                {'R', 0x0400}, // Bank 0, Row 1
                {'R', 0x0000}, // Bank 0, Row 0 (hit)
                {'W', 0x1000}, // Bank 1, Row 0
                {'R', 0x0400}, // Bank 0, Row 1
                {'R', 0x1000}, // Bank 1, Row 0 (hit)
                {'W', 0x2000}, // Bank 2, Row 0
                {'R', 0x0800}, // Bank 0, Row 2
            };
            
            for (const auto& access : test_accesses) {
                processAccess(access.first, access.second);
            }
            
            cout << "Test pattern complete. Processed " << total_accesses << " accesses." << endl;
            return;
        }
        
        // Try to open and process trace file
        ifstream trace(trace_file);
        if (!trace.is_open()) {
            cerr << "Error: Could not open trace file '" << trace_file << "'" << endl;
            return;
        }
        
        cout << "Processing trace file: " << trace_file << endl;
        
        string line;
        int line_num = 0;
        
        while (getline(trace, line)) {
            line_num++;
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            // Parse line: operation address
            char operation;
            uint64_t address;
            
            stringstream ss(line);
            ss >> operation >> hex >> address;
            
            if (ss.fail()) {
                cerr << "Warning: Invalid trace format at line " << line_num << endl;
                continue;
            }
            
            // Process the memory access
            processAccess(operation, address);
            
            // Simulate periodic refresh (every 64000 cycles as per config)
            if (total_accesses % 8192 == 0 && total_accesses > 0) {
                performRefresh();
            }
        }
        
        trace.close();
        cout << "Trace processing complete. Total accesses: " << total_accesses << endl;
    }

    void MemorySimulator::processAccess(char operation, uint64_t address) {
        // Decode address to bank, row, column
        uint32_t bank_index = (address >> 12) & 0x3; // Example: 4 banks
        uint32_t row_index = (address >> 2) & 0x3FF; // Example: 1024 rows
        uint32_t col_index = address & 0x3;          // Example: 4 bytes per row
        
        if (bank_index >= banks.size() || row_index >= banks[bank_index].rows.size() || col_index >= banks[bank_index].rows[row_index].size()) {
            cerr << "Warning: Address out of bounds: " << hex << address << endl;
            return;
        }
        
        total_accesses++;
        
        Bank& bank = banks[bank_index];
        
        // Check for row buffer hit/miss
        if (bank.active_row == row_index) {
            // Row buffer hit
            row_hits++;
            total_latency += 10; // Example latency for hit
        } else {
            // Row buffer miss
            row_misses++;
            total_latency += 50; // Example latency for miss
            
            // Activate new row
            bank.active_row = row_index;
        }
        
        // Perform read or write
        if (operation == 'R') {
            volatile uint8_t data = bank.rows[row_index][col_index];
            (void)data; // Prevent unused variable warning
        } else if (operation == 'W') {
            bank.rows[row_index][col_index] = 0xFF; // Example write value
        } else {
            cerr << "Warning: Unknown operation '" << operation << "'" << endl;
        }
    }

    void MemorySimulator::performRefresh() {
        refresh_cycles += 1;
        total_latency += 100; // Placeholder latency for refresh
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

