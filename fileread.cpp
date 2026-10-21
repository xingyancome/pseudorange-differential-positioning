#include"fileread.h"
//把规则变成函数基本的，规律变成可重复的过程

//传值变量存在问题

extern int getrow(FILE* fp_nav)
{
	int row = 0;
	int flag = 0;
	char buff[MAXRINEX] = { 0 };//用来存放读取到的字符串
	char* lable = buff + 60;
	//gets函数，读取一行，当读取结束后返回NULL指针，格式如下：
	//char * fgets ( char * str, int num, FILE * stream );
	while (fgets(buff, MAXRINEX, fp_nav))
	{
		//strstr:查找字符串中的指定字符或字符串，格式如下：
		//const char * strstr ( const char * str1, const char * str2 );
		if (flag == 1)
		{
			row++;
			continue;
		}
		if (strstr(lable, "END OF HEADER"))
		{
			flag = 1;
		}
	}
	return row;
}

static double strtonum(const char* buff, int i, int n)
{
	double value = 0.0;
	char str[256] = { 0 };
	char* p = str;
	/************************************
	* 当出现以下三种情况报错，返回0.0
	* 1.起始位置<0
	* 2.读取字符串个数<i
	* 3.str里面存放的字节数<n
	*************************************/
	if (i < 0 || (int)strlen(buff) < i || (int)sizeof(str) - 1 < n)
	{
		return 0.0;
	}
	for (buff += i; *buff && --n >= 0; buff++)
	{
		//三目操作符：D和d为文件中科学计数法部分，将其转换成二进制能读懂的e
		*p++ = ((*buff == 'D' || *buff == 'd') ? 'e' : *buff);
	}
	*p = '\0';
	//三目操作符，将str中存放的数以格式化读取到value中。
	return sscanf(str, "%lf", &value) == 1 ? value : 0.0;
}

