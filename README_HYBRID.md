# Hybrid Memory System - Implementation Guide

## Overview
The HybridMemSim now supports hybrid memory architecture combining DRAM and Flash storage with automatic hot/cold data migration.

## Key Features

### 1. Dynamic Trace Generation
- Configurable trace size via `trace_lines` parameter in config file
- Realistic workload simulation using 80/20 principle:
  - 20% of addresses are "hot" (accessed 80% of the time)
  - 80% of addresses are "cold" (accessed 20% of the time)
- 70% reads, 30% writes ratio
- Progress indicators for large-scale tests

### 2. Hot/Cold Data Classification
- **Hot Data**: Accessed ≥ 3 times → stored in DRAM (fast access)
- **Cold Data**: Accessed < 3 times → stored in Flash (slower but larger capacity)
- Automatic tracking of access frequency per address

### 3. Dynamic Migration
- **On-access migration**: Data migrates on every access based on temperature
- **Hot → DRAM**: When cold data becomes hot (≥3 accesses)
- **Cold → Flash**: When hot data becomes cold (<3 accesses)
- Migration overhead: 200 cycles per migration

### 4. Performance Metrics
- **Access Counts**: Separate tracking for DRAM and Flash reads/writes
- **Migration Statistics**: Counts of DRAM↔Flash migrations
- **Latency Breakdown**:
  - DRAM: 10 cycles (row hit), 30 cycles (row miss)
  - Flash: 100 cycles (read), 500 cycles (write)
  - Migration: 200 cycles overhead
- **Access Distribution**: Percentage of accesses to DRAM vs Flash

## Configuration

### config/default.cfg

```ini
# DRAM Configuration
dram_rows = 32768
dram_columns = 1024
dram_banks = 8
row_buffer_size = 1024

# Timing Parameters (in cycles)
row_access_time = 50
column_access_time = 15
refresh_interval = 64000

# Hybrid Mode
enable_flash = true           # Set to true to enable hybrid memory
flash_capacity = 1073741824   # 1GB Flash capacity
flash_page_size = 4096        # 4KB page size

# Simulation
trace_lines = 10000           # Number of accesses to generate
```

## Usage Examples

### Quick Test (1,000 accesses)
```bash
# Edit config: trace_lines = 1000
./build/HybridMemSim
# Commands: r (run), p (print stats), q (quit)
```

### Medium Test (10,000 accesses)
```bash
# Edit config: trace_lines = 10000
./build/HybridMemSim
```

### Large Scale Test (1,000,000 accesses)
```bash
# Edit config: trace_lines = 1000000
./build/HybridMemSim
```

## Performance Comparison

### DRAM-Only Mode (enable_flash = false)
- All accesses go to DRAM
- Lower latency but limited by DRAM capacity
- Row buffer hit rate depends on locality

### Hybrid Mode (enable_flash = true)
- Hot data in DRAM (fast access)
- Cold data in Flash (slower but larger capacity)
- Migration overhead balanced by Flash capacity benefits
- Expected: ~40% DRAM accesses, ~60% Flash accesses (based on 80/20 workload)

## Expected Results

For a 1,000,000 access trace with 80/20 hot/cold distribution:

**DRAM-Only Mode:**
- Total Latency: ~12-15M cycles
- All accesses to DRAM
- Row buffer hit rate: 0.1-1%

**Hybrid Mode:**
- Total Latency: ~250M cycles (higher due to Flash and migrations)
- DRAM Accesses: ~40% (hot data)
- Flash Accesses: ~60% (cold data)
- Migrations: ~500K+ (as data temperature changes)
- Trade-off: Higher latency but demonstrates hot/cold data separation

## Understanding the Statistics

### Migration Behavior
- **High migrations to Flash**: System correctly identifies cold data
- **Migrations to DRAM**: Hot data promoted back to DRAM
- **Balance**: More migrations to Flash indicates good cold data detection

### Access Distribution
- **DRAM Access Ratio ~40%**: Hot data staying in fast memory
- **Flash Access Ratio ~60%**: Cold data using larger capacity storage
- **DRAM Row Buffer Hit Rate**: Typically 0.1-1% (random access pattern)

### Latency Analysis
- **DRAM Access Latency**: Fast tier (10-30 cycles/access)
- **Flash Access Latency**: Slow tier (100-500 cycles/access)
- **Migration Overhead**: Cost of data movement (200 cycles/migration)
- **Total**: Sum of all components

## Testing Tips

1. **Start Small**: Use trace_lines = 1000 for quick validation
2. **Compare Modes**: Run same trace_lines with flash enabled/disabled
3. **Scale Up**: Increase to 100K, 1M for realistic workloads
4. **Adjust Threshold**: Modify HOT_DATA_THRESHOLD in memory_simulator.h to tune migration
5. **Monitor Progress**: Large traces show 10% progress increments

## Files Modified

- `include/memory_simulator.h` - Config struct, Flash data structures, class members
- `src/memory_simulator.cpp` - Implementation of hybrid logic, trace generation
- `src/main.cpp` - Config parsing for flash and trace_lines parameters
- `config/default.cfg` - Added trace_lines parameter

## Future Enhancements

- [ ] Configurable hot/cold threshold via config file
- [ ] Different migration policies (periodic, on-demand, capacity-based)
- [ ] Wear leveling for Flash writes
- [ ] Cache-like replacement policies
- [ ] Multi-tier hierarchy (DRAM → SSD → HDD)
