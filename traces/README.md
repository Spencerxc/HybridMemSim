# Memory Traces

This directory contains memory access trace files for simulation.

## Trace Format

Each line in a trace file should contain:
```
<operation> <address>
```

Where:
- `operation`: R (read) or W (write)
- `address`: Memory address in hexadecimal

Example:
```
R 0x1000
W 0x2000
R 0x1040
```
