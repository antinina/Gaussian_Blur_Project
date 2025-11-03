// 2/10/24

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>

#include <linux/io.h> //iowrite ioread
#include <linux/slab.h>//kmalloc kfree
#include <linux/platform_device.h>//platform driver
#include <linux/of.h>//of_match_table
#include <linux/ioport.h>//ioremap

#include <linux/dma-mapping.h>  //dma access
#include <linux/mm.h>  //dma access
#include <linux/interrupt.h>  //interrupt handlers

MODULE_AUTHOR ("PowerPuffGirls");
MODULE_DESCRIPTION("Driver for GaussianBlur accelerator.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_ALIAS("custom: apply_gaussian driver");

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

#define DEVICE_NAME "apply_gaussian_IP"
#define DRIVER_NAME "gaussian_driver"


#define BUFF_SIZE 10	
#define MAX_PKT_LEN 720*1100*4/2 //whole image 

//addresses for registers
#define START_REG 0
#define RESET_REG 8
#define KERNEL_REG 4
#define READY_REG 12

#define ADDR_FACTOR 4 


/*---------------------- Function Prototypes --------------------------*/

static int gaussian_fasync(int fd, struct file *file, int mode);

static int gaussian_probe(struct platform_device *pdev);
static int gaussian_remove(struct platform_device *pdev);

static int gaussian_open(struct inode *i, struct file *f);
static int gaussian_close(struct inode *i, struct file *f);

static ssize_t gaussian_dma_mmap(struct file *f, struct vm_area_struct *vma_s);
static ssize_t gaussian_read(struct file *f, char __user *buf, size_t len, loff_t *off);
static ssize_t gaussian_write(struct file *f, const char __user *buf, size_t length, loff_t *off);

static int __init gaussian_init(void);
static void __exit gaussian_exit(void);

static irqreturn_t dma_isr(int irq,void*dev_id); //Interrupt Routine
int dma_init(void __iomem *base_address);
int dma_init_read(void __iomem *base_address);
u32 dma_simple_write(dma_addr_t TxBufferPtr, u32 max_pkt_len, void __iomem *base_address); 
u32 dma_simple_read(dma_addr_t RxBufferPtr, u32 max_pkt_len, void __iomem *base_address);



/*--------------------- Globals & Structures -------------------*/

struct gaussian_info 
{
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
	
	int irq_num; 
};


static struct cdev *my_cdev;
static dev_t my_dev_id;
static struct class *my_class;
struct device *axi_dev;

struct fasync_struct *async_queue;

//One driver to many "devices"
static struct gaussian_info *gaussian_core = NULL; //IPB registers_AXI Lite if
static struct gaussian_info *kernel_bram = NULL; // Bram Ctrl if
static struct gaussian_info *axi_dma = NULL; //? cpu2bram, ip2cpu(read,write)



// File ops
static struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .open = gaussian_open,
    .release = gaussian_close,
    .read = gaussian_read,
    .write = gaussian_write,
    .fasync = gaussian_fasync,
    .mmap =(void *)gaussian_dma_mmap //npr kao void *
};

// Of device id
static struct of_device_id gaussian_of_match[] = 
{
	{ .compatible = "apply_gaussian", }, // gaussian core 
	{ .compatible = "kernel_bram", }, // kernel_bram
	{ .compatible = "axi_dma", }, // axi dma 
	
	{ /* end of list */ },
};

// My driver struct
static struct platform_driver my_driver = {
    .driver = 
	{
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table	= gaussian_of_match,
	},
	.probe = gaussian_probe,
	.remove	= gaussian_remove,
};

// fasync
static int gaussian_fasync(int fd, struct file *file, int mode)
{
	return fasync_helper(fd, file, mode, &async_queue);
}


MODULE_DEVICE_TABLE(of, gaussian_of_match);

// DMA: God, help us!
dma_addr_t tx_phy_buffer; //physical address space for tx data
dma_addr_t rx_phy_buffer;
u32 *tx_vir_buffer; //virtual address space for tx data
u32 *rx_vir_buffer;

