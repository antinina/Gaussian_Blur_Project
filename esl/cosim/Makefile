#u simulator konzolu upisati ovo da ne bi pravio probleme oko opsega
#set rangecnst_severity_level  IGNORE

CC = "g++"
PROJECT = cosim
SRC = sc_main.cpp ip_core.cpp SW.cpp router.cpp utils.cpp apply_gaussian_top.vhd image_processing_block.vhd img_bram.vhd kernel_bram.vhd data_path.vhd control_path.vhd

$(PROJECT) : $(SRC)
	xmsc_run -sc_main -DSC_INCLUDE_DYNAMIC_PROCESSES -DSC_INCLUDE_FX -gui -access +r $(SRC) 
clean:
	rm *.log *.key *.so -r *.d