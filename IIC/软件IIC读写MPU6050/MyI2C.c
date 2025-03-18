#include "stm32f10x.h"                  // Device header
#include "Delay.h"

/*引脚配置层*/

/**
  * 函    数：I2C写SCL引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SCL的电平，范围0~1
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置SCL为低电平，当BitValue为1时，需要置SCL为高电平
  */
void MyI2C_W_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)BitValue);		//根据BitValue，设置SCL引脚的电平
	Delay_us(10);												//延时10us，防止时序频率超过要求
}

/**
  * 函    数：I2C写SDA引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SDA的电平，范围0~1
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置SDA为低电平，当BitValue为1时，需要置SDA为高电平
  */
void MyI2C_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)BitValue);		//根据BitValue，设置SDA引脚的电平，BitValue要实现非0即1的特性
	Delay_us(10);												//延时10us，防止时序频率超过要求
}

/**
  * 函    数：I2C读SDA引脚电平
  * 参    数：无
  * 返 回 值：协议层需要得到的当前SDA的电平，范围0~1
  * 注意事项：此函数需要用户实现内容，当前SDA为低电平时，返回0，当前SDA为高电平时，返回1
  */
uint8_t MyI2C_R_SDA(void)
{
	uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);		//读取SDA电平
	Delay_us(10);												//延时10us，防止时序频率超过要求
	return BitValue;											//返回SDA电平
}

/**
  * 函    数：I2C初始化
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，实现SCL和SDA引脚的初始化
  */
void MyI2C_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB10和PB11引脚初始化为开漏输出
	
	/*设置默认电平*/
	GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);			//设置PB10和PB11引脚初始化后默认为高电平（释放总线状态）
}

/*协议层*/

/**
  * 函    数：I2C起始
  * 参    数：无
  * 返 回 值：无
  *
  *
SCL高电平期间，SDA高电平变低电平

起始条件：最好把释放SDA的放在前面，如果起始条件之前，SCL和SDA已经是高电平，那先释放哪一个是一样的效果
但这里的函数需要兼容重复起始条件，SDA的引脚不敢确定，SCL的电平是低电平。
趁SCL是低电平，先确保释放SDA，再释放SCL，这时SCL和SDA都是高电平，
【SCL高时，可以对SDA读操作，为了防止误读，先将SDA释放】，
然后再拉低SDA，拉低SCL
*/

void MyI2C_Start(void)
{
	MyI2C_W_SDA(1);							//释放SDA，确保SDA为高电平
	MyI2C_W_SCL(1);							//释放SCL，确保SCL为高电平
	MyI2C_W_SDA(0);							//在SCL高电平期间，拉低SDA，产生起始信号
	MyI2C_W_SCL(0);							//起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}


/**
  * 函    数：I2C终止
  * 参    数：无
  * 返 回 值：无
  *
  * 
  *SCL高电平期间，SDA由低电平变为高电平 
  
  终止条件也同理，不确定SDA的电平情况，所以为了确保SDA有上升沿，所以最开始先拉低SDA，再释放SCL，释放SDA
 */
void MyI2C_Stop(void)
{
	MyI2C_W_SDA(0);							//拉低SDA，确保SDA为低电平
	MyI2C_W_SCL(1);							//释放SCL，使SCL呈现高电平
	MyI2C_W_SDA(1);							//在SCL高电平期间，释放SDA，产生终止信号
}

/**
  * 函    数：I2C发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：无
  *
  * 
    发送一个字节。需要一开始SCL低电平。但我们的起始条件，也是以SCL低电平结束，终止条件是以SCL高电平结束。所以除了终止条件是SCL高电平结束，
	其他的单元我们都会确保是以SCL低电平结束，确保各个单元的拼接
	SCL低电平，变换数据
	SCL高电平，保持数据稳定
	由于是高位先行，需要按照先放最高位，再放次高位，最后是最低位。依次把一个字节的每一位放在SDA线上，每放完一位，执行释放SCL，拉低SCL的操作，驱动时钟运转。
	为什么这里不拉低SCL就放数据，因为起始条件已经帮我们做好了拉低SCL。
	MyI2C_W_SDA(Byte &0x80);
	MyI2C_W_SCL(1);
	MyI2C_W_SCL(0);

	MyI2C_W_SDA(Byte &0x40);
	MyI2C_W_SCL(1);
	MyI2C_W_SCL(0);

	MyI2C_W_SDA(Byte &0x20);
	MyI2C_W_SCL(1);
	MyI2C_W_SCL(0);
	....这样来8次这个操作。就可以写入一个字节了

	来个for循环8次
*/
void MyI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i ++)				//循环8次，主机依次发送数据的每一位
	{
		/*两个!可以对数据进行两次逻辑取反，作用是把非0值统一转换为1，即：!!(0) = 0，!!(非0) = 1*/
		MyI2C_W_SDA(!!(Byte & (0x80 >> i)));//使用掩码的方式取出Byte的指定一位数据并写入到SDA线	//i=0 0x80   ; i=1   0x40   ;i=2  0x20;   所以就是0x80右移I位,这里要加括号确保优先级
		MyI2C_W_SCL(1);						//释放SCL，从机在SCL高电平期间读取SDA
		MyI2C_W_SCL(0);						//拉低SCL，主机开始发送下一位数据
	}
}

