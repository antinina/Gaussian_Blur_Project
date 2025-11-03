#include "IPB.hpp"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

IPB::IPB(sc_module_name name) : sc_channel(name)
{

	ready = 1;

	router_socket.register_b_transport(this, &IPB::b_transport);
	
	cout << "IPB ->  Image processing block is constructed." << endl;

}

void IPB::Process(sc_core::sc_time &offset)
{

	if(start == 1 && ready == 1)
	{
		ready = 0;
		offset += sc_core :: sc_time(DELAY, SC_NS);
	
	
	}
	else if(start == 1 && ready == 0)
	{
		offset += sc_core :: sc_time(DELAY, SC_NS);
	}
	else if(start == 0 && ready == 0)
	{
		offset += sc_core :: sc_time(DELAY, SC_NS);
		
		// primena Gaussian blura na slici
		num_t sumPixel(16, 8, Q, O);
        	num_t matrix_kernel(16, 0, Q, O);
		sc_uint<8> image = 0;

        	sc_uint<11> rowIndex = 0, colIndex = 0;
        	sc_int<5> halfSize = kernel_size/2;
        

        	cache_size = kernel_size*height_hw;
		req_cnt = 0;
		bram_addr = 0;
		sc_uint<5> req_cnt_mod = 0;   //represents req_cnt % kernel_size
		sc_uint<16> cache_size_mod = 0;     // represents ((colIndex - req_cnt + req_cnt_mod) * height_hw + rowIndex) % cache_size

						

		for (sc_int<12> j = 0; j < width_hw; j++) {                       
			offset += sc_core :: sc_time(7*DELAY, SC_NS);                                                                        
			for (sc_int<12> i = 0; i < height_hw; i++){
				sumPixel = 0.0;
				
				for (sc_int<5> l = -halfSize; l <= halfSize; l++) { 
					for (sc_int<5> k = -halfSize; k <= halfSize; k++) {
						if(i + k > 0){
							if(i + k < height_hw - 1){
								rowIndex = i + k;
							}
							else{
								rowIndex = height_hw - 1;
							}						
						}
						else{
							rowIndex = 0;
						}						
						if(j + l > 0){
							if(j + l < width_hw - 1){
								colIndex = j + l;
							}
							else{
								colIndex = width_hw - 1;
							}							
						}
						else{
							colIndex = 0;
						}	
   				             				          	
   				          				      
   				          	matrix_kernel = read_kernel_bram((l + halfSize)*kernel_size + k + halfSize);	      			      
   				          	cache_size_mod =  (colIndex - req_cnt + req_cnt_mod) * height_hw + rowIndex;
   							          					

   				          	
   				          	if(cache_size_mod >= cache_size){								
   				          		image = read_img_bram(cache_size_mod -cache_size);					
   				          	}
   				          	else{
   				          		image = read_img_bram(cache_size_mod);
   				          	} 
   				          													
																		   				          														          													
						sumPixel += matrix_kernel * image;
						
						if(kernel_size == 3){ offset += sc_core :: sc_time(2*DELAY, SC_NS); }				
						offset += sc_core :: sc_time(DELAY, SC_NS);					
																			
					}
				}
				   				          				        
   				if(i==719 && j < (width_hw - halfSize - 1) && j >= halfSize){
   					offset += sc_core :: sc_time(3*7*DELAY, SC_NS);
   					offset += sc_core :: sc_time(3*10*DELAY, SC_NS);   				
					dram_pos = kernel_size + req_cnt;
					bram_addr = req_cnt_mod*height_hw;
					for(sc_uint<11> n = 0; n < height_hw; n++){
						offset += sc_core :: sc_time(DELAY, SC_NS); 
						pixel = read_dram(dram_pos * height_hw + n);
						write_img_bram(bram_addr, pixel);
						bram_addr++;		
					}
					req_cnt++; 
					req_cnt_mod++;
					if(req_cnt_mod == kernel_size){ req_cnt_mod = 0; }
				}
				
					
					
				write_dram((j*height_hw + i), conv2int(sumPixel,16,8));                                                           				
   				          	
			}
		}
	
		ready = 1;
	}


}



