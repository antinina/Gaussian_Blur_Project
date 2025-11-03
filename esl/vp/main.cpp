#include "SW.hpp"
#include "router.hpp"
#include "utils.hpp"
#include "IPB.hpp"
#include "kernel_bram.hpp"
#include "img_bram.hpp"

using namespace sc_core;

int sc_main(int argc, char* argv[])
{
	SW sw("Software");
	router route("Router");
	IPB ipb("IPB");
	Kernel_BRAM kernel_bram("Kernel_BRAM");
	Img_BRAM img_bram("Image_BRAM");
	
	sw.router_socket.bind(route.sw_socket);
	route.ipb_socket.bind(ipb.router_socket);
	route.kernel_bram_socket.bind(kernel_bram.router_socket);
	route.img_bram_socket.bind(img_bram.router_socket);	
	ipb.dram_socket.bind(sw.ipb_socket);
	ipb.kernel_bram_socket.bind(kernel_bram.ipb_socket);
	ipb.img_bram_socket.bind(img_bram.ipb_socket);
	sw.img_bram_socket.bind(img_bram.dram_socket);
	
	
	sc_start(500, SC_NS);
	
	
	return 0;

}
