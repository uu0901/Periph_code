#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
#include "MPU6050_Reg.h"

#define MPU6050_ADDRESS		0xD0		//MPU6050的I2C从机地址

/**
  * 函    数：MPU6050写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  *
  * 指定地址写多个字节   ？？？？在最后的MyI2C_SendBye(Data); MyI2C_ReceiveAck();用for循环套起来，多执行几遍，然后依次把数组的各个字节发送出去
  */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(MPU6050_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址 
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(Data);				//发送要写入寄存器的数据
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_Stop();						//I2C终止
}
/*
	MPU6050Init();								//此函数只有MyI2C_Init();
	MPU6050_WriteReg(0x6B , 0x00);				//在电源管理器1 0x6B地址下写入0x00，解除Bit6的Sleep位，退出睡眠模式，写入其他的寄存器才有效
	MPU6050_WriteReg(0x19 , 0xAA);				//在采样率分频寄存器0x19地址下写入0xAA，
	uint8_t ID = MPU6050_ReadReg(0x19);			//读取采样率分频寄存器0x19地址下的数据
	OLED_ShowHexNum(1,1, ID, 2);
	寄存器也是一种存储器，只不过普通的存储器只能写和读，里面的数据并没有赋予什么实际意义，但是寄存器就不一样了，
	寄存器的每一位数据，都对应了硬件电路的状态，寄存器和外设的硬件电路，是可以进行互动的，所以，可以通过寄存器来控制电路
*/
/**
  * 函    数：MPU6050读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  * 
	在最后需要主机发送一个应答，参数给1就是不给从机应答，
	如果要读多个字节，就要给从机应答0，不想继续读了就要给从机非应答1，主机收回总线的控制权，防止之后进入从机一位你还想要，但实际不想要的冲突状态
	MPU6050_ReadReg(uint8_t RegAddress);
  */
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(MPU6050_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	
	MyI2C_Start();						//I2C重复起始
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);	//发送从机地址，读写位为1，表示即将读取
	MyI2C_ReceiveAck();					//接收应答
	Data = MyI2C_ReceiveByte();			//接收指定寄存器的数据
	MyI2C_SendAck(1);					//发送应答，给从机非应答，终止从机的数据输出
	MyI2C_Stop();						//I2C终止
	
	return Data;
}
/*
	MPU6050Init();							//此函数只有MyI2C_Init();
	uint8_t ID = MPU6050_ReadReg(0x75);		//读取WHO_AM_I 0x75地址,返回芯片ID号
	OLED_ShowHexNum(1,1, ID, 2);			//ID = 0x68*/
/**
  * 函    数：MPU6050初始化
  * 参    数：无
  * 返 回 值：无
  */
void MPU6050_Init(void)
{
	MyI2C_Init();									//先初始化底层的I2C,把底层初始化一下，有点像类的继承
	/*还要再写入一些寄存器，对MPU6050硬件电路进行初始化配置--解除睡眠，选择陀螺仪时钟，6个轴均不待机，采样分频为10，滤波参数给最大，陀螺仪和加速度计都选择最大量程，
	配置完成之后，陀螺仪内部就在连续不断地进行数据转换了，输出的数据 就放在数据寄存器里了*/

	/*MPU6050寄存器初始化，需要对照MPU6050手册的寄存器描述配置，此处仅配置了部分重要的寄存器*/
	//配置电源管理寄存器1   设备复位给0不复位        睡眠模式给0 解除睡眠    循环模式给0 不需要循环    无关位 给0    温度传感器失能    给0不失能    最后3位选择时钟 给000 选择内部时钟    但是它非常建议我们选择陀螺仪时钟 所以给个001，选择x轴的陀螺仪时钟 选择哪个时钟 影响并不大
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);		//电源管理寄存器1，取消睡眠模式，选择时钟源为X轴陀螺仪.
	//配置电源管理寄存器2    前两位 循环模式唤醒频率 给00 不需要        后6位 每一个轴的待机位 全部给0 不需要待机
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);		//电源管理寄存器2，保持默认值0，所有轴均不待机
	//配置采样率分频寄存器    这8位决定了数据输出的快慢    值越小越快，可以根据实际的需求来 给个0x09 也就是10分频
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);		//采样率分频寄存器，配置采样率
	//配置配置寄存器 前5位外部同步 全都给0 不需要        后3位数字低通滤波器 根据需求来 可以给个110 这就是最平滑的滤波	
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);			//配置寄存器，配置DLPF
	//配置陀螺仪配置寄存器        前面3位是自测使能 我们就不自测了 给000     满量程选择 根据实际需求来 给11 选择最大量程        后面3位无关位给000
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);	//陀螺仪配置寄存器，选择满量程为±2000°/s
	//配置加速度计配置寄存器    前面3位是自测使能 我们就不自测了 给000    满量程选择 根据实际需求来 给11 选择最大量程        最后3位高通滤波器 我们用不到 给000
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);	//加速度计配置寄存器，选择满量程为±16g
	//解除睡眠 选择陀螺仪时钟
    //6个轴均不待机
    //采样分频为10
    //滤波参数给最大
    //陀螺仪和加速度计都选择最大量程
}