volatile int tx_intr_done=0;
/*------------------------------------------------------------------*/



// IMPLEMENTATION OF DMA related functions

static irqreturn_t dma_isr(int irq,void*dev_id)
{
	u32 IrqStatus;  
	/* Read pending interrupts */
	IrqStatus = ioread32(axi_dma->base_addr + 4);//read irq status from MM2S_DMASR register
	iowrite32(IrqStatus | 0x00007000, axi_dma->base_addr + 4);//clear irq status in MM2S_DMASR register
	//(clearing is done by writing 1 on 13. bit in MM2S_DMASR (IOC_Irq)
	printk(KERN_DEBUG "In interrupt routine \n");


	tx_intr_done = 1;
	kill_fasync(&async_queue, SIGIO, POLL_IN);
	printk(KERN_DEBUG "TX interrupt happened!: %d\n", tx_intr_done);
	return IRQ_HANDLED;;
}

// Before every transac, you should initialize DMA: direction-> CPU-DMA-IP
int dma_init(void __iomem *base_address)
{
	u32 reset = 0x00000004;
	u32 IOC_IRQ_EN; 
	u32 ERR_IRQ_EN;
	u32 MM2S_DMACR_reg;
	u32 S2MM_DMACR_reg;
	u32 en_interrupt;
	u32 en_interrupt_rx;

	IOC_IRQ_EN = 1 << 12; // this is IOC_IrqEn bit in MM2S_DMACR register
	ERR_IRQ_EN = 1 << 14; // this is Err_IrqEn bit in MM2S_DMACR register

	iowrite32(reset, base_address); // writing to MM2S_DMACR register. Seting reset bit (3. bit)
	iowrite32(reset, base_address + 48);

	MM2S_DMACR_reg = ioread32(base_address); // Reading from MM2S_DMACR register inside DMA
	S2MM_DMACR_reg = ioread32(base_address + 48);
	en_interrupt = MM2S_DMACR_reg | IOC_IRQ_EN | ERR_IRQ_EN;// seting 13. and 15.th bit in MM2S_DMACR
	en_interrupt_rx = S2MM_DMACR_reg | IOC_IRQ_EN | ERR_IRQ_EN;
	iowrite32(en_interrupt, base_address); // writing to MM2S_DMACR register  
	iowrite32(en_interrupt_rx, base_address + 48);
	return 0;
}

//!



// Init DMA transaction

int dma_init_read(void __iomem *base_address)
{
	u32 reset = 0x00000004;//reset bit na poziciji 2
	u32 IOC_IRQ_EN; 
	u32 ERR_IRQ_EN;
	u32 S2MM_DMACR_reg; //na poziciji 0x30
	u32 en_interrupt;

	IOC_IRQ_EN = 1 << 12; // this is IOC_IrqEn bit in MM2S_DMACR register  //!!!
	ERR_IRQ_EN = 1 << 14; // this is Err_IrqEn bit in MM2S_DMACR register  //!!!

	iowrite32(reset, base_address); // writing to MM2S_DMACR register. Seting reset bit (3. bit) //!!!

	S2MM_DMACR_reg = ioread32(base_address); // Reading from MM2S_DMACR register inside DMA //!!!
	en_interrupt = S2MM_DMACR_reg | IOC_IRQ_EN | ERR_IRQ_EN;// seting 13. and 15.th bit in MM2S_DMACR//!!!
	iowrite32(en_interrupt, base_address); // writing to MM2S_DMACR register  //!!!
	return 0;
}



u32 dma_simple_write(dma_addr_t TxBufferPtr, u32 max_pkt_len, void __iomem *base_address) {
	u32 MM2S_DMACR_reg;

	MM2S_DMACR_reg = ioread32(base_address); // READ from MM2S_DMACR register

	iowrite32(0x1 |  MM2S_DMACR_reg, base_address); // set RS bit in MM2S_DMACR register (this bit starts the DMA)

	iowrite32((u32)TxBufferPtr, base_address + 24); // Write into MM2S_SA register the value of TxBufferPtr.
	// With this, the DMA knows from where to start.

	iowrite32(max_pkt_len, base_address + 40); // Write into MM2S_LENGTH register. This is the length of a tranaction.
	// In our case this is the size of the image (640*480*4)
	return 0;
}


