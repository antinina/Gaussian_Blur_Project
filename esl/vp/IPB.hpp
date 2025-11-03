#ifndef IPB_HPP_
#define IPB_HPP_

#include "utils.hpp"
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>




typedef std::deque<num_t> array_t;
typedef std::deque<array_t> matrix_t;	
typedef std::vector<std::vector<double>> orig_array_t;

class IPB : public sc_core::sc_channel
{

	public:
	
		IPB(sc_core::sc_module_name);  
		
		
  		tlm_utils::simple_target_socket<IPB> router_socket;  	
  		tlm_utils::simple_initiator_socket<IPB> kernel_bram_socket;	
  		tlm_utils::simple_initiator_socket<IPB> img_bram_socket;
  		tlm_utils::simple_initiator_socket<IPB> dram_socket;
		
	
	protected:
		pl_t pl; 
		sc_core::sc_time offset;
        	
		sc_uint<5> kernel_size;   // SAHE 0
//		int sigma;         // SAHE 1
		sc_uint<1> start = 0;	// SAHE 2
		sc_uint<1> ready;	// SAHE 3
		
		sc_uint<8> pixel = 0;

		  
		sc_uint<11> req_cnt = 0;
		sc_uint<11> dram_pos = 0;
		sc_uint<16> cache_size = 0;
		sc_uint<16> bram_addr = 0;
		
		void b_transport(pl_t&, sc_core::sc_time&);
		void Process(sc_core::sc_time &);
		
		sc_uint<8> conv2int (num_t dec, int W, int I);
		
		void write_dram(sc_uint<64> addr, sc_uint<8> val);
		num_t read_kernel_bram(sc_dt::uint64 addr);
		sc_uint<8> read_img_bram(sc_uint<64> addr);
		sc_uint<8> read_dram(sc_uint<64> addr);
		void write_img_bram(sc_uint<64> addr, sc_uint<8> val);

};


#endif
