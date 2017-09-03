#include "lpc82x.h"
#define I2C_STAT_SLVPENDING (0x100)
#define I2C_STAT_SLVSTATE (0x600)
#define I2C_SLVCTL_SLVCONTINUE (0x1)

#define SPI_CFG_ENABLE (0x1)
#define SPI_CFG_MASTER (0x4)
#define SPI_STAT_RXRDY (0x1)
#define SPI_STAT_TXRDY (0x2)
#define SPI_STAT_SSD (0x20)
#define SPI_STAT_MSTIDLE (0x100)
#define SPI_TXDATCTL_SSEL_N(s) ((s) << 16)
#define SPI_TXDATCTL_EOT (1 << 20)
#define SPI_TXDATCTL_EOF (1 << 21)
#define SPI_TXDATCTL_RXIGNORE (1 << 22)
#define SPI_TXDATCTL_FLEN(l) ((l) << 24)
  
	uint32_t timer[20]={0};
	uint32_t *count=timer;
	uint32_t *count_end=&timer[5];
  int time1=0,data_i2c=0;
	unsigned int pin=0;
//Initialization function of PLL, from test, the main clock can be rised to 36Mhz, although the max clock is 30Mhz in datasheet
void I2C_init()
{
	
	LPC_SYSCON->SYSAHBCLKCTRL=LPC_SYSCON->SYSAHBCLKCTRL | (0x20);  /* SYSAHBCLKCTRL (bit31 ... bit0), change bit5(I2C0) from 0(default) to 1(enable I2C0 clock)*/
	/*set bit6(I2C0_RST_N) to 1 to clear the I2C0 reset, but it is default 1, LPC_SYSCON->PRESETCTRL*/ 
	LPC_SWM->PINENABLE0=LPC_SWM->PINENABLE0 &(0xFFFFEFFF); /* enable I2C0_SCL function on PIO0_10/I2C0_SCL pin(pin9 of LPC824) */
	                                                   /*set bit12 of PINENABLE0(I2C0_SCL) from 1(default) to 0(enable I2C0_SCL function on pin PIO0_10) */
	LPC_SWM->PINENABLE0=LPC_SWM->PINENABLE0 &(0xFFFFF7FF); /* enable I2C0_SDA function on PIO0_11/I2C0_SDA pin(pin8 of LPC824) */
	                                                   /*set bit11 of PINENABLE0(I2C0_SDA) from 1(default) to 0(enable I2C0_SDA function on pin PIO0_11) */	

	LPC_I2C0->CLKDIV=LPC_I2C0->CLKDIV | (0x000F); //Divide the system clock (I2C_PCLK) by a factor of 2 before use by the I2C function
	LPC_I2C0->MSTTIME=LPC_I2C0->MSTTIME & (0xFFFFFF88); //Set the SCL high and low times to 2 clock cycles each
	
	LPC_I2C0->SLVADR0=LPC_I2C0->SLVADR0 & (0xFFFFFFFE); //set bit0(SADISABLE) of SLVADR0 register from 1(default) to 1(enable Slave Address 0)
	LPC_I2C0->SLVADR0=LPC_I2C0->SLVADR0 | (0x22); //set bit7:bit1(SLVADR) of SLVADR0 register from 0(default) to 16
	LPC_I2C0->CFG=LPC_I2C0->CFG | (0x2); /*set bit1(SLVEN(slave enable)) of CFG(I2C0) from 0(default) to 1(enable) */

}

int I2C0_read_adres_data()
{
	int I2C_trigerdata;
	I2C_trigerdata=0x00;
	while(1)  // solve problem: I2C data for other slave come to LPC824-->stay in the while loop of SLVSTATE(received data)
	{
  while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */
	while(LPC_I2C0->STAT & I2C_STAT_SLVSTATE);  /* wait in the loop until that master sends right slave address */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
	                                                               /* acknowledge the address by setting SLVCONTINUE 1*/ 
	while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	if((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)==0x200)  /* wait in the loop until that received data is available */ //doubt if work or not??
	{
		break; //when SLVSTATE is receive data, leave while loop 
	}
	}
	I2C_trigerdata=LPC_I2C0->SLVDAT & (0xFF) ; // save bit7:bit0(received data) of SLVDAT register from master into the variable(I2C_trigerdata)
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
                                                                  /* acknowledge the data by setting SLVCONTINUE 1*/ 
	return I2C_trigerdata; //return received data to main function to trigger sub functions
	
}

