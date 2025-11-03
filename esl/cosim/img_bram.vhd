library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity img_bram is
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
end img_bram;

architecture Behavioral of img_bram is
    type ram_type is array(0 to SIZE - 1) of std_logic_vector(DATA_WIDTH - 1 downto 0);
    signal RAM: ram_type;
    
    attribute ram_style: string;
    attribute ram_style of RAM: signal is "block";
    
begin
    
    
    
    
process(clk)
begin
--        if(rising_edge(clk)) then
--            if (en_b = '1') then
--                if (we_b /= "0000") then
--                    RAM(to_integer(unsigned(addr_b))) <= data_i_b;
--                end if;
--            end if;
--        end if;    
        
        if(rising_edge(clk)) then   
          -- if (en = '1' ) then          
             --   data_o <= RAM(to_integer(unsigned(addr)));
              -- if(we /= "0000") then
               --    RAM(to_integer(unsigned(addr))) <= data_i;                  
              -- end if;   
          -- end if;                
		 if (en = '1' ) then --and we = "0000"
			if(to_integer(unsigned(addr)) < 18000)then          
                	data_o <= RAM(to_integer(unsigned(addr)));
			end if;
                if( we /= "0000") then   --els	en = '1' and
			if(to_integer(unsigned(addr)) < 18000)then
                    	RAM(to_integer(unsigned(addr))) <= data_i;  
			end if;	  
		end if;              
                end if;   
            	--end if;    

      end if;    
end process;    
    
end Behavioral;
