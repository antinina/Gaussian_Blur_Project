#ifndef UTILS_HPP_INCLUDED
#define UTILS_HPP_INCLUDED

#define SC_INCLUDE_FX
#include <iostream>
#include <systemc>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <deque>
#include <stdio.h>
#include <cmath>
#include <string.h>
#include <math.h>
#include <iomanip>
#include <tlm> 
#include <bitset>
#include <cstdint>

using namespace std;
using namespace sc_core;
using namespace sc_dt;

#define Q sc_dt::SC_RND
#define O sc_dt::SC_SAT


//regs
#define IPB_START_SAHE 0
#define IPB_KERNEL_SAHE 1
#define IPB_READY_SAHE 2

#define DELAY 10


//addresses
#define ADDR_IP_H 0x0200001c
#define ADDR_IP_L 0x02000000
#define ADDR_BASE_DRAM 0x04000000
#define ADDR_HIGH_DRAM 0x04FFFFFF
#define ADDR_BASE_KERNEL_BRAM 0x03000000
#define ADDR_HIGH_KERNEL_BRAM 0x03FFFFFF
#define ADDR_BASE_IMG_BRAM 0x05000000
#define ADDR_HIGH_IMG_BRAM 0x05FFFFFF

//typedefs
typedef sc_dt::sc_ufix_fast num_t;
typedef tlm::tlm_base_protocol_types::tlm_payload_type pl_t;
	

//img
static int height = 720;
static int width = 1100;
static sc_uint<11> height_hw = 720;
static sc_uint<11> width_hw = 1100;
static int number_of_pos = 69;

static const int DATA_WIDTH = 16;
static const int FIXED_POINT_WIDTH = 16;
static const int CHAR_LEN = 8;
static const int CHARS_AMOUNT = DATA_WIDTH / CHAR_LEN;

//functions
int uchar_to_int(unsigned char *buf);
void int_to_uchar(unsigned char *buf, int d);

void numt_to_uchar(unsigned char *c, num_t d);
void to_char (unsigned char *buf, string s);
num_t to_fixed (unsigned char *buf);

#endif
