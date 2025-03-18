/**
 ****************************************************************************************************
 * @file        myiic.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2024-11-01
 * @brief       IIC 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 M48Z-M3最小系统板STM32F103版
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */
 
#ifndef __MYIIC_H
#define __MYIIC_H

#include "sys.h"


/******************************************************************************************/
/* 引脚 定义 */

#define IIC_SCL_GPIO_PORT               GPIOB
#define IIC_SCL_GPIO_PIN                GPIO_Pin_13
#define IIC_SCL_GPIO_CLK_ENABLE()       do{RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);}while(0)     /* PA口时钟使能 */

#define IIC_SDA_GPIO_PORT               GPIOB
#define IIC_SDA_GPIO_PIN                GPIO_Pin_12
#define IIC_SDA_GPIO_CLK_ENABLE()       do{RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);}while(0)     /* PA口时钟使能 */
/******************************************************************************************/
/* IO操作 */

#define IIC_SCL(x)        do{ x ? \
                              GPIO_WriteBit(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, Bit_SET) : \
                              GPIO_WriteBit(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, Bit_RESET); \
                          }while(0)         /* SCL */

#define IIC_SDA(x)        do{ x ? \
                              GPIO_WriteBit(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, Bit_SET) : \
                              GPIO_WriteBit(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, Bit_RESET); \
                          }while(0)         /* SDA */

#define IIC_READ_SDA     GPIO_ReadInputDataBit(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN)          /* 读取SDA */


/******************************************************************************************/
                          
void iic_init(void);                        /* 初始化IIC的IO口 */
void iic_start(void);                       /* 发送IIC开始信号 */
void iic_stop(void);                        /* 发送IIC停止信号 */
void iic_ack(void);                         /* IIC发送ACK信号 */
void iic_nack(void);                        /* IIC不发送ACK信号 */
uint8_t iic_wait_ack(void);                 /* IIC等待ACK信号 */
void iic_send_byte(uint8_t txd);            /* IIC发送一个字节 */
uint8_t iic_read_byte(unsigned char ack);   /* IIC读取一个字节 */

#endif

