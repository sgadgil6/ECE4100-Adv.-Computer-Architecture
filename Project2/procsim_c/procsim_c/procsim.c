#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "procsim.h"

//function definitions
void state_update(void);
void execute(void);
void schedule(void);
void dispatch(void);
int RS_isFull(void);
bool ROB_isFull(void);
int Pregs_Avail(void);
int comparator(const void*, const void*);

//struct to represent an entry in the ROB
typedef struct ROB_entry {
	int32_t dest_preg_index;
	int32_t dest_areg_index;
	int32_t prev_preg_index;
	int ready;
	long entry_num;
} ROB_entry;

//struct to represent an entry in the Reservation Station
typedef struct RS_entry {
	int valid;
	int FU;
	int32_t dest_preg_index;
	int32_t src1_reg_index;
	int32_t src2_reg_index;
	long entry_num;
	int fired;
} RS_entry;

//struct to represent an entry in the Physical Register File
typedef struct PRF_entry {
	int ready;
	int free;
} PRF_entry;

//struct to represent entries for the k0 type functional units
typedef struct scoreboard_k0_entry {
	int busy;
	int32_t dest_preg_index;
	long entry_num;
} scoreboard_k0_entry;

//struct to represent entries for the k1 type functional units
typedef struct scoreboard_k1_entry {
	int busy;
	int32_t dest_preg_index;
	long entry_num;
} scoreboard_k1_entry;

//struct to represent entries for the k2 type functional units
typedef struct scoreboard_k2_entry {
	int busy;
	int32_t dest_preg_index;
	long entry_num;
} scoreboard_k2_entry;

//variables required to represent ROB and RS
int rob_head;
int rob_tail;
int rob_size;
int num_rob_entries;
int num_RS_entries;

//decalring arrays required for simulation
struct ROB_entry* ROB;
struct RS_entry* RS;
struct PRF_entry* PRF;
int* RAT;
int* ARF;
struct scoreboard_k0_entry* scoreboard_k0;
struct scoreboard_k1_entry* scoreboard_k1;
struct scoreboard_k2_entry* scoreboard_k2;
uint64_t fetch_width;;
int clock_cycle;
int instructions_fetched;
int instructions_retired;
bool first_time;
int num_preg_entries;
int k0_units;
int k1_units;
int k2_units;
unsigned long retired_instructions;
unsigned long fired_instructions;
long entry_num;
bool trace_ended;
bool all_retired;
proc_inst_t p_inst;


/**
* Subroutine for initializing the processor. You many add and initialize any global or heap
* variables as needed.
* XXX: You're responsible for completing this routine
*
* @k0 Number of k0 FUs
* @k1 Number of k1 FUs
* @k2 Number of k2 FUs
* @f Number of instructions to fetch
* @ROB Number of ROB Entries
* @PREG Number of registers in the PRF
*/
void setup_proc(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t rob, uint64_t preg)
{
	//allocating memory to arrays
	scoreboard_k0 = (scoreboard_k0_entry*)calloc(k0, sizeof(scoreboard_k0_entry));
	scoreboard_k1 = (scoreboard_k1_entry*)calloc(k1, sizeof(scoreboard_k1_entry));
	scoreboard_k2 = (scoreboard_k2_entry*)calloc(k2, sizeof(scoreboard_k2_entry));

	ROB = (ROB_entry*)calloc(rob, sizeof(ROB_entry));
	RS = (RS_entry*)calloc(rob, sizeof(RS_entry));

	ARF = (int*)calloc(32, sizeof(int));
	PRF = (PRF_entry*)calloc((32 + preg), sizeof(PRF_entry));
	RAT = (int*)calloc(32, sizeof(int));
	int i;
	//all physical registers are free and not ready
	for (i = 32; i < (32 + preg); i++) {
		PRF[i].free = 1;
	}
	//all architectural registers are ready but not free
	for (i = 0; i < 32; i++) {
		PRF[i].ready = 1;
	}
	//initializing RAT
	for (i = 0; i < 32; i++) {
		RAT[i] = i;
	}

	//initializing the different variables
	clock_cycle = 0;
	fetch_width = f;
	num_rob_entries = rob;
	num_RS_entries = rob;
	rob_head = 0;
	rob_tail = 0;
	rob_size = 0;
	instructions_fetched = 0;
	instructions_retired = 0;
	first_time = true;
	num_preg_entries = preg;
	k0_units = k0;
	k1_units = k1;
	k2_units = k2;
	retired_instructions = 0;
	fired_instructions = 0;
	entry_num = 1;
	all_retired = false;
	trace_ended = false;
}

/**
* Subroutine that simulates the processor.
*   The processor should fetch instructions as appropriate, until all instructions have executed
* XXX: You're responsible for completing this routine
*
* @p_stats Pointer to the statistics structure
*/
void run_proc(proc_stats_t* p_stats)
{
	//propagate instructions through different stages until all instructions from the trace are retired
	while (!all_retired) {
		first_time = false;
		clock_cycle++;
		state_update();
		execute();
		schedule();
		dispatch();
		if (trace_ended && rob_size == 0)
			all_retired = true;

	}
}

