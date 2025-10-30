# HybridMemSim Architecture

## Overview

This simulator models a DRAM subsystem with optional Flash memory in a hybrid configuration.

## Components

### DRAM Subsystem
- Row buffers
- Access queues
- Refresh mechanism

### Flash Memory (Optional)
- Page-based access
- Wear leveling (simplified)

### Memory Controller
- Request scheduling
- Address mapping
- Hybrid tier management

## Simulation Flow

1. Load memory trace
2. Parse memory requests
3. Process requests through memory hierarchy
4. Track timing and statistics
5. Output performance metrics