u32 dma_simple_read(dma_addr_t RxBufferPtr, u32 max_pkt_len, void __iomem *base_address) {
	u32 S2MM_DMACR_reg;

	S2MM_DMACR_reg = ioread32(base_address + 48); // READ from MM2S_DMACR register

	iowrite32(0x1 |  S2MM_DMACR_reg, base_address + 48); // set RS bit in MM2S_DMACR register (this bit starts the DMA)

	iowrite32((u32)RxBufferPtr, base_address + 72); // Write into MM2S_SA register the value of TxBufferPtr.
	// With this, the DMA knows from where to start.

	iowrite32(max_pkt_len, base_address + 88); // Write into S2MM_LENGTH register. This is the length of a tranaction.
	// In our case this is the size of the image (640*480*4)
	return 0;
}




int mmap_flag = 0;
static ssize_t gaussian_dma_mmap(struct file *f, struct vm_area_struct *vma_s) 
{
	int ret = 0;
	long length = vma_s->vm_end - vma_s->vm_start;


	if(length > MAX_PKT_LEN*2)
	{
		return -EIO;
		printk(KERN_ERR "Trying to mmap more space than it's allocated\n");
	}

	


	if(mmap_flag%2==0){
		ret = dma_mmap_coherent(axi_dev, vma_s, tx_vir_buffer, tx_phy_buffer, length);
		printk(KERN_INFO "mmaped Tx\n");
	}
	else{
		ret = dma_mmap_coherent(axi_dev, vma_s, rx_vir_buffer, rx_phy_buffer, length);
		printk(KERN_INFO "mmaped Rx\n");
	
	}
	//1st mmap is for Tx, 2nd for Rx
	mmap_flag ++;

	if(ret<0)
	{
		printk(KERN_ERR "memory map failed\n");
		return ret;
	}
	return 0;
}
/****************************************************/

//---

// Probe & Remove 

/* Probe Function: When kernel finds a device in dev. tree and driver in compatible field
	then it calls the *probe* func */
	
int probe_counter = 0;
u32 reset = 0x00000000;

