# TLB Simulator

A Translation Lookaside Buffer (TLB) simulator written in C++ that supports set-associative TLB with LRU and FIFO replacement policies.

## What is a TLB?

A TLB is a hardware cache used by the CPU to speed up virtual-to-physical address translation. Instead of walking the page table every time, the CPU checks the TLB first. If the VPN (Virtual Page Number) is found — it's a **HIT**. If not — it's a **MISS**, and the page table is consulted.

## How It Works

1. Reads a page table file mapping VPNs to PFNs (Physical Frame Numbers)
2. Reads a list of virtual addresses to access
3. Simulates TLB lookups with the given associativity and replacement policy
4. Outputs physical addresses with HIT/MISS labels, plus a summary

## Project Structure

```
.
├── main.cpp          # TLB simulator source code
├── page_table.txt    # Sample page table input
├── va_accesses.txt   # Sample virtual address accesses
└── README.md
```

## Building

```bash
g++ -std=c++17 -Wall -O2 -o tlb_sim main.cpp -lm
```

Or if you have a Makefile:

```bash
make
```

## Running

```bash
./tlb_sim <page_table_file> <va_file> <replacement_policy> <ways>
```

| Argument             | Description                                      |
|----------------------|--------------------------------------------------|
| `page_table_file`    | File containing VPN to PFN mappings              |
| `va_file`            | File containing virtual addresses to simulate    |
| `replacement_policy` | `LRU` (Least Recently Used) or `FIFO`            |
| `ways`               | TLB associativity (e.g. 1, 2, 4)                |

### Example

```bash
./tlb_sim page_table.txt va_accesses.txt LRU 4
```

## Input File Formats

### Page Table File (`page_table.txt`)

First line must be the header `VPN      PFN`, followed by VPN-PFN pairs:

```
VPN      PFN
0 10
1 20
2 30
3 40
4 50
```

### Virtual Address File (`va_accesses.txt`)

First line is the page size as a power of 2 (e.g. `12` means page size = 2^12 = 4096 bytes), followed by virtual addresses one per line:

```
12
0
4096
8192
0
4096
```

## Output

The output file is automatically named:

```
23335_<page_table_file>_<va_file>_<policy>_<ways>
```

Example output:

```
TOTAL_ACCESSES = 8
TOTAL_MISSES = 5
TOTAL_HITS = 3
40960 MISS
81920 MISS
122880 MISS
40960 HIT
81920 HIT
163840 MISS
204800 MISS
40960 HIT
```

## Address Translation

Physical Address is calculated as:

```
VPN      = Virtual Address / Page Size
Offset   = Virtual Address % Page Size
PA       = (PFN × Page Size) + Offset
```

## Replacement Policies

- **LRU** — evicts the entry that was least recently used. Counters are updated on both hits and misses.
- **FIFO** — evicts the entry that was loaded first. Counters only increment on misses.
