#include "MOSI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOSI_protocol::MOSI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
	// Initialize lines to not have the data yet!
	this->state = MOSI_CACHE_I;
}

MOSI_protocol::~MOSI_protocol ()
{    
}

void MOSI_protocol::dump (void)
{
    const char *block_states[9] = {"X","I","S","O","M", "IM", "IS", "SM", "OM"};
    fprintf (stderr, "MOSI_protocol - state: %s\n", block_states[state]);
}

void MOSI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
	case MOSI_CACHE_M: do_cache_M(request); break;
	case MOSI_CACHE_O: do_cache_O(request); break;
	case MOSI_CACHE_S: do_cache_S(request); break;
	case MOSI_CACHE_I: do_cache_I(request); break;
	case MOSI_CACHE_IM: do_cache_IM(request); break;
	case MOSI_CACHE_IS: do_cache_IS(request); break;
	case MOSI_CACHE_SM: do_cache_SM(request); break;
	case MOSI_CACHE_OM: do_cache_OM(request); break;
    default:
        fatal_error ("Invalid Cache State for MOSI Protocol\n");
    }
}

void MOSI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
	case MOSI_CACHE_M: do_snoop_M(request); break;
	case MOSI_CACHE_O: do_snoop_O(request); break;
	case MOSI_CACHE_S: do_snoop_S(request); break;
	case MOSI_CACHE_I: do_snoop_I(request); break;
	case MOSI_CACHE_IM: do_snoop_IM(request); break;
	case MOSI_CACHE_IS: do_snoop_IS(request); break;
	case MOSI_CACHE_SM: do_snoop_SM(request); break;
	case MOSI_CACHE_OM: do_snoop_OM(request); break;
    default:
    	fatal_error ("Invalid Cache State for MOSI Protocol\n");
    }
}

inline void MOSI_protocol::do_cache_I (Mreq *request)
{
	//invalid state
	switch (request->msg) {
	case LOAD:
		//send GETS on the bus
		send_GETS(request->addr);
		//transition to transient state and wait for data
		state = MOSI_CACHE_IS;
		Sim->cache_misses++;
		break;
	case STORE:
		//send GETM on bus and wait for data
		state = MOSI_CACHE_IM;
		send_GETM(request->addr);
		Sim->cache_misses++;
		break;
	default:	
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_S (Mreq *request)
{
	//shared state
	switch (request->msg) {
	case LOAD:
		//we have the data and read access, so send it to the processor
		send_DATA_to_proc(request->addr);
		break;
	case STORE:
		//no write access send GETM on bus and wait for data
		state = MOSI_CACHE_SM;
		send_GETM(request->addr);
		Sim->cache_misses++;
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: S state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_O (Mreq *request)
{
	//Owned state
	switch (request->msg) {
	case LOAD:
		//we have the data and read access, so send it to the processor
		send_DATA_to_proc(request->addr);
		break;
	case STORE:
		//no write access send GETM on bus, transition to M and wait for data
		state = MOSI_CACHE_OM;
		send_GETM(request->addr);
		Sim->cache_misses++;
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: S state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_M (Mreq *request)
{
	//modified state
	switch (request->msg) {
	case LOAD:
		//we have read access and data, send it to proc
		send_DATA_to_proc(request->addr);
		break;
	case STORE:
		//we have write access and data, send it to proc
		send_DATA_to_proc(request->addr);
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: M state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_IS(Mreq *request) 
{
	/* If the block is in the transient state that means it sent out a GET message
	* and is waiting on DATA.  Therefore the processor should be waiting
	* on a pending request. Therefore we should not be getting any requests from
	* the processor.
	*/
	switch (request->msg) {
	case LOAD:
	case STORE:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Should only have one outstanding request per processor!");
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: IM state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_IM(Mreq *request) {
	do_cache_IS(request);
}

inline void MOSI_protocol::do_cache_SM(Mreq *request) {
	do_cache_IS(request);
}

inline void MOSI_protocol::do_cache_OM(Mreq *request) {
	do_cache_IS(request);
}

inline void MOSI_protocol::do_snoop_I (Mreq *request)
{
	//invald state
	switch (request->msg) {
	case GETS:
		break;
	case GETM:
		break;
	case DATA:
		//don't have the data, cannot downgrade further, do nothing
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_S (Mreq *request)
{
	//shared state
	switch (request->msg) {
	case GETS:
	case GETM:
		//set shared line to show data is being shared on the bus
		set_shared_line();
		//if the request was a GETM, need to downgrade to I since the calling block will go to M
		if (request->msg == GETM)
			state = MOSI_CACHE_I;
		break;
	case DATA:
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_O (Mreq *request)
{
	switch (request->msg) {
	case GETS:
		//on a GETS, stay in O since there can be other blocks in S
	case GETM:
		if (request->msg == GETM)
			//change to I on a GETM since another block will be going to M
		state = MOSI_CACHE_I;
		//we have the data so send the data on the bus on a GET request
		send_DATA_on_bus(request->addr, request->src_mid);
		set_shared_line();
		break;
	case DATA:
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_M (Mreq *request)
{
	switch (request->msg) {
	case GETS:
	case GETM:
		//change to O on a GETS since another block will be going to S
		state = MOSI_CACHE_O;
		if (request->msg == GETM)
			//change to I on a GETM since another block will be going to M 
			state = MOSI_CACHE_I;
		//set shared line to show data is being shared on the bus
		set_shared_line();
		//we have the data so send the data on the bus on a GET request
		send_DATA_on_bus(request->addr, request->src_mid);
		break;
	case DATA:
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_IM(Mreq *request)
{
	//transient state
	switch (request->msg) {
	case GETS:
		break;
	case GETM:
		break;
	case DATA:
		/** IM state meant that the block had sent the GET and was waiting on DATA.
		* Now that Data is received we can send the DATA to the processor and finish
		* the transition to M.
		*/
		send_DATA_to_proc(request->addr);
		state = MOSI_CACHE_M;
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");

	}
}

inline void MOSI_protocol::do_snoop_IS(Mreq *request)
{
	switch (request->msg) {
	case GETS:
		break;
	case GETM:
		break;
	case DATA:
		/** IS state meant that the block had sent the GET and was waiting on DATA.
		* Now that Data is received we can send the DATA to the processor and finish
		* the transition to S
		*/
		send_DATA_to_proc(request->addr);
		state = MOSI_CACHE_S;
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");

	}
}

inline void MOSI_protocol::do_snoop_SM(Mreq *request)
{
	switch (request->msg) {
	case GETS:
		set_shared_line();
		break;
	case GETM:
		set_shared_line();
		break;
	case DATA:
		/** SM state meant that the block had sent the GET and was waiting on DATA.
		* Now that Data is received we can send the DATA to the processor and finish
		* the transition to M.
		*/
		send_DATA_to_proc(request->addr);
		state = MOSI_CACHE_M;
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");

	}
}

inline void MOSI_protocol::do_snoop_OM(Mreq *request)
{
	//transient state
	switch (request->msg) {
	case GETS:
		//on a GETS send data to bus since we have the data
		send_DATA_on_bus(request->addr, request->src_mid);
		set_shared_line();
		break;
	case GETM:
		//on a GETM send data to bus since we have the data
		state = MOSI_CACHE_SM;
		send_DATA_on_bus(request->addr, request->src_mid);
		set_shared_line();
		break;
	case DATA:
		//once we get the data, we can now transition to M
		send_DATA_to_proc(request->addr);
		state = MOSI_CACHE_M;
		break;
	default:
		request->print_msg(my_table->moduleID, "ERROR");
		fatal_error("Client: I state shouldn't see this message\n");

	}
}