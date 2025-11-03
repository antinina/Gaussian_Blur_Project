#include "SW.hpp"
#include "utils.hpp"


using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace sc_dt;

SC_HAS_PROCESS(SW);

SW::SW(sc_module_name name) : sc_module(name), offset(sc_core::SC_ZERO_TIME)
{
	ipb_socket.register_b_transport(this, &SW::b_transport);
	dram_data.reserve(height*width);

    	ifstream f;
    	f.open("gray_12_free_spaces.txt");
    	    	

    	if(f.is_open()){
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    f >> dram_data[j*height + i];
                }
            }
    	}
        f.close();
        	        	

    	cout << "SW -> Software is constructed" << endl;
    	SC_THREAD(Cpu);
}

void SW::Cpu()
{

	offset = sc_core::SC_ZERO_TIME;	
	sc_core::sc_time offset_init = sc_core::SC_ZERO_TIME;
	sc_core::sc_time offset_ipb = sc_core::SC_ZERO_TIME;
	sc_core::sc_time offset_full = sc_core::SC_ZERO_TIME;

	
			
	vector<vector<uint16_t>> imgGray(height, vector<uint16_t>(width));


            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                	imgGray[i][j] = dram_data[j*height + i];
                }
            }


        
        int sigma = 1; int kernel_size = 3;
        
        offset += sc_core :: sc_time(10*DELAY, SC_NS);
        for(int i = 0; i < kernel_size*height; i++){
        	write_img_bram(i, dram_data[i]);
        }
        
        if (sigma == 0) { sigma = 0.3 * ((kernel_size - 1) * 0.5 - 1) + 0.8; }

	// stvara Gaussian kernel koristeci zadate parametre i Gausovu funkciju
	vector<vector<double>> kernel(kernel_size, vector<double>(kernel_size));
	num_t matrix_kernel(16, 0, Q, O); 
	double sum = 0.0;
	double exponent = 0.0;
	int halfSize = kernel_size / 2;
	for (int i = -halfSize; i <= halfSize; i++) {
		for (int j = -halfSize; j <= halfSize; j++) {
			exponent = -(i * i + j * j) / (2.0 * sigma * sigma);
			kernel[i + halfSize][j + halfSize] = (1.0 / (2.0 * 3.14159 * sigma * sigma)) * exp(exponent);			
			sum += kernel[i + halfSize][j + halfSize];	
		}
	}



	offset += sc_core :: sc_time(10*DELAY, SC_NS);
	for (int i = 0; i < kernel_size; i++) {
		for (int j = 0; j < kernel_size; j++) {
			kernel[i][j] /= sum;
			matrix_kernel = kernel[i][j];
			write_kernel_bram(i*kernel_size + j, matrix_kernel);
		}
	}

	offset_init = offset;
	

 	cout << "Sending command for Gaussian blur 3x3." << endl;
	write_router(IPB_KERNEL_SAHE, kernel_size);


	write_router(IPB_START_SAHE, 1);

	int ready = 1;

		
	while(ready){	
	
		ready = read_router(IPB_READY_SAHE);
	
	}

	write_router(IPB_START_SAHE | ADDR_IP_L, 0);

	while(!ready){	
	
		ready = read_router(IPB_READY_SAHE);
	
	}
	
	offset_ipb = offset;
	offset_full = offset;
	cout << endl;
	cout << "	Simulation time - one photo with a " << kernel_size << " x " << kernel_size << " kernel: " << endl;
	cout << "	Initialization time: " << offset_init.to_string() << "." << endl;	
	cout << "	Processing time: " << (offset_ipb - offset_init).to_string() << "." << endl;
  	cout << "	Total simulation time: " << offset_ipb.to_string() << "." << endl << endl;

    	fstream f_o;
    	f_o.open("result3x3.txt", fstream::out);
    	for (int i = 0; i < height; i++) {
        	for (int j = 0; j < width; j++) {
        		
			f_o << dram_data[j*height + i];
                	f_o << " ";
        	}
        	f_o << endl;
    	}

   	f_o.close();	
	

	vector<vector<uint16_t>> imgThreshold(height, vector<uint16_t>(width));
	vector<vector<uint16_t>> mean(height, vector<uint16_t>(width)); 	
	
	for (int k = 0; k < height; k++) {
		for (int l = 0; l < width; l++) {
			mean[k][l] = dram_data[l*height + k];
		}
	} 
	
	
	sigma = 0; kernel_size = 25;
	
	offset += sc_core :: sc_time(10*DELAY, SC_NS);
	for(int i = 0; i < kernel_size*height; i++){
        	write_img_bram(i, dram_data[i]);
        }
	
        if (sigma == 0) { sigma = 0.3 * ((kernel_size - 1) * 0.5 - 1) + 0.8; }
            	

	// stvara Gaussian kernel koristeci zadate parametre i Gausovu funkciju
	vector<vector<double>> kernell(kernel_size, vector<double>(kernel_size));
	sum = 0.0;
	exponent = 0.0;
	halfSize = kernel_size / 2;
	kernel.clear();
	for (int i = -halfSize; i <= halfSize; i++) {
		for (int j = -halfSize; j <= halfSize; j++) {
			exponent = -(i * i + j * j) / (2.0 * sigma * sigma);
			kernell[i + halfSize][j + halfSize] = (1.0 / (2.0 * 3.14159 * sigma * sigma)) * exp(exponent);			
			sum += kernell[i + halfSize][j + halfSize];	
		}
	}
	

	offset += sc_core :: sc_time(10*DELAY, SC_NS);
	for (int i = 0; i < kernel_size; i++) {
		for (int j = 0; j < kernel_size; j++) {
			kernell[i][j] /= sum;
			matrix_kernel = kernell[i][j];
			write_kernel_bram(i*kernel_size + j, matrix_kernel);
		}
	}

	
	offset_init = offset;	
	    
    	cout << "Sending command for Gaussian blur 25x25." << endl;
	write_router(IPB_KERNEL_SAHE, kernel_size);

	
	write_router(IPB_START_SAHE, 1);
	
	while(ready){	

		ready = read_router(IPB_READY_SAHE);

	}

	write_router(IPB_START_SAHE | ADDR_IP_L, 0);	

	while(!ready){	

		ready = read_router(IPB_READY_SAHE);

	}

	offset_ipb = offset;
	cout << endl;
	cout << "	Simulation time - one photo with a " << kernel_size << " x " << kernel_size << " kernel: " << endl;
	cout << "	Initialization time: " << (offset_init - offset_full).to_string() << "." << endl;	
	cout << "	Processing time: " << (offset_ipb - offset_init - offset_full).to_string() << "." << endl;
  	cout << "	Total simulation time: " << (offset_ipb - offset_full).to_string() << "." << endl << endl;

        for(unsigned int j = 0; j < width; j++){
        	for(unsigned int i = 0; i < height; i++){
        		
        		mean[i][j] = dram_data[j*height + i];
        		
        		
        	}
        }	

	fstream f_out;
    	f_out.open("result25x25.txt", fstream::out);
    	for (int i = 0; i < mean.size(); i++) {
        	for (int j = 0; j < mean[0].size(); j++) {
        	    	f_out << mean[i][j];
            		f_out << " ";
        	}
        	f_out << endl;
    	}

   	f_out.close();	
   	
   	int delta = 16;
   	int maxValue = 255;
	
	unsigned int* tab;
	tab = new unsigned int[768];

	for (int i = 0; i < 768; i++)
		tab[i] = (i - 255 <= -delta ? maxValue : 0);


	for (int k = 0; k < height; k++) {
		for (int l = 0; l < width; l++) {
			imgThreshold[k][l] = tab[imgGray[k][l] - mean[k][l] + 255];
		}
	}
		
	
	vector<vector<uint16_t>> imgDilate(height, vector<uint16_t>(width));
	imgDilate = dilateImage(imgThreshold, 3);
	
	int *resArray = new int[NUMBER_OF_POS];
	checkParkingSpace(imgDilate, 800, resArray);
	int num_of_parking_spots = 0;

	for(int i = 0; i < NUMBER_OF_POS; i++){
		if(resArray[i] == 1){num_of_parking_spots++;}
	}
	cout << "There are " << num_of_parking_spots << " free parking spaces." << endl;
	
	offset_full = offset;
	cout << endl;
  	cout << "	Total simulation time - one photo, whole algorithm: " << offset_full.to_string() << "." << endl << endl;	
	
 }   
 
 


