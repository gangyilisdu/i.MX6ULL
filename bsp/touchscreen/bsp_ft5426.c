#include "bsp_ft5426.h"
#include "bsp_i2c.h"
#include "bsp_int.h"
#include "bsp_delay.h"
#include "stdio.h"


struct ft5426_dev_struc ft5426_dev;

void ft5426_init(void)
{
    /* init i2c io*/
    unsigned char reg_value[2];
    ft5426_dev.initflag = FT5426_INIT_NOTFINISHED;

    IOMUXC_SetPinMux(IOMUXC_UART5_TX_DATA_I2C2_SCL, 1);
    IOMUXC_SetPinMux(IOMUXC_UART5_RX_DATA_I2C2_SDA, 1);
    IOMUXC_SetPinConfig(IOMUXC_UART5_TX_DATA_I2C2_SCL, 0x70b0);
    IOMUXC_SetPinConfig(IOMUXC_UART5_RX_DATA_I2C2_SDA, 0x70b0);

    /* init touchscreen int io and reset io */
    gpio_pin_config_t ctinpin_config;
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO09_GPIO1_IO09, 0);
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER9_GPIO5_IO09, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO09_GPIO1_IO09, 0XF080);
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER9_GPIO5_IO09, 0X10B0);

    /* init int io */
    ctinpin_config.direction = kGPIO_DigitialInput; 
    ctinpin_config.interruptMode = kGPIO_IntRisingOrFallingEdge;
    gpio_init(GPIO1, 9, &ctinpin_config);
    GIC_EnableIRQ(GPIO1_Combined_0_15_IRQn);
    system_register_irqhandler(GPIO1_Combined_0_15_IRQn, (system_irq_handler_t)gpio1_io9_irqhandler, NULL);
    gpio_enableint(GPIO1, 9);

    /* init reset io */
    ctinpin_config.direction = kGPIO_DigitialOutput;
    ctinpin_config.interruptMode = kGPIO_Nointmode; 
    ctinpin_config.outputLogic = 1; 
    gpio_init(GPIO5, 9, &ctinpin_config);

    /* init i2c*/
    i2c_init(I2C2);

    /* init FT5426 */
    gpio_pinwrite(GPIO5, 9, 0);
    delay_ms(20);
    gpio_pinwrite(GPIO5, 9, 1); 
    delay_ms(20);
    ft5426_write_byte(FT5426_ADDR, FT5426_DEVICE_MODE, 0);
    ft5426_write_byte(FT5426_ADDR, FT5426_IDG_MODE, 1);
    ft5426_read_len(FT5426_ADDR, FT5426_IDGLIB_VERSION, 2, reg_value);
    printf("Touch Frimeware Version : %#X\r\n", ((unsigned short)reg_value[0] << 8) + reg_value[1]);
    ft5426_dev.initflag = FT5426_INIT_FINISHED;
    ft5426_dev.intflag = 0;
}

void gpio1_io9_irqhandler(void)
{
    if(ft5426_dev.initflag == FT5426_INIT_FINISHED)
    {
        ft5426_read_tpcoord();
    }
    gpio_clearintflags(GPIO1, 9);
}

unsigned char ft5426_write_byte(unsigned char addr, unsigned char reg, unsigned char data)
{
    unsigned char status = 0;
    unsigned char writedata = data; 
    struct i2c_transfer masterXfer; 

    masterXfer.slaveaddress = addr; 
    masterXfer.direction = kI2C_Write; 
    masterXfer.subaddress = reg;
    masterXfer.subaddressSize = 1; 
    masterXfer.data = &writedata; 
    masterXfer.dataSize = 1; 

    if(i2c_master_transfer(I2C2, &masterXfer))
        status = 1; 
    return status; 
}

unsigned char ft5426_read_byte(unsigned char addr, unsigned char reg)
{
    unsigned char val = 0;

    struct i2c_transfer masterXfer; 
    masterXfer.slaveaddress = addr; 
    masterXfer.direction = kI2C_Read;
    masterXfer.subaddress = reg;
    masterXfer.subaddressSize = 1; 
    masterXfer.data = &val; 
    masterXfer.dataSize  = 1;
    i2c_master_transfer(I2C2, &masterXfer); 

    return val; 
}

void ft5426_read_len(unsigned char addr, unsigned char reg, unsigned char len, unsigned char *buf)
{
    struct i2c_transfer masterXfer; 

    masterXfer.slaveaddress = addr;
    masterXfer.direction = kI2C_Read;
    masterXfer.subaddress = reg;
    masterXfer.subaddressSize = 1; 
    masterXfer.data = buf; 
    masterXfer.dataSize = len;
    i2c_master_transfer(I2C2, &masterXfer); 
}

void ft5426_read_tpcoord(void)
{
    unsigned char i = 0; 
    unsigned char type = 0; 
    unsigned char pointbuf[FT5426_XYCOORDREG_NUM];

    ft5426_dev.point_num = ft5426_read_byte(FT5426_ADDR, FT5426_TD_STATUS); 
    ft5426_read_len(FT5426_ADDR, FT5426_TOUCH1_XH, FT5426_XYCOORDREG_NUM, pointbuf);

    for(i=0;i<ft5426_dev.point_num;i++)
    {
        unsigned char *buf = &pointbuf[i*6];
        ft5426_dev.x[i] = ((buf[2] << 8) | buf[3]) & 0x0fff;
		ft5426_dev.y[i] = ((buf[0] << 8) | buf[1]) & 0x0fff;
        type = buf[0] >> 6;
        if(type == FT5426_TOUCH_EVENT_DOWN || type == FT5426_TOUCH_EVENT_ON){
        }
        else{

        }
    }
}
