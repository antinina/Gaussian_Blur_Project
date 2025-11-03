#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>

using namespace std;

#define NUMBER_OF_POS 69

void applyGaussian(vector<vector<uint16_t>> &blurred, vector<vector<double>> &kern_mat, vector<vector<uint16_t>> &gray_img, int half_size);
void gaussianBlur(vector<vector<uint16_t>>& image, int kernel_size, double sigma);
vector<vector<uint16_t>> adaptiveThreshold(vector<vector<uint16_t>> src, int w, int h, int maxValue, int blockSize, int delta);
vector<vector<uint16_t>> dilateImage(vector<vector<uint16_t>> src, int kernel_size);
vector<vector<uint16_t>> cropImage(vector<vector<uint16_t>> src, int parking_w, int parking_h, int x, int y); 
int countNonZero(vector<vector<uint16_t>> src);
int checkParkingSpace(vector<vector<uint16_t>> src,  int limit, int resultArray[NUMBER_OF_POS]);
