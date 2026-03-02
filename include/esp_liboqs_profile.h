// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file esp_liboqs_profile.h
 * @brief ESP32-specific profiling infrastructure for liboqs
 * 
 * This header provides lightweight profiling capabilities for liboqs functions
 * using ESP32 hardware timers and CPU cycle counters.
 */

// Maximum number of unique functions to profile
#ifndef ESP_LIBOQS_MAX_PROFILE_ENTRIES
#define ESP_LIBOQS_MAX_PROFILE_ENTRIES 100
#endif

/**
 * @brief Profile entry for a single function
 */
typedef struct {
    const char *name;           // Function name
    uint32_t call_count;        // Number of times called
    uint64_t total_time_us;     // Total time in microseconds
    uint64_t total_cycles;      // Total CPU cycles (if available)
    uint64_t min_time_us;       // Minimum execution time
    uint64_t max_time_us;       // Maximum execution time
    uint64_t start_time_us;     // Temporary: start time for current call
    uint32_t start_cycle;       // Temporary: start cycle for current call
    bool in_progress;           // Flag to detect nested calls
} esp_liboqs_profile_entry_t;

/**
 * @brief Global profiling state
 */
typedef struct {
    esp_liboqs_profile_entry_t entries[ESP_LIBOQS_MAX_PROFILE_ENTRIES];
    int entry_count;
    bool enabled;
    uint32_t rejection_count;   // Track rejection loop iterations
} esp_liboqs_profile_state_t;

/**
 * @brief Initialize the profiling system
 */
void esp_liboqs_profile_init(void);

/**
 * @brief Enable profiling
 */
void esp_liboqs_profile_enable(void);

/**
 * @brief Disable profiling
 */
void esp_liboqs_profile_disable(void);

/**
 * @brief Reset all profiling counters
 */
void esp_liboqs_profile_reset(void);

/**
 * @brief Register a function for profiling (returns index)
 * @param name Function name
 * @return Profile index for this function
 */
int esp_liboqs_profile_register(const char *name);

/**
 * @brief Start timing a function
 * @param idx Profile index (returned by esp_liboqs_profile_register)
 */
void esp_liboqs_profile_start(int idx);

/**
 * @brief End timing a function
 * @param idx Profile index
 */
void esp_liboqs_profile_end(int idx);

/**
 * @brief Increment rejection counter (for signature rejection loops)
 */
void esp_liboqs_profile_rejection(void);

/**
 * @brief Print profiling results sorted by total time
 */
void esp_liboqs_profile_print(void);

/**
 * @brief Print profiling results sorted by call count
 */
void esp_liboqs_profile_print_by_calls(void);

/**
 * @brief Get profiling results as a structure (for custom processing)
 * @return Pointer to profiling state (read-only)
 */
const esp_liboqs_profile_state_t* esp_liboqs_profile_get_results(void);

/**
 * @brief Get current CPU cycle count (if available)
 * @return CPU cycle count or 0 if not available
 */
static inline uint32_t esp_liboqs_get_cycle_count(void) {
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
    uint32_t ccount;
    __asm__ __volatile__("rsr %0, ccount" : "=a"(ccount));
    return ccount;
#elif defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
    // RISC-V: Use mcycle CSR
    uint32_t cycle;
    __asm__ __volatile__("csrr %0, mcycle" : "=r"(cycle));
    return cycle;
#else
    return 0;
#endif
}

// Convenience macros for profiling
#ifdef CONFIG_LIBOQS_ENABLE_PROFILING

// Static registration: use when the function name is known at compile time
#define ESP_LIBOQS_PROFILE_REGISTER(var_name, func_name) \
    static int var_name = -1; \
    if (var_name == -1) { \
        var_name = esp_liboqs_profile_register(func_name); \
    }

#define ESP_LIBOQS_PROFILE_START(idx) esp_liboqs_profile_start(idx)
#define ESP_LIBOQS_PROFILE_END(idx) esp_liboqs_profile_end(idx)
#define ESP_LIBOQS_PROFILE_REJECTION() esp_liboqs_profile_rejection()

// Scoped profiling: automatically ends when leaving scope
#define ESP_LIBOQS_PROFILE_FUNCTION() \
    ESP_LIBOQS_PROFILE_REGISTER(_profile_idx, __func__); \
    ESP_LIBOQS_PROFILE_START(_profile_idx); \
    struct _profile_cleanup { \
        int idx; \
        ~_profile_cleanup() { esp_liboqs_profile_end(idx); } \
    } _profile_guard = {_profile_idx};

#else
// Profiling disabled - all macros become no-ops
#define ESP_LIBOQS_PROFILE_REGISTER(var_name, func_name)
#define ESP_LIBOQS_PROFILE_START(idx)
#define ESP_LIBOQS_PROFILE_END(idx)
#define ESP_LIBOQS_PROFILE_REJECTION()
#define ESP_LIBOQS_PROFILE_FUNCTION()
#endif

#ifdef __cplusplus
}
#endif