int I2C0_read_adres_intdata(){
  int I2C_trigerdata;
	I2C_trigerdata=0x00;
	
  while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */
	while(LPC_I2C0->STAT & I2C_STAT_SLVSTATE);  /* wait in the loop until that master sends right slave address */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
	
	while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	while((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)!=0x200);  /* wait in the loop until that received data is available */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
  I2C_trigerdata|= (LPC_I2C0->SLVDAT & (0xFF))<<24 ;                                                                /* acknowledge the data by setting SLVCONTINUE 1*/ 
	
	while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	while((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)!=0x200);  /* wait in the loop until that received data is available */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
  I2C_trigerdata|= (LPC_I2C0->SLVDAT & (0xFF))<<16 ;
	
	while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	while((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)!=0x200);  /* wait in the loop until that received data is available */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)/* acknowledge the data by setting SLVCONTINUE 1*/ 
	I2C_trigerdata|= (LPC_I2C0->SLVDAT & (0xFF))<<8 ;
	
	while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	while((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)!=0x200);  /* wait in the loop until that received data is available */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)/* acknowledge the data by setting SLVCONTINUE 1*/ 
	I2C_trigerdata|= (LPC_I2C0->SLVDAT & (0xFF)) ;
  
	return I2C_trigerdata; //return received data to main function to trigger sub functions                                                                /* acknowledge the data by setting SLVCONTINUE 1*/ 
}

int I2C0_read_adres_2bytesdata(){
  int I2C_trigerdata;
	I2C_trigerdata=0x00;
	
  while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */
	while(LPC_I2C0->STAT & I2C_STAT_SLVSTATE);  /* wait in the loop until that master sends right slave address */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
	
	while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	while((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)!=0x200);  /* wait in the loop until that received data is available */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
  I2C_trigerdata|= (LPC_I2C0->SLVDAT & (0xFF))<<8 ;                                                                /* acknowledge the data by setting SLVCONTINUE 1*/ 
	
	while(~LPC_I2C0->STAT & I2C_STAT_SLVPENDING); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	while((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)!=0x200);  /* wait in the loop until that received data is available */ //doubt if work or not??
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
  I2C_trigerdata|= (LPC_I2C0->SLVDAT & (0xFF))<<0 ;
  
	return I2C_trigerdata; //return received data to main function to trigger sub functions                                                                /* acknowledge the data by setting SLVCONTINUE 1*/ 
}

void I2C0_send_adres_data(int I2Cdata)  
{
	//LPC_SYSCON->SYSAHBCLKDIV=(0x01);    /* divide SPI0 clock(=system clock) by 1 --> 30MHz(default setting) */ 
  while(!(LPC_I2C0->STAT & I2C_STAT_SLVPENDING)); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */
	while(LPC_I2C0->STAT & I2C_STAT_SLVSTATE);  /* wait in the loop until that master sends right slave address */ 
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
                                                              /* acknowledge the address by setting SLVCONTINUE 1*/ 
	while(!(LPC_I2C0->STAT & I2C_STAT_SLVPENDING)); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	while((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)!=0x400);  /* wait in the loop until that the data can be transmitted */
	LPC_I2C0->SLVDAT=(I2Cdata) ; // send bit7:bit0(transmit data) of SLVDAT register to master from the variable(I2Cdata)
	LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)

	//while(!(LPC_I2C0->STAT & I2C_STAT_SLVPENDING)); /* wait in the loop until SLVPENDING(Slave Pending) is 1 (default 0) */   
	//while((LPC_I2C0->STAT & I2C_STAT_SLVSTATE)!=0x400);  /* wait in the loop until that the data can be transmitted */
	//LPC_I2C0->SLVDAT=(0xf0) ; // send bit7:bit0(transmit data) of SLVDAT register to master from the variable(I2Cdata)
	//LPC_I2C0->SLVCTL=LPC_I2C0->SLVCTL | I2C_SLVCTL_SLVCONTINUE; //set bit0(SLVCONTINUE) of SLVCTL register from 0(default) to 1(slave continue)
}

