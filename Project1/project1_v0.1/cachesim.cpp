#include "cachesim.hpp"
#include <cmath>
#include <iostream>
using namespace std;
//declaring the LRU counter to keep track of the LRU block
int time_counter;

//declaring functions used in the program
unsigned createMask(unsigned, unsigned);
void setValues(int, int, long, int);

//the cache block is declared as a struct object with 
//required components
struct cache_block {
	int dirty_bit;
	int valid_bit;
	long tag;
	long LRUNum;
};

//declaring variables to measure hit/miss statistics
uint64_t accesses, reads, writes, read_hits_l1, write_hits_l1, total_hits_l1, read_misses_l1, write_misses_l1, total_misses_l1, write_back_l1;

//the cache is a double pointer to the struct object (essentially a matrix of struct objects)
//Each pointer is a way pointing to the sets in that way
struct cache_block ** cache;

//declaring global variables to keep track of the different bits of the address 
int blocksize_bits, tag_bits, set_bits, way_num, num_sets, S;

/**
 * Subroutine for initializing the cache. You many add and initialize any global or heap
 * variables as needed.
 * XXX: You're responsible for completing this routine
 *
 * @c1 The total number of bytes for data storage in L1 is 2^c
 * @b1 The size of L1's blocks in bytes: 2^b-byte blocks.
 * @s1 The number of blocks in each set of L1: 2^s blocks per set.
 */
void setup_cache(uint64_t c1, uint64_t b1, uint64_t s1) {
	int cachesize = pow(2, c1);
	int blocksize = pow(2, b1);
	//finding the number of blocks, number of sets, and number of ways in each set
	int num_blocks = cachesize / blocksize;
	way_num = pow(2, s1);
	S = s1;
	num_sets = num_blocks / way_num;
	//allocate memory for each way
	cache = new cache_block *[way_num];
	int i;
	for (i = 0; i < way_num; i++) {
		//allocate memory for each set in a way
		cache[i] = new cache_block[num_sets];
		int j;
		for (j = 0; j < num_sets; j++) {
			//initializing the struct object fields for a cold start
			cache[i][j].dirty_bit = 0;
			cache[i][j].valid_bit = 0;
			cache[i][j].tag = 0;
			cache[i][j].LRUNum = 0;
		}
	}

	blocksize_bits = b1;
	set_bits = log2(num_sets);
	//initializing the LRU counter
	time_counter = 0;
}

/**
 * Subroutine that simulates the cache one trace event at a time.
 * XXX: You're responsible for completing this routine
 *
 * @type The type of event, can be READ or WRITE.
 * @arg  The target memory address
 * @p_stats Pointer to the statistics structure
 */
