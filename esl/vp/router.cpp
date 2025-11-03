#include "router.hpp"
#include "utils.hpp"

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace sc_dt;


router::router(sc_module_name name) : sc_module(name)
{
	sw_socket.register_b_transport(this, &router::b_transport);
	cout << "Router ->  Router is constructed!" << endl;
}


void router::b_transport(pl_t &pl, sc_core::sc_time &offset)
{
	uint64 addr = pl.get_address();
    	uint64 taddr = addr& 0x00ffffff;   	
    
        if( addr >= ADDR_IP_L && addr <= ADDR_IP_H)
        {    

		pl.set_address(taddr);
		ipb_socket -> b_transport(pl, offset);
		pl.set_address(addr);
		
		pl.set_response_status(TLM_OK_RESPONSE);
	
	}
	else if(addr >= ADDR_BASE_KERNEL_BRAM && addr <= ADDR_HIGH_KERNEL_BRAM)
	{
	
		pl.set_address(taddr);
		kernel_bram_socket -> b_transport(pl, offset);
		pl.set_address(addr);
		
		pl.set_response_status(TLM_OK_RESPONSE);	
	
	}	
	else if(addr >= ADDR_BASE_IMG_BRAM && addr <= ADDR_HIGH_IMG_BRAM)
	{
	
		pl.set_address(taddr);
		img_bram_socket -> b_transport(pl, offset);
		pl.set_address(addr);
		
		pl.set_response_status(TLM_OK_RESPONSE);		
	
	}
	else{
        
                pl.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
                cout << "Router -> Error while trying to write data" << endl;
	
	}

         offset+=sc_time(2, SC_NS); 

}