void I2C0_send_adres_intdata(int I2Cdata)  
{
  I2C0_send_adres_data(I2Cdata>>28);
	I2C0_send_adres_data(I2Cdata>>21);
	I2C0_send_adres_data(I2Cdata>>14);
	I2C0_send_adres_data(I2Cdata>>7);
	I2C0_send_adres_data(I2Cdata);
}

void initial_pll(void){
	LPC_SYSCON->PDRUNCFG     &=(~(1<<5));    //power system pll, make 7th bit 0
	LPC_SYSCON->SYSPLLCLKSEL =0;             //choose IRC as input for PLL
	LPC_SYSCON->SYSPLLCLKUEN =0;             //enable the input of PLL
	LPC_SYSCON->SYSPLLCLKUEN =1;
	//LPC_SYSCON->SYSPLLCTRL   =0x25;          //MSEL: 00101 PSEL: 01  output clock:36MHz
	LPC_SYSCON->SYSPLLCTRL   =0x24;        //MSEL: 00100 PSEL: 01  output clock:30MHz
	while((LPC_SYSCON->SYSPLLSTAT&0x1)!=1);  //waite until PLL locked
	LPC_SYSCON->SYSAHBCLKDIV=1;              // devide the output clock of PLL
	LPC_SYSCON->MAINCLKSEL|=3;               //choose PLL output as main clock
	LPC_SYSCON->MAINCLKUEN=0;
	LPC_SYSCON->MAINCLKUEN=1;
}

void initial_pinint(){
/////disable all pin interrupts	
	NVIC_DisableIRQ(24);
	NVIC_DisableIRQ(25);
	NVIC_DisableIRQ(26);
	NVIC_DisableIRQ(27);
	NVIC_DisableIRQ(28);
	NVIC_DisableIRQ(29);
	NVIC_DisableIRQ(30);
	NVIC_DisableIRQ(31);
	LPC_PIN_INT->ISEL   = 0;    // Sets pin interrupt mode to edge sensitive
	LPC_PIN_INT->IENR   = 0xFF; // Enable rising edge interrupt
	LPC_PIN_INT->SIENR  = 1;    // Write to enable rising edge interrupt
	LPC_PIN_INT->IENF   = 0xff; // Enable falling edge interrupt
	LPC_PIN_INT->SIENF  = 1;    // Write to enable falling edge interrupt
///select pins for pin interrupt
	LPC_SYSCON->PINTSEL0= 17;   // select pin9 as the pin for pin interrupt0
	LPC_SYSCON->PINTSEL1= 23;   // select pin1 as the pin for pin interrupt1
	LPC_SYSCON->PINTSEL2= 4;   // select pin15 as the pin for pin interrupt2
	LPC_SYSCON->PINTSEL3= 13;   // select pin9 as the pin for pin interrupt3
	LPC_SYSCON->PINTSEL4= 1;    // select pin9 as the pin for pin interrupt0
	LPC_SYSCON->PINTSEL5= 9;    // select pin9 as the pin for pin interrupt0
	LPC_SYSCON->PINTSEL6= 0;    // select pin9 as the pin for pin interrupt0
	LPC_SYSCON->PINTSEL7= 14;   // select pin9 as the pin for pin interrupt0
}

void initial_MRT(void){
	LPC_SYSCON->SYSAHBCLKCTRL|=(1<<10);   //enable MRT
	LPC_SYSCON->PRESETCTRL|=(1<<7);       //clear the number of MRT
	//LPC_MRT->INTVAL1=0xFFFFFFFF;          //give MRT channel1 a initial value
	//LPC_MRT->CTRL1=0x03;                  //set INTEN to 1: enable interrupt    set MODE to 01: one-shot interrupt
}

