#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Structural Limits
#define RING_CAPACITY 8
#define INDEX_MASK    (RING_CAPACITY - 1)

// Execution Status Codes
#define STATUS_OK       0
#define STATUS_OVERFLOW -1
#define STATUS_UNDERFLOW -2

// FIFO Control block using a pointer-based tracking system
typedef struct {
    uint8_t data_space[RING_CAPACITY];
    uint8_t *wr_ptr;      // Points to the current write location (head)
    uint8_t *rd_ptr;      // Points to the current read location (tail)
    uint32_t active_bytes; // Tracks stored quantity
} fifo_buffer_t;

// API Declarations
void fifo_clear(fifo_buffer_t *fifo);
bool fifo_is_full(const fifo_buffer_t *fifo);
bool fifo_is_empty(const fifo_buffer_t *fifo);
uint32_t fifo_level(const fifo_buffer_t *fifo);
int32_t fifo_push(fifo_buffer_t *fifo, uint8_t val);
int32_t fifo_pop(fifo_buffer_t *fifo, uint8_t *out_val);

// Initialize or reset the buffer status
void fifo_clear(fifo_buffer_t *fifo) {
    if (fifo != NULL) {
        fifo->wr_ptr = &fifo->data_space[0];
        fifo->rd_ptr = &fifo->data_space[0];
        fifo->active_bytes = 0;
    }
}

// Return true if the buffer has hit maximum capacity
bool fifo_is_full(const fifo_buffer_t *fifo) {
    return (fifo->active_bytes == RING_CAPACITY);
}

// Return true if the buffer has no elements
bool fifo_is_empty(const fifo_buffer_t *fifo) {
    return (fifo->active_bytes == 0);
}

// Fetch the current total number of elements stored
uint32_t fifo_level(const fifo_buffer_t *fifo) {
    return fifo->active_bytes;
}

// Add a byte into the FIFO space
int32_t fifo_push(fifo_buffer_t *fifo, uint8_t val) {
    if (fifo_is_full(fifo)) {
        return STATUS_OVERFLOW;
    }

    // Insert byte at the current write position
    *(fifo->wr_ptr) = val;

    /*
     * BONUS EXPLANATION:
     * Instead of using a modulo block, pointer wrap-around is managed by calculating 
     * the new offset relative to the base address. Using a bitwise AND mask with 
     * (RING_CAPACITY - 1) ensures bounds compliance in 1 CPU cycle, which optimizes 
     * performance for low-power MCUs missing hardware division blocks. This mask 
     * logic strictly relies on the capacity being a true power of two.
     */
    uintptr_t current_offset = (uintptr_t)(fifo->wr_ptr - fifo->data_space);
    uintptr_t next_offset = (current_offset + 1) & INDEX_MASK;
    fifo->wr_ptr = fifo->data_space + next_offset;
    
    fifo->active_bytes++;
    return STATUS_OK;
}

// Pull a byte out of the FIFO space
int32_t fifo_pop(fifo_buffer_t *fifo, uint8_t *out_val) {
    if (fifo_is_empty(fifo) || out_val == NULL) {
        return STATUS_UNDERFLOW;
    }

    // Extract the element from the current read position
    *out_val = *(fifo->rd_ptr);

    uintptr_t current_offset = (uintptr_t)(fifo->rd_ptr - fifo->data_space);
    uintptr_t next_offset = (current_offset + 1) & INDEX_MASK;
    fifo->rd_ptr = fifo->data_space + next_offset;

    fifo->active_bytes--;
    return STATUS_OK;
}

int main(void) {
    fifo_buffer_t raw_fifo;
    fifo_clear(&raw_fifo);

    uint8_t initial_burst[8] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48};
    uint8_t target_byte = 0;
    int32_t result;

    // 1. Initial burst load sequence
    for (int idx = 0; idx < 8; idx++) {
        result = fifo_push(&raw_fifo, initial_burst[idx]);
        if (result == STATUS_OK) {
            printf("[WRITE] 0x%02X -> OK (count=%d)", initial_burst[idx], fifo_level(&raw_fifo));
            if (fifo_is_full(&raw_fifo)) {
                printf(" FULL");
            }
            printf("\n");
        }
    }

    // 2. Validate overflow handling boundary
    result = fifo_push(&raw_fifo, 0x99);
    if (result == STATUS_OVERFLOW) {
        printf("[WRITE] 0x99 -> FAIL (buffer full)\n");
    }

    // 3. Partial depletion pass (3 items)
    for (int idx = 0; idx < 3; idx++) {
        result = fifo_pop(&raw_fifo, &target_byte);
        if (result == STATUS_OK) {
            printf("[READ] -> 0x%02X (count=%d)\n", target_byte, fifo_level(&raw_fifo));
        }
    }

    // 4. Reload pass into available slots
    uint8_t secondary_burst[3] = {0x49, 0x4A, 0x4B};
    for (int idx = 0; idx < 3; idx++) {
        result = fifo_push(&raw_fifo, secondary_burst[idx]);
        if (result == STATUS_OK) {
            printf("[WRITE] 0x%02X -> OK (count=%d)\n", secondary_burst[idx], fifo_level(&raw_fifo));
        }
    }

    // 5. Run full loop depletion
    while (!fifo_is_empty(&raw_fifo)) {
        result = fifo_pop(&raw_fifo, &target_byte);
        if (result == STATUS_OK) {
            printf("[READ] -> 0x%02X (count=%d)\n", target_byte, fifo_level(&raw_fifo));
        }
    }

    // 6. Validate underflow protection handling
    result = fifo_pop(&raw_fifo, &target_byte);
    if (result == STATUS_UNDERFLOW) {
        printf("[READ] (empty) -> FAIL (buffer empty)\n");
    }

    return 0;
}
