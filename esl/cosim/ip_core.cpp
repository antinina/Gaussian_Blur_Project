#include "ip_core.hpp"
#include <tlm>

using namespace sc_core;
using namespace sc_dt;
using namespace std;
using namespace tlm;

SC_HAS_PROCESS(Ip_Core);

Ip_Core::Ip_Core(sc_module_name name):
	sc_module(name),
	core("core"),
	clk("clk", DELAY, sc_core::SC_NS)
{
	router_socket.register_b_transport(this, &Ip_Core::b_transport); //onaj odakle dolazi
	dram2bram_socket.register_b_transport(this, &Ip_Core::b1_transport); // kada inicijalizuje bram

	core.clk( clk.signal() );
	core.reset( reset );

	core.start( start);
    core.kernel_size_i( kernel_size_i);
    core.ready_o(ready_o);
   
    core.kernel_en_i( kernel_en_i);
    core.kernel_we_i( kernel_we_i);
    core.kernel_waddr_i( kernel_waddr_i);
    core.kernel_wdata_i( kernel_wdata_i);
    
    core.axi_wdata_o( axi_wdata_o);
    core.axi_wvalid_o( axi_wvalid_o);    
    core.axi_wready_i( axi_wready_i);
    core.axi_wlast_o( axi_wlast_o);  
    
    core.axi_rdata_i( axi_rdata_i);
    core.axi_rvalid_i( axi_rvalid_i);    
    core.axi_rready_o( axi_rready_o);
    core.axi_rlast_i( axi_rlast_i);
	
	//ovo su inicijalizacije
	reset.write(static_cast<sc_logic> (0));
	start.write(static_cast<sc_logic> (0));
	kernel_size_i.write(static_cast< sc_lv<5> > (0));
	
	kernel_en_i.write(static_cast<sc_logic> (0));
	kernel_we_i.write(static_cast<sc_lv<4>> (0));
	kernel_waddr_i.write(static_cast< sc_lv<10> > (0));
	kernel_wdata_i.write(static_cast< sc_lv<16> > (0));
	
	axi_wdata_o.write(static_cast< sc_lv<8> > (0));
	axi_wvalid_o.write(static_cast<sc_logic> (0));
	axi_wready_i.write(static_cast<sc_logic> (0));
	axi_wlast_o.write(static_cast<sc_logic> (0));
	
	axi_rdata_i.write(static_cast< sc_lv<8> > (0));
	axi_rvalid_i.write(static_cast<sc_logic> (0));
	axi_rready_o.write(static_cast<sc_logic> (0));
	axi_rlast_i.write(static_cast<sc_logic> (0));
		
	SC_REPORT_INFO("Ip_Core", "Constructed.");

}


//-----------------------------------------------------------------

