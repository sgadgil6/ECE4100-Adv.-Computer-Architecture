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
}

MOSI_protocol::~MOSI_protocol ()
{    
}

void MOSI_protocol::dump (void)
{
    const char *block_states[5] = {"X","I","S","O","M"};
    fprintf (stderr, "MOSI_protocol - state: %s\n", block_states[state]);
}

void MOSI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {

    default:
        fatal_error ("Invalid Cache State for MOSI Protocol\n");
    }
}

void MOSI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {

    default:
    	fatal_error ("Invalid Cache State for MOSI Protocol\n");
    }
}

inline void MOSI_protocol::do_cache_I (Mreq *request)
{

}

inline void MOSI_protocol::do_cache_S (Mreq *request)
{

}

inline void MOSI_protocol::do_cache_O (Mreq *request)
{

}

inline void MOSI_protocol::do_cache_M (Mreq *request)
{

}

inline void MOSI_protocol::do_snoop_I (Mreq *request)
{

}

inline void MOSI_protocol::do_snoop_S (Mreq *request)
{

}

inline void MOSI_protocol::do_snoop_O (Mreq *request)
{

}

inline void MOSI_protocol::do_snoop_M (Mreq *request)
{

}

