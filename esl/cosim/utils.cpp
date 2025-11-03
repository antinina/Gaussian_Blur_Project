#include "utils.hpp"



int uchar_to_int(unsigned char *buf)
{
  int sum = 0;
  sum += ((int)buf[0]) << 24;
  sum += ((int)buf[1]) << 16;
  sum += ((int)buf[2]) << 8;
  sum += ((int)buf[3]);
  return sum;
}

unsigned int kernel_uchar_to_int(unsigned char *buf)
{
unsigned  int sum = 0;
  sum += ((unsigned int)buf[0]) << 8;
  sum += ((unsigned int)buf[1]) ;

  return sum;
}


void int_to_uchar(unsigned char *buf, int d)
{
  buf[0] = (char) (d >> 24);
  buf[1] = (char) (d >> 16);
  buf[2] = (char) (d >> 8);
  buf[3] = (char) (d);
}



void to_char(unsigned char *buf, string s)
{
  s.erase(0,2); // remove "0b"
  if(FIXED_POINT_WIDTH == DATA_WIDTH){
  	s.erase(0, 2);  
  }
  else{
  	s.erase(DATA_WIDTH-FIXED_POINT_WIDTH, 1); // remove the dot
  }
  
  char single_char[CHAR_LEN];
  for (int i = 0; i<CHARS_AMOUNT; ++i)
    {
      s.copy(single_char,CHAR_LEN,i*CHAR_LEN); // copy 8 letters (0s and 1s) to char array
      int char_int = stoi(single_char, nullptr, 2); // binary string -> int
      buf[i] = (unsigned char) char_int;
      
    }
	
}

void numt_to_uchar(unsigned char *c, num_t d)
{
  to_char(c,d.to_bin());
}

num_t to_fixed (unsigned char *buf)
{
  string concated = "";
  for (int i = 0; i<CHARS_AMOUNT; ++i) // concatenate char array into eg. "10101101000"
    concated += bitset<CHAR_LEN>((int)buf[i]).to_string();
  
  double multiplier = 1;

  double sum = 0;
  for (int i = 0; i<DATA_WIDTH; ++i)
    {
      sum += (concated[i]-'0') * pow(2.0, DATA_WIDTH - FIXED_POINT_WIDTH - i - 1);
    }
 
  num_t result(16, 0, Q, O);
  result = sum*multiplier;
  return result;
}
  