static int gaussian_probe(struct platform_device *pdev)
{
	struct resource *r_mem;
	int rc = 0;

	printk(KERN_INFO "Probing\n");

// Get phisical register adress space from device tree
	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);


	if (!r_mem) 
	{
		printk(KERN_ALERT "gaussian_probe: invalid address\n");
		return -ENODEV;
	}
	/*------------------------------------------------------*/
	switch (probe_counter)
	{
		case 0: // Gaussian Core - AXI Lite if 
		
		// Get memory for structure gaussian_info
			gaussian_core = (struct gaussian_info *) kmalloc(sizeof(struct gaussian_info), GFP_KERNEL);
			if (!gaussian_core)
			{
				printk(KERN_ALERT "gaussian_probe: Could not allocate gaussian_core device\n");
				return -ENOMEM;
			}
			
			// Put phisical adresses in gaussian_info structure
			gaussian_core->mem_start = r_mem->start;
			gaussian_core->mem_end   = r_mem->end;
			// Reserve that memory space for this driver
			if(!request_mem_region(gaussian_core->mem_start, gaussian_core->mem_end - gaussian_core->mem_start+1, DRIVER_NAME))
			{
				printk(KERN_ALERT "gaussian_probe: Couldn't lock memory region at %p\n",(void *)gaussian_core->mem_start);
				rc = -EBUSY;
				goto error1;
			}
			// Remap phisical to virtual adresses
			gaussian_core->base_addr = ioremap(gaussian_core->mem_start, gaussian_core->mem_end - gaussian_core->mem_start + 1);
			if (!gaussian_core->base_addr)
			{
				printk(KERN_ALERT "gaussian_probe: Could not allocate gaussian_core iomem\n");
				rc = -EIO;
				goto error2;
			}
			
			probe_counter++;
			printk(KERN_INFO "gaussian_probe: gaussian_core driver registered :)\n");
			return 0;
			
			error2:
			release_mem_region(gaussian_core->mem_start, gaussian_core->mem_end - gaussian_core->mem_start + 1);
			
			error1:
			return rc;
		
			break;
			
		case 1: // Kernel Bram : Bram Ctrl if
			kernel_bram = (struct gaussian_info *) kmalloc(sizeof(struct gaussian_info), GFP_KERNEL);
			if (!kernel_bram)
			{
				printk(KERN_ALERT "gaussian_probe: Cound not allocate kernel_bram device\n");
				return -ENOMEM;
			}
			
			kernel_bram->mem_start = r_mem->start;
			kernel_bram->mem_end   = r_mem->end;
			if(!request_mem_region(kernel_bram->mem_start, kernel_bram->mem_end - kernel_bram->mem_start+1, DRIVER_NAME))
			{
				printk(KERN_ALERT "gaussian_probe: Couldn't lock memory region at %p\n",(void *)kernel_bram->mem_start);
				rc = -EBUSY;
				goto error3;
			}
			
			kernel_bram->base_addr = ioremap(kernel_bram->mem_start, kernel_bram->mem_end - kernel_bram->mem_start + 1);
			if (!kernel_bram->base_addr)
			{
				printk(KERN_ALERT "gaussian_probe: Could not allocate kernel_bram iomem\n");
				rc = -EIO;
				goto error4;
			}
			
			probe_counter++;
			printk(KERN_INFO "gaussian_probe: kernel_bram driver registered.\n");
			return 0;
			
			error4:
			release_mem_region(kernel_bram->mem_start, kernel_bram->mem_end - kernel_bram->mem_start + 1);
			
			error3:
			return rc;
			
			break;
			
		case 2: // axi_dma -> DMA Ctrl if
			axi_dma = (struct gaussian_info *) kmalloc(sizeof(struct gaussian_info), GFP_KERNEL);
			if (!axi_dma)
			{
				printk(KERN_ALERT "gaussian_probe: Could not allocate axi_dma device\n");
				return -ENOMEM;
			}
			
			axi_dma->mem_start = r_mem->start;
			axi_dma->mem_end   = r_mem->end;

//			printk(KERN_DEBUG "PH address: %lu  %lu", (u32)(r_mem->start), (u32)(r_mem->end));

			if(!request_mem_region(axi_dma->mem_start, axi_dma->mem_end - axi_dma->mem_start+1, DRIVER_NAME))
			{
				printk(KERN_ALERT "gaussian_probe: Couldn't lock memory region at %p\n",(void *)axi_dma->mem_start);
				rc = -EBUSY;
				goto error5;
			}
			
			axi_dma->base_addr = ioremap(axi_dma->mem_start, axi_dma->mem_end - axi_dma->mem_start + 1);
			if (!axi_dma->base_addr)
			{
				printk(KERN_ALERT "gaussian_probe: Could not allocate axi_dma iomem\n");
				rc = -EIO;
				goto error6;
			}
			
			// Get irq num 
			axi_dma->irq_num = platform_get_irq(pdev, 0);
			if(!axi_dma->irq_num)
			{
				printk(KERN_ERR "gaussian_probe: Could not get IRQ resource\n");
				rc = -ENODEV;
				goto error6;
			}

			if (request_irq(axi_dma->irq_num, dma_isr, 0, DEVICE_NAME, NULL)) {
				printk(KERN_ERR "@dma_probe: Could not register IRQ %d\n", axi_dma->irq_num);
				return -EIO;
				goto error7;
			}
			else {
				printk(KERN_INFO "gaussian_probe: Registered IRQ %d\n", axi_dma->irq_num);
			}
					
			
			printk(KERN_INFO "gaussian_probe: axi_dma driver registered.\n");
		//	printk(KERN_INFO "Dancing Queeeeen! Young and sweeeeet!.\n");
			return 0; //ALL OK
			
			error7:
			iounmap(axi_dma->base_addr);

			error6:
			release_mem_region(axi_dma->mem_start, axi_dma->mem_end - axi_dma->mem_start + 1);
		

			error5:			
			return rc;
			break;
			
			
		default:
			printk(KERN_INFO "gaussian_probe: Counter has illegal value. \n");
			return -1;
	}
	return 0;
}	
	

