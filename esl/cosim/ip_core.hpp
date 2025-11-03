#ifndef _IP_HARD_H_
#define _IP_HARD_H_

#ifndef SC_INCLUDE_FX
#define SC_INCLUDE_FX
#endif

#include <systemc>
//#include <sysc/datatypes/fx/sc_fixed.h>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>

#include "utils.hpp"
#include "apply_gaussian_top.hpp"

using namespace std;

class Ip_Core :
	public sc_core::sc_module
{
public:
	Ip_Core(sc_core::sc_module_name);

	sc_event run;
	tlm_utils::simple_target_socket<Ip_Core> router_socket;
	tlm_utils::simple_initiator_socket<Ip_Core> dram_socket;//ok
	tlm_utils::simple_target_socket<Ip_Core> dram2bram_socket;

protected:
	pl_t pl;
	int index_counter = 0;
	sc_core::sc_time offset;
	sc_uint<5> kernel_size = 3;
	
	void Process(sc_core::sc_time &offset);

	void write_dram( sc_uint<8> val);
	char read_dram();
	apply_gaussian_top core;
	sc_core::sc_clock clk;
	
	sc_core::sc_signal< sc_dt::sc_logic > reset;
	sc_core::sc_signal< sc_dt::sc_logic > start;
	sc_core::sc_signal< sc_dt::sc_lv<5> > kernel_size_i;
	sc_core::sc_signal< sc_dt::sc_logic > ready_o;

	
	sc_core::sc_signal< sc_dt::sc_logic > kernel_en_i;
	sc_core::sc_signal< sc_dt::sc_lv<4> > kernel_we_i;
	sc_core::sc_signal< sc_dt::sc_lv<10> > kernel_waddr_i;
	sc_core::sc_signal< sc_dt::sc_lv<16> > kernel_wdata_i;
	
	sc_core::sc_signal< sc_dt::sc_lv<8> > axi_wdata_o;
	sc_core::sc_signal< sc_dt::sc_logic >axi_wvalid_o;
	sc_core::sc_signal< sc_dt::sc_logic >axi_wready_i;
	sc_core::sc_signal< sc_dt::sc_logic >axi_wlast_o;
	
	sc_core::sc_signal< sc_dt::sc_lv<8> > axi_rdata_i;
	sc_core::sc_signal< sc_dt::sc_logic > axi_rvalid_i;
	sc_core::sc_signal< sc_dt::sc_logic > axi_rready_o;
	sc_core::sc_signal< sc_dt::sc_logic > axi_rlast_i;
 
	void b_transport(pl_t&, sc_core::sc_time&);
	void b1_transport(pl_t&, sc_core::sc_time&);
};

#ifndef SC_MAIN
SC_MODULE_EXPORT(Ip_Core)
#endif

#endif