/**
* Subroutine to simulate dispatch stage
*/
void dispatch() {
	int i;
	//try to fetch instructions equal to fetch width
	for (i = 0; i < fetch_width; i++) {
		int avail_preg_index = Pregs_Avail();
		int avail_RS_index = RS_isFull();
		if (avail_RS_index != -1 && !ROB_isFull() && avail_preg_index != -1) {
			trace_ended = !read_instruction(&p_inst);
			if (!trace_ended) {
				//dispatch instruction only if ROB is not full, RS is not full, and there is a PREG available
				rob_tail = (rob_head + rob_size) % num_rob_entries;
				RS[avail_RS_index].valid = 1;
				RS[avail_RS_index].FU = p_inst.op_code;
				RS[avail_RS_index].entry_num = entry_num;
				RS[avail_RS_index].fired = 0;
				ROB[rob_tail].entry_num = entry_num;
				entry_num++;
				if (p_inst.src_reg[0] != -1)
					RS[avail_RS_index].src1_reg_index = RAT[p_inst.src_reg[0]];
				else
					RS[avail_RS_index].src1_reg_index = -1;
				if (p_inst.src_reg[1] != -1)
					RS[avail_RS_index].src2_reg_index = RAT[p_inst.src_reg[1]];
				else
					RS[avail_RS_index].src2_reg_index = -1;

				ROB[rob_tail].dest_areg_index = p_inst.dest_reg;
				//handle case where destination register is -1
				if (p_inst.dest_reg != -1) {
					ROB[rob_tail].prev_preg_index = RAT[p_inst.dest_reg];
					RAT[p_inst.dest_reg] = avail_preg_index;
					RS[avail_RS_index].dest_preg_index = RAT[p_inst.dest_reg];
					ROB[rob_tail].dest_preg_index = RAT[p_inst.dest_reg];
					PRF[RAT[p_inst.dest_reg]].ready = 0;
					PRF[RAT[p_inst.dest_reg]].free = 0;
					ROB[rob_tail].ready = 0;
				}
				else {
					ROB[rob_tail].prev_preg_index = -1;
					ROB[rob_tail].dest_preg_index = -1;
					RS[avail_RS_index].dest_preg_index = -1;
					ROB[rob_tail].ready = 0;
				}
				rob_size++;
				instructions_fetched++;
				qsort(RS, num_RS_entries, sizeof(struct RS_entry), comparator);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}
}

/**
* Subroutine to specify relative ordering for sorting.
* @s1 void pointer for first argument
* @s2 void pointer for second argument
* @return int which argument is greater based on relative ordering
*/
int comparator(const void* s1, const void* s2) {
	struct RS_entry* e1 = (struct RS_entry*)s1;
	struct RS_entry* e2 = (struct RS_entry*)s2;

	return e1->entry_num - e2->entry_num;
}


/**
* Subroutine to check if reservation station is full or not
* @return int returns the index which is empty otherwise -1 if RS is full
*/
int RS_isFull() {
	int i;
	for (i = 0; i < num_RS_entries; i++) {
		if (RS[i].valid == 0) {
			return i;
		}
	}

	return -1;
}


/**
* Subroutine to check if ROB is full or not
* @return bool return true if empty slot available otherwise false
*/
bool ROB_isFull() {
	if (rob_size == num_rob_entries)
		return true;
	return false;
}


/**
* Subroutine to check if free PREGs are available or not
* @return int return free PREG if available else -1
*/
int Pregs_Avail() {
	int i;
	for (i = 32; i < (32 + num_preg_entries); i++) {
		if (PRF[i].free == 1) {
			return i;
		}
	}

	return -1;
}


/**
* Subroutine to simulate schedule stage
*/
void schedule() {
	int i;
	for (i = 0; i < num_RS_entries; i++) {
		if (RS[i].valid == 1) {
			if ((RS[i].src1_reg_index == -1 || PRF[RS[i].src1_reg_index].ready == 1) && (RS[i].src2_reg_index == -1 || PRF[RS[i].src2_reg_index].ready == 1)) {
				if (RS[i].FU == 0) {
					int j;
					for (j = 0; j < k0_units; j++) {
						if (scoreboard_k0[j].busy == 0) {
							//schedule instruction if both src registers are ready and the functional unit is available
							scoreboard_k0[j].busy = 1;
							scoreboard_k0[j].dest_preg_index = RS[i].dest_preg_index;
							scoreboard_k0[j].entry_num = RS[i].entry_num;
							RS[i].fired = 1;
							fired_instructions++;
							break;
						}

					}
				}
				else if (RS[i].FU == 1 || RS[i].FU == -1) {
					int j;
					for (j = 0; j < k1_units; j++) {
						if (scoreboard_k1[j].busy == 0) {
							//schedule instruction if both src registers are ready and the functional unit is available
							scoreboard_k1[j].busy = 1;
							scoreboard_k1[j].dest_preg_index = RS[i].dest_preg_index;
							scoreboard_k1[j].entry_num = RS[i].entry_num;
							RS[i].fired = 1;
							fired_instructions++;
							break;
						}

					}
				}
				else if (RS[i].FU == 2) {
					int j;
					for (j = 0; j < k2_units; j++) {
						if (scoreboard_k2[j].busy == 0) {
							//schedule instruction if both src registers are ready and the functional unit is available
							scoreboard_k2[j].busy = 1;
							scoreboard_k2[j].dest_preg_index = RS[i].dest_preg_index;
							scoreboard_k2[j].entry_num = RS[i].entry_num;
							RS[i].fired = 1;
							fired_instructions++;
							break;
						}

					}
				}
			}
		}
	}
}

/**
* Subroutine to simulate exceute stage
*/
void execute() {
	int i;
	//iterate through k0 type units to check if a functional unit has completed execution
	for (i = 0; i < k0_units; i++) {
		if (scoreboard_k0[i].busy == 1) {
			scoreboard_k0[i].busy = 0;
			int j;
			for (j = rob_head; j < (rob_head + rob_size); j++) {
				int index = j % num_rob_entries;
				if ((ROB[index].dest_preg_index == scoreboard_k0[i].dest_preg_index) && (ROB[index].entry_num == scoreboard_k0[i].entry_num)) {
					//once an instruction has completed, set the corresponding ROB entry as ready
					ROB[index].ready = 1;
					if (scoreboard_k0[i].dest_preg_index != -1)
						PRF[scoreboard_k0[i].dest_preg_index].ready = 1;
				}
			}
		}
	}


	//iterate through k1 type units to check if a functional unit has completed execution
	for (i = 0; i < k1_units; i++) {
		if (scoreboard_k1[i].busy == 1) {
			scoreboard_k1[i].busy = 0;
			int j;
			for (j = rob_head; j < (rob_head + rob_size); j++) {
				int index = j % num_rob_entries;
				if ((ROB[index].dest_preg_index == scoreboard_k1[i].dest_preg_index) && (ROB[index].entry_num == scoreboard_k1[i].entry_num)) {
					//once an instruction has completed, set the corresponding ROB entry as ready
					ROB[index].ready = 1;
					if (scoreboard_k1[i].dest_preg_index != -1)
						PRF[scoreboard_k1[i].dest_preg_index].ready = 1;
				}
			}
		}
	}

	//iterate through k1 type units to check if a functional unit has completed execution
	for (i = 0; i < k2_units; i++) {
		if (scoreboard_k2[i].busy == 1) {
			scoreboard_k2[i].busy = 0;
			int j;
			for (j = rob_head; j < (rob_head + rob_size); j++) {
				int index = j % num_rob_entries;
				if ((ROB[index].dest_preg_index == scoreboard_k2[i].dest_preg_index) && (ROB[index].entry_num == scoreboard_k2[i].entry_num)) {
					//once an instruction has completed, set the corresponding ROB entry as ready
					ROB[index].ready = 1;
					if (scoreboard_k2[i].dest_preg_index != -1)
						PRF[scoreboard_k2[i].dest_preg_index].ready = 1;
				}
			}
		}
	}

	for (i = 0; i < num_RS_entries; i++) {
		if (RS[i].fired == 1 && RS[i].valid == 1) {
			RS[i].valid = 0;
		}
	}

}

/**
* Subroutine to simulate state update stage
*/
void state_update() {
	while (ROB[rob_head].ready == 1 && rob_size != 0) {
		//retire ready instructions from the ROB head
		if ((ROB[rob_head].prev_preg_index != -1) && (ROB[rob_head].prev_preg_index >= 32)) {
			PRF[ROB[rob_head].prev_preg_index].free = 1;
		}

		rob_head = (rob_head + 1) % num_rob_entries;
		rob_size--;
		instructions_retired++;

	}
}

/**
* Subroutine for cleaning up any outstanding instructions and calculating overall statistics
* such as average IPC, average fire rate etc.
* XXX: You're responsible for completing this routine
*
* @p_stats Pointer to the statistics structure
*/
void complete_proc(proc_stats_t *p_stats)
{
	//free all arrays
	free(ROB);
	free(RS);
	free(PRF);
	free(RAT);
	free(ARF);
	free(scoreboard_k0);
	free(scoreboard_k1);
	free(scoreboard_k2);
	p_stats->cycle_count = clock_cycle;
	p_stats->retired_instruction = instructions_retired;
	p_stats->avg_inst_fired = (float)fired_instructions / clock_cycle;
	p_stats->avg_inst_retired = (float)instructions_retired / clock_cycle;
}
