#ifndef _SW_HPP_
#define _SW_HPP_

#include "utils.hpp"
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

#define NUMBER_OF_POS 69

class SW : public sc_core::sc_module
{
	public:
		SW(sc_core::sc_module_name);
		
		//**** Interfaces ****//
		tlm_utils::simple_initiator_socket<SW> router_socket;
  	        tlm_utils::simple_initiator_socket<SW> img_bram_socket;		
		tlm_utils::simple_target_socket<SW> ipb_socket;	

		
	protected:
		int ready;
		int start;
		
		pl_t pl; 
		sc_core::sc_time offset;
        	
        	void Cpu();
        	
        	std::vector<int> dram_data;
        	
        	vector<vector<uint16_t>> dilateImage(vector<vector<uint16_t>> &src, int kernel_size);
        	vector<vector<uint16_t>> cropImage(vector<vector<uint16_t>> &src, int parking_w, int parking_h, int x, int y);
        	int countNonZero(vector<vector<uint16_t>> &src);
        	void checkParkingSpace(vector<vector<uint16_t>> &src,  int limit, int resultArray[NUMBER_OF_POS]);
        	
		void write_kernel_bram(int addr, num_t val);
		int read_router(sc_dt :: uint64 addr);
		void write_router(sc_dt :: uint64 addr, int val);
		void write_img_bram(int, uint16_t val);			

		void b_transport(pl_t&, sc_core::sc_time&);		

};



#endif
