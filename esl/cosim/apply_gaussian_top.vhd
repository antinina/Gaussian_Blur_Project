----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 07/20/2024 07:08:37 PM
-- Design Name: 
-- Module Name: apply_gaussian_top - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity apply_gaussian_top is
  Generic(
    IMG_WIDTH: positive := 1100;
    IMG_HEIGHT: positive := 720;
     
    KERNEL_ADDR_WIDTH : positive := 10;
    KERNEL_DATA_WIDTH: positive := 16;
    KERNEL_SIZE: positive := 625;
      
    IMG_BRAM_DATA_WIDTH: positive := 8;
    IMG_BRAM_ADDR_WIDTH: positive := 16; 
    IMG_BRAM_SIZE: positive := 18000; 
     
    DRAM_DATA_WIDTH: positive := 8
  );  
  Port( 
    ---------- Clocking and reset interface----------
    clk: in std_logic;
    reset: in std_logic;  
    
    ---------- AXI Lite Input ------------
    kernel_size_i: in std_logic_vector(4 downto 0);
    
    ---------- AXI Lite - Command interface ---------
    start: in std_logic;
    
    ---------- AXI Lite Status interface ----------
    ready_o: out std_logic;                 
    
    ---------- Kernel BRAM to CPU -------------------
    kernel_en_i: in std_logic;
    kernel_we_i: in std_logic_vector(3 downto 0);
    kernel_waddr_i: in std_logic_vector(KERNEL_ADDR_WIDTH - 1 downto 0);
    kernel_wdata_i: in std_logic_vector(KERNEL_DATA_WIDTH - 1 downto 0);
    


    ---------- AXI Stream ---------------------------
    --- IP to DDR ---    
    axi_wdata_o: out std_logic_vector(DRAM_DATA_WIDTH - 1 downto 0);
    axi_wvalid_o: out std_logic;     
    axi_wready_i: in std_logic;
    axi_wlast_o: out std_logic;    
    --- DDR to IP ---    
    axi_rdata_i: in std_logic_vector(IMG_BRAM_DATA_WIDTH - 1 downto 0);   
    axi_rvalid_i: in std_logic;    
    axi_rready_o: out std_logic;
    axi_rlast_i: in std_logic
       
  );
end apply_gaussian_top;
-------------------------------------------------------------------------------------------------------------------------------*
architecture Behavioral of apply_gaussian_top is


    
    ----- kernel BRAM to IPB -----
    signal kernel_en_s: std_logic;
    signal kernel_raddr_s: std_logic_vector(KERNEL_ADDR_WIDTH - 1 downto 0);
    signal kernel_rdata_s: std_logic_vector(KERNEL_DATA_WIDTH - 1 downto 0);
    
    
    ----- IMG BRAM to IPB -----
    signal img_bram_en_s: std_logic;
    signal img_bram_we_s: std_logic_vector(3 downto 0);
    signal img_bram_raddr_s: std_logic_vector(IMG_BRAM_ADDR_WIDTH - 1 downto 0);
    signal img_bram_rdata_s: std_logic_vector(IMG_BRAM_DATA_WIDTH - 1 downto 0);


--KRECU KOMPONENTE----------------------


component image_processing_block --is
  Generic(
    IMG_WIDTH: positive := 1100;
    IMG_HEIGHT: positive := 720;
     
    KERNEL_ADDR_WIDTH : positive := 10;
    KERNEL_DATA_WIDTH: positive := 16;
     
    IMG_BRAM_DATA_WIDTH: positive := 8;
    IMG_BRAM_ADDR_WIDTH: positive := 16; 
     
    DRAM_DATA_WIDTH: positive := 8
  );
  Port ( 
  ------ Clocking and reset interface ------
    clk: in std_logic;
    reset: in std_logic;
    
  ------ Control --------------------------- 
    start: in std_logic;
  
  ------ AXI Lite input --------------------  
    kernel_size_i: in std_logic_vector(4 downto 0);

  ------ Img BRAM interface ----------------    
    img_bram_en_o: out std_logic;
    img_bram_we_o: out std_logic_vector(3 downto 0);
    img_bram_raddr_o: out std_logic_vector(IMG_BRAM_ADDR_WIDTH - 1 downto 0);
    img_bram_rdata_i: in std_logic_vector(IMG_BRAM_DATA_WIDTH - 1 downto 0); 
    
  ------ Kernel BRAM interface -------------  
    kernel_bram_en_o: out std_logic;
    kernel_bram_raddr_o: out std_logic_vector(KERNEL_ADDR_WIDTH - 1 downto 0);
    kernel_bram_rdata_i: in std_logic_vector(KERNEL_DATA_WIDTH - 1 downto 0);
    
    
  ------ Status ----------------------------  
    ready: out std_logic;    
    
  ------ AXI Stream ------------------------
     --- IPB to DDR ---    
    axi_wdata_o: out std_logic_vector(DRAM_DATA_WIDTH - 1 downto 0);
    axi_wvalid_o: out std_logic;     
    axi_wready_i: in std_logic;
    axi_wlast_o: out std_logic;    
     --- DDR to IPB ---    
    axi_rvalid_i: in std_logic;    
    axi_rready_o: out std_logic;
    axi_rlast_i: in std_logic
  );
end component;

