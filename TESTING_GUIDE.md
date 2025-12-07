# Hybrid Memory Simulator - Testing Guide

## Quick Start

### 1. Build the Project
```bash
make clean && make
```

### 2. Run Interactive Simulator
```bash
./build/HybridMemSim
```

**Interactive Commands:**
- `r` - Run simulation with generated trace
- `p` - Print memory statistics
- `c` - Clear memory and reset stats
- `h` - Display help menu
- `q` - Quit simulator

### 3. Quick Comparison Test
```bash
./compare_modes.sh
```
This runs both DRAM-only and Hybrid modes and compares results.

## Configuration

Edit `config/default.cfg` to customize:

```ini
# Enable/disable hybrid memory
enable_flash = true          # true = hybrid, false = DRAM-only

# Control test size
trace_lines = 10000          # Number of memory accesses to generate
```

## Test Scenarios

### Scenario 1: Quick Validation (1,000 accesses)
**Purpose:** Fast sanity check

```bash
# Edit config/default.cfg: trace_lines = 1000
./build/HybridMemSim
# Commands: r, p, q
```

**Expected Time:** < 1 second
**Expected Results:**
- DRAM-only: ~30K-50K cycles total latency
- Hybrid: ~250K cycles (includes Flash and migration overhead)

---

### Scenario 2: Medium Scale (10,000 accesses)
**Purpose:** Standard testing

```bash
# Edit config/default.cfg: trace_lines = 10000
./build/HybridMemSim
# Commands: r, p, q
```

**Expected Time:** < 5 seconds
**Expected Results:**
- Total Migrations: ~5,000-6,000
- DRAM Access Ratio: ~40%
- Flash Access Ratio: ~60%

---

### Scenario 3: Large Scale (1,000,000 accesses)
**Purpose:** Performance analysis at scale

```bash
# Edit config/default.cfg: trace_lines = 1000000
./build/HybridMemSim
# Commands: r, p, q
```

**Expected Time:** 30-60 seconds
**Expected Results:**
- Total Migrations: ~500,000+
- Clear hot/cold data separation
- Progress indicators during generation

---

### Scenario 4: Automated Comparison
**Purpose:** Compare DRAM-only vs Hybrid modes

```bash
./compare_modes.sh
```

**Output:** Side-by-side comparison of both modes

---

## Understanding the Output

### DRAM-Only Mode Statistics

```
=== DRAM Memory Statistics ===
Total Memory Accesses:     10000
Row Buffer Hits:           41
Row Buffer Misses:         4083
Total Latency:             122410 cycles
Average Access Latency:    12.24 cycles
```

**Key Metrics:**
- Low average latency (all DRAM)
- Row buffer hit rate typically < 1% (random access pattern)

---

### Hybrid Mode Statistics

```
=== Hybrid Memory Statistics ===

--- Access Counts ---
DRAM Accesses:             4085 (hits: 41, misses: 4044)
Flash Reads:               4141
Flash Writes:              1774
Total Flash Accesses:      5915

--- Migration Statistics ---
Migrations to DRAM:        1556
Migrations to Flash:       3832
Total Migrations:          5388
Migration Overhead:        1077600 cycles

--- Latency Breakdown ---
DRAM Access Latency:       122540 cycles
Flash Access Latency:      1300127 cycles
Migration Latency:         1077600 cycles
Total Latency:             2500617 cycles

--- Access Distribution ---
DRAM Access Ratio:         40.85% (4085 accesses)
Flash Access Ratio:        59.15% (5915 accesses)
DRAM Row Buffer Hit Rate:  1.00%
```

**Key Insights:**
1. **Hot Data Detection:** ~40% of accesses go to DRAM (hot addresses)
2. **Cold Data Storage:** ~60% of accesses go to Flash (cold addresses)
3. **Migration Activity:** System actively moves data based on temperature
4. **Latency Trade-off:** Higher total latency but demonstrates capacity benefits

---

## Interpreting Results

### Good Indicators (Hybrid Mode Working Correctly)

✅ **DRAM Access Ratio: 35-45%**
- Hot addresses being kept in DRAM

✅ **Flash Access Ratio: 55-65%**
- Cold addresses stored in Flash

✅ **More Migrations to Flash than DRAM**
- System identifies and demotes cold data

✅ **Row Buffer Hit Rate < 1%**
- Expected for random access patterns

### Adjusting Behavior

To make more data stay in DRAM (less aggressive migration):
```cpp
// Edit include/memory_simulator.h
const uint64_t HOT_DATA_THRESHOLD = 3;  // Increase to 5 or 10
```

To test different workload patterns:
```cpp
// Edit src/memory_simulator.cpp, generateTrace() function
// Adjust hot_access percentage:
bool is_hot_access = (access_type(rng) < 80);  // Change 80 to 90 for more hot accesses
```

---

## Trace Size Recommendations

| Trace Size | Use Case | Time | Migrations |
|-----------|----------|------|------------|
| 1,000 | Quick test | <1s | ~500 |
| 10,000 | Development | <5s | ~5,000 |
| 100,000 | Standard test | ~5s | ~50,000 |
| 1,000,000 | Performance test | ~60s | ~500,000 |
| 10,000,000 | Stress test | ~10min | ~5,000,000 |

---

## Troubleshooting

### Issue: Very low row buffer hit rate
**Cause:** Random access pattern (by design)
**Expected:** This is normal for the generated trace

### Issue: No migrations happening
**Check:**
1. Is `enable_flash = true` in config?
2. Is trace size large enough (>100 accesses)?

### Issue: All accesses going to Flash
**Cause:** Hot data threshold too low
**Fix:** Increase HOT_DATA_THRESHOLD in header file

### Issue: Compilation errors
**Fix:**
```bash
make clean
make
```

---

## Next Steps

1. **Enable Flash:** Set `enable_flash = true` in config
2. **Start Small:** Test with `trace_lines = 1000`
3. **Compare Modes:** Run `./compare_modes.sh`
4. **Scale Up:** Increase to 100K, 1M accesses
5. **Analyze:** Study migration patterns and latency breakdown

## Files Reference

- `config/default.cfg` - Configuration file
- `README_HYBRID.md` - Detailed implementation guide
- `compare_modes.sh` - Automated comparison script
- `build/HybridMemSim` - Compiled executable

---

**Ready to test!** Start with the Quick Validation scenario and work your way up to large-scale tests.
