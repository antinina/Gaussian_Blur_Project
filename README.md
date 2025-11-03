#Gaussian Blur Project - Parking Space Detection System
 _Final-year project done in a three-member team._
 
 - The system detects free parking spaces from an image using various image processing algorithms.
 - The Gaussian Blur algorithm is specifically targeted for hardware acceleration within a custom Image Processing hardware block.

## Project Development Flow

 - 1 System-Level Design (**esl**) - Initial algorithm in Python and C++, bit analysis, SystemC model, performance estimation, HW/SW co-simulation
 - 2 RTL System Design – Design of an IP core with some optimizations
 - 3 Functional Verification - UVM testbench that verifies the correct functionality of the design
 - 4 Linux Driver and User Application(**eos**) - Implemented software drivers and an application to interact with the hardware design
 
 The driver and application development phase (Part 4) was the primary contribution to my Bachelor’s thesis. 
 
 Documentation of parts 1 and 4 are given in **documentation** directory 

