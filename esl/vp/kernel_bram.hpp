#ifndef KERNEL_BRAM_HPP_INCLUDED
#define KERNEL_BRAM_HPP_INCLUDED

#include "utils.hpp"

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

class Kernel_BRAM: public sc_core::sc_channel
{
	public:
		Kernel_BRAM(sc_core::sc_module_name);
		
        	tlm_utils::simple_target_socket<Kernel_BRAM> router_socket;
        	tlm_utils::simple_target_socket<Kernel_BRAM> ipb_socket;
        	
        protected:
        
        	std::vector<num_t> kernel_bram_data;
        	
        	void b_transport(pl_t&, sc_core::sc_time&);			

};

#endif  // IMG_BRAM_HPP_INCLUDED