//********** DILATE IMAGE **********
vector<vector<uint16_t>> SW::dilateImage(vector<vector<uint16_t>> &src, int kernel_size){
	vector<vector<uint16_t>> dst(src.size(), vector<uint16_t>(src[0].size()));
	vector<vector<uint16_t>> src_border(src.size()+2, vector<uint16_t>(src[0].size()+2));


	src_border[0][0] = src[0][0];
	src_border[0][src_border[0].size() - 1] = src[0][src[0].size() - 1];
	src_border[src_border.size() - 1][0] = src[src.size() - 1][0];
	src_border[src_border.size() - 1][src_border[0].size() - 1] = src[src.size() - 1][src[0].size() - 1];
	
	for (int i = 0; i < src.size(); i++) {
		src_border[i + 1][0] = src[i][0];
		src_border[i + 1][src_border[0].size() - 1] = src[i][src[0].size() - 1];


		for (int j = 0; j < src[0].size(); j++) {
			src_border[i + 1][j + 1] = src[i][j];
			if (i == 0) {
				src_border[0][j+1] = src[0][j];
			}
			if (i == src.size() - 1) {
				src_border[src_border.size() - 1][j+1] = src[src.size() - 1][j];
			}
		}
	}


	unsigned char present = 0;

	for (int i = 0; i < src_border.size() - 2; i++) {
		for (int j = 0; j < src_border[0].size() - 2; j++) {
			present = 0;
			for (int k = 0; k < kernel_size; k++) {
				for (int l = 0; l < kernel_size; l++) {
					if (src_border[i + k][j + l] == 255) {	
						present++;
					}
				}
			}
			if (present > 0) { dst[i][j] = 255; }
			else { dst[i][j] = 0; }
		}
	}


	return dst;
}