void cache_access(char type, uint64_t arg, cache_stats_t* p_stats) {
	//increment accesses every time this function is called
	accesses++;
	//extract the required bits from the address and calculated the set number and tag
	int total_bits = floor(log2(arg)) + 1;
	tag_bits = total_bits - (blocksize_bits + set_bits);
	int set_num;
	long tag_value;
	if (tag_bits != total_bits) {
		set_num = (createMask(blocksize_bits, blocksize_bits + set_bits - 1) & arg) >> blocksize_bits;
		tag_value = (createMask(blocksize_bits + set_bits, total_bits - 1) & arg) >> (blocksize_bits + set_bits);
	}
	else {
		//if c,b,s are all zeros, there is only one block in the entire cache
		tag_value = arg;
		set_num = 0;
	}
	//increment reads or writes based on access type
	if (type == 'r')
		reads++;
	else
		writes++;
	
	bool hit = false;
	struct cache_block* curr_set;
	int i;
	//loop through all the blocks in a set to check a hit
	for (i = 0; i < way_num; i++) {
		curr_set = &cache[i][set_num];

		if (curr_set->tag == tag_value && curr_set->valid_bit == 1) {
			//There is a hit
			hit = true;
			break;
		}
	}

	if (hit) {
		//increase hit counters, set the LRU value and set dirty bit if required
		cout << "H" << endl;
		curr_set->LRUNum = time_counter;
		time_counter++;
		total_hits_l1++;
		if (type == 'r')
			read_hits_l1++;
		else
			write_hits_l1++;

		if (type == 'w' && curr_set->dirty_bit == 0)
			curr_set->dirty_bit = 1;
	}
	else {
		//Miss
		cout << "M" << endl;
		//increment miss counters
		total_misses_l1++;
		if (type == 'r')
			read_misses_l1++;
		else
			write_misses_l1++;

		bool block_empty = false;
		int i;
		//loop through blocks to find if there is an empty block
		for (i = 0; i < way_num; i++) {
			if (cache[i][set_num].valid_bit == 0) {
				//empty block found
				block_empty = true;
				break;
			}
		}

		if (block_empty) {
			//bring in the required block and set the struct fields
			int dirty = 0;
			if (type == 'w')
				dirty = 1;
			setValues(i, set_num, tag_value, dirty);
		}
		else {
			//set is full, need to find a victim
			//LRU replacement policy used
			long smallest_LRUNum = cache[0][set_num].LRUNum;
			int evict_num = 0;
			int i;
			//find block with smallest LRUNum to evict it
			for (i = 0; i < way_num; i++) {
				if (cache[i][set_num].LRUNum < smallest_LRUNum) {
					smallest_LRUNum = cache[i][set_num].LRUNum;
					evict_num = i;
				}
			}

			//increase write backs only when a block is evicted and is dirty
			if (cache[evict_num][set_num].dirty_bit == 1)
				write_back_l1++;

			int dirty = 0;
			if (type == 'w')
				dirty = 1;
			//overwrite the evicted block with the new field values
			setValues(evict_num, set_num, tag_value, dirty);
		}
	}
}

/**
 * Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_cache(cache_stats_t *p_stats) {
	int i;
	for (i = 0; i < way_num; i++) {
		delete[] cache[i];
	}

	delete[] cache;
	float HT = 2 + (0.2 * S);
	float MP = 20;
	float MR = total_misses_l1 / (float) accesses;
	p_stats->accesses = accesses;
	p_stats->reads = reads;
	p_stats->read_hits_l1 = read_hits_l1;
	p_stats->read_misses_l1 = read_misses_l1;
	p_stats->writes = writes;
	p_stats->write_hits_l1 = write_hits_l1;
	p_stats->write_misses_l1 = write_misses_l1;
	p_stats->write_back_l1 = write_back_l1;
	p_stats->total_hits_l1 = total_hits_l1;
	p_stats->total_misses_l1 = total_misses_l1;
	p_stats->total_hit_ratio = total_hits_l1 / (float) accesses;
	p_stats->total_miss_ratio = total_misses_l1 / (float) accesses;
	p_stats->read_hit_ratio = read_hits_l1 / (float) reads;
	p_stats->read_miss_ratio = read_misses_l1 / (float)reads;
	p_stats->write_hit_ratio = write_hits_l1 / (float) writes;
	p_stats->write_miss_ratio = write_misses_l1 / (float) writes;
	p_stats->avg_access_time_l1 = HT + (MR * MP);

}

/**
* Subroutine for setting the different fields of a block
*
* @i the way number
* @set_num the set number
* @tag_value the tag
* @dirty block is dirty or not
*/
void setValues(int i, int set_num, long tag_value, int dirty) {
	cache[i][set_num].dirty_bit = dirty;
	cache[i][set_num].valid_bit = 1;
	cache[i][set_num].tag = tag_value;
	cache[i][set_num].LRUNum = time_counter++;
}

/**
* Subroutine for creating a mask to get required bits from an address
*
* @a the starting index of the required bits
* @b the ending index of the required bits
*/
unsigned createMask(unsigned a, unsigned b)
{
	unsigned r = 0;
	unsigned i;
	for (i = a; i <= b; i++)
		r |= 1 << i;

	return r;
}
