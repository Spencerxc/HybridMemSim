#include "./../include/memory_simulator.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <random>
#include <ctime>

#define initial_value 0 


using namespace std;

//class MemorySimulator{

//public:
    MemorySimulator::MemorySimulator(config config) {
        // Store configuration
        sim_config = config;

        // Initialize DRAM banks
        banks.reserve(config.bank_count);
        for (int i = 0; i < config.bank_count; i++) {
            banks.emplace_back(config.bank_size, 1024);
        }

        // Initialize DRAM statistics
        total_accesses = 0;
        row_hits = 0;
        row_misses = 0;
        total_latency = 0;
        refresh_cycles = 0;

        // Initialize flash/hybrid memory
        hybrid_mode = config.enable_flash;
        if (hybrid_mode) {
            flash = new FlashMemory(config.flash_capacity, config.flash_page_size);
            flash_reads = 0;
            flash_writes = 0;
            migrations_to_dram = 0;
            migrations_to_flash = 0;
            dram_access_latency = 0;
            flash_access_latency = 0;
            migration_overhead = 0;
            cout << "Hybrid mode enabled: Flash memory initialized with "
                 << config.flash_capacity << " bytes ("
                 << (config.flash_capacity / config.flash_page_size) << " pages)" << endl;
        } else {
            flash = nullptr;
            flash_reads = 0;
            flash_writes = 0;
            migrations_to_dram = 0;
            migrations_to_flash = 0;
            dram_access_latency = 0;
            flash_access_latency = 0;
            migration_overhead = 0;
        }
    }

    MemorySimulator::~MemorySimulator() {
        // Clean up flash memory if allocated
        if (flash != nullptr) {
            delete flash;
            flash = nullptr;
        }
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

        // Initialize Flash memory (if hybrid mode)
        if (hybrid_mode && flash != nullptr) {
            cout << "\nInitializing Flash subsystem..." << endl;

            // Reset all flash pages
            for (size_t i = 0; i < flash->pages.size(); i++) {
                fill(flash->pages[i].data.begin(), flash->pages[i].data.end(), 0);
                flash->pages[i].write_count = 0;
                flash->pages[i].last_access_time = 0;
            }

            // Clear tracking maps
            access_frequency.clear();
            data_location.clear();

            // Reset flash statistics
            flash_reads = 0;
            flash_writes = 0;
            migrations_to_dram = 0;
            migrations_to_flash = 0;
            dram_access_latency = 0;
            flash_access_latency = 0;
            migration_overhead = 0;

            cout << "Flash initialization complete!" << endl;
            cout << "Flash capacity: " << flash->total_capacity << " bytes ("
                 << flash->pages.size() << " pages x "
                 << flash->page_size << " bytes/page)" << endl;
            cout << "\nHybrid memory system ready!" << endl;
        }
    }

    void MemorySimulator::clear() {
        // Clear DRAM banks
        for (auto& bank : banks) {
            // Reset all rows to zero
            for (auto& row : bank.rows) {
                fill(row.begin(), row.end(), 0);
            }
            // Reset active row indicator
            bank.active_row = -1;
        }

        // Reset DRAM statistics
        total_accesses = 0;
        row_hits = 0;
        row_misses = 0;
        total_latency = 0;
        refresh_cycles = 0;

        // Clear Flash memory (if hybrid mode)
        if (hybrid_mode && flash != nullptr) {
            // Reset all flash pages
            for (auto& page : flash->pages) {
                fill(page.data.begin(), page.data.end(), 0);
                page.write_count = 0;
                page.last_access_time = 0;
            }

            // Clear tracking maps
            access_frequency.clear();
            data_location.clear();

            // Reset flash statistics
            flash_reads = 0;
            flash_writes = 0;
            migrations_to_dram = 0;
            migrations_to_flash = 0;
            dram_access_latency = 0;
            flash_access_latency = 0;
            migration_overhead = 0;
        }
    }


    void MemorySimulator::run() {
        cout << "Starting memory simulation..." << endl;

        // Prompt for trace file
        string trace_file;
        cout << "Enter path to trace file (or press Enter for generated trace): ";
        getline(cin, trace_file);

        // If no file provided, generate dynamic trace
        if (trace_file.empty()) {
            cout << "Generating dynamic trace with " << sim_config.trace_lines << " accesses..." << endl;
            generateTrace(sim_config.trace_lines);
            cout << "Trace generation complete. Processed " << total_accesses << " accesses." << endl;
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
        total_accesses++;

        // Hybrid memory logic (if enabled)
        if (hybrid_mode) {
            // Update access frequency for this address
            updateAccessFrequency(address);

            // Check if data location is tracked, if not initialize to DRAM
            if (data_location.find(address) == data_location.end()) {
                data_location[address] = true; // Start in DRAM by default
            }

            bool in_dram = data_location[address];
            bool is_hot = isHotData(address);

            // Migration logic (on every access)
            if (is_hot && !in_dram) {
                // Hot data in Flash -> migrate to DRAM
                migrateToDRAM(address);
                in_dram = true;
            } else if (!is_hot && in_dram) {
                // Cold data in DRAM -> migrate to Flash
                migrateToFlash(address);
                in_dram = false;
            }

            // Access data from appropriate memory
            if (in_dram) {
                // Access from DRAM
                uint32_t bank_index = (address >> 12) & 0x3; // 4 banks
                uint32_t row_index = (address >> 2) & 0x3FF; // 1024 rows
                uint32_t col_index = address & 0x3;          // 4 bytes per row

                if (bank_index >= banks.size() || row_index >= banks[bank_index].rows.size() ||
                    col_index >= banks[bank_index].rows[row_index].size()) {
                    cerr << "Warning: Address out of bounds: " << hex << address << endl;
                    return;
                }

                Bank& bank = banks[bank_index];

                // Check for row buffer hit/miss
                uint32_t access_latency;
                if (bank.active_row == row_index) {
                    row_hits++;
                    access_latency = ROW_HIT_LATENCY;
                } else {
                    row_misses++;
                    access_latency = ROW_MISS_LATENCY;
                    bank.active_row = row_index;
                }

                total_latency += access_latency;
                dram_access_latency += access_latency;

                // Perform operation
                if (operation == 'R') {
                    volatile uint8_t data = bank.rows[row_index][col_index];
                    (void)data;
                } else if (operation == 'W') {
                    bank.rows[row_index][col_index] = 0xFF;
                }
            } else {
                // Access from Flash
                if (flash != nullptr) {
                    uint32_t page_index = address / flash->page_size;
                    uint32_t byte_offset = address % flash->page_size;

                    if (page_index >= flash->pages.size()) {
                        cerr << "Warning: Flash address out of bounds: " << hex << address << endl;
                        return;
                    }

                    FlashPage& page = flash->pages[page_index];
                    uint32_t access_latency;

                    if (operation == 'R') {
                        flash_reads++;
                        access_latency = FLASH_READ_LATENCY;
                        if (byte_offset < page.data.size()) {
                            volatile uint8_t data = page.data[byte_offset];
                            (void)data;
                        }
                    } else if (operation == 'W') {
                        flash_writes++;
                        access_latency = FLASH_WRITE_LATENCY;
                        if (byte_offset < page.data.size()) {
                            page.data[byte_offset] = 0xFF;
                            page.write_count++;
                        }
                    } else {
                        cerr << "Warning: Unknown operation '" << operation << "'" << endl;
                        return;
                    }

                    page.last_access_time = total_accesses;
                    total_latency += access_latency;
                    flash_access_latency += access_latency;
                }
            }
        } else {
            // Non-hybrid mode: standard DRAM-only access
            uint32_t bank_index = (address >> 12) & 0x3;
            uint32_t row_index = (address >> 2) & 0x3FF;
            uint32_t col_index = address & 0x3;

            if (bank_index >= banks.size() || row_index >= banks[bank_index].rows.size() ||
                col_index >= banks[bank_index].rows[row_index].size()) {
                cerr << "Warning: Address out of bounds: " << hex << address << endl;
                return;
            }

            Bank& bank = banks[bank_index];

            // Row buffer hit/miss logic
            if (bank.active_row == row_index) {
                row_hits++;
                total_latency += ROW_HIT_LATENCY;
            } else {
                row_misses++;
                total_latency += ROW_MISS_LATENCY;
                bank.active_row = row_index;
            }

            // Perform operation
            if (operation == 'R') {
                volatile uint8_t data = bank.rows[row_index][col_index];
                (void)data;
            } else if (operation == 'W') {
                bank.rows[row_index][col_index] = 0xFF;
            } else {
                cerr << "Warning: Unknown operation '" << operation << "'" << endl;
            }
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

        // Hybrid memory statistics (if enabled)
        if (hybrid_mode) {
            cout << "\n=== Hybrid Memory Statistics ===" << endl;
            cout << "-----------------------------------" << endl;

            cout << "\n--- Access Counts ---" << endl;
            // Calculate DRAM vs Flash access counts
            // In hybrid mode, row_hits/misses count DRAM accesses only
            uint64_t dram_accesses = row_hits + row_misses;
            uint64_t flash_accesses = flash_reads + flash_writes;

            cout << "DRAM Accesses:             " << dram_accesses
                 << " (hits: " << row_hits << ", misses: " << row_misses << ")" << endl;
            cout << "Flash Reads:               " << flash_reads << endl;
            cout << "Flash Writes:              " << flash_writes << endl;
            cout << "Total Flash Accesses:      " << flash_accesses << endl;

            cout << "\n--- Migration Statistics ---" << endl;
            cout << "Migrations to DRAM:        " << migrations_to_dram << endl;
            cout << "Migrations to Flash:       " << migrations_to_flash << endl;
            cout << "Total Migrations:          " << (migrations_to_dram + migrations_to_flash) << endl;
            cout << "Migration Overhead:        " << migration_overhead << " cycles" << endl;

            cout << "\n--- Latency Breakdown ---" << endl;
            cout << "DRAM Access Latency:       " << dram_access_latency << " cycles" << endl;
            cout << "Flash Access Latency:      " << flash_access_latency << " cycles" << endl;
            cout << "Migration Latency:         " << migration_overhead << " cycles" << endl;
            cout << "Total Latency:             " << total_latency << " cycles" << endl;

            cout << "\n--- Access Distribution ---" << endl;
            if (total_accesses > 0) {
                uint64_t dram_accesses = row_hits + row_misses;
                uint64_t flash_accesses = flash_reads + flash_writes;
                double dram_ratio = (double)dram_accesses / total_accesses * 100.0;
                double flash_ratio = (double)flash_accesses / total_accesses * 100.0;
                double dram_hit_ratio = dram_accesses > 0 ? (double)row_hits / dram_accesses * 100.0 : 0.0;

                cout << "DRAM Access Ratio:         " << fixed << setprecision(2)
                     << dram_ratio << "% (" << dram_accesses << " accesses)" << endl;
                cout << "Flash Access Ratio:        " << fixed << setprecision(2)
                     << flash_ratio << "% (" << flash_accesses << " accesses)" << endl;
                cout << "DRAM Row Buffer Hit Rate:  " << fixed << setprecision(2)
                     << dram_hit_ratio << "%" << endl;
            }

            cout << "\n--- Flash Memory Status ---" << endl;
            if (flash != nullptr) {
                cout << "Total Flash Capacity:      " << flash->total_capacity << " bytes" << endl;
                cout << "Flash Page Size:           " << flash->page_size << " bytes" << endl;
                cout << "Total Flash Pages:         " << flash->pages.size() << endl;
            }

            cout << "-----------------------------------" << endl;
        }

        cout << "-----------------------------------" << endl;
    }

    // Helper methods for hybrid memory management
    bool MemorySimulator::isHotData(uint64_t address) {
        // Check if address has been accessed frequently (hot data)
        if (access_frequency.find(address) != access_frequency.end()) {
            return access_frequency[address] >= HOT_DATA_THRESHOLD;
        }
        return false;
    }

    void MemorySimulator::updateAccessFrequency(uint64_t address) {
        // Increment access count for this address
        access_frequency[address]++;
    }

    void MemorySimulator::migrateToDRAM(uint64_t address) {
        // Migrate data from Flash to DRAM
        if (!hybrid_mode || flash == nullptr) return;

        // Check if data is actually in Flash
        if (data_location.find(address) != data_location.end() && data_location[address] == true) {
            return; // Already in DRAM
        }

        // Perform migration
        migrations_to_dram++;
        migration_overhead += MIGRATION_LATENCY;
        total_latency += MIGRATION_LATENCY;

        // Update data location
        data_location[address] = true; // true = DRAM
    }

    void MemorySimulator::migrateToFlash(uint64_t address) {
        // Migrate data from DRAM to Flash
        if (!hybrid_mode || flash == nullptr) return;

        // Check if data is actually in DRAM
        if (data_location.find(address) != data_location.end() && data_location[address] == false) {
            return; // Already in Flash
        }

        // Perform migration
        migrations_to_flash++;
        migration_overhead += MIGRATION_LATENCY;
        total_latency += MIGRATION_LATENCY;

        // Update data location
        data_location[address] = false; // false = Flash
    }

    void MemorySimulator::generateTrace(uint64_t num_accesses) {
        // Initialize random number generator
        std::mt19937_64 rng(time(nullptr));

        // Define access patterns to simulate realistic workloads
        // Pattern 1: Hot addresses (20% of addresses, 80% of accesses) - Zipfian-like
        // Pattern 2: Cold addresses (80% of addresses, 20% of accesses)

        const uint64_t address_space = 0x100000; // 1MB address space
        const uint64_t hot_address_count = address_space / 5;  // 20% are hot

        // Distributions
        std::uniform_int_distribution<uint64_t> hot_dist(0, hot_address_count - 1);
        std::uniform_int_distribution<uint64_t> cold_dist(hot_address_count, address_space - 1);
        std::uniform_int_distribution<int> access_type(0, 99); // 0-99 for percentages
        std::uniform_int_distribution<int> op_type(0, 99);     // Read vs Write ratio

        cout << "\nTrace Generation Configuration:" << endl;
        cout << "  Address space: 0x0 - 0x" << hex << address_space << dec << endl;
        cout << "  Hot addresses: " << hot_address_count << " (accessed 80% of the time)" << endl;
        cout << "  Cold addresses: " << (address_space - hot_address_count) << " (accessed 20% of the time)" << endl;
        cout << "  Read/Write ratio: 70% reads, 30% writes" << endl;

        if (hybrid_mode) {
            cout << "\n  Hybrid Mode: ENABLED" << endl;
            cout << "  Hot data threshold: " << HOT_DATA_THRESHOLD << " accesses" << endl;
            cout << "  Expected behavior: Hot addresses -> DRAM, Cold addresses -> Flash" << endl;
        } else {
            cout << "\n  Hybrid Mode: DISABLED (DRAM-only)" << endl;
        }

        cout << "\nGenerating " << num_accesses << " memory accesses..." << endl;

        // Generate and process accesses
        for (uint64_t i = 0; i < num_accesses; i++) {
            // Determine if this is a hot or cold access (80/20 rule)
            bool is_hot_access = (access_type(rng) < 80);

            // Generate address
            uint64_t address;
            if (is_hot_access) {
                address = hot_dist(rng);
            } else {
                address = cold_dist(rng);
            }

            // Determine operation (70% reads, 30% writes)
            char operation = (op_type(rng) < 70) ? 'R' : 'W';

            // Process the access
            processAccess(operation, address);

            // Periodic refresh (every 8192 accesses)
            if (i % 8192 == 0 && i > 0) {
                performRefresh();
            }

            // Progress indicator for large traces
            if (num_accesses >= 10000 && i % (num_accesses / 10) == 0 && i > 0) {
                cout << "  Progress: " << (i * 100 / num_accesses) << "% (" << i << " / " << num_accesses << ")" << endl;
            }
        }

        cout << "\nTrace generation complete!" << endl;
    }

