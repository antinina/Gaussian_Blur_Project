#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <cmath>


using namespace std;


#define NUMBER_OF_POS 69


extern "C"{

static int height = 720;
static int width = 1100;

void applyGaussian(vector<vector<uint8_t>> &blurred, vector<vector<double>> &kern_mat, vector<vector<uint8_t>> &gray_img, int half_size){
	double sumPixel = 0.0;
	int rowIndex = 0;
	int colIndex = 0;
	

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			sumPixel = 0.0;

			// konvolucija matrice kernela (filtra) i matrice slike
			for (int k = -half_size; k <= half_size; k++) {
				for (int l = -half_size; l <= half_size; l++) {
					rowIndex = min(max(i + k, 0), height - 1);
					colIndex = min(max(j + l, 0), width - 1);
	
					sumPixel += kern_mat[k + half_size][l + half_size] * gray_img[rowIndex][colIndex];

				}
			}

			blurred[i][j] = static_cast<uint8_t>(sumPixel);

		}
	}
	

}

//********** GAUSSIAN BLUR **********
void gaussianBlur(vector<vector<uint8_t>>& image, int kernel_size, double sigma) {
	vector<vector<uint8_t>> blurredImage(height, vector<uint8_t>(width));

	if (sigma == 0) { sigma = 0.3 * ((kernel_size - 1) * 0.5 - 1) + 0.8; }

	// stvara Gaussian kernel koristeci zadate parametre i Gausovu funkciju
	vector<vector<double>> kernel(kernel_size, vector<double>(kernel_size));
	double sum = 0.0;
	int halfSize = kernel_size / 2;
	for (int i = -halfSize; i <= halfSize; i++) {
		for (int j = -halfSize; j <= halfSize; j++) {
			double exponent = -(i * i + j * j) / (2.0 * sigma * sigma);
			double value = (1.0 / (2.0 * 3.14159 * sigma * sigma)) * exp(exponent);			
			kernel[i + halfSize][j + halfSize] = value;
			sum += value;
		}
	}


	// normalizacija kernela
	for (int i = 0; i < kernel_size; i++) {
		for (int j = 0; j < kernel_size; j++) {
			kernel[i][j] /= sum;
		}
	}

	applyGaussian(blurredImage, kernel, image, halfSize);

	image = blurredImage;
}




//********** ADAPTIVE THRESHOLD **********
vector<vector<uint8_t>> adaptiveThreshold(vector<vector<uint8_t>> src, int w, int h, int maxValue, int blockSize, int delta) {
	vector<vector<uint8_t>> dst(h, vector<uint8_t>(w));
	vector<vector<uint8_t>> mean(h, vector<uint8_t>(w));


	for (int k = 0; k < src.size(); k++) {
		for (int l = 0; l < src[0].size(); l++) {
			mean[k][l] = src[k][l];
		}
	}

	gaussianBlur(mean, blockSize, 0);

	unsigned int* tab;
	tab = new unsigned int[768];

	for (int i = 0; i < 768; i++)
		tab[i] = (i - 255 <= -delta ? maxValue : 0);


	for (int k = 0; k < src.size(); k++) {
		for (int l = 0; l < src[0].size(); l++) {
			dst[k][l] = tab[src[k][l] - mean[k][l] + 255];
		}
	}


	return dst;
}




//********** DILATE IMAGE **********
vector<vector<uint8_t>> dilateImage(vector<vector<uint8_t>> src, int kernel_size) {
	vector<vector<uint8_t>> dst(src.size(), vector<uint8_t>(src[0].size()));
	vector<vector<uint8_t>> src_border(src.size()+2, vector<uint8_t>(src[0].size()+2));


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
vector<vector<uint8_t>> cropImage(vector<vector<uint8_t>> src, int parking_w, int parking_h, int x, int y) {
	vector<vector<uint8_t>> dst(parking_h, vector<uint8_t>(parking_w));
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
int countNonZero(vector<vector<uint8_t>> src) {
	int non0_counter = 0;

	for (int i = 0; i < src.size(); i++) {
		for (int j = 0; j < src[0].size(); j++) {
			if (src[i][j] != 0) { non0_counter++; }
		}
	}

	return non0_counter;
}




//********** FIND FREE PARKING SPACES **********
 void checkParkingSpace(vector<vector<uint8_t>> src,  int limit, int resultArray[NUMBER_OF_POS]) {
	int pixelCounter = 0;
	int parking_width = 90;
	int parking_height = 40;


	vector<vector<uint8_t>> imgCrop(src.size(), vector<uint8_t>(src[0].size()));

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




int* func(uint8_t *image)
{


	vector<vector<uint8_t>> imgGray(height, vector<uint8_t>(width));

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			imgGray[i][j] = image[i*width+j];	
		}
	}
	
			
	gaussianBlur(imgGray, 3, 1);
	

	vector<vector<uint8_t>> imgThreshold(height, vector<uint8_t>(width));


	imgThreshold = adaptiveThreshold(imgGray, width, height, 255, 25, 16);



	vector<vector<uint8_t>> imgDilate(height, vector<uint8_t>(width));


	imgDilate = dilateImage(imgThreshold, 3);


	int *resArray = new int[NUMBER_OF_POS];

	checkParkingSpace(imgDilate, 800, resArray);

 return resArray;

}



}
