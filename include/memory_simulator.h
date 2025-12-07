#ifndef MEMORY_SIMULATOR_H
#define MEMORY_SIMULATOR_H

#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>

typedef struct config{
    // DRAM Configuration
    int bank_count;
    int bank_size;
    int initial_value;
    uint32_t dram_rows;
    uint32_t dram_columns;
    uint32_t dram_banks;

    // Flash/Hybrid Configuration
    bool enable_flash;
    uint64_t flash_capacity;
    uint32_t flash_page_size;

    // Timing Parameters (in cycles)
    uint32_t row_access_time;
    uint32_t column_access_time;
    uint32_t refresh_interval;

    // Simulation Parameters
    uint64_t trace_lines;  // Number of trace lines to generate for testing
} Config;

struct Bank {
    std::vector<std::vector<uint8_t>> rows;
    uint32_t active_row;

    Bank(uint32_t num_rows, uint32_t cols) : active_row(-1) {
        rows.resize(num_rows, std::vector<uint8_t>(cols, 0));
    }
};

struct FlashPage {
    std::vector<uint8_t> data;
    uint32_t write_count;       // For wear leveling tracking
    uint64_t last_access_time;  // For hot/cold detection

    FlashPage(uint32_t page_size) : write_count(0), last_access_time(0) {
        data.resize(page_size, 0);
    }
};

struct FlashMemory {
    std::vector<FlashPage> pages;
    uint32_t page_size;
    uint64_t total_capacity;

    FlashMemory(uint64_t capacity, uint32_t page_sz)
        : page_size(page_sz), total_capacity(capacity) {
        uint32_t num_pages = capacity / page_sz;
        pages.reserve(num_pages);
        for(uint32_t i = 0; i < num_pages; i++) {
            pages.emplace_back(page_sz);
        }
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
    void processAccess(char operation, uint64_t address);
    void performRefresh();

private:
    // Helper methods for hybrid memory management
    bool isHotData(uint64_t address);
    void migrateToDRAM(uint64_t address);
    void migrateToFlash(uint64_t address);
    void updateAccessFrequency(uint64_t address);
    void generateTrace(uint64_t num_accesses);

    // Configuration
    Config sim_config;

    // DRAM components
    std::vector<Bank> banks;
    uint64_t total_accesses;
    uint64_t row_hits;
    uint64_t row_misses;
    uint64_t total_latency;
    uint64_t refresh_cycles;

    // Flash/Hybrid memory components
    FlashMemory* flash;
    bool hybrid_mode;
    std::map<uint64_t, uint64_t> access_frequency;  // Track access counts per address
    std::map<uint64_t, bool> data_location;         // true = DRAM, false = Flash

    // Flash statistics
    uint64_t flash_reads;
    uint64_t flash_writes;
    uint64_t migrations_to_dram;
    uint64_t migrations_to_flash;
    uint64_t dram_access_latency;
    uint64_t flash_access_latency;
    uint64_t migration_overhead;

    // Timing constants
    const uint32_t ROW_HIT_LATENCY = 10;
    const uint32_t ROW_MISS_LATENCY = 30;
    const uint32_t REFRESH_LATENCY = 50;
    const uint32_t FLASH_READ_LATENCY = 100;   // ~2-5x slower than DRAM
    const uint32_t FLASH_WRITE_LATENCY = 500;  // ~10x slower than DRAM
    const uint32_t MIGRATION_LATENCY = 200;    // Cost to move data between DRAM/Flash
    const uint64_t HOT_DATA_THRESHOLD = 3;     // Access count threshold for hot data
};

#endif // MEMORY_SIMULATOR_H