void initial_SPI(void){
   LPC_SYSCON->SYSAHBCLKCTRL|=1<<11;
	LPC_SWM->PINASSIGN4 &= 0xFFFFFF00;
	LPC_SWM->PINASSIGN4 |= 0xC;
	LPC_SWM->PINASSIGN3 &= 0x00FFFFFF;
	LPC_SWM->PINASSIGN3 |= 0x08000000;
	
	LPC_SPI0->DIV=0xf;
	LPC_SPI0->CFG |= SPI_CFG_MASTER; 
	LPC_SPI0->CFG |= SPI_CFG_ENABLE | (1<<4); // change the phase
  
	LPC_GPIO_PORT->DIR0|=1<<4;
}

void test(int pin){
	LPC_MRT->INTVAL1=0xFFFFFFFF;
	switch(pin){
			case 0x00:
				count=timer;
				NVIC_EnableIRQ(24);
			break;
			case 0x01:
				count=timer;
				NVIC_EnableIRQ(25);
			break;
			case 0x02:
				count=timer;
				NVIC_EnableIRQ(26);
			break;
			case 0x03:
				count=timer;
				NVIC_EnableIRQ(27);
			break;
			case 0x04:
				count=timer;
				NVIC_EnableIRQ(28);
			break;
			case 0x05:
				count=timer;
				NVIC_EnableIRQ(29);
			break;
			case 0x06:
				count=timer;
				NVIC_EnableIRQ(30);
			break;
			case 0x07:
				count=timer;
				NVIC_EnableIRQ(31);
			break;
		}
 		while(1){
		if(count==count_end){
			time1=timer[1]-timer[2];
			I2C0_send_adres_intdata(time1);
			time1=timer[2]-timer[3];
			I2C0_send_adres_intdata(time1);
			break;
		}
	}
}

void changechannel(int voltage){

}
int main (void){
	//int time1=0;	
	initial_pll();
	I2C_init();
	initial_pinint();
	initial_MRT();
	while(1){
		
		data_i2c=I2C0_read_adres_2bytesdata();
		switch(data_i2c&0x80){
			case 0:
				test(data_i2c&0x7);
				break;
			//case 1:
			//	changechannel(data_i2c&0x7&0xff);
			//	break;
		}
		//pin=data_i2c & 0xFF;
		//NVIC_EnableIRQ(10);	                //enable the MRT interrupt
		//LPC_MRT->INTVAL1=0xFFFFFF;        //set the time-out
	
	}
}

void PIN_INT0_IRQHandler(void){
	*count=LPC_MRT->TIMER1;
	count++;
	LPC_PIN_INT->IST|=1;
  if(count==count_end){
			NVIC_DisableIRQ(24);
  }	
}

void PIN_INT1_IRQHandler(void){
	*count=LPC_MRT->TIMER1;
	count++;
	LPC_PIN_INT->IST|=0xff;
  if(count==count_end){
			NVIC_DisableIRQ(25);
  }		
}

void PIN_INT2_IRQHandler(void){
	*count=LPC_MRT->TIMER1;
	count++;
	LPC_PIN_INT->IST|=1;
  if(count==count_end){
			NVIC_DisableIRQ(26);
  }			
}

void PIN_INT3_IRQHandler(void){
	*count=LPC_MRT->TIMER1;
	count++;
	LPC_PIN_INT->IST|=1;
  
	if(count==count_end){
  	NVIC_DisableIRQ(27);
  }			
}

void PIN_INT4_IRQHandler(void){
	*count=LPC_MRT->TIMER1;
	count++;
	LPC_PIN_INT->IST|=1;
  if(count==count_end){
			NVIC_DisableIRQ(28);
  }			
}

void PIN_INT5_IRQHandler(void){
	*count=LPC_MRT->TIMER1;
	count++;
	LPC_PIN_INT->IST|=1;  
  if(count==count_end){
			NVIC_DisableIRQ(29);
  }		  
}


void PIN_INT6_IRQHandler(void){
	*count=LPC_MRT->TIMER1;
	count++;
	LPC_PIN_INT->IST|=1;
  if(count==count_end){
			NVIC_DisableIRQ(30);
  }			
}

void PIN_INT7_IRQHandler(void){
	*count=LPC_MRT->TIMER1;
	count++;
	LPC_PIN_INT->IST|=1;  
  if(count==count_end){
			NVIC_DisableIRQ(31);
  }		  
}
