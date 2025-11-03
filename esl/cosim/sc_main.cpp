#define SC_MAIN

//#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include "SW.hpp"
#include "router.hpp"
#include "utils.hpp"
#include "ip_core.hpp"

//#include "kernel_bram.hpp"
//#include "img_bram.hpp"

using namespace sc_core;
//using sc_core::sc_spawn;

int sc_main(int argc, char* argv[])
{
	SW sw("Software");
	router route("Router");
	Ip_Core ip_core("core");
	
	
	sw.router_socket.bind(route.sw_socket);//ostaje

	route.ipb_socket.bind(ip_core.router_socket); //ostaje
	
	
	ip_core.dram_socket.bind(sw.ipb_socket);//ostaje

	//ipb.kernel_bram_socket.bind(kernel_bram.ipb_socket);
	//ipb.img_bram_socket.bind(img_bram.ipb_socket);

	sw.img_bram_socket.bind(ip_core.dram2bram_socket);//novo - za inicijalno punjeneje img brama
	
	
	//sc_start(500, SC_NS);
	sc_start();
	
	return 0;

}