extern void readrinex_n(FILE* fp_nav, pnav_head nav_h, pnav_body nav_b, int n_n)
{
	char buff[MAXRINEX] = { 0 };
	char* lable = buff + 60;
	int i = 0;
	int j = 0;
	while (fgets(buff, MAXRINEX, fp_nav))
	{
		if (strstr(lable, "RINEX VERSION / TYPE"))
		{
			nav_h->ver = strtonum(buff, 0, 9);
			strncpy((nav_h->type), buff + 20, 15);
			continue;
		}
		else if (strstr(lable, "ION ALPHA"))
		{
			/*电离层改正参数*/
			nav_h->ION_ALPHA[0] = strtonum(buff, 3, 12);
			nav_h->ION_ALPHA[1] = strtonum(buff, 3 + 12, 12);
			nav_h->ION_ALPHA[2] = strtonum(buff, 3 + 12 + 12, 12);
			nav_h->ION_ALPHA[3] = strtonum(buff, 3 + 12 + 12 + 12, 12);
			continue;
		}
		else if (strstr(lable, "ION BETA"))
		{
			nav_h->ION_BETA[0] = strtonum(buff, 3, 12);
			nav_h->ION_BETA[1] = strtonum(buff, 3 + 12, 12);
			nav_h->ION_BETA[2] = strtonum(buff, 3 + 12 + 12, 12);
			nav_h->ION_BETA[3] = strtonum(buff, 3 + 12 + 12 + 12, 12);
			continue;
		}
		else if (strstr(lable, "DELTA-UTC: A0,A1,T,W"))
		{
			nav_h->DELTA_UTC[0] = strtonum(buff, 3, 19);
			nav_h->DELTA_UTC[1] = strtonum(buff, 3 + 19, 19);
			nav_h->DELTA_UTC[2] = strtonum(buff, 3 + 19 + 19, 9);
			nav_h->DELTA_UTC[3] = strtonum(buff, 3 + 19 + 19 + 9, 9);
			continue;
		}
		else if (strstr(lable, "LEAP SECONDS"))
		{
			nav_h->leap = (int)strtonum(buff, 6, 2);
		}
		else if (strstr(lable, "END OF HEADER"))//这时开始对数据进行读取
		{
			for (i = 0; i < (n_n / 8); i++)//n_n为不包含头的行数，除以8为数据块数量（每块是八行）
			{
				for (j = 0; j < 8; j++)
				{
					fgets(buff, MAXRINEX, fp_nav);
					switch (j)
					{
					case 0:
						nav_b[i].sPRN = (int)strtonum(buff, 0, 2);
						nav_b[i].TOC_Y = (int)strtonum(buff, 3, 2) + 2000;
						nav_b[i].TOC_M = (int)strtonum(buff, 6, 2);
						nav_b[i].TOC_D = (int)strtonum(buff, 9, 2);
						nav_b[i].TOC_H = (int)strtonum(buff, 12, 2);
						nav_b[i].TOC_Min = (int)strtonum(buff, 15, 2);
						nav_b[i].TOC_Sec = strtonum(buff, 18, 2);
						nav_b[i].sa0 = strtonum(buff, 22, 19);
						nav_b[i].sa1 = strtonum(buff, 22 + 19, 19);
						nav_b[i].sa2 = strtonum(buff, 22 + 19 + 19, 19);
						break;
					case 1:
						nav_b[i].IODE = strtonum(buff, 3, 19);
						nav_b[i].Crs = strtonum(buff, 3 + 19, 19);
						nav_b[i].deltan = strtonum(buff, 3 + 19 + 19, 19);
						nav_b[i].M0 = strtonum(buff, 3 + 19 + 19 + 19, 19);
						break;
					case 2:
						nav_b[i].Cuc = strtonum(buff, 3, 19);
						nav_b[i].e = strtonum(buff, 3 + 19, 19);
						nav_b[i].Cus = strtonum(buff, 3 + 19 + 19, 19);
						nav_b[i].sqrtA = strtonum(buff, 3 + 19 + 19 + 19, 19);
						break;
					case 3:
						nav_b[i].TOE = strtonum(buff, 3, 19);
						nav_b[i].Cic = strtonum(buff, 3 + 19, 19);
						nav_b[i].OMEGA = strtonum(buff, 3 + 19 + 19, 19);
						nav_b[i].Cis = strtonum(buff, 3 + 19 + 19 + 19, 19);
						break;
					case 4:
						nav_b[i].i0 = strtonum(buff, 3, 19);
						nav_b[i].Crc = strtonum(buff, 3 + 19, 19);
						nav_b[i].omega = strtonum(buff, 3 + 19 + 19, 19);
						nav_b[i].deltaomega = strtonum(buff, 3 + 19 + 19 + 19, 19);
						break;
					case 5:
						nav_b[i].IDOT = strtonum(buff, 3, 19);
						nav_b[i].L2code = strtonum(buff, 3 + 19, 19);
						nav_b[i].GPSweek = strtonum(buff, 3 + 19 + 19, 19);
						nav_b[i].L2Pflag = strtonum(buff, 3 + 19 + 19 + 19, 19);
						break;
					case 6:
						nav_b[i].sACC = strtonum(buff, 3, 19);
						nav_b[i].sHEA = strtonum(buff, 3 + 19, 19);
						nav_b[i].TGD = strtonum(buff, 3 + 19 + 19, 19);
						nav_b[i].IODC = strtonum(buff, 3 + 19 + 19 + 19, 19);
						break;
					case 7:
						nav_b[i].TTN = strtonum(buff, 3, 19);
						nav_b[i].fit = strtonum(buff, 3 + 19, 19);
						nav_b[i].spare1 = strtonum(buff, 3 + 19 + 19, 19);
						nav_b[i].spare2 = strtonum(buff, 3 + 19 + 19 + 19, 19);
						break;
					}
				}
			}
		}
	}
}

void setstr(char* des, const char* src, int n)
{
	char* p = des;
	const char* q = src;
	while (*q && q < src + n)
	{
		*p++ = *q++;
	}
	*p-- = '\0';
	//去掉尾部空格
	while (p >= des && *p == ' ')
	{
		*p-- = '\0';
	}
}

