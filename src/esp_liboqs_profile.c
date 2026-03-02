// SPDX-License-Identifier: MIT

#include "esp_liboqs_profile.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "liboqs_profile";

// Global profiling state
static esp_liboqs_profile_state_t g_profile_state = {
    .entry_count = 0,
    .enabled = false,
    .rejection_count = 0
};

void esp_liboqs_profile_init(void) {
    memset(&g_profile_state, 0, sizeof(g_profile_state));
    g_profile_state.enabled = true;
    ESP_LOGI(TAG, "Profiling initialized (max %d functions)", ESP_LIBOQS_MAX_PROFILE_ENTRIES);
}

void esp_liboqs_profile_enable(void) {
    g_profile_state.enabled = true;
    ESP_LOGI(TAG, "Profiling enabled");
}

void esp_liboqs_profile_disable(void) {
    g_profile_state.enabled = false;
    ESP_LOGI(TAG, "Profiling disabled");
}

void esp_liboqs_profile_reset(void) {
    for (int i = 0; i < g_profile_state.entry_count; i++) {
        g_profile_state.entries[i].call_count = 0;
        g_profile_state.entries[i].total_time_us = 0;
        g_profile_state.entries[i].total_cycles = 0;
        g_profile_state.entries[i].min_time_us = UINT64_MAX;
        g_profile_state.entries[i].max_time_us = 0;
        g_profile_state.entries[i].in_progress = false;
    }
    g_profile_state.rejection_count = 0;
    ESP_LOGI(TAG, "Profiling counters reset");
}

int esp_liboqs_profile_register(const char *name) {
    if (!name) {
        return -1;
    }

    // Check if already registered
    for (int i = 0; i < g_profile_state.entry_count; i++) {
        if (strcmp(g_profile_state.entries[i].name, name) == 0) {
            return i;
        }
    }

    // Register new entry
    if (g_profile_state.entry_count >= ESP_LIBOQS_MAX_PROFILE_ENTRIES) {
        ESP_LOGW(TAG, "Profile entries full, cannot register '%s'", name);
        return -1;
    }

    int idx = g_profile_state.entry_count++;
    esp_liboqs_profile_entry_t *entry = &g_profile_state.entries[idx];
    
    entry->name = name;
    entry->call_count = 0;
    entry->total_time_us = 0;
    entry->total_cycles = 0;
    entry->min_time_us = UINT64_MAX;
    entry->max_time_us = 0;
    entry->in_progress = false;

    ESP_LOGD(TAG, "Registered function '%s' at index %d", name, idx);
    return idx;
}

void esp_liboqs_profile_start(int idx) {
    if (!g_profile_state.enabled || idx < 0 || idx >= g_profile_state.entry_count) {
        return;
    }

    esp_liboqs_profile_entry_t *entry = &g_profile_state.entries[idx];
    
    // Handle nested calls - don't override start time
    if (entry->in_progress) {
        return;
    }

    entry->start_time_us = esp_timer_get_time();
    entry->start_cycle = esp_liboqs_get_cycle_count();
    entry->in_progress = true;
}

void esp_liboqs_profile_end(int idx) {
    if (!g_profile_state.enabled || idx < 0 || idx >= g_profile_state.entry_count) {
        return;
    }

    esp_liboqs_profile_entry_t *entry = &g_profile_state.entries[idx];
    
    if (!entry->in_progress) {
        return;
    }

    uint64_t end_time_us = esp_timer_get_time();
    uint32_t end_cycle = esp_liboqs_get_cycle_count();

    uint64_t elapsed_us = end_time_us - entry->start_time_us;
    uint32_t elapsed_cycles = end_cycle - entry->start_cycle; // Handles wrap-around

    entry->call_count++;
    entry->total_time_us += elapsed_us;
    entry->total_cycles += elapsed_cycles;

    if (elapsed_us < entry->min_time_us) {
        entry->min_time_us = elapsed_us;
    }
    if (elapsed_us > entry->max_time_us) {
        entry->max_time_us = elapsed_us;
    }

    entry->in_progress = false;
}

void esp_liboqs_profile_rejection(void) {
    if (g_profile_state.enabled) {
        g_profile_state.rejection_count++;
    }
}

const esp_liboqs_profile_state_t* esp_liboqs_profile_get_results(void) {
    return &g_profile_state;
}

// Comparison functions for sorting
static int compare_by_total_time(const void *a, const void *b) {
    const esp_liboqs_profile_entry_t *ea = (const esp_liboqs_profile_entry_t *)a;
    const esp_liboqs_profile_entry_t *eb = (const esp_liboqs_profile_entry_t *)b;
    
    if (eb->total_time_us > ea->total_time_us) return 1;
    if (eb->total_time_us < ea->total_time_us) return -1;
    return 0;
}

