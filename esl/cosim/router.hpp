#ifndef ROUTER_HPP_
#define ROUTER_HPP_
#include <systemc>
#include "utils.hpp"
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>

class router : public sc_core::sc_module
{
	public:
		router(sc_core::sc_module_name);
		
		
       		tlm_utils::simple_initiator_socket<router> ipb_socket;
       		//tlm_utils::simple_initiator_socket<router> kernel_bram_socket;  
       		//tlm_utils::simple_initiator_socket<router> img_bram_socket;         		     		
  		tlm_utils::simple_target_socket<router> sw_socket;

	protected:
		pl_t pl; 
		sc_core::sc_time offset;
        	void b_transport(pl_t&, sc_core::sc_time&);
};

#endif
