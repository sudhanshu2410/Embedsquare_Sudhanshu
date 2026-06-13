# Embedsquare - Embedded Developer Assignment (Ring Buffer)

## Description
[cite_start]This repository contains my implementation of a circular FIFO ring buffer utility using standard C (C99) for the fresher firmware technical assessment[cite: 3]. [cite_start]The module implements a fixed-capacity 8-byte buffer designed to manage asynchronous data transfer between a producer and a consumer[cite: 6, 19, 21, 43].

[cite_start]To optimize performance for resource-constrained microcontrollers that lack a hardware divider, this implementation replaces traditional modulo operations (`% BUFFER_SIZE`) with a fast bitwise AND mask (`& (BUFFER_SIZE - 1)`) for pointer wrap-around calculations[cite: 40, 41, 80]. [cite_start]This optimization works efficiently because the buffer capacity is strictly a power of 2[cite: 41, 81].

---

## Technical Stack
* [cite_start]**Language:** C (C99 standard) [cite: 9]
* [cite_start]**Compiler Requirements:** gcc with strict warning tracking (`-Wall -std=c99`) [cite: 10]
* [cite_start]**Dependencies:** Standard C Library headers only (`<stdio.h>`, `<stdint.h>`, `<stdbool.h>`) [cite: 14]

---

## Build and Execution Instructions

### 1. Prerequisites
[cite_start]Ensure you have a standard GCC toolchain installed on your local environment (Linux, macOS, or Windows via WSL/MinGW)[cite: 11].

### 2. Compilation
[cite_start]To compile the standalone test program with zero errors and zero warnings, run the following command in your terminal[cite: 10, 85]:

```bash
gcc -Wall -std=c99 ringbuf.c -o ringbuf