void get_n_data(const char* nav_name, pnav_head nav_h , pnav_body nav_b ,int &n_n_a)
{
	//数据读取
	FILE* fp_nav = NULL;//导航星历文件指针
	//pnav_head nav_h = NULL;//这是个局部变量，函数只要调用结束就会自带释放内存
	//pnav_body nav_b = NULL;
	//N文件读取
	fp_nav = fopen(nav_name, "r");//以只读的方式打开N文件
	if (!fp_nav)
	{
		printf("N文件打开失败!");
		exit(3);
	}
	else
	{
		printf("N文件打开成功!\n");
	}
	int n_n = getrow(fp_nav);//获取导航文件观测行数
	n_n_a = n_n;//计算需要
	printf("导航文件的观测值行数为：%d\n", n_n);
	rewind(fp_nav);//将文件指针返回值起始位置
	nav_h = (pnav_head)malloc(sizeof(nav_head));//给N文件头开辟空间需要配合free（）函数一起使用且只能释放一次忘记的话就会导致内存泄漏嗷
	nav_b = (pnav_body)malloc(sizeof(nav_body) * (n_n / 8));//这一块只是一个指针而已，本身并不存在实体所以一结束就自动释放掉了
	if (nav_h && nav_b)
	{
		readrinex_n(fp_nav, nav_h, nav_b, n_n);//读取数据
	}
	fclose(fp_nav);//关闭N文件
	printf("文件读取成功!\n");
}

//观测历元搞不对(就是需要先在大局观上有一个历元的完整计算过程先大体在局部理顺思路，后面的细节再逐个进行完善)
extern int get_epochnum(FILE* fp_obs)
{
	int n = 0;//记录历元数
	//int satnum = 0;//记录每个历元的卫星数（没用上）
	//char flag[3];//存放卫星标志符号'G'
	char buff[MAXRINEX];//存放读取的字符串
	int year = 0;
	//加一个跳到end of head 的操作
	char* lable = buff + 60;
	while (fgets(buff, MAXRINEX, fp_obs))
	{
		//获取当前观测值年份
		if (strstr(lable, "TIME OF FIRST OBS"))
		{
			year = (int)strtonum(buff, 0, 6);
			continue;
		}
		if (strstr(lable, "END OF HEADER"))
			break;
	}
	//年份就行18+2000一比较就知道了 逻辑是没有问题的
	/*while (fgets(buff, MAXRINEX, fp_obs))//返回的fp会自己++；
	{
		//satnum = (int)strtonum(buff, 30, 2);卫星数量的位置
		//strncpy(&flag, buff , 1);
		setstr(&flag, buff+1, 1);//buff+是起始位置
		if (flag == '1')//此处并不是长久的办法，只是因为是18年所以如此
		{
			n++;
		}
		//当卫星数超过12个时，一行存不下会转到下下一行，并且与上一行对齐
		//所以当出现这种情况时，要减1
		//if (flag == '1' && satnum > 12)
		//{
		//	n--;
		//}
	}*/
	while (fgets(buff, MAXRINEX, fp_obs))
	{
		//setstr(flag, buff , 3);//获取字符的
		if (strtonum(buff,0,3)+2000==year)
		{
			n++;
		}
	}
	return n;
}

