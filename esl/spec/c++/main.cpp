#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <cmath>
#include <time.h>
#include <chrono>

#include "utils.hpp"

using namespace std;


#define NUMBER_OF_POS 69



static int height = 720;
static int width = 1100;



int main()
{


	vector<vector<uint16_t>> imgGray(height, vector<uint16_t>(width));

    	ifstream f_im;
    	f_im.open("dara.txt");

    	if(f_im.is_open()) {
            for (int i = 0; i < imgGray.size(); i++) {
                for (int j = 0; j < imgGray[0].size(); j++) {
                    f_im >> imgGray[i][j];
                }
            }
    	}
        f_im.close();
        
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();	
			
	//gaussianBlur(imgGray, 3, 1);

	vector<vector<uint16_t>> imgThreshold(height, vector<uint16_t>(width));
	vector<vector<uint16_t>> imgDilate(height, vector<uint16_t>(width));
	int *resArray = new int[NUMBER_OF_POS];
	int free_parking_spaces = 0;
		
	
	imgThreshold = adaptiveThreshold(imgGray, width, height, 255, 25, 16);


	imgDilate = dilateImage(imgThreshold, 3);


	free_parking_spaces = checkParkingSpace(imgDilate, 800, resArray);
	cout << "There are " << free_parking_spaces << " free parking spaces." << endl;
  	
  	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);	
	cout << "The application took " << elapsed.count() << " seconds to run." << endl;

        
        return 0;
}