void Ip_Core::b_transport(pl_t &pl, sc_core::sc_time &offset)
{
	
	
	tlm::tlm_command cmd = pl.get_command();
 	sc_dt::uint64 addr = pl.get_address();
	sc_dt::uint64 taddr = addr& 0xfffffff;
	unsigned int len = pl.get_data_length();
 	unsigned char *buf = pl.get_data_ptr();
 	pl.set_response_status( tlm::TLM_OK_RESPONSE );
 	
	switch(cmd)
 	{	
	 	case tlm::TLM_WRITE_COMMAND: //softver upisuje u ipb
			
			//cout<<"usao u konfig registara"<<endl;
			if (addr ==  ADDR_IP_L + IPB_START_SAHE)
			{
				start.write(static_cast<sc_logic> (uchar_to_int(buf)));
				int start = uchar_to_int(buf);
				if(start == 1){
				cout << "Start bit written!" << endl;
				//started = 1;
				//cout<<"is started"<<endl;
				//axi_wready_i.write(static_cast<sc_logic> (1));
				//axi_rvalid_i.write(static_cast<sc_logic> (1));
				Process(offset);	
				}
				//printf("write %d in start\n", uchar_to_int(buf));
				
			}

			else if (addr ==  ADDR_IP_L + IPB_RESET_SAHE)//init radi
			{
				reset.write(static_cast<sc_logic> (uchar_to_int(buf)));
				printf("write %d in reset\n", uchar_to_int(buf));
				wait(DELAY, SC_NS);
			}

			else if (addr ==  ADDR_IP_L + IPB_KERNEL_SAHE)//init radi
			{
				kernel_size_i.write(static_cast <sc_lv<5>> (uchar_to_int(buf)));//static_cast
				//moja interna promenljiva treba mi za uslov
				kernel_size = uchar_to_int(buf);								

				printf("write %d in kernel\n", uchar_to_int(buf));
				wait(DELAY, SC_NS);
			}
		//***********************************		
			else if (addr >= ADDR_BASE_KERNEL_BRAM and addr <= ADDR_HIGH_KERNEL_BRAM)
			{
				//cout<<"Usao da upise u kernel addresa koja stigne "<<addr<<"podatak"<<to_fixed(buf)<<endl;
				//cout<<"kad se pretvoti u int"<<uchar_to_int(buf)<<endl;
				//cout<<"***"<<buf[1]<<"***"<<buf[0]<<endl;
	
				if(addr == ADDR_BASE_KERNEL_BRAM)
				{//upis u kernel_bram
				kernel_en_i.write(static_cast<sc_logic> (1));
				kernel_we_i.write(static_cast<sc_lv<4>> (1));	
				wait(clk.posedge_event());
				}
				//ovde bio uslov za zadnjeg				
				

				taddr = (addr & 0x000003FF);
				//taddr = (addr & 0x000003FF)>>1; // 10 bit address

				//cout<<"taddr je"<<taddr<<endl;
				kernel_waddr_i.write(static_cast< sc_lv<10> > (taddr));
				//cout<<(kernel_uchar_to_int(buf))<<endl;
				
				kernel_wdata_i.write(static_cast< sc_lv<16> > (kernel_uchar_to_int(buf)));
			
				if(taddr == (kernel_size*kernel_size - 1) )
				{	
					wait(clk.posedge_event()); 
					kernel_en_i.write(static_cast<sc_logic> (0));
					kernel_we_i.write(static_cast<sc_lv<4>> (0));
					cout<< "Gotov upis u kernel bram!"<<endl;
				}
				wait(clk.posedge_event());
				wait(DELAY, SC_NS);
				
			}
			
			else
			{
				pl.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
				cout << "Wrong write address!" << endl;
			}
			break;
	/*****************************************************************************************/		
		case tlm::TLM_READ_COMMAND:
			if (addr == ADDR_IP_L + IPB_READY_SAHE)
			{
				wait(DELAY, SC_NS);

				
				if (ready_o.read() == '1')
				{
					int_to_uchar(buf, 1);
					
				}				
				else
				{	
					int_to_uchar(buf, 0);
					
				}			


			}
		
			else
			{
				pl.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
				cout << "Wrong read address!" << endl;
			}
			break;
		default:
			pl.set_response_status( tlm::TLM_COMMAND_ERROR_RESPONSE );
			cout << "Wrong command!" << endl;
	}
	

}