/*可以通用的头部读取*/
extern void read_obs_h(FILE* fp_obs, pobs_head obs_h)
{
	char buff[MAXRINEX] = { 0 };
	char* lable = buff + 60;
	int i = 0;
	int j = 0;
	while (fgets(buff, MAXRINEX, fp_obs))//fp_obs会的返回自动指向下一行
	{
		if (strstr(lable, "RINEX VERSION / TYPE"))
		{
			obs_h->ver = strtonum(buff, 0, 9);
			setstr(obs_h->type, buff + 20, 30);
			continue;
		}
		else if (strstr(lable, "APPROX POSITION XYZ"))
		{
			/*接收机近似坐标获取*/
			obs_h->apX = strtonum(buff, 0, 14);
			obs_h->apY = strtonum(buff, 0 + 14, 14);
			obs_h->apZ = strtonum(buff, 0 + 14 + 14, 14);
			continue;
		}
		else if (strstr(lable, "ANTENNA: DELTA H/E/N"))
		{
			obs_h->ANTH = strtonum(buff, 0, 14);
			obs_h->ANTdeltaE = strtonum(buff, 14, 14);
			obs_h->ANTdeltaN = strtonum(buff, 14 + 14, 14);
			continue;
		}
		else if (strstr(lable, "WAVELENGTH FACT L1/2"))
		{
			obs_h->WAFL1 = (int)strtonum(buff, 0, 6);
			obs_h->WAFL2 = (int)strtonum(buff, 6, 6);
			obs_h->WAFflag = (int)strtonum(buff, 6 + 6, 6);
			continue;
		}
		else if (strstr(lable, "# / TYPES OF OBSERV"))
		{
			obs_h->obstypenum = (int)strtonum(buff, 0, 6);
			char obstype [3]= {};
			char a[3] = {'C','1','\0'};
			/*观测值类型，一行最多存9个*/
			/*确认C1在这些观测值类型中的下标位置*/
			/*他这样不能完成任务*/
			if (obs_h->obstypenum <= 9)
			{
				for (i = 0; i < obs_h->obstypenum; i++)
				{
					//setstr(&(obs_h->obstype[i]), buff + 10 * i, 2);
					setstr(obstype, buff + 10 + 6 * i, 2);//从第十个开始是第一个类型4个一个
					if (!strcmp(obstype,a))
					{
						obs_h->P_flag = i;
					}
				}
			}
			else if (obs_h->obstypenum > 9)
			{
				for (i = 0; i < 9; i++)
				{
					//setstr((obstype), buff +10*i, 3);
					//数字符要数对，找到正确的规律
					setstr(obstype, buff + 10 + 6 * i , 2);//从第十个开始是第一个类型4个一个//只获取一个字符
					if (!strcmp(obstype, a))
					{
						obs_h->P_flag = i;
					}
				}
				fgets(buff, MAXRINEX, fp_obs);
				for (i = 0; i < obs_h->obstypenum - 9; i++)
				{
					//setstr(&(obs_h->obstype[i + 9]), buff + 10 * i, 2);
					setstr(obstype, buff + 10 + 6 * i, 2);//从第十个开始是第一个类型4个一个
					if (!strcmp(obstype, a))
					{
						obs_h->P_flag = i + 9;
					}
				}
			}
			continue;
		}
		else if (strstr(lable, "INTERVAL"))
		{
			/*采样间隔*/
			obs_h->interval = strtonum(buff, 0, 11);
			continue;
		}
		else if (strstr(lable, "TIME OF FIRST OBS"))
		{
			obs_h->f_y = (int)strtonum(buff, 0, 6);
			obs_h->f_m = (int)strtonum(buff, 6, 6);
			obs_h->f_d = (int)strtonum(buff, 6 + 6, 6);
			obs_h->f_h = (int)strtonum(buff, 6 + 6 + 6, 6);
			obs_h->f_min = (int)strtonum(buff, 6 + 6 + 6 + 6, 6);
			obs_h->f_sec = strtonum(buff, 6 + 6 + 6 + 6 + 6, 6);
			setstr(obs_h->tsys, buff + 6 + 6 + 6 + 6 + 6 + 18, 3);
			continue;
		}
		else if (strstr(lable, "END OF HEADER"))
			break;
	}
}

