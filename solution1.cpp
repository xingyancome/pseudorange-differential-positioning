// solution1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include"fileread.h"//头文件即是声明，有了它会自己去找的这就是链接器的作用Linking


int main()
{
	FILE* fp_nav = NULL;//导航星历文件指针
	FILE* fp_obs = NULL;
    char nav_file[20] = {}, obs_file[20] = {};
    //定义输入数据指针
    pnav_head nav_h = NULL; pnav_body nav_b = NULL; pobs_head obs_h = NULL; pobs_epoch obs_p = NULL; pobs_body obs_b = NULL;

    int o_pochnum=0, nn=0;

    //read_file(nav_file,obs_file,nav_h,nav_b,obs_h,obs_p,obs_b,o_pochnum,nn);//"bjfs0010.12n""bjfs0010.12o"//局部变量用完就是释放了你就无了这就是问题所在
    
    
    //基准站：

	//文件获取
	cout << "请输入nav文件与基准站obs文件名：" ;
	cin >> nav_file >> obs_file;
	//read_file(nav_file, obs_file, nav_h, nav_b, obs_h, obs_p, obs_b, o_pochnum, nn);

	//N文件读取
	fp_nav = fopen(nav_file, "r");//以只读的方式打开N文件
	if (!fp_nav)
	{
		printf("N文件打开失败!");
		exit(3);
	}
	else
	{
		printf("N文件打开成功!\n");
	}
	nn = getrow(fp_nav);//获取导航文件观测行数
	printf("导航文件的观测值行数为：%d\n", nn);
	rewind(fp_nav);//将文件指针返回值起始位置
	nav_h = (pnav_head)malloc(sizeof(nav_head));//给N文件头开辟空间需要配合free（）函数一起使用且只能释放一次忘记的话就会导致内存泄漏嗷
	nav_b = (pnav_body)malloc(sizeof(nav_body) * (nn / 8));//这一块只是一个指针而已，本身并不存在实体所以一结束就自动释放掉了
	if (nav_h && nav_b)
	{
		readrinex_n(fp_nav, nav_h, nav_b, nn);//读取数据
	}
	fclose(fp_nav);//关闭N文件
	printf("N文件读取成功!\n");
	
	//O文件读取
	fp_obs = fopen(obs_file, "r");//以只读的方式打开O文件
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
	o_pochnum = o_epochnum;
	printf("观测值文件的历元数为：%d\n", o_epochnum);
	rewind(fp_obs);
	obs_h = (pobs_head)malloc(sizeof(obs_head));//给O文件头开辟空间
	obs_p = (pobs_epoch)malloc(sizeof(obs_epoch) * (o_epochnum));
	obs_b = (pobs_body)malloc(sizeof(obs_body) * (o_epochnum));
	if (obs_h && obs_p && obs_b)
	{
		read_obs_h(fp_obs, obs_h);//读取O文件头(fp_obs)会自己向下，继续指向当前的行
		read_obs_b(fp_obs, obs_p, obs_b, obs_h->obstypenum);//读取O文件数据块
	}
	fclose(fp_obs);//关闭O文件
	printf("O文件读取成功！\n");

	//获取基准站计算数据(加上防呆设计，跳过计算历元等操作也要有，舍弃不健康数据、错误的计算数据)
	DGPS_data* dgps_d = NULL;
	dgps_d = (struct DGPS_data*)malloc(sizeof(struct DGPS_data) * o_pochnum);//这样分配只能看到分配的第一个
	//dgps_b = new DGPS_data[o_pochnum_b];
	double b_apX, b_apY, b_apZ;
	cout << "请输入基准站的已知坐标apX、apY、apZ：";
	cin >> b_apX >> b_apY >> b_apZ;
	//存在某些怪异问题，剔除、跳出异常计算情形

	//计算部分
	double** delta_t;//信号传播时间
	double deltat = 0;//判断收敛条件所用
	double** T_S;//信号发射时刻
	double** X_S;//卫星位置
	double** Y_S;
	double** Z_S;
	double delta_tr, Epsilon_ts, delta_ts;//卫星时钟误差所用参数
	double** G0;//卫星有关的几何矩阵
	double** G1;
	double** G2;
	double** G3;
	double** b_Matrix;//矩阵b
	int i, j, m = 0;
	int Gnum;
	double H, A, L;//电离层延时所用参数
	double tk, n0, n, M, E, v, u_, Epsilon_u, Epsilon_r, Epsilon_i, uk, rk, ik, x, y, omega_k, X, Y, Z, R;//计算卫星位置所用参数
	double X0, Y0, Z0;//卫星在以测站为原点的站心坐标系中的位置
	double az, el;//卫星方位角和高度角
	double f, psi, phi, lam, amp, per, I, T;
	double** Pc;
	double** N;//矩阵部分
	double** B_Matrix;
	double** V;
	double** hH;
	double** S;
	double** S_;
	double** HH;
	double* X_R;//待定点坐标和接收机钟差
	double X_sim;//判断收敛条件所用
	double* Y_R;
	double Y_sim;
	double* Z_R;
	double Z_sim;
	double* deltaTR;
	double deltaTR_sim;
	double* PDOP;
	double* VDOP;
	double* HDOP;

	X_R = (double*)malloc((sizeof(double)) * o_pochnum);
	Y_R = (double*)malloc((sizeof(double)) * o_pochnum);
	Z_R = (double*)malloc((sizeof(double)) * o_pochnum);
	deltaTR = (double*)malloc((sizeof(double)) * o_pochnum);

	for (i = 0; i < o_pochnum; i++)//给各个历元定待定点的初始坐标和初始接收机钟差，都设为0
	{
		X_R[i] = 0;
	}
	for (i = 0; i < o_pochnum; i++)
	{
		Y_R[i] = 0;
	}
	for (i = 0; i < o_pochnum; i++)
	{
		Z_R[i] = 0;
	}
	for (i = 0; i < o_pochnum; i++)
	{
		deltaTR[i] = 0;
	}


	//野指针问题，没有释放内存(内存泄露问题)
	delta_t = (double**)malloc((sizeof(double*)) * o_pochnum);//给delta_t分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		delta_t[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	T_S = (double**)malloc(sizeof(double*) * o_pochnum);//给T_S分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		T_S[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	X_S = (double**)malloc(sizeof(double*) * o_pochnum);//给X_S分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		X_S[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	Y_S = (double**)malloc(sizeof(double*) * o_pochnum);//给Y_S分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		Y_S[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	Z_S = (double**)malloc(sizeof(double*) * o_pochnum);//给Z_S分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		Z_S[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	G0 = (double**)malloc(sizeof(double*) * o_pochnum);//给G0分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		G0[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	G1 = (double**)malloc(sizeof(double*) * o_pochnum);//给G1分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		G1[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	G2 = (double**)malloc(sizeof(double*) * o_pochnum);//给G2分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		G2[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	G3 = (double**)malloc(sizeof(double*) * o_pochnum);//给G3分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		G3[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	b_Matrix = (double**)malloc(sizeof(double*) * o_pochnum);//给L分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		b_Matrix[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	Pc = (double**)malloc(sizeof(double*) * o_pochnum);
	for (i = 0; i < o_pochnum; i++)
	{
		Pc[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	N = (double**)malloc(sizeof(double*) * 4);//给N分配内存
	for (i = 0; i < 4; i++)
	{
		N[i] = (double*)malloc((sizeof(double)) * 4);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			N[i][j] = 0;
		}
	}

	V = (double**)malloc(sizeof(double*) * 4);//给V分配内存
	for (i = 0; i < 4; i++)
	{
		V[i] = (double*)malloc((sizeof(double)) * 1);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 1; j++)
		{
			V[i][j] = 0;
		}
	}

	B_Matrix = (double**)malloc(sizeof(double*) * 4);//给BTL分配内存
	for (i = 0; i < 4; i++)
	{
		B_Matrix[i] = (double*)malloc((sizeof(double)) * 1);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 1; j++)
		{
			B_Matrix[i][j] = 0;
		}
	}

	hH = (double**)malloc(sizeof(double*) * 4);//给BTL分配内存
	for (i = 0; i < 4; i++)
	{
		hH[i] = (double*)malloc((sizeof(double)) * 1);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 1; j++)
		{
			hH[i][j] = 0;
		}
	}

	S = (double**)malloc(sizeof(double*) * 4);//给BTL分配内存
	for (i = 0; i < 4; i++)
	{
		S[i] = (double*)malloc((sizeof(double)) * 1);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 1; j++)
		{
			S[i][j] = 0;
		}
	}


	S_ = (double**)malloc(sizeof(double*) * 4);//给BTL分配内存
	for (i = 0; i < 4; i++)
	{
		S_[i] = (double*)malloc((sizeof(double)) * 1);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 1; j++)
		{
			S_[i][j] = 0;
		}
	}

	HH = (double**)malloc(sizeof(double*) * 4);//给BTL分配内存
	for (i = 0; i < 4; i++)
	{
		HH[i] = (double*)malloc((sizeof(double)) * 1);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 1; j++)
		{
			HH[i][j] = 0;
		}
	}


	PDOP = (double*)malloc((sizeof(double)) * o_pochnum);
	VDOP = (double*)malloc((sizeof(double)) * o_pochnum);
	HDOP = (double*)malloc((sizeof(double)) * o_pochnum);
	for (i = 0; i < o_pochnum; i++)
	{
		PDOP[i] = 0;
	}

	for (i = 0; i < o_pochnum; i++)
	{
		VDOP[i] = 0;
	}

	for (i = 0; i < o_pochnum; i++)
	{
		HDOP[i] = 0;
	}

	//计算开始
	for (m = 0; m < o_pochnum; m++)
	{

		X_sim = 1.0;
		Y_sim = 1.0;
		Z_sim = 1.0;
		deltaTR_sim = 1.0;
		//各个历元都要有基准站接收机的准确位置
		dgps_d[m].apX = b_apX;
		dgps_d[m].apY = b_apY;
		dgps_d[m].apZ = b_apZ;
		bool cal = 1;
		int cal_num = 0;
		while (fabs(sqrt(pow(X_sim, 2) + pow(Y_sim, 2) + pow(Z_sim, 2)) - sqrt(pow(X_R[m], 2) + pow(Y_R[m], 2) + pow(Z_R[m], 2))) > 0.0000001 || (fabs(deltaTR_sim - deltaTR[m]) > 0.001))
		{
			cal_num++;
			double GPStime = Time2GPSweek(obs_p[m].y, obs_p[m].m, obs_p[m].d, obs_p[m].h, obs_p[m].min, obs_p[m].sec);//信号接收时间
			if (!dgps_d == NULL)
			{
				dgps_d[m].GPStime = GPStime;
				dgps_d[m].p_flag = m + 1;//历元标志，下标为1为起始
			}
			Gnum = obs_p[m].GPS_sat_num;//GPSNum数据无效
			if (cal_num > 15)
			{
				break;
			}
			for (j = 0, i = 0; j < obs_p[m].GPS_sat_num; j++)
			{
				/*获取基本数据要进行差分的*/
				//传递值上有问题的前后顺序并不一致因为continue(关键点)
				if (!dgps_d == NULL)
				{
					dgps_d[m].sat_num = Gnum;
					dgps_d[m].GsPRN[i] = obs_p[m].sPRN[j];//确定选用的第i颗卫星
					dgps_d[m].P[i] = obs_b[m].obs[obs_p[m].G_data_order[j]][obs_h->P_flag];//确定选用的第i颗卫星的伪距

				}
				else
				{
					exit(5);
				}
				delta_t[m][j] = obs_b[m].obs[obs_p[m].G_data_order[j]][obs_h->P_flag] / C_V;//计算近似的信号传播时间
				int n_poch = seleph(GPStime, obs_p[m].sPRN[j], nav_b, nn, delta_t[m][j]);//根据观测值所属卫星和观测时间获取相应的卫星星历数据
				//迭代计算传播时间
				while (fabs(delta_t[m][j] - deltat) > 0.0000033)//迭代计算传播时间
				{
					T_S[m][j] = GPStime - delta_t[m][j]; //计算信号发射时刻
					tk = T_S[m][j] - nav_b[n_poch].TOE;
					if (tk > 302400)
						tk -= 604800;
					else
					{
						if (tk < -302400)
							tk += 604800;
					}

					//根据广播星历计算卫星位置
					n0 = sqrt(GM) / pow(nav_b[n_poch].sqrtA, 3);//计算卫星运动的平均角速度n
					n = n0 + nav_b[n_poch].deltan;

					M = nav_b[n_poch].M0 + n * tk;//计算观测瞬时平近点角M          
					if (M < 0)
					{
						M += 2 * PI;
					}
					else if (M > 2 * PI)
					{
						M -= 2 * PI;
					}

					E = M; //计算偏近点角,迭代三次
					E = M + nav_b[n_poch].e * sin(E);
					E = M + nav_b[n_poch].e * sin(E);
					E = M + nav_b[n_poch].e * sin(E);

					double a = (cos(E) - nav_b[n_poch].e) / (1 - nav_b[n_poch].e * cos(E));//cos(v)
					double b = sqrt(1 - nav_b[n_poch].e * nav_b[n_poch].e) * sin(E) / (1 - nav_b[n_poch].e * cos(E));//sin(v)
					if (a == 0)//计算真近点角v
					{
						v = PI / 2;
					}
					else if (b == 0)
					{
						v = PI;
					}
					else
					{
						v = atan(fabs(b / a));
						if (a < 0 && b>0)
						{
							v = PI - v;
						}
						else if (a < 0 && b < 0)
						{
							v = v - PI;
						}
						else if (a > 0 && b < 0)
						{
							v = 2 * PI - v;
						}
					}

					u_ = nav_b[n_poch].omega + v;//计算升交点角距u_

					Epsilon_u = nav_b[n_poch].Cuc * cos(2 * u_) + nav_b[n_poch].Cus * sin(2 * u_);//计算摄动改正项
					Epsilon_r = nav_b[n_poch].Crc * cos(2 * u_) + nav_b[n_poch].Crs * sin(2 * u_);
					Epsilon_i = nav_b[n_poch].Cic * cos(2 * u_) + nav_b[n_poch].Cis * sin(2 * u_);

					uk = u_ + Epsilon_u;//进行摄动改正
					rk = pow(nav_b[n_poch].sqrtA, 2) * (1 - nav_b[n_poch].e * cos(E)) + Epsilon_r;
					ik = nav_b[n_poch].i0 + Epsilon_i + nav_b[n_poch].IDOT * tk;

					x = rk * cos(uk);//计算卫星在轨道平面坐标系中的位置
					y = rk * sin(uk);

					omega_k = nav_b[n_poch].OMEGA + (nav_b[n_poch].deltaomega - omega_e) * tk - omega_e * nav_b[n_poch].TOE;//计算信号发射时刻的升交点赤经omega_K

					X = x * cos(omega_k) - y * cos(ik) * sin(omega_k);//计算卫星在信号发射时刻时的地固坐标系下的位置
					Y = x * sin(omega_k) + y * cos(ik) * cos(omega_k);
					Z = y * sin(ik);

					X_S[m][j] = cos(omega_e * delta_t[m][j]) * X + sin(omega_e * delta_t[m][j]) * Y;//通过绕Z轴的旋转变换，计算卫星在信号接收时刻时的地固坐标系下的位置
					Y_S[m][j] = -sin(omega_e * delta_t[m][j]) * X + cos(omega_e * delta_t[m][j]) * Y;
					Z_S[m][j] = Z;

					if (!dgps_d == NULL)
					{

						dgps_d[m].dP[i] = sqrt(pow((X_S[m][j] - dgps_d[m].apX), 2) + pow((Y_S[m][j] - dgps_d[m].apY), 2) + pow((Z_S[m][j] - dgps_d[m].apZ), 2));

					}

					R = sqrt(pow((X - X_R[m]), 2) + pow((Y - Y_R[m]), 2) + pow((Z - Z_R[m]), 2));//重新计算信号的传播时间
					deltat = delta_t[m][j];
					delta_t[m][j] = R / C_V;
				}

				//计算接收机处的大地经度L和纬度B
				L = atan(obs_h->apY / obs_h->apX) + PI;//
				double f_ = 1 / 298.257223563;//椭球体极扁率
				double e_2 = f_ * (2 - f_);//椭球偏心率
				double as = 6378137.0;//基准椭球体长半径
				double B = 0;
				double P = sqrt(obs_h->apX * obs_h->apX + obs_h->apY * obs_h->apY);
				for (n = 0; n < 5; n++)
				{
					A = as / (sqrt(1 - e_2 * sin(B) * sin(B)));
					H = P / cos(B) - A;//大地高度
					double a0 = obs_h->apZ / P;
					double b0 = pow((1 - e_2 * A / (A + H)), -1);
					B = atan(a0 * b0);
				}

				double deltax = X_S[m][j] - X_R[m];
				double deltay = Y_S[m][j] - Y_R[m];
				double deltaz = Z_S[m][j] - Z_R[m];

				//卫星在以测站坐标为原点的站心坐标系中的位置坐标
				X0 = -deltax * sin(B) * cos(L) - deltay * sin(B) * sin(L) + deltaz * cos(B);
				Y0 = -deltax * sin(L) + deltay * cos(L);
				Z0 = deltax * cos(B) * cos(L) + deltay * cos(B) * sin(L) + deltaz * sin(B);

				az = atan(Y0 / X0);//方位角（弧度制）
				el = atan(Z0 / (sqrt(pow(X0, 2) + pow(Y0, 2)))) * (180 / PI);//高度角（角度制）
				//判断高度角，剔除高度角小于5度的卫星
				if (el < 5)
				{
					Gnum--;
					continue;//直接跳过后面的所有步骤但是for中的i++是存在的会继续++=直接排除掉
				}
				//电离层延时模型
				//地心角（半圆）
				psi = 0.0137 / (el * (PI / 180) / PI + 0.11) - 0.022;

				//接收机至卫星连线与电离层交点处的大地纬度（半圆）和大地经度（弧度）
				phi = B / PI + psi * cos(az);
				if (phi > 0.416) phi = 0.416;
				else if (phi < -0.416) phi = -0.416;
				lam = L / PI + psi * sin(az) / cos(phi * PI);

				//地磁纬度（半圆）
				phi += 0.064 * cos((lam - 1.617) * PI);

				//倾斜系数
				f = 1.0 + 16.0 * pow(0.53 - el * (PI / 180) / PI, 3.0);
				//电离层延迟
				amp = nav_h->ION_ALPHA[0] + phi * (nav_h->ION_ALPHA[1] + phi * (nav_h->ION_ALPHA[2] + phi * nav_h->ION_ALPHA[3]));
				per = nav_h->ION_BETA[0] + phi * (nav_h->ION_BETA[1] + phi * (nav_h->ION_BETA[2] + phi * nav_h->ION_BETA[3]));
				amp = amp < 0.0 ? 0.0 : amp;
				per = per < 72000.0 ? 72000.0 : per;
				double tt = 43200.0 * lam + GPStime - delta_t[m][j];
				tt -= floor(tt / 86400.0) * 86400.0;
				x = 2.0 * PI * (tt - 50400.0) / per;
				I = C_V * f * (fabs(x) < 1.57 ? 5E-9 + amp * (1.0 + x * x * (-0.5 + x * x / 24.0)) : 5E-9);

				//对流层延时
				T = 2.47 / (sin(el * (PI / 180)) + 0.0121);

				//卫星时钟误差
				double F = -4.442807633e-10;
				double TOC = Time2GPSweek(nav_b[n_poch].TOC_Y, nav_b[n_poch].TOC_M, nav_b[n_poch].TOC_D, nav_b[n_poch].TOC_H, nav_b[n_poch].TOC_Min, nav_b[n_poch].TOC_Sec) + 16;
				delta_ts = nav_b[n_poch].sa0 + nav_b[n_poch].sa1 * (T_S[m][j] - TOC) + nav_b[n_poch].sa2 * (T_S[m][j] - TOC) * (T_S[m][j] - TOC);
				delta_tr = F * nav_b[n_poch].e * nav_b[n_poch].sqrtA * sin(E);
				Epsilon_ts = delta_ts + delta_tr - nav_b[n_poch].TGD;

				R = sqrt(pow((X_S[m][j] - X_R[m]), 2) + pow((Y_S[m][j] - Y_R[m]), 2) + pow((Z_S[m][j] - Z_R[m]), 2));//接收机与卫星之间的实际距离
				Pc[m][i] = obs_b[m].obs[obs_p[m].G_data_order[j]][obs_h->P_flag] + C_V * Epsilon_ts - I - T;//误差校正后的伪距测量值
				b_Matrix[m][i] = Pc[m][i] - R - deltaTR[m] * C_V;//形成矩阵b


				/*列法方程*/
				G0[m][i] = (X_R[m] - X_S[m][j]) / R;
				G1[m][i] = (Y_R[m] - Y_S[m][j]) / R;
				G2[m][i] = (Z_R[m] - Z_S[m][j]) / R;
				G3[m][i] = 1.0;
				i++;//列了才加，为什么需要两个变量来控制的原因continue
			}

			//计算接收机处的大地经度L和纬度B（近似坐标计算的）
			L = atan(obs_h->apY / obs_h->apX) + PI;//
			double f_ = 1 / 298.257223563;//椭球体极扁率
			double e_2 = f_ * (2 - f_);//椭球偏心率
			double as = 6378137.0;//基准椭球体长半径
			double B = 0;
			double P = sqrt(obs_h->apX * obs_h->apX + obs_h->apY * obs_h->apY);
			for (n = 0; n < 5; n++)
			{
				A = as / (sqrt(1 - e_2 * sin(B) * sin(B)));
				H = P / cos(B) - A;//大地高度
				double a0 = obs_h->apZ / P;
				double b0 = pow((1 - e_2 * A / (A + H)), -1);
				B = atan(a0 * b0);
			}

			// 计算矩阵N   N = （G）^T * G
			N[0][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[0][0] = G0[m][j] * G0[m][j] + N[0][0];
			}
			N[0][1] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[0][1] = G0[m][j] * G1[m][j] + N[0][1];
			}
			N[0][2] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[0][2] = G0[m][j] * G2[m][j] + N[0][2];
			}
			N[0][3] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[0][3] = G0[m][j] * G3[m][j] + N[0][3];
			}
			N[1][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[1][0] = G1[m][j] * G0[m][j] + N[1][0];
			}
			N[1][1] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[1][1] = G1[m][j] * G1[m][j] + N[1][1];
			}
			N[1][2] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[1][2] = G1[m][j] * G2[m][j] + N[1][2];
			}
			N[1][3] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[1][3] = G1[m][j] * G3[m][j] + N[1][3];
			}
			N[2][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[2][0] = G2[m][j] * G0[m][j] + N[2][0];
			}
			N[2][1] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[2][1] = G2[m][j] * G1[m][j] + N[2][1];
			}
			N[2][2] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[2][2] = G2[m][j] * G2[m][j] + N[2][2];
			}
			N[2][3] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[2][3] = G2[m][j] * G3[m][j] + N[2][3];
			}
			N[3][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[3][0] = G3[m][j] * G0[m][j] + N[3][0];
			}
			N[3][1] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[3][1] = G3[m][j] * G1[m][j] + N[3][1];
			}
			N[3][2] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[3][2] = G3[m][j] * G2[m][j] + N[3][2];
			}
			N[3][3] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[3][3] = G3[m][j] * G3[m][j] + N[3][3];
			}


			// 计算矩阵B_Matrix   B_Matrix = (G)^T * b
			B_Matrix[0][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				B_Matrix[0][0] = G0[m][j] * b_Matrix[m][j] + B_Matrix[0][0];
			}
			B_Matrix[1][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				B_Matrix[1][0] = G1[m][j] * b_Matrix[m][j] + B_Matrix[1][0];
			}
			B_Matrix[2][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				B_Matrix[2][0] = G2[m][j] * b_Matrix[m][j] + B_Matrix[2][0];
			}
			B_Matrix[3][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				B_Matrix[3][0] = G3[m][j] * b_Matrix[m][j] + B_Matrix[3][0];
			}

			matinv(N, 4);//求矩阵N的逆矩阵
			matmul(V, N, B_Matrix, 4, 1, 4);//矩阵乘法 V = N * B_Matrix,求矩阵V
			//防呆但是算出来的其实是正确的
			X_sim = X_R[m];
			Y_sim = Y_R[m];
			Z_sim = Z_R[m];
			deltaTR_sim = deltaTR[m];

			X_R[m] = X_R[m] + V[0][0];
			Y_R[m] = Y_R[m] + V[1][0];
			Z_R[m] = Z_R[m] + V[2][0];
			deltaTR[m] = deltaTR[m] + V[3][0] / C_V;


			//计算精度因子
			S[0][0] = -sin(L);
			S[0][1] = cos(L);
			S[0][2] = 0;
			S[0][3] = 0;
			S[1][0] = -sin(B) * cos(L);
			S[1][1] = -sin(B) * sin(L);
			S[1][2] = cos(B);
			S[1][3] = 0;
			S[2][0] = cos(B) * cos(L);
			S[2][1] = cos(B) * sin(L);
			S[2][2] = sin(B);
			S[2][3] = 0;
			S[3][0] = 0;
			S[3][1] = 0;
			S[3][2] = 0;
			S[3][3] = 1;

			S_[0][0] = -sin(L);
			S_[0][1] = -sin(B) * cos(L);
			S_[0][2] = cos(B) * cos(L);
			S_[0][3] = 0;
			S_[1][0] = cos(L);
			S_[1][1] = -sin(B) * sin(L);
			S_[1][2] = cos(B) * sin(L);
			S_[1][3] = 0;
			S_[2][0] = 0;
			S_[2][1] = cos(B);
			S_[2][2] = sin(B);
			S_[2][3] = 0;
			S_[3][0] = 0;
			S_[3][1] = 0;
			S_[3][2] = 0;
			S_[3][3] = 1;

			matmul(hH, S, N, 4, 4, 4);//矩阵乘法 hH = S * N,求矩阵hH
			matmul(HH, hH, S_, 4, 4, 4);//矩阵乘法 HH = hH * S_,求矩阵hH

			PDOP[m] = sqrt(HH[0][0] + HH[1][1] + HH[2][2]);
			VDOP[m] = sqrt(HH[2][2]);
			HDOP[m] = sqrt(HH[0][0] + HH[1][1]);

			//计算东北天向的误差
			double deltax = X_R[m];
			double deltay = Y_R[m];
			double deltaz = Z_R[m];
		}
		/*当前历元基准站数据处理*/
		//如何剔除计算错误的历元？防呆
		if (!dgps_d == NULL)
		{
			dgps_d[m].year = obs_p[m].y, dgps_d[m].month = obs_p[m].m, dgps_d[m].day = obs_p[m].d, dgps_d[m].hour = obs_p[m].h, dgps_d[m].min = obs_p[m].min, dgps_d[m].sec = obs_p[m].sec;
			dgps_d[m].dt0 = deltaTR[m];//接收机钟差
			//对每颗卫星逐个进行伪距改正值的确定（变量传递过程正确吗？）
			//cout << dgps_d[m].sat_num << endl;
			for (int i = 0; i < dgps_d[m].sat_num; i++)
			{
				dgps_d[m].dP[i] = dgps_d[m].dP[i] - dgps_d[m].P[i] + C_V * dgps_d[m].dt0;
				//cout << dgps_d[m].GsPRN[i] << "   " << dgps_d[m].dP[i] << endl;
			}
		}
		else
		{
			exit(6);
		}


		printf("基准站的第%d个历元\n", m + 1);
		printf("X：%lf	Y：%lf	Z：%lf	接收机钟差：%.8lf\n", X_R[m], Y_R[m], Z_R[m], deltaTR[m]);
		//printf("PDOP:%1f      VDOP:%1f       HDOP:%1f\n", PDOP[m], VDOP[m], HDOP[m]);
		continue;

	}
    
	free(PDOP), free(VDOP), free(HDOP);
	
	free(obs_b), free(obs_h), free(obs_p);
    //free_data(nav_h, nav_b, obs_h, obs_p, obs_b);//释放文件分配内存空间


	//流动站部分：
	cout << "请输入流动站obs文件名称：";
	//cout << "请流动站输入nav文件与obs文件名称：" ;
	cin >> obs_file;
	//cin >> nav_file >> obs_file;
	//数据获取部分：
	//read_file(nav_file, obs_file, nav_h, nav_b, obs_h, obs_p, obs_b, o_pochnum, nn);
	

	/*
		//N文件读取
	fp_nav = fopen(nav_file, "r");//以只读的方式打开N文件
	if (!fp_nav)
	{
		printf("N文件打开失败!");
		exit(3);
	}
	else
	{
		printf("N文件打开成功!\n");
	}
	nn = getrow(fp_nav);//获取导航文件观测行数
	printf("导航文件的观测值行数为：%d\n", nn);
	rewind(fp_nav);//将文件指针返回值起始位置
	nav_h = (pnav_head)malloc(sizeof(nav_head));//给N文件头开辟空间需要配合free（）函数一起使用且只能释放一次忘记的话就会导致内存泄漏嗷
	nav_b = (pnav_body)malloc(sizeof(nav_body) * (nn / 8));//这一块只是一个指针而已，本身并不存在实体所以一结束就自动释放掉了
	if (nav_h && nav_b)
	{
		readrinex_n(fp_nav, nav_h, nav_b, nn);//读取数据
	}
	fclose(fp_nav);//关闭N文件
	printf("N文件读取成功!\n");*/

	//O文件读取
	fp_obs = fopen(obs_file, "r");//以只读的方式打开O文件
	if (!fp_obs)
	{
		printf("O文件打开失败!");
		exit(3);
	}
	else
	{
		printf("O文件打开成功!\n");
	}
	o_epochnum = get_epochnum(fp_obs);//获取O文件历元数
	o_pochnum = o_epochnum;
	printf("观测值文件的历元数为：%d\n", o_epochnum);
	rewind(fp_obs);
	obs_h = (pobs_head)malloc(sizeof(obs_head));//给O文件头开辟空间
	obs_p = (pobs_epoch)malloc(sizeof(obs_epoch) * (o_epochnum));
	obs_b = (pobs_body)malloc(sizeof(obs_body) * (o_epochnum));
	if (obs_h && obs_p && obs_b)
	{
		read_obs_h(fp_obs, obs_h);//读取O文件头(fp_obs)会自己向下，继续指向当前的行
		read_obs_b(fp_obs, obs_p, obs_b, obs_h->obstypenum);//读取O文件数据块
	}
	fclose(fp_obs);//关闭O文件
	printf("O文件读取成功！\n");
	//流动站计算部分：

	X_R = (double*)malloc((sizeof(double)) * o_pochnum);
	Y_R = (double*)malloc((sizeof(double)) * o_pochnum);
	Z_R = (double*)malloc((sizeof(double)) * o_pochnum);
	deltaTR = (double*)malloc((sizeof(double)) * o_pochnum);

	for (i = 0; i < o_pochnum; i++)//给各个历元定待定点的初始坐标和初始接收机钟差，都设为0
	{
		X_R[i] = 0;
	}
	for (i = 0; i < o_pochnum; i++)
	{
		Y_R[i] = 0;
	}
	for (i = 0; i < o_pochnum; i++)
	{
		Z_R[i] = 0;
	}
	for (i = 0; i < o_pochnum; i++)
	{
		deltaTR[i] = 0;
	}



	delta_t = (double**)malloc((sizeof(double*)) * o_pochnum);//给delta_t分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		delta_t[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	T_S = (double**)malloc(sizeof(double*) * o_pochnum);//给T_S分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		T_S[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	X_S = (double**)malloc(sizeof(double*) * o_pochnum);//给X_S分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		X_S[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	Y_S = (double**)malloc(sizeof(double*) * o_pochnum);//给Y_S分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		Y_S[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	Z_S = (double**)malloc(sizeof(double*) * o_pochnum);//给Z_S分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		Z_S[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	G0 = (double**)malloc(sizeof(double*) * o_pochnum);//给G0分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		G0[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	G1 = (double**)malloc(sizeof(double*) * o_pochnum);//给G1分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		G1[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	G2 = (double**)malloc(sizeof(double*) * o_pochnum);//给G2分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		G2[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	G3 = (double**)malloc(sizeof(double*) * o_pochnum);//给G3分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		G3[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	b_Matrix = (double**)malloc(sizeof(double*) * o_pochnum);//给L分配内存
	for (i = 0; i < o_pochnum; i++)
	{
		b_Matrix[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	Pc = (double**)malloc(sizeof(double*) * o_pochnum);
	for (i = 0; i < o_pochnum; i++)
	{
		Pc[i] = (double*)malloc((sizeof(double)) * obs_p[i].sat_num);
	}

	PDOP = (double*)malloc((sizeof(double)) * o_pochnum);
	VDOP = (double*)malloc((sizeof(double)) * o_pochnum);
	HDOP = (double*)malloc((sizeof(double)) * o_pochnum);
	for (i = 0; i < o_pochnum; i++)
	{
		PDOP[i] = 0;
	}

	for (i = 0; i < o_pochnum; i++)
	{
		VDOP[i] = 0;
	}

	for (i = 0; i < o_pochnum; i++)
	{
		HDOP[i] = 0;
	}

	double X_mean = 0, Y_mean = 0, Z_mean = 0, std_X = 0, std_Y = 0, CEP95;
	int pass_M = 0;
	int pass_e = 0;
	int P_flag[MAXRNXLEN];
	int k = 0;

	//计算部分（伪距会发生改变完全是因为接收机钟差）（当前历元的接收机位置计算）
	for (m = 0; m < o_pochnum; m++)
	{

		X_sim = 1.0;
		Y_sim = 1.0;
		Z_sim = 1.0;
		deltaTR_sim = 1.0;

		//int M_ephnum = 0;//基准站的观测历元总数
		int M_b = 0;
		//当前历元卫星信号接收时刻
		double GPStime = Time2GPSweek(obs_p[m].y, obs_p[m].m, obs_p[m].d, obs_p[m].h, obs_p[m].min, obs_p[m].sec);

		//确定跳过的历元标志
		if (!pass_e == pass_M)
		{
			pass_e = pass_M;
			P_flag[k] = m;
			k++;//为什么它不运行
		}


		//确定当前历元的基准站数据是否存在（是=继续.否=放弃当前历元计算）使用continue眺for  
		//剔除无计算数据的历元
		if (!selepoch_basic(GPStime, dgps_d, o_pochnum, M_b))//传值
		{
			pass_M++;
			o_pochnum--;
			continue;
		}
		else if (fabs(dgps_d[M_b].dt0) >= 1E-7 || isnan(dgps_d[M_b].dt0))//接收机钟差1E-7以下
		{
			pass_M++;
			o_pochnum--;
			continue;
		}

		bool cal = 1;//判断当前历元的可用卫星数是否大于等于四
		int cal_num = 0;

		double Dstanc = 0;

		//当前历元的一次计算过程

		while (fabs(sqrt(pow(X_sim, 2) + pow(Y_sim, 2) + pow(Z_sim, 2)) - sqrt(pow(X_R[m], 2) + pow(Y_R[m], 2) + pow(Z_R[m], 2))) > 0.0000001 || (fabs(deltaTR_sim - deltaTR[m]) > 0.001))
		{
			Gnum = obs_p[m].GPS_sat_num;
			cal_num++;
			//筛选可改正伪距的卫星数量并进行改正（放后面一起剔除）

			//确定当前历元的卫星数量并列式
			for (j = 0, i = 0; j < obs_p[m].GPS_sat_num; j++)
			{
				//rinex2.11版本各地区的C1的位置不一致所以需要枚举来找到伪距在读取的数据中的位置
				//即数据匹配的各种问题很重要，属于小bug之类的，整体的运行中需要考虑、处理的问题
				delta_t[m][j] = obs_b[m].obs[obs_p[m].G_data_order[j]][obs_h->P_flag] / C_V;//计算近似的信号传播时间
				int n_poch = seleph(GPStime, obs_p[m].sPRN[j], nav_b, nn, delta_t[m][j]);//根据观测值所属卫星和观测时间获取相应的卫星星历数据
				//迭代计算传播时间
				while (fabs(delta_t[m][j] - deltat) > 0.0000033)//迭代计算传播时间
				{
					T_S[m][j] = GPStime - delta_t[m][j]; //计算信号发射时刻
					tk = T_S[m][j] - nav_b[n_poch].TOE;
					if (tk > 302400)
						tk -= 604800;
					else
					{
						if (tk < -302400)
							tk += 604800;
					}

					//根据广播星历计算卫星位置
					n0 = sqrt(GM) / pow(nav_b[n_poch].sqrtA, 3);//计算卫星运动的平均角速度n
					n = n0 + nav_b[n_poch].deltan;

					M = nav_b[n_poch].M0 + n * tk;//计算观测瞬时平近点角M          
					if (M < 0)
					{
						M += 2 * PI;
					}
					else if (M > 2 * PI)
					{
						M -= 2 * PI;
					}

					E = M; //计算偏近点角,迭代三次
					E = M + nav_b[n_poch].e * sin(E);
					E = M + nav_b[n_poch].e * sin(E);
					E = M + nav_b[n_poch].e * sin(E);

					double a = (cos(E) - nav_b[n_poch].e) / (1 - nav_b[n_poch].e * cos(E));//cos(v)
					double b = sqrt(1 - nav_b[n_poch].e * nav_b[n_poch].e) * sin(E) / (1 - nav_b[n_poch].e * cos(E));//sin(v)
					if (a == 0)//计算真近点角v
					{
						v = PI / 2;
					}
					else if (b == 0)
					{
						v = PI;
					}
					else
					{
						v = atan(fabs(b / a));
						if (a < 0 && b>0)
						{
							v = PI - v;
						}
						else if (a < 0 && b < 0)
						{
							v = v - PI;
						}
						else if (a > 0 && b < 0)
						{
							v = 2 * PI - v;
						}
					}

					u_ = nav_b[n_poch].omega + v;//计算升交点角距u_

					Epsilon_u = nav_b[n_poch].Cuc * cos(2 * u_) + nav_b[n_poch].Cus * sin(2 * u_);//计算摄动改正项
					Epsilon_r = nav_b[n_poch].Crc * cos(2 * u_) + nav_b[n_poch].Crs * sin(2 * u_);
					Epsilon_i = nav_b[n_poch].Cic * cos(2 * u_) + nav_b[n_poch].Cis * sin(2 * u_);

					uk = u_ + Epsilon_u;//进行摄动改正
					rk = pow(nav_b[n_poch].sqrtA, 2) * (1 - nav_b[n_poch].e * cos(E)) + Epsilon_r;
					ik = nav_b[n_poch].i0 + Epsilon_i + nav_b[n_poch].IDOT * tk;

					x = rk * cos(uk);//计算卫星在轨道平面坐标系中的位置
					y = rk * sin(uk);

					omega_k = nav_b[n_poch].OMEGA + (nav_b[n_poch].deltaomega - omega_e) * tk - omega_e * nav_b[n_poch].TOE;//计算信号发射时刻的升交点赤经omega_K

					X = x * cos(omega_k) - y * cos(ik) * sin(omega_k);//计算卫星在信号发射时刻时的地固坐标系下的位置
					Y = x * sin(omega_k) + y * cos(ik) * cos(omega_k);
					Z = y * sin(ik);

					X_S[m][j] = cos(omega_e * delta_t[m][j]) * X + sin(omega_e * delta_t[m][j]) * Y;//通过绕Z轴的旋转变换，计算卫星在信号接收时刻时的地固坐标系下的位置
					Y_S[m][j] = -sin(omega_e * delta_t[m][j]) * X + cos(omega_e * delta_t[m][j]) * Y;
					Z_S[m][j] = Z;

					R = sqrt(pow((X - X_R[m]), 2) + pow((Y - Y_R[m]), 2) + pow((Z - Z_R[m]), 2));//重新计算信号的传播时间
					deltat = delta_t[m][j];
					delta_t[m][j] = R / C_V;
				}

				//计算接收机处的大地经度L和纬度B
				L = atan(obs_h->apY / obs_h->apX) + PI;//
				double f_ = 1 / 298.257223563;//椭球体极扁率
				double e_2 = f_ * (2 - f_);//椭球偏心率
				double as = 6378137.0;//基准椭球体长半径
				double B = 0;
				double P = sqrt(obs_h->apX * obs_h->apX + obs_h->apY * obs_h->apY);
				for (n = 0; n < 5; n++)
				{
					A = as / (sqrt(1 - e_2 * sin(B) * sin(B)));
					H = P / cos(B) - A;//大地高度
					double a0 = obs_h->apZ / P;
					double b0 = pow((1 - e_2 * A / (A + H)), -1);
					B = atan(a0 * b0);
				}


				double deltax = X_S[m][j] - X_R[m];
				double deltay = Y_S[m][j] - Y_R[m];
				double deltaz = Z_S[m][j] - Z_R[m];

				//卫星在以测站坐标为原点的站心坐标系中的位置坐标
				X0 = -deltax * sin(B) * cos(L) - deltay * sin(B) * sin(L) + deltaz * cos(B);
				Y0 = -deltax * sin(L) + deltay * cos(L);
				Z0 = deltax * cos(B) * cos(L) + deltay * cos(B) * sin(L) + deltaz * sin(B);

				az = atan(Y0 / X0);//方位角（弧度制）
				el = atan(Z0 / (sqrt(pow(X0, 2) + pow(Y0, 2)))) * (180 / PI);//高度角（角度制）

				int j_b = 0;

				//判断高度角，剔除高度角小于5度的卫星
				if (el < 5)
				{
					Gnum--;
					continue;//j的值并未改变（理解循环，利用机制）
				}
				//补上else if剔除没有改正伪距的卫星并确定卫星伪距改正数的数值（一定要确保传值的正确性）
				else if (!selePRN(obs_p[m].sPRN[j], dgps_d, dgps_d[M_b].sat_num, M_b, j_b))
				{
					Gnum--;
					continue;
				}
				//修改直接使用dgps_d下的伪距改正数
				R = sqrt(pow((X_S[m][j] - X_R[m]), 2) + pow((Y_S[m][j] - Y_R[m]), 2) + pow((Z_S[m][j] - Z_R[m]), 2));//接收机与卫星之间的实际距离
				Pc[m][i] = obs_b[m].obs[obs_p[m].G_data_order[j]][obs_h->P_flag] + dgps_d[M_b].dP[j_b];//改正伪距误差校正后的伪距测量值
				b_Matrix[m][i] = Pc[m][i] - R - deltaTR[m] * C_V;//形成矩阵b


				//卫星数量是否大于等于4决定能否计算出来结果




				//下面是组成系数矩阵方程
				G0[m][i] = (X_R[m] - X_S[m][j]) / R;
				G1[m][i] = (Y_R[m] - Y_S[m][j]) / R;
				G2[m][i] = (Z_R[m] - Z_S[m][j]) / R;
				G3[m][i] = 1.0;
				i++;
			}

			//判断是否能进行计算、计算是否进入死循环
			if (Gnum < 4 || cal_num>15)
			{
				cal = 0;
				break;
			}


			//计算接收机处的大地经度L和纬度B（近似坐标计算的）
			L = atan(obs_h->apY / obs_h->apX) + PI;//
			double f_ = 1 / 298.257223563;//椭球体极扁率
			double e_2 = f_ * (2 - f_);//椭球偏心率
			double as = 6378137.0;//基准椭球体长半径
			double B = 0;
			double P = sqrt(obs_h->apX * obs_h->apX + obs_h->apY * obs_h->apY);
			for (n = 0; n < 5; n++)
			{
				A = as / (sqrt(1 - e_2 * sin(B) * sin(B)));
				H = P / cos(B) - A;//大地高度
				double a0 = obs_h->apZ / P;
				double b0 = pow((1 - e_2 * A / (A + H)), -1);
				B = atan(a0 * b0);
			}

			// 计算矩阵N   N = （G）^T * G
			N[0][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[0][0] = G0[m][j] * G0[m][j] + N[0][0];
			}
			N[0][1] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[0][1] = G0[m][j] * G1[m][j] + N[0][1];
			}
			N[0][2] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[0][2] = G0[m][j] * G2[m][j] + N[0][2];
			}
			N[0][3] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[0][3] = G0[m][j] * G3[m][j] + N[0][3];
			}
			N[1][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[1][0] = G1[m][j] * G0[m][j] + N[1][0];
			}
			N[1][1] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[1][1] = G1[m][j] * G1[m][j] + N[1][1];
			}
			N[1][2] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[1][2] = G1[m][j] * G2[m][j] + N[1][2];
			}
			N[1][3] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[1][3] = G1[m][j] * G3[m][j] + N[1][3];
			}
			N[2][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[2][0] = G2[m][j] * G0[m][j] + N[2][0];
			}
			N[2][1] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[2][1] = G2[m][j] * G1[m][j] + N[2][1];
			}
			N[2][2] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[2][2] = G2[m][j] * G2[m][j] + N[2][2];
			}
			N[2][3] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[2][3] = G2[m][j] * G3[m][j] + N[2][3];
			}
			N[3][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[3][0] = G3[m][j] * G0[m][j] + N[3][0];
			}
			N[3][1] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[3][1] = G3[m][j] * G1[m][j] + N[3][1];
			}
			N[3][2] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[3][2] = G3[m][j] * G2[m][j] + N[3][2];
			}
			N[3][3] = 0;
			for (j = 0; j < Gnum; j++)
			{
				N[3][3] = G3[m][j] * G3[m][j] + N[3][3];
			}


			// 计算矩阵B_Matrix   B_Matrix = (G)^T * b
			B_Matrix[0][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				B_Matrix[0][0] = G0[m][j] * b_Matrix[m][j] + B_Matrix[0][0];
			}
			B_Matrix[1][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				B_Matrix[1][0] = G1[m][j] * b_Matrix[m][j] + B_Matrix[1][0];
			}
			B_Matrix[2][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				B_Matrix[2][0] = G2[m][j] * b_Matrix[m][j] + B_Matrix[2][0];
			}
			B_Matrix[3][0] = 0;
			for (j = 0; j < Gnum; j++)
			{
				B_Matrix[3][0] = G3[m][j] * b_Matrix[m][j] + B_Matrix[3][0];
			}

			matinv(N, 4);//求矩阵N的逆矩阵
			matmul(V, N, B_Matrix, 4, 1, 4);//矩阵乘法 V = N * B_Matrix,求矩阵V
			//问题在这：赋值失败  
			X_sim = X_R[m];
			Y_sim = Y_R[m];
			Z_sim = Z_R[m];
			deltaTR_sim = deltaTR[m];

			X_R[m] = X_R[m] + V[0][0];
			Y_R[m] = Y_R[m] + V[1][0];
			Z_R[m] = Z_R[m] + V[2][0];
			deltaTR[m] = deltaTR[m] + V[3][0] / C_V;


			//计算精度因子
			S[0][0] = -sin(L);
			S[0][1] = cos(L);
			S[0][2] = 0;
			S[0][3] = 0;
			S[1][0] = -sin(B) * cos(L);
			S[1][1] = -sin(B) * sin(L);
			S[1][2] = cos(B);
			S[1][3] = 0;
			S[2][0] = cos(B) * cos(L);
			S[2][1] = cos(B) * sin(L);
			S[2][2] = sin(B);
			S[2][3] = 0;
			S[3][0] = 0;
			S[3][1] = 0;
			S[3][2] = 0;
			S[3][3] = 1;

			S_[0][0] = -sin(L);
			S_[0][1] = -sin(B) * cos(L);
			S_[0][2] = cos(B) * cos(L);
			S_[0][3] = 0;
			S_[1][0] = cos(L);
			S_[1][1] = -sin(B) * sin(L);
			S_[1][2] = cos(B) * sin(L);
			S_[1][3] = 0;
			S_[2][0] = 0;
			S_[2][1] = cos(B);
			S_[2][2] = sin(B);
			S_[2][3] = 0;
			S_[3][0] = 0;
			S_[3][1] = 0;
			S_[3][2] = 0;
			S_[3][3] = 1;

			matmul(hH, S, N, 4, 4, 4);//矩阵乘法 hH = S * N,求矩阵hH
			matmul(HH, hH, S_, 4, 4, 4);//矩阵乘法 HH = hH * S_,求矩阵hH

			PDOP[m] = sqrt(HH[0][0] + HH[1][1] + HH[2][2]);
			VDOP[m] = sqrt(HH[2][2]);
			HDOP[m] = sqrt(HH[0][0] + HH[1][1]);

			//计算东北天向的误差
			double deltax = X_R[m];
			double deltay = Y_R[m];
			double deltaz = Z_R[m];
		}

		//可以考虑使用距离来判断，一般给的接收机近似位置是接近的不会差超过三米
		Dstanc= sqrt(pow((obs_h->apX - X_R[m]), 2) + pow((obs_h->apY - Y_R[m]), 2) + pow((obs_h->apZ - Z_R[m]), 2));
		if (fabs(Dstanc)>=500)
		{
			pass_M++;
			o_pochnum--;
			continue;
		}
		//剔除计算错误的历元及不可计算历元（确实距离远了钟差就会大起来的0.001左右）
		/*if (fabs(deltaTR[m]) >= 3E-4 || isnan(fabs(deltaTR[m])) || fabs(deltaTR[m]) <= 2.9E-4)
		{
			pass_M++;
			o_pochnum--;
			continue;
		}*/
		else if (!cal)
		{
			pass_M++;
			o_pochnum--;
			continue;
		}


		printf("流动站的第%d个历元\n", m + 1 + pass_M);
		printf("X：%lf	Y：%lf	Z：%lf	接收机钟差：%.8lf\n", X_R[m], Y_R[m], Z_R[m], deltaTR[m]);
		printf("PDOP:%1f      VDOP:%1f       HDOP:%1f\n", PDOP[m], VDOP[m], HDOP[m]);

		//求平均上(扩充)
		X_mean += X_R[m];
		Y_mean += Y_R[m];
		Z_mean += Z_R[m];
		/*X_mean = X_mean / allnum;
		Y_mean = Y_mean / allnum;
		Z_mean = Z_mean / allnum;
		std_X += pow(X_R[m] - X_mean, 2);
		std_Y += pow(Y_R[m] - Y_mean, 2);*/
		continue;

	}

	//因为for这样跳过了求平均值不能这样求了
	int allnum = o_pochnum - pass_M;
	//std_X = sqrt(std_X / tal_epoch);
	//std_Y = sqrt(std_Y / tal_epoch);
	printf("平均坐标：  X：%lf	  Y：%lf	 Z：%lf	   \n", X_mean / allnum, Y_mean / allnum, Z_mean / allnum);
	//CEP95 = 1.2272 * (std_X + std_Y);
	//printf("CEP95=%fm", CEP95);

	free(PDOP), free(VDOP), free(HDOP);

	free(obs_b), free(obs_h), free(obs_p), free(nav_b), free(nav_h), free(dgps_d);

	//超过70km接收机的钟差会很大但是不影响
	cout << "计算完成！";

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
