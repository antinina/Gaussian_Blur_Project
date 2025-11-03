#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <cmath>
#include <time.h>
#include <chrono>

using namespace std;

static int height = 720;
static int width = 1100;

#define NUMBER_OF_POS 69
 //********** DILATE IMAGE **********
vector<vector<uint16_t>>dilateImage(vector<vector<uint16_t>> &src, int kernel_size){
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
vector<vector<uint16_t>>cropImage(vector<vector<uint16_t>> &src, int parking_w, int parking_h, int x, int y){
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
int countNonZero(vector<vector<uint16_t>> &src){
	int non0_counter = 0;

	for (int i = 0; i < src.size(); i++) {
		for (int j = 0; j < src[0].size(); j++) {
			if (src[i][j] != 0) { non0_counter++; }
		}
	}

	return non0_counter;
}

//********** FIND FREE PARKING SPACES **********
void checkParkingSpace(vector<vector<uint16_t>> &src,  int limit, int resultArray[NUMBER_OF_POS]){
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


/*--------------------------------------------------------------------------*/
int main()
{


	vector<vector<uint16_t>> imgGray(height, vector<uint16_t>(width));
	vector<vector<uint16_t>> imgGauss(height, vector<uint16_t>(width));


	//original
    	ifstream f_in;
    	f_in.open("gray_12_free_spaces.txt");

    	if(f_in.is_open()) {
            for (int i = 0; i < imgGray.size(); i++) {
                for (int j = 0; j < imgGray[0].size(); j++) {
                    f_in >> imgGray[i][j];
                }
            }
    	}
        f_in.close();
	//new
        ifstream f_out;
    	f_out.open("../app/img_final.txt");
	
    	if(f_out.is_open()) {
            for(int j = 0; j < 1100; j++)
	    	for(int i =0; i < 720; i++)
		{
			 
		 f_out >> imgGauss[i][j];
	
	
		}
    	}
        f_out.close();
	

	vector<vector<uint16_t>> imgThreshold(height, vector<uint16_t>(width));
	vector<vector<uint16_t>> mean(height, vector<uint16_t>(width));
		
        for(unsigned int j = 0; j < width; j++){
        	for(unsigned int i = 0; i < height; i++){
        		
        		mean[i][j] = imgGauss[i][j];
        		
        		
        	}
        }	

//ok
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
	
	int *resArray = new int[69];
	checkParkingSpace(imgDilate, 800, resArray);
	int num_of_parking_spots = 0;

	for(int i = 0; i < 69; i++){
		if(resArray[i] == 1){num_of_parking_spots++;}
	}
	cout<<endl;
	cout<<"**************************************************"<<endl;
	cout << "	There are " << num_of_parking_spots << " free parking spaces." << endl;
	cout<<"--------------------------------------------------"<<endl;
	cout<<endl;
	

	return 0;	
	
 }   
 