extern void read_obs_b(FILE* fp_obs, pobs_epoch obs_e, pobs_body obs_b, int type)//p本身就是指针
{
	int n = 0;//历元数
	int i = 0;//第i颗卫星
	int j = 0;//第i颗卫星的第j行观测值
	int k = 0;//第j行第k个观测值
	char buff[MAXRINEX] = { 0 };
	char flag = { 0 };//判断符号
	while (fgets(buff, MAXRINEX, fp_obs))
	{
		//按照格式将历元参考时间依次存入，年需+2000
		//除秒以外，均转换成整型
		obs_e[n].y = (int)strtonum(buff, 1, 2) + 2000;
		obs_e[n].m = (int)strtonum(buff, 4, 2);
		obs_e[n].d = (int)strtonum(buff, 7, 2);
		obs_e[n].h = (int)strtonum(buff, 10, 2);
		obs_e[n].min = (int)strtonum(buff, 13, 2);
		obs_e[n].sec = strtonum(buff, 15, 11);
		obs_e[n].p_flag = (int)strtonum(buff, 28, 1);
		//输入卫星数
		obs_e[n].sat_num = strtonum(buff, 29, 3);
		obs_e[n].GPS_sat_num = 0;//初始化G卫星的数量（检查读取部分问题所在）
		strncpy(&flag, buff + 32, 1);//第一个
		//判断卫星数是否超过12，分类处理；且区分类型为GPS卫星这两个应该被分开的这样达不到处理效果
		/*if (obs_e[n].sat_num <= 12 && flag == 'G')
		{
			for (i = 0; i < obs_e[n].sat_num; i++)
			{
				//将卫星编号存储到第n个历元中（每个历元会有不同的观测卫星个数），第i个PRN位置（只读数字，不读'G'）
				//两个卫星编号的间隔为3个字符
				obs_e[n].sPRN[i] = strtonum(buff, 33 + 3 * i, 2);
				//obs_e[n].GPS_sat_num++;
			}
			obs_e[n].GPS_sat_num++;
		}
		//如果卫星数超过12个，读完第一行继续下一行继续执行读取卫星数
		else if (obs_e[n].sat_num >= 12 && flag == 'G')
		{
			for (i = 0; i < 12; i++)
			{
				obs_e[n].sPRN[i] = strtonum(buff, 33 + 3 * i, 2);
				//obs_e[n].GPS_sat_num++;
			}
			fgets(buff, MAXRINEX, fp_obs);
			for (i = 0; i < obs_e[n].sat_num - 12; i++)//表达式不会改变变量的值
			{
				obs_e[n].sPRN[i + 12] = strtonum(buff, 33 + 3 * i, 2);
				//obs_e[n].GPS_sat_num++;
			}
		}*/

		//区分出来GPS卫星星号及确定下标位于的位置
		if (obs_e[n].sat_num<=12)
		{
			for (int i = 0,k=0; i < obs_e[n].sat_num ; i++)
			{
				strncpy(&flag, buff + 32 + 3 * i, 1);//第一个
				if (flag=='G')
				{
					obs_e[n].sPRN[i] = strtonum(buff, 33 + 3 * i, 2);
					obs_e[n].GPS_sat_num++;
					obs_e[n].G_data_order[k] = i;
					k++;
				}
			}
		}
		else if (obs_e[n].sat_num > 12)
		{
			int k = 0;
			for (int i = 0; i < obs_e[n].sat_num; i++)
			{
				strncpy(&flag, buff + 32 + 3 * i, 1);//第一个
				if (flag == 'G')
				{
					obs_e[n].sPRN[i] = strtonum(buff, 33 + 3 * i, 2);
					obs_e[n].GPS_sat_num++;
					obs_e[n].G_data_order[k] = i;
					k++;
				}
			}
			fgets(buff, MAXRINEX, fp_obs);
			for (i = 0; i < obs_e[n].sat_num - 12; i++)//表达式不会改变变量的值
			{
				obs_e[n].sPRN[i + 12] = strtonum(buff, 33 + 3 * i, 2);
				strncpy(&flag, buff + 32 + 3 * i, 1);//第一个
				if (flag == 'G')
				{
					obs_e[n].sPRN[i+12] = strtonum(buff, 33 + 3 * i, 2);
					obs_e[n].GPS_sat_num++;
					obs_e[n].G_data_order[k] = i;
					k++;
				}
			}
			if (obs_e[n].sat_num > 24)
			{
				fgets(buff, MAXRINEX, fp_obs);
				for (i = 0; i < obs_e[n].sat_num - 24; i++)//表达式不会改变变量的值
				{
					obs_e[n].sPRN[i + 12] = strtonum(buff, 33 + 3 * i, 2);
					strncpy(&flag, buff + 32 + 3 * i, 1);//第一个
					if (flag == 'G')
					{
						obs_e[n].sPRN[i + 24] = strtonum(buff, 33 + 3 * i, 2);
						obs_e[n].GPS_sat_num++;
						obs_e[n].G_data_order[k] = i;
						k++;
					}
				}
			}
		}

		//对一块历元中的i个卫星进行数据读取（全读取）
		for (i = 0; i < obs_e[n].sat_num; i++)//同一历元的下标与历元下的卫星数据下标相同
		{
			//根据头文件读取的观测值类型判断每一颗卫星循环几行（每一行最多放五个数据）
			for (j = 0; j < (int)ceil(type / 5.0); j++)//ceil（X），返回比X大的最小整数
			{
				int count = type / 5;//计数
				fgets(buff, MAXRINEX, fp_obs);
				//依次读取一行中的观测值
				//这里加个三目操作符判断一下，是不是到了最后一行，如果到了最后一行则不进行整行读取
				//此处可以加上一个判断函数用以确定伪距观测值的位置/找出伪距观测值所存储的下标号
				for (k = 0; j == count ? k < type % 5 : k < 5; k++)
				{
					obs_b[n].obs[i][k + 5 * j] = strtonum(buff, 16 * k, 16);//访问越界了
					//cout << obs_b[n].obs[i][k + 5 * j] << "   ";//检验数据录入是否成功
				}
			}
		}
		n++;
	}
}