/**
  * 函    数：MPU6050获取ID号
  * 参    数：无
  * 返 回 值：MPU6050的ID号
  */
uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);		//返回WHO_AM_I寄存器的值
}

/**
  * 函    数：MPU6050获取数据
  * 参    数：AccX AccY AccZ 加速度计X、Y、Z轴的数据，使用输出参数的形式返回，范围：-32768~32767
  * 参    数：GyroX GyroY GyroZ 陀螺仪X、Y、Z轴的数据，使用输出参数的形式返回，范围：-32768~32767
  * 返 回 值：无
  */
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	/*
	这6个参数 均是int16_t的指针类型 之后在主函数里定义变量 通过指针，把主函数变量的地址传递到子函数来，子函数中，通过传递过来的地址，操作主函数的变量，
	这样子函数结束之后，主函数变量的值，就是子函数想要返回的值
    子函数中，想要获取数据，就通过ReadReg函数，读取数据寄存器
    分别读取6个轴数据寄存器的高位和低位，拼接成16位的数据，在通过指针变量返回，我们是用读取一个寄存器的函数，连续调用了12次，菜读取完12个寄存器
    还有一种更高效的方法，I2C读取多个字节的时序，从一个基地址开始，连续读取一片的寄存器，因为我们这个寄存器的地址是连续的，所以可以从第一个寄存器的地址0X3B开始，
	连续读取14个字节，这样就可以一次性的吗，把加速度值、陀螺仪值、2个字节的温度值都读出来的，这样在时序上，读取的效率就会大大提升
	*/
	uint8_t DataH, DataL;								//定义数据高8位和低8位的变量
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);		//读取加速度计X轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);		//读取加速度计X轴的低8位数据
	*AccX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	/*
	问：DataH是8位的数据，它再左移8位，会不会出问题；答：没问题，因为最终赋值的变量是16位的，
	所以8位数据左移之后，会自动进行类型转换，移出去的位并不会丢失，若不放心，可以把两个数据改成16位的
    这个16位数据是一个用补码表示的有符号数，所以最终直接赋值给int16_t 也是没问题的
	*/
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);		//读取加速度计Y轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);		//读取加速度计Y轴的低8位数据
	*AccY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);		//读取加速度计Z轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);		//读取加速度计Z轴的低8位数据
	*AccZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);		//读取陀螺仪X轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);		//读取陀螺仪X轴的低8位数据
	*GyroX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);		//读取陀螺仪Y轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);		//读取陀螺仪Y轴的低8位数据
	*GyroY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);		//读取陀螺仪Z轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);		//读取陀螺仪Z轴的低8位数据
	*GyroZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	/*
	分别读取6个轴数据寄存器的高位和低位，拼接成16位的数据 再通过指针变量返回
	这里 使用的是读取一个寄存器的函数
	连续调用了12次才取完12个寄存器
	更高效的方法 使用I2C读取多个字节的时序 
	从一个基地址开始，连续读取一片的寄存器 因为寄存器的地址是连续的 所以可以从第一个寄存器的地址0X3B开始
	连续读取14个字节 这样可以一次性的 把加速度值 陀螺仪值  两个字节的温度值 都读出来了
	在时序上 读取效率就会大大提升
	*/
}
