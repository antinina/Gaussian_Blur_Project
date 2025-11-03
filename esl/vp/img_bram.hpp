#ifndef IMG_BRAM_HPP_INCLUDED
#define IMG_BRAM_HPP_INCLUDED

#include "utils.hpp"

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

class Img_BRAM: public sc_core::sc_channel
{
	public:
		Img_BRAM(sc_core::sc_module_name);
		
        	tlm_utils::simple_target_socket<Img_BRAM> dram_socket;
        	tlm_utils::simple_target_socket<Img_BRAM> ipb_socket;
        	tlm_utils::simple_target_socket<Img_BRAM> router_socket;        	
        	
        protected:
        
        	std::vector<sc_uint<8>> img_bram_data;
        	
        	void b_transport(pl_t&, sc_core::sc_time&);			

};

#endif // IMG_BRAM_HPP_INCLUDED