void Ip_Core::b1_transport(pl_t &pl, sc_core::sc_time &offset)
{
	//cout <<" package recieved" << endl;
	
	tlm::tlm_command cmd = pl.get_command();
 	sc_dt::uint64 addr = pl.get_address();
	sc_dt::uint64 taddr = addr& 0x00fffff;
	unsigned int len = pl.get_data_length();
 	unsigned char *buf = pl.get_data_ptr();
 	pl.set_response_status( tlm::TLM_OK_RESPONSE );
 	
	switch(cmd)
 	{	
	 	case tlm::TLM_WRITE_COMMAND: //softver upisuje u ipb
		
		
		
		//upis u img bram
		 if (addr >= ADDR_BASE_IMG_BRAM and addr <= ADDR_HIGH_IMG_BRAM)
		{		if(addr ==ADDR_BASE_IMG_BRAM){
				axi_rvalid_i.write(static_cast<sc_logic> (1));
				wait(clk.posedge_event());				
				}
				taddr = addr & 0x0000FFFF; //16 bit address
			//	cout<<"Usao sam u upis u bram!"<<endl;
			//	cout<<"adresa"<<taddr<<" podatak "<<(unsigned int)( buf[0])<<endl;
																

				if(axi_rready_o.read() == '0')
					wait(axi_rready_o.posedge_event());
				
				axi_rdata_i.write(static_cast< sc_lv<8> > ((unsigned int)buf[0]) );
			//	cout<<"!!!data "<<((unsigned int)buf[0])<<endl;
//				wait(clk.posedge_event());
				
				
				if(taddr == (kernel_size*720-1))//2159 za 3x3
				{	
					axi_rlast_i.write(static_cast<sc_logic> (1));
					wait(clk.posedge_event());
					
					axi_rlast_i.write(static_cast<sc_logic> (0));
					axi_rvalid_i.write(static_cast<sc_logic> (0));
					cout<< "zavrsen upis u img_bram "<<taddr<<endl;
				}
				wait(clk.posedge_event());
				
				
			}
				
			

			else
			{
				pl.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
				cout << "Wrong write address!" << endl;
			}
			break;
	/*****************************************************************************************/		
		case tlm::TLM_READ_COMMAND:
			cout<<"Not implemented"<<endl;
			
			
			break;
		default:
			pl.set_response_status( tlm::TLM_COMMAND_ERROR_RESPONSE );
			cout << "Wrong command!" << endl;
	}
	
	//offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
}
/*-------------------------------------------------------------------------------------------------*/
void Ip_Core::Process(sc_core::sc_time &offset)//
{	
	cout<<"Pocetak obrade slike ..."<<endl;
	axi_wready_i.write(static_cast<sc_logic> (1));
	axi_rvalid_i.write(static_cast<sc_logic> (1));
	//int i = 0;
	sc_lv<8> tmp_wdata = 0;
	unsigned char tmp_rdata = 0;
	index_counter = 0;

	do
	{
		if(axi_wvalid_o.read() == '1'){  

	 		tmp_wdata = axi_wdata_o.read(); 		    
			write_dram((char)(tmp_wdata.to_int()));
		}

		else if(axi_rready_o.read() == '1')
		{
		
			tmp_rdata = read_dram();	
			index_counter ++;
			axi_rdata_i.write(static_cast< sc_lv<8> > (tmp_rdata) );	

			if(index_counter == (width*height - (kernel_size * height))){ 
			index_counter = 0;
			axi_rlast_i.write(static_cast<sc_logic> (1));
			wait(clk.posedge_event());
			}
		
		axi_rlast_i.write(static_cast<sc_logic> (0));
		}
		
		wait(clk.posedge_event());

	}while(axi_wlast_o.read() == 0);
	
	axi_wready_i.write(static_cast<sc_logic> (0));
	axi_rvalid_i.write(static_cast<sc_logic> (0));
	cout<<"GOTOVO!"<<endl;
}
//--------------------------------------------------

void Ip_Core::write_dram(sc_uint<8> val)
{
	pl_t pl;
	unsigned char buf[1];
	buf[0] = (unsigned char)val; //ok
	pl.set_address(ADDR_BASE_DRAM);
	pl.set_data_length(1);
	pl.set_data_ptr(buf);
	pl.set_command( tlm::TLM_WRITE_COMMAND );
 	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
	dram_socket->b_transport(pl,offset);


}

//--------------------------

char Ip_Core::read_dram()
{
	int i = 0;
	//if(kernel_size == 25){cout<<"cita dram "<<i<<endl; i++;}
	pl_t pl;
	unsigned char buf[1];
	pl.set_address(ADDR_BASE_DRAM);
	pl.set_data_length(1);
	pl.set_data_ptr(buf);
	pl.set_command( tlm::TLM_READ_COMMAND );
	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
	sc_core::sc_time offset = sc_core::SC_ZERO_TIME;
	dram_socket->b_transport(pl,offset);

	
	return buf[0];

}





