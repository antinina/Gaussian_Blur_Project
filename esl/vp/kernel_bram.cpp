#include "kernel_bram.hpp"

using namespace std;
using namespace sc_core;
using namespace sc_dt;


Kernel_BRAM::Kernel_BRAM(sc_module_name name) : sc_channel(name)
{
	router_socket.register_b_transport(this, &Kernel_BRAM::b_transport);
	ipb_socket.register_b_transport(this, &Kernel_BRAM::b_transport);
	
	kernel_bram_data.reserve(2000);

    	cout << "Kernel BRAM -> is constructed!" << endl;

}


void Kernel_BRAM::b_transport(pl_t &pl, sc_core::sc_time &offset)
{
	uint64 addr = pl.get_address();
	uint64 taddr = addr & 0x00ffffff;
	tlm::tlm_command cmd = pl.get_command();
	unsigned int length = pl.get_data_length();
        unsigned char *buf = pl.get_data_ptr();


	switch(cmd){
        	case tlm::TLM_WRITE_COMMAND:
        	{
 			for(unsigned int index = 0; index < length; index++)
			{
					
				kernel_bram_data[taddr] = buf[index];
				taddr++;
				
			}       				
       		break;
       		} 
		case tlm::TLM_READ_COMMAND:
		{
			for(int index = 0; index < length; index++)
			{
				buf[index] = kernel_bram_data[taddr];
				taddr++;
				
			}
		break;
		}
       		default:
       		{
       			cout<<"Kernel_BRAM -> COMMAND not valid!!!"<<endl;       		
       		break;
      		}
      		
       }     

}
