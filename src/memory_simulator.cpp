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
            cache_promotions = 0;
            cache_evictions = 0;
            dram_cache_hits = 0;
            dram_cache_misses = 0;
            dram_access_latency = 0;
            flash_access_latency = 0;
            cache_overhead = 0;
            cout << "Hybrid mode enabled (DRAM-as-cache model): Flash memory initialized with "
                 << config.flash_capacity << " bytes ("
                 << (config.flash_capacity / config.flash_page_size) << " pages)" << endl;
        } else {
            flash = nullptr;
            flash_reads = 0;
            flash_writes = 0;
            cache_promotions = 0;
            cache_evictions = 0;
            dram_cache_hits = 0;
            dram_cache_misses = 0;
            dram_access_latency = 0;
            flash_access_latency = 0;
            cache_overhead = 0;
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
            data_in_dram_cache.clear();

            // Reset flash statistics
            flash_reads = 0;
            flash_writes = 0;
            cache_promotions = 0;
            cache_evictions = 0;
            dram_cache_hits = 0;
            dram_cache_misses = 0;
            dram_access_latency = 0;
            flash_access_latency = 0;
            cache_overhead = 0;

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
            data_in_dram_cache.clear();

            // Reset flash statistics
            flash_reads = 0;
            flash_writes = 0;
            cache_promotions = 0;
            cache_evictions = 0;
            dram_cache_hits = 0;
            dram_cache_misses = 0;
            dram_access_latency = 0;
            flash_access_latency = 0;
            cache_overhead = 0;
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

        // Hybrid memory logic: DRAM-as-cache model
        if (hybrid_mode && flash != nullptr) {
            // Update access frequency
            updateAccessFrequency(address);

            // Check if address is cached in DRAM
            bool cached_in_dram = (data_in_dram_cache.find(address) != data_in_dram_cache.end())
                                  && data_in_dram_cache[address];
            bool is_hot = isHotData(address);

            // Cache management logic
            if (is_hot && !cached_in_dram) {
                // HOT data not in cache → PROMOTE to DRAM cache
                cache_promotions++;
                cache_overhead += CACHE_PROMOTION_LATENCY;
                total_latency += CACHE_PROMOTION_LATENCY;
                data_in_dram_cache[address] = true;
                cached_in_dram = true;
            } else if (!is_hot && cached_in_dram) {
                // COLD data in cache → EVICT from DRAM cache
                cache_evictions++;
                cache_overhead += CACHE_EVICTION_LATENCY;
                total_latency += CACHE_EVICTION_LATENCY;
                data_in_dram_cache[address] = false;
                cached_in_dram = false;
            }

            // Access the data
            if (cached_in_dram) {
                // ========== CACHE HIT: Access from DRAM cache (FAST!) ==========
                dram_cache_hits++;

                uint32_t bank_index = (address >> 12) & 0x3;
                uint32_t row_index = (address >> 2) & 0x3FF;
                uint32_t col_index = address & 0x3;

                if (bank_index >= banks.size() || row_index >= banks[bank_index].rows.size() ||
                    col_index >= banks[bank_index].rows[row_index].size()) {
                    cerr << "Warning: Address out of bounds: " << hex << address << endl;
                    return;
                }

                Bank& bank = banks[bank_index];

                // Row buffer hit/miss
                uint32_t access_latency;
                if (bank.active_row == row_index) {
                    row_hits++;
                    access_latency = ROW_HIT_LATENCY;  // 10 cycles - FAST!
                } else {
                    row_misses++;
                    access_latency = ROW_MISS_LATENCY; // 30 cycles
                    bank.active_row = row_index;
                }

                total_latency += access_latency;
                dram_access_latency += access_latency;

                // Perform DRAM operation
                if (operation == 'R') {
                    volatile uint8_t data = bank.rows[row_index][col_index];
                    (void)data;
                } else if (operation == 'W') {
                    bank.rows[row_index][col_index] = 0xFF;
                    // Write-through: also update Flash
                    flash_writes++;
                    flash_access_latency += FLASH_WRITE_LATENCY;
                    total_latency += FLASH_WRITE_LATENCY;
                }

            } else {
                // ========== CACHE MISS: Access from Flash only (SLOW) ==========
                dram_cache_misses++;

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
                    access_latency = FLASH_READ_LATENCY;  // 100 cycles - slower than DRAM
                    if (byte_offset < page.data.size()) {
                        volatile uint8_t data = page.data[byte_offset];
                        (void)data;
                    }
                } else if (operation == 'W') {
                    flash_writes++;
                    access_latency = FLASH_WRITE_LATENCY;  // 500 cycles - much slower
                    if (byte_offset < page.data.size()) {
                        page.data[byte_offset] = 0xFF;
                        page.write_count++;
                    }
                }

                page.last_access_time = total_accesses;
                total_latency += access_latency;
                flash_access_latency += access_latency;
            }

        } else {
            // ========== DRAM-only mode: No caching, pure DRAM ==========
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

        // Hybrid memory statistics (DRAM-as-cache model)
        if (hybrid_mode) {
            cout << "\n=== Hybrid Memory Statistics (DRAM-as-Cache) ===" << endl;
            cout << "-----------------------------------" << endl;

            cout << "\n--- Cache Performance ---" << endl;
            cout << "DRAM Cache Hits:           " << dram_cache_hits << endl;
            cout << "DRAM Cache Misses:         " << dram_cache_misses << endl;
            if (total_accesses > 0) {
                double cache_hit_rate = (double)dram_cache_hits / total_accesses * 100.0;
                cout << "Cache Hit Rate:            " << fixed << setprecision(2)
                     << cache_hit_rate << "% (hot data cached in DRAM)" << endl;
            }

            cout << "\n--- Cache Management ---" << endl;
            cout << "Cache Promotions:          " << cache_promotions
                 << " (cold→hot, loaded into DRAM)" << endl;
            cout << "Cache Evictions:           " << cache_evictions
                 << " (hot→cold, evicted from DRAM)" << endl;
            cout << "Cache Overhead:            " << cache_overhead << " cycles" << endl;

            cout << "\n--- Access Breakdown ---" << endl;
            uint64_t dram_row_accesses = row_hits + row_misses;
            cout << "DRAM Accesses (cached):    " << dram_row_accesses
                 << " (row hits: " << row_hits << ", misses: " << row_misses << ")" << endl;
            if (dram_row_accesses > 0) {
                double row_hit_rate = (double)row_hits / dram_row_accesses * 100.0;
                cout << "  DRAM Row Buffer Hit Rate:" << fixed << setprecision(2)
                     << row_hit_rate << "%" << endl;
            }
            cout << "Flash Reads (cache miss):  " << flash_reads << endl;
            cout << "Flash Writes:              " << flash_writes
                 << " (write-through + direct)" << endl;

            cout << "\n--- Latency Breakdown ---" << endl;
            cout << "DRAM Cache Latency:        " << dram_access_latency << " cycles (FAST path)" << endl;
            cout << "Flash Access Latency:      " << flash_access_latency << " cycles (SLOW path)" << endl;
            cout << "Cache Management:          " << cache_overhead << " cycles (promotion/eviction)" << endl;
            cout << "Total Latency:             " << total_latency << " cycles" << endl;

            if (total_accesses > 0) {
                double avg_latency = (double)total_latency / total_accesses;
                cout << "Average Access Latency:    " << fixed << setprecision(2)
                     << avg_latency << " cycles" << endl;
            }

            cout << "\n--- Performance Analysis ---" << endl;
            if (dram_cache_hits > 0) {
                double avg_cache_hit_latency = (double)dram_access_latency / dram_cache_hits;
                cout << "Avg Cache Hit Latency:     " << fixed << setprecision(2)
                     << avg_cache_hit_latency << " cycles (when in DRAM)" << endl;
            }
            if (dram_cache_misses > 0) {
                double avg_cache_miss_latency = (double)flash_access_latency / dram_cache_misses;
                cout << "Avg Cache Miss Latency:    " << fixed << setprecision(2)
                     << avg_cache_miss_latency << " cycles (when in Flash only)" << endl;
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
        // Legacy method - not used in cache model
        (void)address;
    }

    void MemorySimulator::migrateToFlash(uint64_t address) {
        // Legacy method - not used in cache model
        (void)address;
    }

    void MemorySimulator::generateTrace(uint64_t num_accesses) {
        // Initialize random number generator
        std::mt19937_64 rng(time(nullptr));

        // Define access patterns to simulate realistic workloads
        // Pattern 1: Hot addresses (20% of addresses, 80% of accesses) - Zipfian-like
        // Pattern 2: Cold addresses (80% of addresses, 20% of accesses)

        const uint64_t address_space = 0x100000; // 1MB address space
        const uint64_t hot_address_count = address_space / 5;  // 20% are hot

        // Create row-aligned address ranges for better locality
        // Each row is 4 bytes wide (based on address decoding: col_index = address & 0x3)
        // Row changes every 4 addresses
        const uint64_t row_size = 4;  // bytes per row

        // Distributions
        std::uniform_int_distribution<uint64_t> hot_row_dist(0, (hot_address_count / row_size) - 1);
        std::uniform_int_distribution<uint64_t> cold_row_dist(hot_address_count / row_size, (address_space / row_size) - 1);
        std::uniform_int_distribution<uint64_t> column_dist(0, row_size - 1); // Column within row
        std::uniform_int_distribution<int> access_type(0, 99); // 0-99 for percentages
        std::uniform_int_distribution<int> op_type(0, 99);     // Read vs Write ratio
        std::uniform_int_distribution<int> locality_check(0, 99); // For sequential locality

        cout << "\nTrace Generation Configuration:" << endl;
        cout << "  Address space: 0x0 - 0x" << hex << address_space << dec << endl;
        cout << "  Hot addresses: " << hot_address_count << " (accessed 80% of the time)" << endl;
        cout << "  Cold addresses: " << (address_space - hot_address_count) << " (accessed 20% of the time)" << endl;
        cout << "  Read/Write ratio: 70% reads, 30% writes" << endl;
        cout << "  Spatial locality: 30% (sequential row accesses)" << endl;

        if (hybrid_mode) {
            cout << "\n  Hybrid Mode: ENABLED" << endl;
            cout << "  Hot data threshold: " << HOT_DATA_THRESHOLD << " accesses" << endl;
            cout << "  Expected behavior: Hot addresses -> DRAM, Cold addresses -> Flash" << endl;
        } else {
            cout << "\n  Hybrid Mode: DISABLED (DRAM-only)" << endl;
        }

        cout << "\nGenerating " << num_accesses << " memory accesses..." << endl;

        // Track last accessed row for locality
        uint64_t last_row = 0;
        bool last_was_hot = false;

        // Generate and process accesses
        for (uint64_t i = 0; i < num_accesses; i++) {
            // Determine if this is a hot or cold access (80/20 rule)
            bool is_hot_access = (access_type(rng) < 80);

            // Generate address with spatial locality
            // 30% of the time, access the same row as last access (if same hot/cold category)
            uint64_t address;
            bool use_locality = (locality_check(rng) < 30) && (i > 0) && (is_hot_access == last_was_hot);

            if (use_locality) {
                // Access same row, different column for row buffer hit
                uint64_t column = column_dist(rng);
                address = (last_row * row_size) + column;
            } else {
                // Generate new random address
                uint64_t row;
                if (is_hot_access) {
                    row = hot_row_dist(rng);
                } else {
                    row = cold_row_dist(rng);
                }
                uint64_t column = column_dist(rng);
                address = (row * row_size) + column;
                last_row = row;
                last_was_hot = is_hot_access;
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