static int compare_by_call_count(const void *a, const void *b) {
    const esp_liboqs_profile_entry_t *ea = (const esp_liboqs_profile_entry_t *)a;
    const esp_liboqs_profile_entry_t *eb = (const esp_liboqs_profile_entry_t *)b;
    
    if (eb->call_count > ea->call_count) return 1;
    if (eb->call_count < ea->call_count) return -1;
    return 0;
}

void esp_liboqs_profile_print(void) {
    if (g_profile_state.entry_count == 0) {
        ESP_LOGI(TAG, "No profiling data available");
        return;
    }

    // Create a copy for sorting
    esp_liboqs_profile_entry_t *sorted = malloc(sizeof(esp_liboqs_profile_entry_t) * g_profile_state.entry_count);
    if (!sorted) {
        ESP_LOGE(TAG, "Failed to allocate memory for sorting");
        return;
    }

    // Copy only entries with data
    int count = 0;
    for (int i = 0; i < g_profile_state.entry_count; i++) {
        if (g_profile_state.entries[i].call_count > 0) {
            sorted[count++] = g_profile_state.entries[i];
        }
    }

    if (count == 0) {
        ESP_LOGI(TAG, "No profiling data recorded");
        free(sorted);
        return;
    }

    // Sort by total time
    qsort(sorted, count, sizeof(esp_liboqs_profile_entry_t), compare_by_total_time);

    ESP_LOGI(TAG, "================== PROFILING RESULTS (by total time) ==================");
    ESP_LOGI(TAG, "%-50s %10s %15s %12s %12s %12s", 
             "Function", "Calls", "Total(μs)", "Avg(μs)", "Min(μs)", "Max(μs)");
    ESP_LOGI(TAG, "------------------------------------------------------------------------");

    uint64_t grand_total_us = 0;
    uint32_t grand_total_calls = 0;

    for (int i = 0; i < count; i++) {
        esp_liboqs_profile_entry_t *e = &sorted[i];
        uint64_t avg_us = e->total_time_us / e->call_count;
        
        ESP_LOGI(TAG, "%-50s %10u %15llu %12llu %12llu %12llu",
                 e->name,
                 e->call_count,
                 e->total_time_us,
                 avg_us,
                 e->min_time_us,
                 e->max_time_us);

        grand_total_us += e->total_time_us;
        grand_total_calls += e->call_count;
    }

    ESP_LOGI(TAG, "------------------------------------------------------------------------");
    ESP_LOGI(TAG, "Total: %u calls, %llu μs (%.3f ms)", 
             grand_total_calls, grand_total_us, grand_total_us / 1000.0);
    
    if (g_profile_state.rejection_count > 0) {
        ESP_LOGI(TAG, "Signature rejection loop iterations: %u", g_profile_state.rejection_count);
    }
    
    ESP_LOGI(TAG, "========================================================================");

    free(sorted);
}

void esp_liboqs_profile_print_by_calls(void) {
    if (g_profile_state.entry_count == 0) {
        ESP_LOGI(TAG, "No profiling data available");
        return;
    }

    // Create a copy for sorting
    esp_liboqs_profile_entry_t *sorted = malloc(sizeof(esp_liboqs_profile_entry_t) * g_profile_state.entry_count);
    if (!sorted) {
        ESP_LOGE(TAG, "Failed to allocate memory for sorting");
        return;
    }

    // Copy only entries with data
    int count = 0;
    for (int i = 0; i < g_profile_state.entry_count; i++) {
        if (g_profile_state.entries[i].call_count > 0) {
            sorted[count++] = g_profile_state.entries[i];
        }
    }

    if (count == 0) {
        ESP_LOGI(TAG, "No profiling data recorded");
        free(sorted);
        return;
    }

    // Sort by call count
    qsort(sorted, count, sizeof(esp_liboqs_profile_entry_t), compare_by_call_count);

    ESP_LOGI(TAG, "================== PROFILING RESULTS (by call count) ==================");
    ESP_LOGI(TAG, "%-50s %10s %15s %12s", 
             "Function", "Calls", "Total(μs)", "Avg(μs)");
    ESP_LOGI(TAG, "------------------------------------------------------------------------");

    for (int i = 0; i < count; i++) {
        esp_liboqs_profile_entry_t *e = &sorted[i];
        uint64_t avg_us = e->total_time_us / e->call_count;
        
        ESP_LOGI(TAG, "%-50s %10u %15llu %12llu",
                 e->name,
                 e->call_count,
                 e->total_time_us,
                 avg_us);
    }

    ESP_LOGI(TAG, "========================================================================");

    free(sorted);
}
