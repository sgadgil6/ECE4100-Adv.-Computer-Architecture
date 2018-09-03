#ifndef CACHESIM_HPP
#define CACHESIM_HPP

#ifdef CCOMPILER
#include <stdint.h>
#else
#include <cstdint>
#endif

struct cache_stats_t {
    uint64_t accesses;
    uint64_t reads;
    uint64_t read_hits_l1;
    uint64_t read_misses_l1;
    uint64_t writes;
    uint64_t write_hits_l1;
    uint64_t write_misses_l1;
    uint64_t write_back_l1;
    uint64_t total_hits_l1;
    uint64_t total_misses_l1;
    double total_hit_ratio;
    double total_miss_ratio;
    double read_hit_ratio;
    double read_miss_ratio;
    double write_hit_ratio;
    double write_miss_ratio;
    double avg_access_time_l1;
};

void setup_cache(uint64_t c1, uint64_t b1, uint64_t s1);

void cache_access(char type, uint64_t arg, cache_stats_t* p_stats);
void complete_cache(cache_stats_t *p_stats);

static const uint64_t DEFAULT_C1 = 12;   /* 4KB Cache */
static const uint64_t DEFAULT_B1 = 5;    /* 32-byte blocks */
static const uint64_t DEFAULT_S1 = 3;    /* 8 blocks per set */

/** Argument to cache_access rw. Indicates a load */
static const char     READ = 'r';
/** Argument to cache_access rw. Indicates a store */
static const char     WRITE = 'w';

#endif /* CACHESIM_HPP */