------------------
component control_path --is
  Generic(
    IMG_HEIGHT: positive := 720
  );  
  Port (
  ---------- Clocking and reset interface----------
    clk: in std_logic;
    reset: in std_logic;  
    
  ---------- Command interface ----------
    start: in std_logic;  
  
  ---------- Status interface -----------
    ready: out std_logic;
    
  ---------- Img BRAM interface ---------
    img_bram_en_o: out std_logic;  
    img_bram_we_o: out std_logic_vector(3 downto 0);
    
  ---------- Kernel BRAM interface ------
    kernel_bram_en_o: out std_logic;    
    
  ---------- Data path interface --------
    l_loop_end_i: in std_logic; 
    l_loop_end_pix_i: in std_logic;
    i_loop_end_i: std_logic;
    j_loop_end_i: in std_logic;
    kern_size_delay_i: in std_logic;
    sumPix_rst_o: out std_logic_vector(1 downto 0);
    loop_en_o: out std_logic;
    kl_loop_init_o: out std_logic;
    switch_read_mode_o: out std_logic;   
    rready_ctrl_o: out std_logic;

    
    ---- AXI STREAM ----
    ---- write result channel ----
    wvalid_o: out std_logic;
    wready_i: in std_logic;
    wlast_o: out std_logic;
    ---- read, cache add channel ----
    rvalid_i: in std_logic;
    rready_o: out std_logic;    
    rlast_i: in std_logic
  );
end component;
-----------------------


component img_bram --is
    Generic(
        DATA_WIDTH: positive := 8;
        SIZE: positive := 18000;
	    ADDR_WIDTH: positive := 16
	);
    Port(
        clk : in std_logic;
    
        ---- dvopristupni bram ------
        ---- IPB -----  
        en: in std_logic;
        we: in std_logic_vector(3 downto 0);
        addr: in std_logic_vector(ADDR_WIDTH - 1 downto 0);
        data_o: out std_logic_vector(DATA_WIDTH - 1 downto 0);
        data_i: in std_logic_vector(DATA_WIDTH - 1 downto 0)   -- ovo zapravo ide od drama

          
        ---- DRAM ----
 --       en_b: in std_logic;
 --       we_b: in std_logic_vector(3 downto 0);
 --       addr_b: in std_logic_vector(ADDR_WIDTH - 1 downto 0);
 --       data_i_b: in std_logic_vector(DATA_WIDTH - 1 downto 0)
    );
end component;

--
component kernel_bram --is
    generic (
        WIDTH: positive := 16; --  I = 2, W = 16 signed
        SIZE: positive := 625; 
        SIZE_WIDTH: positive := 10 -- vezano za adresu 
    );
    port (
        clk : in std_logic;
        
        --- IPB interface ---
        en: in std_logic;
        addr: in std_logic_vector(SIZE_WIDTH - 1 downto 0);
        data: out std_logic_vector(WIDTH - 1 downto 0);
        
        --- CPU interface ---
        en_b: in std_logic;
        we_b: in std_logic_vector(3 downto 0);
        addr_b: in std_logic_vector(SIZE_WIDTH - 1 downto 0);
        data_b: in std_logic_vector(WIDTH - 1 downto 0)
    );
end component;









--dobroe


              

begin

ipb1: image_processing_block
    Generic map(
        IMG_WIDTH => IMG_WIDTH,
        IMG_HEIGHT => IMG_HEIGHT,
        KERNEL_ADDR_WIDTH => KERNEL_ADDR_WIDTH,
        KERNEL_DATA_WIDTH => KERNEL_DATA_WIDTH,
        IMG_BRAM_ADDR_WIDTH => IMG_BRAM_ADDR_WIDTH,
        IMG_BRAM_DATA_WIDTH => IMG_BRAM_DATA_WIDTH,
        DRAM_DATA_WIDTH => DRAM_DATA_WIDTH
    )
    Port map(
        clk => clk,
        reset => reset,
        start => start,
        kernel_size_i => kernel_size_i,
        ready => ready_o,
        kernel_bram_en_o => kernel_en_s,
        kernel_bram_raddr_o => kernel_raddr_s,
        kernel_bram_rdata_i => kernel_rdata_s,
        img_bram_en_o => img_bram_en_s,
        img_bram_we_o => img_bram_we_s,
        img_bram_raddr_o => img_bram_raddr_s,
        img_bram_rdata_i => img_bram_rdata_s,
        axi_wdata_o => axi_wdata_o,
        axi_rready_o => axi_rready_o,
        axi_rlast_i => axi_rlast_i,
        axi_rvalid_i => axi_rvalid_i,
        axi_wvalid_o => axi_wvalid_o,
        axi_wready_i => axi_wready_i,
        axi_wlast_o => axi_wlast_o            
    );
    
kernel_bram1: kernel_bram
    Generic map(
        WIDTH => KERNEL_DATA_WIDTH,
        SIZE_WIDTH => KERNEL_ADDR_WIDTH,
        SIZE => KERNEL_SIZE
    )
    Port map(
        clk => clk,
        en => kernel_en_s,
        addr => kernel_raddr_s,
        data => kernel_rdata_s,    
        en_b => kernel_en_i,
        we_b => kernel_we_i,
        addr_b => kernel_waddr_i,
        data_b => kernel_wdata_i
    );  
    
     
    
img_bram1: img_bram
    Generic map(
        DATA_WIDTH => IMG_BRAM_DATA_WIDTH,
        ADDR_WIDTH => IMG_BRAM_ADDR_WIDTH,
        SIZE => IMG_BRAM_SIZE
    )
    Port map(
        clk => clk,
        en => img_bram_en_s,
        we => img_bram_we_s,
        addr => img_bram_raddr_s,
        data_o => img_bram_rdata_s,
        data_i => axi_rdata_i
    );       
    
      



end Behavioral;