void IPB::b_transport(pl_t &pl, sc_core::sc_time &offset)
{
    	uint64 addr = pl.get_address();
    	uint64 taddr = addr& 0x00ffffff;
    	tlm :: tlm_command cmd = pl.get_command();
    	unsigned int length = pl.get_data_length();
    	unsigned char *data = pl.get_data_ptr();   	
    
       	switch(cmd){
       		case tlm::TLM_WRITE_COMMAND:
       			switch(addr)
			{
				case IPB_KERNEL_SAHE:
					kernel_size = uchar_to_int(data);
					break;
			
				case IPB_START_SAHE:
					start = uchar_to_int(data);
					cout << "Start bit written!" << endl;
					Process(offset);	
					break;	
			
				default:
					cout << "IPB -> This SAHE does not exist!" << endl;
					break;	
			}	       
       			break;
       
       		case tlm::TLM_READ_COMMAND:
	       		switch(addr)
	       		{
	       			case IPB_READY_SAHE:
		       			int_to_uchar(data, ready);
		       			cout << "Ready bit read!" << endl;
	       				break;	
	       				
	       			default:
	       				cout << "IPB -> This SAHE does not exist!" << endl;
	       				break;
	       		}
       			break;
       
       		default:
       			cout<<"IPB -> COMMAND not valid!!!"<<endl;
       		break;
       
       	}

	offset+=sc_time(DELAY, SC_NS); 

}

sc_uint<8> IPB::conv2int(num_t dec, int W, int I)
{
	uint64_t pom = (dec*100);
	pom = pom/100;
	return (sc_uint<8>)pom;
}

sc_uint<8> IPB::read_dram(sc_uint<64> addr)
{
	pl_t pl;
	unsigned char buf[1];
  	pl.set_address(addr | ADDR_BASE_DRAM);
  	pl.set_data_length(1);
  	pl.set_data_ptr(buf); 
  	pl.set_command( tlm::TLM_READ_COMMAND );
  	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
  	dram_socket->b_transport(pl,offset);
  
  	return (sc_uint<8>)buf[0];
}

void IPB::write_img_bram(sc_uint<64> addr, sc_uint<8> val)
{
  	pl_t pl;
  	unsigned char buf[1];
  	buf[0] = (unsigned char)val; 
  	
  	pl.set_address((addr) | ADDR_BASE_IMG_BRAM);
  	pl.set_data_length(1);
  	pl.set_data_ptr(buf);
  	pl.set_command( tlm::TLM_WRITE_COMMAND );
  	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
  	img_bram_socket->b_transport(pl, offset);
}


void IPB::write_dram(sc_uint<64> addr, sc_uint<8> val)
{
	pl_t pl;
	unsigned char buf[1];
	buf[0] = (unsigned char)val;
	pl.set_address(addr | ADDR_BASE_DRAM);
	pl.set_data_length(1);
	pl.set_data_ptr(buf);
	pl.set_command( tlm::TLM_WRITE_COMMAND );
 	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
	dram_socket->b_transport(pl,offset);
}


num_t IPB::read_kernel_bram(sc_dt::uint64 addr)
{
  	pl_t pl;
  	unsigned char buf[2];
  	pl.set_address((addr*2) | ADDR_BASE_KERNEL_BRAM);
  	pl.set_data_length(2);
  	pl.set_data_ptr(buf);
  	pl.set_command( tlm::TLM_READ_COMMAND );
  	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
  	kernel_bram_socket->b_transport(pl,offset);
  	
  	return to_fixed(buf); 
}

sc_uint<8> IPB::read_img_bram(sc_uint<64> addr)
{

	pl_t pl;
	unsigned char buf[1];
	pl.set_address(addr | ADDR_BASE_IMG_BRAM);
	pl.set_data_length(1);
	pl.set_data_ptr(buf);
	pl.set_command( tlm::TLM_READ_COMMAND );
	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
	sc_core::sc_time offset = sc_core::SC_ZERO_TIME;
	img_bram_socket->b_transport(pl,offset);

	return (sc_uint<8>)buf[0];

}
