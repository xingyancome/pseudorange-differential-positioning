#pragma once
#define _CRT_SECURE_NO_WARNINGS //vs环境下,必须定义该宏,否则fopen函数报错(文件读写的非安全版本)
#include <iomanip>
#include"malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
using namespace std;


/*宏定义*/
#define MAXRNXLEN   250   //rinex数据读取最大长度
#define C_V         299792458  //光速（m/s）
#define GM          398600500000000 //地心引力常数 
#define omega_e     7.2921151467e-5//地球自转角速度  
#define PI          3.141592653589793
#define _USE_MATH_DEFINES


#define MAXRINEX 100//一行最大字符数（大于80均可）
/*调试是必不可少的部分*//*依据o文件来筛选星历，确定使用的星历，确定相应的卫星位置，然后进行接收机的位置迭代计算*/

			  
			  /*****************************数据块*****************************/

//自己弄弄这个读取星历是通用的同一天之内所有测站都一样

/*版本不同拿到的输入方法是不同的*/
/*N文件版本RINEX2.1.1*/
// 导航头文件结构体
typedef struct nav_head
{
	double ver;//rinex 版本号
	char type[20];//读取的数据类型
	double ION_ALPHA[4];//8个电离层参数
	double ION_BETA[4];
	double DELTA_UTC[4];
	int leap;
}nav_head, * pnav_head;

//导航数据结构体
typedef struct nav_body
{
	//数据块第一行内容：
	int sPRN;//卫星PRN号
	//历元：TOC中卫星钟的参考时刻
	int TOC_Y;//年
	int TOC_M;//月
	int TOC_D;//日
	int TOC_H;//时
	int TOC_Min;//分
	int TOC_Sec;//秒
	double sa0;//卫星钟差
	double sa1;//卫星钟偏
	double sa2;//卫星钟漂

	//数据块第二行内容：
	double IODE;//数据、星历发布时间(数据期龄)
	double Crs;//轨道半径的正弦调和改正项的振幅（单位：m）
	double deltan;//卫星平均运动速率与计算值之差(rad/s)
	double M0;//参考时间的平近点角(rad)

	//数据块第三行内容：
	double Cuc;//维度幅角的余弦调和改正项的振幅(rad)
	double e;//轨道偏心率
	double Cus;//轨道幅角的正弦调和改正项的振幅(rad)
	double sqrtA;//长半轴平方根

	//数据块第四行内容：
	double TOE;//星历的参考时刻(GPS周内秒)
	double Cic;//轨道倾角的余弦调和改正项的振幅(rad)
	double OMEGA;//参考时刻的升交点赤经
	double Cis;//维度倾角的正弦调和改正项的振幅(rad)

	//数据块第五行内容：
	double i0;//参考时间的轨道倾角(rad)
	double Crc;//轨道平径的余弦调和改正项的振幅(m)
	double omega;//近地点角距
	double deltaomega;//升交点赤经变化率(rad)

	//数据块第六行内容：
	double IDOT;//近地点角距(rad/s)
	double L2code;//L2上的码
	double GPSweek;//GPS周,于TOE一同表示
	double L2Pflag;//L2,p码数据标记

	//数据块第七行内容
	double sACC;//卫星精度
	double sHEA;//卫星健康状态
	double TGD;//sec
	double IODC;//钟的数据龄期

	//数据块第八行内容
	double TTN;//电文发送时间
	double fit;//拟合区间
	double spare1;//空
	double spare2;//空

}nav_body, * pnav_body;


/*O文件版本RINEX2.1.1*/
//观测值头文件，一个o文件就一次
typedef struct obs_head
{
	double ver;//RINEX文件版本号
	char type[30];//文件类型
	double apX;//测站近似位置XYZ（WGS84）
	double apY;
	double apZ;
	double ANTH;//天线高
	double ANTdeltaE;//天线中心对于测站标志在东方向上的偏移量
	double ANTdeltaN;//天线中心对于测站标志在北方向上的偏移量
	int WAFL1;//缺省的L1载波的波长因子
	int WAFL2;//缺省的L2载波的波长因子
	int WAFflag;
	int obstypenum;//观测值类型数量
	//作为局部变量在头文件读取中利用即可
	//char obstype[3];//rinex2.0观测值类型列表(用于筛选查找比较确认伪距观测值是哪一个，枚举法没有必要找到伪距就可以了(等于是在枚举了)
	int P_flag;//伪距观测值的数据下标
	double interval;//观测值的历元间隔
	int f_y;//第一个观测记录的时刻
	int f_m;
	int f_d;
	int f_h;
	int f_min;
	double f_sec;
	char tsys[5];//时间系统
}obs_head, * pobs_head;

//观测值历元数据结构体
typedef struct obs_epoch
{
	//观测历元时刻
	int y;
	int m;
	int d;
	int h;
	int min;
	double sec;
	int pseudorange_flag;//记录伪距观测值的位置
	int p_flag;//历元标志
	int sat_num;//当前历元所观测到的卫星数量
	int GPS_sat_num;//GPS卫星的数量
	int sPRN[24];//当前历元所能观测到的卫星的PRN列表
	int G_data_order[24];//当前卫星观测数据的下标（上下一一对应），obs[][]第一个括号对应的内容
}obs_epoch, * pobs_epoch;



