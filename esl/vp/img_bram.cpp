#include "img_bram.hpp"

using namespace std;
using namespace sc_core;
using namespace sc_dt;


Img_BRAM::Img_BRAM(sc_module_name name) : sc_channel(name)
{
	dram_socket.register_b_transport(this, &Img_BRAM::b_transport);
	ipb_socket.register_b_transport(this, &Img_BRAM::b_transport);
	router_socket.register_b_transport(this, &Img_BRAM::b_transport);	
	
	img_bram_data.reserve(25*height);

    	cout << "Image BRAM -> is constructed!" << endl;

}


void Img_BRAM::b_transport(pl_t &pl, sc_core::sc_time &offset)
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
					
				img_bram_data[taddr] = (sc_uint<8>)buf[index];
				taddr++;
				
			}       				
       		break;
       		} 
		case tlm::TLM_READ_COMMAND:
		{
		
			for(int index = 0; index < length; index++)
			{
				buf[index] = (unsigned char)img_bram_data[taddr];
				taddr++;
				
			}
		break;
		}
       		default:
       		{
       			cout<<"Img_BRAM -> COMMAND not valid!!!"<<endl;       		
       		break;
      		}
      		
       }

}