void get_o_data(const char* obs_name, pobs_head obs_h , pobs_epoch obs_e , pobs_body obs_b, int &o_epochnum_a)//需要注意保存指针和释放空间(即分配的存储指针应使用全局变量或者参数传入)
{
	//数据读取
	FILE* fp_obs = NULL;
	//O文件读取
	fp_obs = fopen(obs_name, "r");//以只读的方式打开O文件
	if (!fp_obs)
	{
		printf("O文件打开失败!");
		exit(3);
	}
	else
	{
		printf("O文件打开成功!\n");
	}
	int o_epochnum = get_epochnum(fp_obs);//获取O文件历元数
	o_epochnum_a = o_epochnum;
	printf("观测值文件的历元数为：%d\n", o_epochnum);
	rewind(fp_obs);
	obs_h = (pobs_head)malloc(sizeof(obs_head));//给O文件头开辟空间
	obs_e = (pobs_epoch)malloc(sizeof(obs_epoch) * o_epochnum);
	obs_b = (pobs_body)malloc(sizeof(obs_body) * o_epochnum);
	if (obs_h && obs_e && obs_b)
	{
		read_obs_h(fp_obs, obs_h);//读取O文件头
		read_obs_b(fp_obs, obs_e, obs_b, obs_h->obstypenum);//读取O文件数据块
	}
	fclose(fp_obs);//关闭O文件
	printf("O文件读取成功！\n");
}


//直接调用就可以
void read_file(const char* nav_name, const char* obs_name, pnav_head nav_h, pnav_body nav_b, pobs_head obs_h, pobs_epoch obs_e, pobs_body obs_b, int& o_epochnum_a, int& n_n_a)
{
	get_n_data(nav_name, nav_h, nav_b,n_n_a);/*检验输入的数据是不正确的*///这一步走完直接释放掉了
	get_o_data(obs_name, obs_h, obs_e, obs_b, o_epochnum_a);
}

void free_data(pnav_head nav_h, pnav_body nav_b, pobs_head obs_h, pobs_epoch obs_e, pobs_body obs_b)
{
	free(nav_h), free(nav_b), free(obs_h), free(obs_e), free(obs_b);
	nav_h = NULL, nav_b = NULL, obs_b = NULL, obs_e = NULL, obs_h = NULL;
}



/*inverse of matrix 求逆矩阵*/
extern int  matinv(double** A, int n)
{
	//n  维数
	//A  矩阵
	double tmp1, tmp2, b_tmp[40], c_tmp[40];
	int k, k1, k2, k3, j, i, j1, i1, kme[40], kmf[40];
	i1 = j1 = 0;

	for (k = 0; k < n; k++)
	{
		tmp2 = 0.0;
		for (i = k; i < n; i++)
		{
			for (j = k; j < n; j++)
			{
				if (fabs(A[i][j]) <= fabs(tmp2))
					continue;
				tmp2 = A[i][j];
				i1 = i;
				j1 = j;
			}
		}
		if (i1 != k)
		{
			for (j = 0; j < n; j++)
			{
				tmp1 = A[i1][j];
				A[i1][j] = A[k][j];
				A[k][j] = tmp1;
			}
		}
		if (j1 != k)
		{
			for (i = 0; i < n; i++)
			{
				tmp1 = A[i][j1];//在此处出现错误
				A[i][j1] = A[i][k];
				A[i][k] = tmp1;
			}
		}
		kme[k] = i1;
		kmf[k] = j1;
		for (j = 0; j < n; j++)
		{
			if (j == k)
			{
				b_tmp[j] = 1.0 / tmp2;
				c_tmp[j] = 1.0;
			}
			else
			{
				b_tmp[j] = -A[k][j] / tmp2;
				c_tmp[j] = A[j][k];
			}
			A[k][j] = 0.0;
			A[j][k] = 0.0;
		}
		for (i = 0; i < n; i++)
		{
			for (j = 0; j < n; j++)
			{
				A[i][j] = A[i][j] + c_tmp[i] * b_tmp[j];
			}
		}
	}
	for (k3 = 0; k3 < n; k3++)
	{
		k = n - k3 - 1;
		k1 = kme[k];
		k2 = kmf[k];
		if (k1 != k)
		{
			for (i = 0; i < n; i++)
			{
				tmp1 = A[i][k1];
				A[i][k1] = A[i][k];
				A[i][k] = tmp1;
			}
		}
		if (k2 != k)
		{
			for (j = 0; j < n; j++)
			{
				tmp1 = A[k2][j];
				A[k2][j] = A[k][j];
				A[k][j] = tmp1;
			}
		}
	}
	return (0);
}


