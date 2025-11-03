//5.10.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <string.h>
#include <math.h>
#include "stdint.h"
#include "signal.h"


#define HEIGHT 720
#define WIDTH 1100
#define MAX_PKT_SIZE 720*1100*2 
#define MAX_KERNEL_SIZE 720*25
#define START_REG 0
#define RESET_REG 8
#define KERNEL_REG 4
#define READY_REG 12


#include "img_concat.h"


//HW functions
void write_reg (const int reg, const int value);
void write_kernel_bram(int pos, unsigned int kern_coeff);
int read_ready_reg();


int gotsignal = 0;
void sighandler(int signo)
{
	if(signo == SIGIO){
		gotsignal = 1;
		
	}	
}


int main(int argc, char *argv[])
{	
	
	FILE *dma_f;
	struct sigaction action;
	
	uint32_t rx_img[792000/2], final_img[792000/2];

	int i = 0, j = 0;
	int kernel_size = 3 ; double sigma = 1;

	while(i<HEIGHT*WIDTH/2)
	{
		rx_img[i] = 0;final_img[i] = 0;
		i++;
	}
	
	//matrix of Gaussian kernel coeffs
	double kernel[25][25]; 
	double sum = 0.0;
	double exponent = 0.0;
	unsigned int kern_coeff = 0;
	int halfSize = kernel_size / 2;
	for (int i = -halfSize; i <= halfSize; i++) {
		for (int j = -halfSize; j <= halfSize; j++) {
			exponent = -(i * i + j * j) / (2.0 * sigma * sigma);
			kernel[i + halfSize][j + halfSize] = (1.0 / (2.0 * 3.14159 * sigma * sigma)) * exp(exponent);			
			sum += kernel[i + halfSize][j + halfSize];	
		}
	}

	//normalization
	int pos = 0;
	for (int i = 0; i < kernel_size; i++) {
		for (int j = 0; j < kernel_size; j++) {
			kernel[i][j] /= sum;
			kern_coeff =(int) (round(kernel[i][j] * pow(2,16)));
			
			write_kernel_bram(pos*4,kern_coeff);
			pos++;
		}
	}


	write_reg(RESET_REG, 1);
	usleep(2000); 
	write_reg(RESET_REG, 0); 
	write_reg(KERNEL_REG, kernel_size);
	
	int *p; int *q;
	int packet_len = HEIGHT*kernel_size;		

		int fd = open("/dev/axi_dma", O_RDWR | O_NONBLOCK);
		
		memset(&action, 0, sizeof(action));
		action.sa_handler = sighandler;
		sigaction(SIGIO, &action, NULL);
	
		fcntl(fd, F_SETOWN, getpid());
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC);			
		printf("Sending pic... \n");
		
		//for tx
		p=(int*)mmap(0,1584000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
		int fd_2 = open("/dev/axi_dma", O_RDWR | O_NONBLOCK);
		//for rx
		q = (int*)mmap(0, 1584000, PROT_READ | PROT_WRITE, MAP_SHARED, fd_2, 0);
		
		//memcpy(p, img_concat_after3, HEIGHT*kernel_size*2);
		memcpy(p, img_concat, HEIGHT*kernel_size*2);
		//sending packet length
		dma_f = fopen("/dev/axi_dma", "w");
		fprintf(dma_f, "%d", 720*kernel_size*2);
		fclose(dma_f);
		
		usleep(500);

	while(!gotsignal){};
	
	usleep(10000); // pause before new transaction
	//memcpy(p, img_concat_after3 + kernel_size*HEIGHT/2, (1100-kernel_size)*720*2);
	memcpy(p, img_concat + kernel_size*HEIGHT/2, (1100-kernel_size)*720*2);
	usleep(1);
	dma_f = fopen("/dev/axi_dma", "w");
	fprintf(dma_f, "%d", HEIGHT*WIDTH*2);
	fclose(dma_f);
	usleep(1);

	gotsignal = 0;	
	write_reg(START_REG, 1); 
	
	while(read_ready_reg());
	write_reg(START_REG, 0); 

	printf("Working...\n");
	while(!read_ready_reg());//vrti se dok je ready na 0
	printf("Img with kernel 3x3 done\n");
	sleep(2);
	memcpy(rx_img, q, 720*1100*2); //Img 3x3 done!
	

	/*---New Transac---*/
	kernel_size = 25 ;
	sigma = 0.3 * ((kernel_size - 1)*0.5 -1) + 0.8; 

	 sum = 0.0;
	 exponent = 0.0;
	 kern_coeff = 0;
	 halfSize = kernel_size / 2;
	//kernel coeefs for matrix 25x25
	for (int i = -halfSize; i <= halfSize; i++) {
		for (int j = -halfSize; j <= halfSize; j++) {
			exponent = -(i * i + j * j) / (2.0 * sigma * sigma);
			kernel[i + halfSize][j + halfSize] = (1.0 / (2.0 * 3.14159 * sigma * sigma)) * exp(exponent);			
			sum += kernel[i + halfSize][j + halfSize];	
		}
	}


	//normalization
	pos = 0;
	for (int i = 0; i < kernel_size; i++) {
		for (int j = 0; j < kernel_size; j++) {
			kernel[i][j] /= sum;
			kern_coeff =(int) (round(kernel[i][j] * pow(2,16)));
			
			write_kernel_bram(pos*4,kern_coeff);
			pos++;
		}
	}


	write_reg(RESET_REG, 1);
	usleep(2000); //20us pause
	write_reg(RESET_REG, 0); 
	write_reg(KERNEL_REG, kernel_size);
	
	 packet_len = HEIGHT*kernel_size;
	

		fd = open("/dev/axi_dma", O_RDWR | O_NONBLOCK);
		
		printf("Starting new transaction \n");

		memcpy(p, rx_img, HEIGHT*kernel_size*2);

		dma_f = fopen("/dev/axi_dma", "w");
		fprintf(dma_f, "%d", 720*kernel_size*2);
		fclose(dma_f);
		


		usleep(500);//probaj bez ovoga ili da ga skratis

	
	while(!gotsignal){};

	usleep(10000);
	memcpy(p, rx_img + kernel_size*HEIGHT/2, (1100-kernel_size)*720*2);
	usleep(1);
	dma_f = fopen("/dev/axi_dma", "w");
	fprintf(dma_f, "%d", HEIGHT*WIDTH*2);
	fclose(dma_f);
	usleep(1);

	gotsignal = 0;	
	write_reg(START_REG, 1); 
	
	while(read_ready_reg());
	
	write_reg(START_REG, 0); 

	printf("Working...\n");
	while(!read_ready_reg());//vrti se dok je ready na 0

	sleep(2);
	
	memcpy(final_img, q, 720*1100*2); //ovde dobije prvi reyultat	



	int pom = 0;
	int pix = 0;
	FILE *fptr;
	fptr = fopen("img_final.txt", "w");
	for(pom = 0; pom < HEIGHT*WIDTH/2; pom++){
		fprintf(fptr, "%d\n%d\n", (final_img[pom] >> 8), final_img[pom] & 255);	
		pix+=2;
	}
	printf("Num of pixels: %d\n", pix);
	fclose(fptr);

		gotsignal = 0;
		munmap(p, MAX_PKT_SIZE);
		munmap(q, MAX_PKT_SIZE);
		close(fd);
		close(fd_2);
	printf("Done!\n");
return 0;
}

void write_reg (const int reg, const int value)
{
	FILE *apply_gaussian;
	apply_gaussian = fopen ("/dev/apply_gaussian", "w");
	//writting into file
	fprintf (apply_gaussian, "%d %d\n", reg, value);	
//	printf ("app: writting to reg %d, value %d \n", reg, value);
	
	fclose (apply_gaussian);
}

int read_ready_reg()
{
	FILE *apply_gaussian;
	int val;
	
	apply_gaussian = fopen("/dev/apply_gaussian", "r");
	fscanf(apply_gaussian, "%d\n", &val);
	fclose(apply_gaussian);
	
	return val;
}

//void write_kernel_bram(int bram_a, unsigned char * val, int length)
void write_kernel_bram(int pos, unsigned int  kern_coeff)
{
	FILE *bram_file;
	
	bram_file = fopen("/dev/kernel_bram", "w");
	//fflush: to clear internal buffer associated with a file stream
	//used in order to immediately write buffered data to file ensuring data is saved promptly

	fflush(bram_file);
	
		fprintf(bram_file, "%d %d\n",pos,kern_coeff);// data and address
		fflush(bram_file);

	
	fclose(bram_file);
}

