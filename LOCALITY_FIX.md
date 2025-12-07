# Row Buffer Hit Rate Fix - Spatial Locality Improvement

## Problem Identified

The original trace generation had **very low row buffer hit rates** (~0.1%):
- Completely random address generation
- No spatial locality
- Nearly every access caused a row buffer miss

## Root Cause

```cpp
// OLD: Completely random addresses
address = random_distribution(0, address_space);
```

This created a worst-case scenario where:
- Each access likely hit a different row
- Row buffer was constantly being flushed
- No benefit from row buffer caching

## Solution Implemented

### 1. Row-Aligned Address Generation
```cpp
// NEW: Generate row + column separately
uint64_t row = row_distribution(rng);
uint64_t column = column_distribution(rng);
uint64_t address = (row * row_size) + column;
```

### 2. Spatial Locality (30% probability)
```cpp
// 30% of accesses hit the same row as previous access
if (use_locality && same_category) {
    // Keep same row, vary column
    address = (last_row * row_size) + random_column;
} else {
    // New random row
    address = (new_row * row_size) + random_column;
}
```

## Expected Improvements

### Before (0.1% hit rate)
```
Row Buffer Hits:           974
Row Buffer Misses:         999026
Row Buffer Hit Ratio:      0.10%
```

### After (Expected: 25-30% hit rate)
```
Row Buffer Hits:           ~250,000-300,000
Row Buffer Misses:         ~700,000-750,000
Row Buffer Hit Ratio:      25-30%
```

## Why This Matters

### DRAM-Only Mode
- **Lower average latency**: More hits = more 10-cycle accesses vs 30-cycle misses
- **Better throughput**: Fewer row activations
- **More realistic**: Real workloads have spatial locality

### Hybrid Mode
- **Better DRAM efficiency**: Hot data benefits from row buffer
- **Clearer performance comparison**: Can see DRAM's true advantage
- **More realistic migration patterns**: Locality affects hot/cold classification

## Verification

Run a test and check the statistics:

```bash
./build/HybridMemSim
# Commands: r, p, q
```

Look for:
```
Row Buffer Hit Ratio:      25-30%  ← Should be much higher now!
```

### Quick Comparison Test

```bash
# Test with 10,000 accesses
# Edit config: trace_lines = 10000

# DRAM-only mode
./build/HybridMemSim
```

Expected results:
- **Hit ratio**: 25-30% (vs old 0.1%)
- **Average latency**: ~24-26 cycles (vs old ~30 cycles)
- **More realistic workload**: Demonstrates row buffer effectiveness

## Configuration

The spatial locality is now built into the trace generation:
- **30% of accesses** have spatial locality (same row as previous)
- **70% of accesses** are to random rows

To adjust locality percentage:
```cpp
// Edit src/memory_simulator.cpp, line ~585
bool use_locality = (locality_check(rng) < 30);  // Change 30 to adjust %
```

Higher values (40-50) = more locality = higher hit rates
Lower values (10-20) = less locality = lower hit rates

## Impact on Hybrid Memory

With better spatial locality:
1. **Hot data in DRAM** benefits from row buffer hits
2. **Cold data in Flash** doesn't benefit (Flash has different access patterns)
3. **DRAM advantage** becomes more apparent in statistics
4. **Migration decisions** are more meaningful (hot data gets real performance boost)

## Technical Details

### Address Decoding
Based on the processAccess() function:
```cpp
uint32_t bank_index = (address >> 12) & 0x3;  // Bits 13-12: Bank
uint32_t row_index = (address >> 2) & 0x3FF;   // Bits 11-2:  Row
uint32_t col_index = address & 0x3;            // Bits 1-0:   Column
```

### Row Size
Each row spans 4 bytes (columns 0-3):
- Address 0-3: Row 0
- Address 4-7: Row 1
- Address 8-11: Row 2
- etc.

### Locality Implementation
- Tracks `last_row` and `last_was_hot`
- 30% chance to reuse same row with different column
- Only reuses if hot/cold category matches
- Ensures hits happen in correct memory tier

## Testing

1. **Build**: `make clean && make`
2. **Run**: `./build/HybridMemSim`
3. **Check stats**: Look for improved hit ratio in output

The row buffer hit rate should now be **25-30%** instead of **0.1%**! 🎉