//multiply matrix 矩阵乘法
/*
   a_matrix=b_matrix*c_matrix
   krow  :行数
   kline :列数
*/
extern double matmul(double** a_matrix, double** b_matrix, double** c_matrix, int krow, int kline, int kmiddle)
{
	int j, j1, j2;
	double stmp;

	for (j = 0; j < krow; j++)
	{
		for (j1 = 0; j1 < kline; j1++)
		{
			stmp = 0.0;
			for (j2 = 0; j2 < kmiddle; j2++)
			{
				stmp += b_matrix[j][j2] * c_matrix[j2][j1];
			}
			a_matrix[j][j1] = stmp;
		}
	}
	return (0);

}



extern double Time2GPSweek(int year, int month, int day, int hour, int minute, double second)//o文件观测历元时刻转换为GPS周秒
{
	int DayofYear = 0;
	int DayofMonth = 0;
	int secOfweek;

	for (int i = 1980; i < year; i++)//从1980年到当前年的上一年经过的天数
	{
		if ((i % 4 == 0 && i % 100 != 0) || i % 400 == 0)
			DayofYear += 366;
		else
			DayofYear += 365;
	}
	for (int i = 1; i < month; i++)//从一月到当前月的上一月经历的天数
	{
		if (i == 1 || i == 3 || i == 5 || i == 7 || i == 8 || i == 10 || i == 12)
			DayofMonth += 31;
		else if (i == 4 || i == 6 || i == 9 || i == 11)
			DayofMonth += 30;
		else
		{
			if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
				DayofMonth += 29;
			else
				DayofMonth += 28;
		}
	}
	int Day;
	Day = DayofMonth + day + DayofYear - 6;
	secOfweek = (double)(Day % 7) * 86400 + (double)(hour) * 3600 + (double)(minute) * 60 + second;
	return secOfweek;
}

extern int seleph(double sec, int sPRN, struct nav_body* nav_b, int n, double t)//选择观测值对应的星历
{
	int i = 0, n_poch = 0;
	double d_value = 7200;
	for (i = 0; i < (n / 8); i++)
	{
		double 	D_value = fabs(sec - t - nav_b[i].TOE);
		if ((sPRN == nav_b[i].sPRN) && (D_value <= d_value))
		{
			n_poch = i;
			d_value = D_value;
		}
	}
	return n_poch;
}

//卫星PRN编号对比确定当前卫星观测值在DGPS_data当前历元中的下标位置
//m当前历元标志，j基准站当前历元可见卫星总数
bool selePRN(int obs_PRN, struct DGPS_data* DGPS_data, int j, int m, int& j_b)//m当前历元标志，j基准站当前历元可见卫星总数
{
	bool R_b = 0;
	j_b = 0;//当前历元下对应卫星的伪距观测值的位置（与卫星星号下标一致）
	for (int i = 0; i < j; i++)
	{
		if (obs_PRN == DGPS_data[m].GsPRN[j_b])
		{
			R_b = true;
			break;
		}
		j_b++;
	}
	if (!R_b)
	{
		return false;
		//继续下一颗卫星
		// Gnum--;
		//continue;
	}
	else
	{
		return true;
	}
}

//确定基准站改正数据历元（未考虑采样间隔、未考虑无法查找到可用的改正历元）
bool selepoch_basic(int GPStime_m, struct DGPS_data* DGPS_data, int m, int& m_b)//(m为基准站的所有观测历元)
{
	//m_b = 0;//确定的历元下标
	bool a = 0;//没找到
	for (int i = 0; i < m; i++)
	{
		if (GPStime_m == DGPS_data[m_b].GPStime)
		{
			a = 1;
			break;
		}
		m_b++;//此处有bug（m_b会一直++）当等于0时没有跳出的bug串逻辑没串好
	}
	if (!a)
	{
		return false;
	}
	else
	{
		return true;
	}
}