//********** CROP IMAGE **********
vector<vector<uint16_t>> SW::cropImage(vector<vector<uint16_t>> &src, int parking_w, int parking_h, int x, int y){
	vector<vector<uint16_t>> dst(parking_h, vector<uint16_t>(parking_w));
	int x_counter = x;

	for (int i = 0; i < dst.size(); i++) {
		for (int j = 0; j < dst[0].size(); j++) {
			dst[i][j] = src[y][x_counter];
			x_counter++;
		}
		x_counter = x;
		y++;
	}

	return dst;
}



//********** COUNT NON ZERO PIXELS **********
int SW::countNonZero(vector<vector<uint16_t>> &src){
	int non0_counter = 0;

	for (int i = 0; i < src.size(); i++) {
		for (int j = 0; j < src[0].size(); j++) {
			if (src[i][j] != 0) { non0_counter++; }
		}
	}

	return non0_counter;
}

//********** FIND FREE PARKING SPACES **********
void SW::checkParkingSpace(vector<vector<uint16_t>> &src,  int limit, int resultArray[NUMBER_OF_POS]){
	int pixelCounter = 0;
	int parking_width = 90;
	int parking_height = 40;


	vector<vector<uint16_t>> imgCrop(src.size(), vector<uint16_t>(src[0].size()));

	ifstream f_pos;
	f_pos.open("CarParkPosFinal.txt");

	vector<unsigned int> x(NUMBER_OF_POS);
	vector<unsigned int> y(NUMBER_OF_POS);

	int index_x = 0;
	int index_y = 0;

	if (f_pos.is_open()) {
		for (int i = 0; i < 2*NUMBER_OF_POS; i++) {
			if (i % 2 == 0) {
				f_pos >> x[index_x];
				index_x++;
			}
			else {
				f_pos >> y[index_y];
				index_y++;
			}
		}

	}

	f_pos.close();

	for (int i = 0; i < NUMBER_OF_POS; i++) {
		imgCrop = cropImage(src, parking_width, parking_height, x[i], y[i]);

		
		pixelCounter = countNonZero(imgCrop);

		if (pixelCounter < limit) { 
			resultArray[i] = 1;
		}
		else {
			resultArray[i] = 0;
		}

	}


}


