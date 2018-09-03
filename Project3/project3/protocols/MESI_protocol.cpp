#include "MESI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MESI_protocol::MESI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
}

MESI_protocol::~MESI_protocol ()
{    
}

void MESI_protocol::dump (void)
{
    const char *block_states[5] = {"X","I","S","E","M"};
    fprintf (stderr, "MESI_protocol - state: %s\n", block_states[state]);
}

void MESI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {

    default:
        fatal_error ("Invalid Cache State for MESI Protocol\n");
    }
}

void MESI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {


    default:
    	fatal_error ("Invalid Cache State for MESI Protocol\n");
    }
}

inline void MESI_protocol::do_cache_I (Mreq *request)
{

}

inline void MESI_protocol::do_cache_S (Mreq *request)
{

}

inline void MESI_protocol::do_cache_E (Mreq *request)
{

}

inline void MESI_protocol::do_cache_M (Mreq *request)
{

}

inline void MESI_protocol::do_snoop_I (Mreq *request)
{

}

inline void MESI_protocol::do_snoop_S (Mreq *request)
{

}

inline void MESI_protocol::do_snoop_E (Mreq *request)
{

}

inline void MESI_protocol::do_snoop_M (Mreq *request)
{

}