static int gaussian_remove(struct platform_device *pdev)
{
	switch (probe_counter)
	{
		case 0: // gaussian_core
			printk(KERN_ALERT "gaussian_remove: gaussian_core platform driver removed\n");
			iowrite32(0, gaussian_core->base_addr);
			iounmap(gaussian_core->base_addr);
			release_mem_region(gaussian_core->mem_start, gaussian_core->mem_end - gaussian_core->mem_start + 1);
			kfree(gaussian_core);
			break;
			
		case 1: // kernel_bram
			printk(KERN_ALERT "gaussian_remove: kernel_bram platform driver removed\n");
			iowrite32(0, kernel_bram->base_addr);
			iounmap(kernel_bram->base_addr);
			release_mem_region(kernel_bram->mem_start, kernel_bram->mem_end - kernel_bram->mem_start + 1);
			kfree(kernel_bram);
			probe_counter--;
			break;
			
		case 2: // axi_dma

			 reset = 0x00000004;
			// writing to MM2S_DMACR register. Seting reset bit (3. bit)

			iowrite32(reset, axi_dma->base_addr); 
			printk(KERN_ALERT "gaussian_remove: axi_dma platform driver removed\n");
			free_irq(axi_dma->irq_num, NULL);
	

			//iowrite32(0, axi_dma->base_addr);
			iounmap(axi_dma->base_addr);
			release_mem_region(axi_dma->mem_start, axi_dma->mem_end - axi_dma->mem_start + 1);
			kfree(axi_dma);
			probe_counter--;
			break;
			
			
		default:
			printk(KERN_INFO "gaussian_remove: Counter has illegal value. \n");
			return -1;
	}
	return 0;
}



/*------------------------------------------------------------------*/

// Open & Close Funcs

static int gaussian_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "gaussian_open: It works!\n");
	return 0;
}

static int gaussian_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "gaussian_close: It works!\n");
	return 0;
}


// Read & Write Funcs

int endRead = 0;

int ready = 1;

ssize_t gaussian_read(struct file *pfile, char __user *buf, size_t length, loff_t *off)
{

	char r_buff[BUFF_SIZE];
	long int len = 0;
	int value_read;
	int minor = MINOR(pfile->f_inode->i_rdev);
	int max_packet_read = 792000;

	if (endRead == 1)
	{		
		endRead = 0;

		printk(KERN_INFO "Izlaz iz citanja %d!\n", minor);
		return 0;
	}



	if(minor == 2) //dma_read
	{

	printk(KERN_ERR "gaussian_read: Can not read axi_dma content!\n");
	endRead = 1;

	}
	else if(minor == 1) //kernel read - not supported
	{	
		printk(KERN_ERR "gaussian_read: Can not read kernel_bram content!\n");
		endRead = 1;
	}
	else if(minor == 0) // can only read from ready reg
	{
		
		value_read = ioread32(gaussian_core->base_addr + READY_REG);

		len = scnprintf(r_buff,BUFF_SIZE, "%d", value_read); 
		if(copy_to_user(buf, r_buff, len)) //dest, source, length
		{
			printk(KERN_ERR "gaussian_read:Ready reg can not be read.\n");
				return -EFAULT;		
		}
		endRead = 1;
	}
	else
	{
		printk(KERN_ERR "gaussian_read: Invalid minor. \n");
			endRead = 1;
	}

	printk(KERN_INFO "gaussian_read: Here I am, oce again!\n");
	return len;

}

//-------------------Write------------------------------------