void SW::b_transport(pl_t &pl, sc_core::sc_time &offset)
{
	uint64 addr = pl.get_address();
	uint64 taddr = addr & 0x00ffffff;
	tlm::tlm_command cmd = pl.get_command();
	unsigned int length = pl.get_data_length();
        unsigned char *buf = pl.get_data_ptr();


	switch(cmd){
        	case tlm::TLM_WRITE_COMMAND:
        	{

        			if(addr >= ADDR_BASE_DRAM && addr <= ADDR_HIGH_DRAM)
        			{

 					for(unsigned int index = 0; index < length; index++)
					{
					
						dram_data[taddr] = (int)buf[index];
				
					}       									
       				}
       				else 
       				{
               				cout << "DRAM -> Error while trying to write data" << endl;
	
       				}

       		break;
       		}	
       				 
		case tlm::TLM_READ_COMMAND:
		{
			for(int index = 0; index < length; index++)
			{
				buf[index] = (unsigned char)dram_data[taddr];
				
			}
		break;
		}
		
       		default:
       		{
       			cout<<"DRAM -> COMMAND not valid!!!"<<endl;       		
       		break;
      		}
      		
       }


}

//read and write functions
void SW::write_kernel_bram(int addr, num_t val)
{
	offset += sc_core :: sc_time(DELAY, SC_NS);
  	pl_t pl;
  	unsigned char buf[2];
  	numt_to_uchar(buf,val);
  	pl.set_address((addr*2) | ADDR_BASE_KERNEL_BRAM);
  	pl.set_data_length(2);
  	pl.set_data_ptr(buf);
  	pl.set_command( tlm::TLM_WRITE_COMMAND );
  	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
  	router_socket->b_transport(pl, offset);
}

void SW::write_img_bram(int addr, uint16_t val)
{
	offset += sc_core :: sc_time(DELAY, SC_NS);
  	pl_t pl;
  	unsigned char buf[1];
  	buf[0] = (unsigned char)val; 
  	
  	pl.set_address((addr) | ADDR_BASE_IMG_BRAM);
  	pl.set_data_length(1);
  	pl.set_data_ptr(buf);
  	pl.set_command( tlm::TLM_WRITE_COMMAND );
  	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
  	router_socket->b_transport(pl, offset);
}

 	
int SW::read_router(sc_dt :: uint64 addr)
{
  	pl_t pl;
  	unsigned char buf[4];
  	pl.set_address(addr | ADDR_IP_L);
  	pl.set_data_length(4);
  	pl.set_data_ptr(buf); 
  	pl.set_command( tlm::TLM_READ_COMMAND );
  	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
  	router_socket->b_transport(pl,offset);
  
  	return uchar_to_int(buf);
}

void SW::write_router(sc_dt :: uint64 addr, int val)
{
  	pl_t pl;
  	unsigned char buf[4];
  	int_to_uchar(buf, val);

  	pl.set_address(addr | ADDR_IP_L);
  	pl.set_data_length(4);
  	pl.set_data_ptr(buf); 
  	pl.set_command( tlm::TLM_WRITE_COMMAND );
  	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
  	router_socket->b_transport(pl,offset);
}



