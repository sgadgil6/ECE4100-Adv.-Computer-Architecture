#include "MOESIF_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOESIF_protocol::MOESIF_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
}

MOESIF_protocol::~MOESIF_protocol ()
{    
}

void MOESIF_protocol::dump (void)
{
    const char *block_states[9] = {"X","I","S","E","O","M","F"};
    fprintf (stderr, "MOESIF_protocol - state: %s\n", block_states[state]);
}

void MOESIF_protocol::process_cache_request (Mreq *request)
{
	switch (state) {

    default:
        fatal_error ("Invalid Cache State for MOESIF Protocol\n");
    }
}

void MOESIF_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {

    default:
    	fatal_error ("Invalid Cache State for MOESIF Protocol\n");
    }
}

inline void MOESIF_protocol::do_cache_F (Mreq *request)
{

}

inline void MOESIF_protocol::do_cache_I (Mreq *request)
{

}

inline void MOESIF_protocol::do_cache_S (Mreq *request)
{

}

inline void MOESIF_protocol::do_cache_E (Mreq *request)
{

}

inline void MOESIF_protocol::do_cache_O (Mreq *request)
{

}

inline void MOESIF_protocol::do_cache_M (Mreq *request)
{

}

inline void MOESIF_protocol::do_snoop_F (Mreq *request)
{

}

inline void MOESIF_protocol::do_snoop_I (Mreq *request)
{

}

inline void MOESIF_protocol::do_snoop_S (Mreq *request)
{

}

inline void MOESIF_protocol::do_snoop_E (Mreq *request)
{

}

inline void MOESIF_protocol::do_snoop_O (Mreq *request)
{

}

inline void MOESIF_protocol::do_snoop_M (Mreq *request)
{

}