ssize_t gaussian_write(struct file *pfile, const char __user *buf, size_t length, loff_t *off)
{
	char buff[BUFF_SIZE];
	int minor = MINOR(pfile->f_inode->i_rdev);
	int pos = 0;
	int val = 0;
	
	int ret = 0;
	int write_pos = 0;
	int write_offset;
	
	if (minor == 2)//dma write
	{
		if (copy_from_user(buff, buf, length))
			return -EFAULT;
		buff[length]='\0';
		printk(KERN_DEBUG "sadrzina bafera %s i %d\n",buff, buff );

		sscanf(buff, "%d", &val);
		printk(KERN_INFO "value of max pkt: %d\n", val);


		dma_init(axi_dma -> base_addr);//1st : Instead of base_addr should be start_addr
		printk(KERN_DEBUG "dma je inicijalizovan\n");
		if(tx_intr_done){
			printk(KERN_DEBUG "Usli u drugi deo\n");
			
			dma_simple_read(rx_phy_buffer, val, axi_dma -> base_addr);
			printk(KERN_DEBUG "posle read inita\n");
			dma_simple_write(tx_phy_buffer, val-write_offset, axi_dma->base_addr); //2nd	
			tx_intr_done = 0;
		}
		else{
			printk(KERN_DEBUG "sad sam ovde\n");
			write_offset = val;//ili sa 2
			dma_simple_write(tx_phy_buffer, val, axi_dma->base_addr); //2nd			
		}

	}
	else if (minor == 1)// kernel_bram
	{


	ret = copy_from_user(buff, buf, length);  //from usr buf to our buff
	

	if(ret){
		printk("gaussian_write(toKERNEL_BRAM):copy from user failed \n");
		return -EFAULT;
	}  
	buff[length] = '\0';
	sscanf(buff, "%d %d", &pos, &val); //address and value
//	printk(KERN_DEBUG "Write into kernel bram pos %d val %d ", pos, val);
	
	iowrite32(val, kernel_bram->base_addr + pos);
	
	}
	else if (minor == 0) // gaussian core registers
	{

		if (copy_from_user(buff, buf, length))
			return -EFAULT;
		buff[length]='\0';

		sscanf(buff, "%d %d", &pos, &val);
//		printk(KERN_INFO "value %d position %d", val,pos);
		switch(pos)
		{
			case START_REG://0
			if(val == 0 || val==1){	
			  
			    iowrite32(val, gaussian_core->base_addr + START_REG);
//			    printk(KERN_DEBUG "start %d", val);
			}
			else
				printk(KERN_ERR "Invalid value!\n");
					    
			break;
			
			case RESET_REG://8
			if(val == 0 || val == 1){
			    iowrite32(val, gaussian_core->base_addr + RESET_REG);
//			    printk(KERN_DEBUG "reset %d", val);
			}
			else
				printk(KERN_ERR "Invalid value!\n");


			break;
			case KERNEL_REG://4
			if(val >= 3 && val <= 25 && val%2 == 1){
			    iowrite32(val, gaussian_core->base_addr + KERNEL_REG);
//			    printk(KERN_DEBUG "kernel size %d", val);
			}
		    	else
				printk(KERN_ERR "Invalid value!\n");	
			break;
			default:
			    printk(KERN_ERR "gaussian_write: Invalid reg address\n");
			break;
		}

	}
 	
	printk(KERN_INFO "gaussian_write: Here I am!\n");

	return length;
}




/*------------------------------------------------------------------*/
int i = 0;

