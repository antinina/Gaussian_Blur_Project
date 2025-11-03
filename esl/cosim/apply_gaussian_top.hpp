#ifndef _APPLY_GAUSSIAN_TOP_HPP_
#define _APPLY_GAUSSIAN_TOP_HPP_

#include "systemc.h"

class apply_gaussian_top : public sc_core::sc_foreign_module
{
public:
	apply_gaussian_top(sc_core::sc_module_name name) :
		sc_core::sc_foreign_module(name),
	clk("clk"),
    //config regs
	reset("reset"),
	start("start"),
    kernel_size_i("kernel_size_i"),
	//status reg
    ready_o("ready_o"),
   
    //kernel init ctrl
    kernel_en_i("kernel_en_i"),
    kernel_we_i("kernel_we_i"),
    kernel_waddr_i("kernel_waddr_i"),
    kernel_wdata_i("kernel_wdata_i"),
    
	//AXI Stream (DMA) write if
    axi_wdata_o("axi_wdata_o"),
    axi_wvalid_o("axi_wvalid_o"),    
    axi_wready_i("axi_wready_i"),
    axi_wlast_o("axi_wlast_o"),  
    
	//AXI Stream (DMA) read if
    axi_rdata_i("axi_rdata_i"),
    axi_rvalid_i("axi_rvalid_i"),    
    axi_rready_o("axi_rready_o"),
    axi_rlast_i("axi_rlast_i")
  

	{
	}			
	
	sc_core::sc_in< bool > clk;
	sc_core::sc_in< sc_dt::sc_logic > reset;
	sc_core::sc_in< sc_dt::sc_logic > start;
	sc_core::sc_in< sc_dt::sc_lv<5> > kernel_size_i;
	sc_core::sc_out< sc_dt::sc_logic > ready_o;

	
	sc_core::sc_in< sc_dt::sc_logic > kernel_en_i;
	sc_core::sc_in< sc_dt::sc_lv<4> > kernel_we_i;
	sc_core::sc_in< sc_dt::sc_lv<10> > kernel_waddr_i;
	sc_core::sc_in< sc_dt::sc_lv<16> > kernel_wdata_i;
	
	sc_core::sc_out< sc_dt::sc_lv<8> > axi_wdata_o;
	sc_core::sc_out< sc_dt::sc_logic >axi_wvalid_o;
	sc_core::sc_in< sc_dt::sc_logic >axi_wready_i;
	sc_core::sc_out< sc_dt::sc_logic >axi_wlast_o;
	
	sc_core::sc_in< sc_dt::sc_lv<8> > axi_rdata_i;
	sc_core::sc_in< sc_dt::sc_logic > axi_rvalid_i;
	sc_core::sc_out< sc_dt::sc_logic > axi_rready_o;
	sc_core::sc_in< sc_dt::sc_logic > axi_rlast_i;
	
	const char* hdl_name() const { return "apply_gaussian_top"; } 
};

#endif