/**
  * 函    数：I2C接收一个字节
  * 参    数：无
  * 返 回 值：接收到的一个字节数据，范围：0x00~0xFF
  * 
  * 
	一开始SCL低电平，此时从机需要把数据放在SDA上，为了防止主机干扰从机写入数据，主机需要释放SDA，
	释放SDA也相当于切换为输入模式.在SCL低电平时。从机会把数据放在SDA，从机想发1，释放SDA,从机想发0，
	就拉低SDA，然后主机释放SCL。在SCL高电平期间，读取SDA，再拉低SCL。低电平器件，从机就会把下一位数据放在SDA上，
	这样重复8次，主机就能读到一个字节了。SCL低电平放数据。SCL高电平读数据，是一种读写分离的操作。
	低电平定义为写的时间，高电平定义为读的时间。在SCL高电平期间，变换SDA的数据的话，就会破坏数据的传输，
	使其变成了起始时序或者终止时序。这样可以快速定位起始条件和终止条件，因为起始条件和终止条件，和数据传输的波形有本质区别。
	数据传输，SCL高电平期间。SDA不允许变换数据。起始条件时，SCL高电平期间，SDA必须变化数据
	进来之后，SCL是低电平，因为每一个时序都是以SCL低电平为结束。
	主机释放SDA，从机把数据放到SDA
	主机释放SCL，SCL高电平。主机架就能读取数据了
	如果第一次读SDA为1，那么就Byte最高位置1，Byte |= 0x80;
	如果读SDA为0，if不成立，Byte默认为0x00;就相当于写入0了。
	读取一位之后，再把SCL拉低，这时从机就会把下一位数据放到SDA上
	uint8_t Byte = 0x00;
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	if( MyI2C_R_SDA() == 1 )  { Byte |= 0x80；  }
	MyI2C_W_SCL(0)；
	执行相同的流程8次，就能接收一个字节了。
	所以写个FOR循环
	uint8_t i, Byte = 0x00;
	MyI2C_W_SDA(1);
	for(i = 0; i<8 ; i++)
	{
		MyI2C_W_SCL(1);
		if( MyI2C_R_SDA() == 1 )  { Byte |= (0x80 >> i)；  }
		MyI2C_W_SCL(0)；
	}
	return Byte;
	问：for循环里都没有写入SDA，SDA读出来应该是一个值
	答：I2C是在通信，通信是有从机的，当主机在不断驱动SCL时钟，从机就有义务去改变SDA的电平，所以主机每次循环读SDA时。
	读取到的数据是从机控制的。这个数据正是从机要给我们发送的数据，所以时序叫做接收一个字节，如果自己写SDA再读SDA，
	那还要通信干啥，通信的时候，有些引脚之前读和之后读的电平是不一样的
  */
uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;								//定义接收的数据，并赋初值0x00，此处必须赋初值0x00，后面会用到
	MyI2C_W_SDA(1);										//接收前，主机先确保释放SDA，避免干扰从机的数据发送
	for (i = 0; i < 8; i ++)							//循环8次，主机依次接收数据的每一位
	{
		MyI2C_W_SCL(1);									//释放SCL，主机机在SCL高电平期间读取SDA
		if (MyI2C_R_SDA()){Byte |= (0x80 >> i);}		//读取SDA数据，并存储到Byte变量
														//当SDA为1时，置变量指定位为1，当SDA为0时，不做处理，指定位为默认的初值0
		MyI2C_W_SCL(0);									//拉低SCL，从机在SCL低电平期间写入SDA
	}
	return Byte;										//返回接收到的一个字节数据
}

/**
  * 函    数：I2C发送应答位
  * 参    数：Byte 要发送的应答位，范围：0~1，0表示应答，1表示非应答
  * 返 回 值：无
  *
  * 
	发送一个字节的简化版，发送一个字节是发8位，发送应答就是发1位
	函数进来时SCL低电平。主机把ACK放到SDA上
	SCL高电平，从机读取应答
	SCL低电平。进入下一个时序单元
	MyI2C_W_SDA(AckBit);
	MyI2C_W_SCL(1);
	MyI2C_W_SCL(0);
	*/
void MyI2C_SendAck(uint8_t AckBit)
{
	MyI2C_W_SDA(AckBit);					//主机把应答位数据放到SDA线
	MyI2C_W_SCL(1);							//释放SCL，从机在SCL高电平期间，读取应答位
	MyI2C_W_SCL(0);							//拉低SCL，开始下一个时序模块
}

/**
  * 函    数：I2C接收应答位
  * 参    数：无
  * 返 回 值：接收到的应答位，范围：0~1，0表示应答，1表示非应答
  * 
	接收一个字节的简化版，发送一个字节是收8位，接收应答就是收1位
	函数进来时，SCL低电平，主机释放SDA,防止干扰从机，同时，从机把应答位放在SDA上，SCL高电平，主机读取应答位，SCL低电平，进入下一个时序单元。
	uint8_t AckBit;
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	AckBit = MyI2C_R_SDA() ;
	MyI2C_W_SCL(0)；
	return AckBit;
	问：主机先把SDA置1了。再读SDA，这应答位肯定是1，这是什么意义？
	答：1：I2C的引脚是开漏加弱上拉的配置，主机输出1，并不是强制SDA为高电平。而是释放SDA
		2：I2C是正在通信，主机释放了SDA，那从机在的话，是有义务把SDA拉低的，所以即使主机把SDA置1，之后再读SDA,读到的值也有可能是0，读到0，说明从机给了应答，读到1，说明从机没给应答
  */
uint8_t MyI2C_ReceiveAck(void)
{
	uint8_t AckBit;							//定义应答位变量
	MyI2C_W_SDA(1);							//接收前，主机先确保释放SDA，避免干扰从机的数据发送
	MyI2C_W_SCL(1);							//释放SCL，主机机在SCL高电平期间读取SDA
	AckBit = MyI2C_R_SDA();					//将应答位存储到变量里
	MyI2C_W_SCL(0);							//拉低SCL，开始下一个时序模块
	return AckBit;							//返回定义应答位变量
}
/*
测试
	MyI2C_Init();
	MyI2C_Start();
	MyI2C_SendByte(0xD0);//此形参为MPU6050的地址+读写位 所以为 1101 000    0   下划线为MPU6050的地址 0为写入
	uint8_t Ack = MyI2C_ReceiveAck();//应答0  非应答1
	MyI2C_Stop();
	OLED_ShowNum(1, 1, Ack, 3);
*/