// Init & Exit
static int __init gaussian_init(void)
{	

	printk(KERN_INFO "\n");
	printk(KERN_INFO "Gaussian driver starting insmod.\n");

	if (alloc_chrdev_region(&my_dev_id, 0, 3, "gaussian_region") < 0) // msm da ovde treba 3
	{
		printk(KERN_ERR "failed to register char device\n");
		return -1;
	}
	printk(KERN_INFO "char device region allocated\n");

	my_class = class_create(THIS_MODULE, "gaussian_class");
	if (my_class == NULL)
	{
		printk(KERN_ERR "failed to create class\n");
		goto fail_0;
	}
	printk(KERN_INFO "class created\n");
/* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
	if (device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 0), NULL, "apply_gaussian") == NULL)  // naziv iz .compatible polja
	{
		printk(KERN_ERR "failed to create device gaussian_core\n");
		goto fail_1;
	}
	printk(KERN_INFO "device created - gaussian_core\n");

	if (device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 1), NULL, "kernel_bram") == NULL) 
	{
		printk(KERN_ERR "failed to create device kernel_bram\n");
		goto fail_2;
	}
	printk(KERN_INFO "device created - kernel_bram\n");

	axi_dev = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id),2), NULL, "axi_dma");
 
	if(axi_dev == NULL)
	{
		printk(KERN_ERR "failed to create device axi_dma\n");
		goto fail_3;
	}
	printk(KERN_INFO "device created - axi_dma\n");

	my_cdev = cdev_alloc();
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;

	if (cdev_add(my_cdev, my_dev_id, 3) == -1) // we have 3 devices - 3 minor nums
	{
		printk(KERN_ERR "failed to add cdev\n");
		goto fail_4;
	}
	printk(KERN_INFO "cdev added\n");

	if(dma_set_coherent_mask(axi_dev, DMA_BIT_MASK(32)))
	{
		printk(KERN_ERR "Mask: invalid\n");
		goto fail_5;
	}	

	tx_vir_buffer = dma_alloc_coherent(axi_dev, MAX_PKT_LEN*2, &tx_phy_buffer, GFP_DMA | GFP_KERNEL); //Ideja zdruzi oba uslova za bafere
	rx_vir_buffer = dma_alloc_coherent(axi_dev, MAX_PKT_LEN*2, &rx_phy_buffer, GFP_DMA | GFP_KERNEL); //Ideja zdruzi oba uslova za bafere

	if(!tx_vir_buffer || !rx_vir_buffer){
		printk(KERN_ALERT "gaussian_init: Could not allocate dma_alloc_coherent for img");
		
		goto fail_5;
	}
	else
		printk("gaussian_init: Successfully allocated memory for dma tx buffer\n");
	for (i = 0; i < MAX_PKT_LEN/4;i++)
	{	
		tx_vir_buffer[i] = 0x00000000; // TxBuffer Initialization
		rx_vir_buffer[i] = 0x00000000; // RxBuffer Initialization
	}
	printk(KERN_INFO "axi_dma_init: DMA memory reset.\n");

	return platform_driver_register(&my_driver); // if all ok

	fail_5:
		printk(KERN_ERR "FAIL 5\n");
		cdev_del(my_cdev); 
		
	fail_4:
		printk(KERN_ERR "FAIL 4\n");
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),2));
	
	fail_3:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),1));
	fail_2:
		device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	fail_1:
		printk(KERN_ERR "FAIL 1\n");
		class_destroy(my_class);
	fail_0:
		printk(KERN_ERR "FAIL 0\n");
		unregister_chrdev_region(my_dev_id, 1);
	return -1;
}

static void __exit gaussian_exit(void)
{
	printk(KERN_INFO "gaussian driver starting rmmod.\n");
	
	//Reset DMA memory

	for (i = 0; i < MAX_PKT_LEN/4; i++) 
	{	tx_vir_buffer[i] = 0x00000000; // ovde bi mozda valjalo isto i za rx buffer
		rx_vir_buffer[i] = 0x00000000; // ovde bi mozda valjalo isto i za rx buffer
	}
	printk(KERN_INFO "gaussian_exit: DMA memory reset\n");

	
	platform_driver_unregister(&my_driver);
	cdev_del(my_cdev);

	
	
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),2));
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),1));
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id, 1);	//zasto nije 3?

	dma_free_coherent(axi_dev, MAX_PKT_LEN*2, tx_vir_buffer, tx_phy_buffer);
	dma_free_coherent(axi_dev, MAX_PKT_LEN*2, rx_vir_buffer, rx_phy_buffer);
	printk(KERN_INFO "gaussian driver exited.\n");
}

module_init(gaussian_init);
module_exit(gaussian_exit);




/*------------------------------------------------------------------*/