//观测值数据结构体
//需要特别注意的地方：obs[][]两个中括号的对应数据的匹配问题
typedef struct obs_body
{
	//int P_flag;//伪距标签的位置//obs[][]第二个括号对应的内容（处理各地数据类型编号不同问题）
	double obs[40][20];//观测值24对应同时观测到的最大卫星数（可能有其他的卫星溢出问题），15对应着15种类型的观测值
}obs_body, * pobs_body;


//基准站传递的差分数据
struct DGPS_data
{
	int year;//观测历元时刻
	int month;
	int day;
	int hour;
	int min;
	double sec;
	double GPStime;
	double apX;//接收机真实位置
	double apY;
	double apZ;
	int p_flag;//历元标志(可以用于判断相同历元，并返回历元标志)
	int sat_num;//当前历元所观测到的GPS卫星数
	int GsPRN[20];//当前历元所观测到卫星的PRN列表
	double P[40];//当前历元的伪距观测值
	double dP[20];//当前历元所观测到卫星的PRN列表对应的伪距改正值
	double dt0;//当前基准站的接收机钟差
};
/*****************************读取功能******************************/

/*N文件*/

//获取文件数据块行数，从END OF HEADER后开始起算
extern int getrow(FILE* fp_nav);
//将字符串转换为浮点数,i起始位置，n输入多少个字符
static double strtonum(const char* buff, int i, int n);
//读取N文件
extern void readrinex_n(FILE* fp_nav, pnav_head nav_h, pnav_body nav_b, int n_n);
//开辟内存保存数据（都是局部的所以会报错）
void get_n_data(const char* nav_name, pnav_head nav_h , pnav_body nav_b, int& n_n_a);

/*O文件*/

//获取O文件历元数返回历元个数
extern int get_epochnum(FILE* fp_obs);
//读取O文件数据头
extern void read_obs_h(FILE* fp_obs, pobs_head obs_h);
//读取O文件数据块
extern void read_obs_b(FILE* fp_obs, pobs_epoch obs_e, pobs_body obs_b, int type);
//开辟内存保存数据
void get_o_data(const char* obs_name, pobs_head obs_h, pobs_epoch obs_e, pobs_body obs_b, int& o_epochnum_a);

//需要自行创建五个部分的结构体指针，并将其初始化为空（最后记得释放malloc分配的空间）
void read_file(const char* nav_name, const char* obs_name, pnav_head nav_h, pnav_body nav_b, pobs_head obs_h, pobs_epoch obs_e, pobs_body obs_b, int& o_epochnum_a, int& n_n_a);
//释放分配内存空间
void free_data(pnav_head nav_h, pnav_body nav_b, pobs_head obs_h, pobs_epoch obs_e, pobs_body obs_b);


/************************************矩阵计算************************************/

//obj 文件中不可以存在定义行为只能做声明

//存放矩阵以及相关矩阵运算
//有矩阵类就好办了代码可以更加简洁


/*inverse of matrix 求逆矩阵*/
extern int  matinv(double** A, int n);


//multiply matrix 矩阵乘法
/*
   a_matrix=b_matrix*c_matrix
   krow  :行数
   kline :列数
*/
extern double matmul(double** a_matrix, double** b_matrix, double** c_matrix, int krow, int kline, int kmiddle);




/*******************************计算信息储存块***********************************/

/*存储当前观测历元需要的每颗卫星单独的计算数据，需要根据伪距观测文件确定当前历元观测的卫星个数来确定大小*/
typedef struct//存储卫星计算相关的信息
{
	double GPS_week;//GPS周数
	double GPS_win;//GPS周内秒
	double X;//该卫星的位置WGS-84
	double Y;
	double Z;
	double CA;//相应的伪距观测值
	double dts;//卫星钟差
	double E;//卫星俯仰角
	double A;//卫星方位角
	double dD;//相对论效应改正数
	double dPs;//同时历元下相同卫星的伪距差分量
}satposition, * psatposition;


/*不同的历元数会产生不同的接收机位置，需要取平均值即可*/
typedef struct//存储接收机坐标、改正数
{
	double x0;/*WGS-84坐标*/
	double y0;
	double z0;
	double dx;/*坐标改正数*/
	double dy;
	double dz;
	double dtc;/*接收机钟差*/
	double B;//维度
	double L;//经度
	double H;//高
}Receiverposition, * pReceiverposition;


/*********************************单个历元计算功能*************************************/

//o文件观测历元时刻转换为GPS周秒
extern double Time2GPSweek(int year, int month, int day, int hour, int minute, double second);

extern int seleph(double sec, int sPRN, struct nav_body* nav_b, int n, double t);

//卫星PRN编号对比确定当前卫星观测值在DGPS_data当前历元中的下标位置
//m当前历元标志，j基准站当前历元可见卫星总数
bool selePRN(int obs_PRN, struct DGPS_data* DGPS_data, int j, int m, int& j_b);

//确定基准站改正数据历元（未考虑采样间隔、未考虑无法查找到可用的改正历元）
bool selepoch_basic(int GPStime_m, struct DGPS_data* DGPS_data, int m, int& m_b);//(m为基准站的所有观测历元)
