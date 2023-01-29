/*
 * Gas_Gauge driver for CW2015/2013
 * Copyright (C) 2012, CellWise
 * Copyright (C) 2018 XiaoMi, Inc.
 *
 * Authors: ChenGang <ben.chen@cellwise-semi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.And this driver depends on 
 * I2C and uses IIC bus for communication with the host.
 *
 */
// 自行增加 ----------------------------------------
#include "I2C_Wrapper.h"
#include "common_inc.h"

/* CW2015 I2C 7bit 地址 */
#define CW2015_ADDR_7BIT 0x62U
#define msleep osDelay
#define printk(...)

/**
 * 需要由其他线程修改
 */
//extern int FG_charging_type ;	//标记充电类型，AC还是USB
//extern int FG_charging_status ;	//标记充电状态，在充电还是未充电
int FG_charging_type = 0;	//标记充电类型，AC还是USB
int FG_charging_status = 0;	//标记充电状态，在充电还是未充电
//extern int g_platform_boot_mode;//boot阶段的标记
int g_platform_boot_mode = 8;

/**
 * @brief  container_of - get pointer to enclosing structure		// 获取指向封闭结构的指针
 * @param  ptr: pointer to the structure member						// 指向结构体成员的指针
 * @param  type: the type this member is within						// 包括该成员的结构体的类型
 * @param  member: the name of this member within the structure.	// 该成员在结构体内的名字
 *
 * https://github.com/avivgr/stm32-u2f/blob/master/list.h
 * https://github.com/rustyrussell/ccan/blob/master/ccan/container_of/container_of.h
 */
#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})

struct timestamp {
	uint32_t	tv_sec;		/* seconds */
	uint32_t	tv_nsec;	/* and nanoseconds */
};

/**
 * @brief 	获取系统启动以来所经过的时间，不包含休眠时间
 * 			对于STM32，systick，模式会每 ms 产生一次中断，在 MCU 进入睡眠模式的时候，
 * 			需要调用 HAL_SuspendTick(); 来关闭 systick 中断
 * 			这样 systick时间 就停止增加，systick 不包含休眠时间
 */
static void ktime_get_ts(struct timestamp *ts)
{
	ts->tv_sec = HAL_GetTick() / 1000;
}

/**
 * @brief 	获取系统启动以来所经过的时间，包含休眠时间
 * 			对于STM32，STOP1模式时，将 systick 设置为停止增加，但RTC可由VBAT供电继续记录时间，
 * 			BATFET导通开机时将开机RTC时间戳记录到backup register，STM32获取系统启动以来
 * 			所经过的时间，包含休眠时间，那就先读取RTC时间，再与开机RTC时间戳相减就是
 */
static void get_monotonic_boottime(struct timestamp *ts)
{
	/**
	 * TODO: @brief 描述的实现方法
	 */
	ts->tv_sec = HAL_GetTick() / 1000 + 2;	//暂时先这样
}

/**
 * https://github.com/ibilux/android_kernel_xiaomi_hermes/
 * blob/44c688bdf8303ba4f35a141267c3dcce41f8467a/
 * drivers/power/z2_battery.c
 */
enum {
	POWER_SUPPLY_STATUS_UNKNOWN = 0,
	POWER_SUPPLY_STATUS_CHARGING,
	POWER_SUPPLY_STATUS_DISCHARGING,
	POWER_SUPPLY_STATUS_NOT_CHARGING,
	POWER_SUPPLY_STATUS_FULL,
	POWER_SUPPLY_STATUS_CMD_DISCHARGING,
};


/* Battery relative fucntion
 * https://github.com/ibilux/android_kernel_xiaomi_hermes/
 * blob/44c688bdf8303ba4f35a141267c3dcce41f8467a/
 * drivers/misc/mediatek/usb20/mt6795/usb20.h
 * DS-PPK实现的得
 * 	1. STANDARD_HOST 检测usb为suspend 则设置 BQ25601 REG02 ICHG 为 480mA，将FG_charging_type赋值为STANDARD_HOST
 * 	2. 其他模式
 * */
typedef enum {
    CHARGER_UNKNOWN = 0,	// 未充电
    STANDARD_HOST,          // USB : 450mA		// 被usb host充电
    CHARGING_HOST,			// 不知道是个啥，居然还有这种USB HOST类？
    NONSTANDARD_CHARGER,    // AC : 450mA~1A
    STANDARD_CHARGER,       // AC : ~1A
} CHARGER_TYPE;


osThreadId_t cwBatTaskHandle;
const uint32_t cwBatTaskStackSize = 512 * 4;
const osThreadAttr_t cwBatTask_attributes = {
    .name = "cwBatTask",
    .stack_size = cwBatTaskStackSize,
    .priority = (osPriority_t) osPriorityLow,
};

// ---------------------------------------------------------------------------------------------
//#define BAT_CHANGE_ALGORITHM
#include "cw2015_battery.h"

#ifdef BAT_CHANGE_ALGORITHM
#define FILE_PATH "/data/lastsoc"
#define CPSOC  90
#endif

// cw2015 寄存器地址
#define REG_VERSION             0x0
#define REG_VCELL               0x2
#define REG_SOC                 0x4
#define REG_RRT_ALERT           0x6
#define REG_CONFIG              0x8
#define REG_MODE                0xA
#define REG_BATINFO             0x10

#define MODE_SLEEP_MASK         (0x3<<6)
#define MODE_SLEEP              (0x3<<6)	// 0x3 : 11000000 模式寄存器用于Master控制IC。默认值11，写入11 强制CW2015进入休眠模式； 写 00 唤醒
#define MODE_NORMAL             (0x0<<6)
#define MODE_QUICK_START        (0x3<<4)
#define MODE_RESTART            (0xf<<0)	//  0xf : 00001111  默认值 0000，写入 1111 以完全重启 IC，就像断电一样

#define CONFIG_UPDATE_FLG       (0x1<<1)	// UFG bit 为 1 ，表示电池信息已被主机写入更新
#define ATHD                    (0x0<<3)        //ATHD = 0%

#define CW_I2C_SPEED            100000          // default i2c speed set 100khz
#define BATTERY_UP_MAX_CHANGE   420             // the max time allow battery change quantity
#define BATTERY_DOWN_CHANGE   60                // the max time allow battery change quantity
#define BATTERY_DOWN_MIN_CHANGE_RUN 30          // the min time allow battery change quantity when run
#define BATTERY_DOWN_MIN_CHANGE_SLEEP 1800      // the min time allow battery change quantity when run 30min

#define BATTERY_DOWN_MAX_CHANGE_RUN_AC_ONLINE 1800
#define DEVICE_RUN_TIME_FIX_VALUE 40

#define NO_STANDARD_AC_BIG_CHARGE_MODE 1
#define SYSTEM_SHUTDOWN_VOLTAGE  3400000        //set system shutdown voltage related in battery info.
#define BAT_LOW_INTERRUPT    1

#define USB_CHARGER_MODE        1
#define AC_CHARGER_MODE         2

#define USE_MTK_INIT_VOL		//源文件是使能的


#define FG_CW2015_DEBUG                0
#define FG_CW2015_TAG                  "[FG_CW2015]"
#if FG_CW2015_DEBUG
#define FG_CW2015_FUN(f)               printk(KERN_ERR FG_CW2015_TAG"%s\n", __FUNCTION__)
#define FG_CW2015_ERR(fmt, args...)    printk(KERN_ERR FG_CW2015_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define FG_CW2015_LOG(fmt, args...)    printk(KERN_ERR FG_CW2015_TAG fmt, ##args)
#endif
#define CW2015_DEV_NAME     "CW2015"

int g_cw2015_capacity = 0;
int g_cw2015_vol = 0;
int g_mtk_init_vol = -10;

int CW2015_test_init=0;	//测试初始化成功的标志

extern int Charger_enable_Flag; //add by longcheer_liml_2015_10_12

#define cw_work_freq  8000 	//ms

/*Chaman add for create sysfs start*/
static int file_sys_state = 1;
/*Chaman add for create sysfs end*/
static uint8_t config_info_des[SIZE_BATINFO] = { //desay
		0x17,0xF9,0x6D,0x6D,0x6B,0x67,0x65,0x64,0x58,0x6D,0x6D,0x48,0x57,0x5D,0x4A,0x43,
		0x37,0x31,0x2B,0x20,0x24,0x35,0x44,0x55,0x20,0x37,0x0B,0x85,0x2A,0x4A,0x56,0x68,
		0x74,0x6B,0x6D,0x6E,0x3C,0x1A,0x5C,0x45,0x0B,0x30,0x52,0x87,0x8F,0x91,0x94,0x52,
		0x82,0x8C,0x92,0x96,0x64,0xB4,0xDB,0xCB,0x2F,0x7D,0x72,0xA5,0xB5,0xC1,0xA5,0x42
};

int hmi_battery_version=2;

static struct cw_bat_platform_data cw_bat_platdata = {
        .is_dc_charge = 0,
		.dc_det_pin = 0,
		.dc_det_level = 0,

		.is_usb_charge = 0,
		.chg_mode_sel_pin = 0,
		.chg_mode_sel_level = 0,

		.bat_low_pin = 0,
		.bat_low_level = 0,
		.chg_ok_pin = 0,
		.chg_ok_level = 0,

		.cw_bat_config_info = config_info_des,	//电池模型
};

struct cw_battery {
	struct workqueue_struct *battery_workqueue;
	struct cw_bat_platform_data *plat_data;

	// 容量变化到现在的睡眠时间，容量变化时置0
	// the sleep time from capacity change to present, it will set 0 when capacity change
	long sleep_time_capacity_change;
	long run_time_capacity_change;

	// 从插入交流电到现在的睡眠时间，当插入交流电时，它将设置为0
	// the sleep time from insert ac to present, it will set 0 when insert ac
	long sleep_time_charge_start;
	long run_time_charge_start;

	int dc_online;
	int usb_online;
	int charger_mode;		//usb充电状态、0、1、2 ，分别反映USB无供电，接入USB Host取电，接入AC充电头的USB充电
	int charger_init_mode;
	int capacity;
	int voltage;
	int status;
	int time_to_empty;
	int alt;

	/**
	 * 在以下函数中改为0
	 * 	cw2015_i2c_probe()
	 * 	cw_bat_work()
	 * 在以下函数中改为1
	 * 	rk_bat_update_capacity()
	 * 	rk_bat_update_vol()
	 * 	rk_bat_update_status()
	 * 	rk_bat_update_time_to_empty()
	 */
	int bat_change;
};
struct cw_battery __cw_bat;
struct cw_battery *cw_bat = &__cw_bat;

#ifdef BAT_CHANGE_ALGORITHM
/**
 * http://m.cbea.com/qyjs/201806/440445.html
 * SOC(State of Charge )为荷电状态
 * 表示在一定的放电倍率下，电池使用一段时间或长期搁置后剩余容量与其完全充电状态的容量的比值。
 * “开路电压 + 安时积分”法利用开路电压法估算出电池初始状态荷电容量SOC0，然后利用安时积分法
 * 求得电池运行消耗的电量，消耗电量为放电电流与放电时间的乘积，则剩余电量等于初始电量与
 * 消耗电量的差值。开路电压与安时积分结合估算SOC 数学表达式为：
 *
 */
struct cw_store{
	long bts; 		// timestamp 时间戳？
	int OldSOC;		// 旧SOC
	int DetSOC;		//detection 检测SOC
	int AlRunFlag;   
};
#endif


#ifdef BAT_CHANGE_ALGORITHM
static int PowerResetFlag = -1;		//cw2015的重启标志， 1 表示重启成功，仅在 cw_update_config_info() 里写POR位重启CW2015成功后 置 1
static int alg_run_flag = -1;		//使用软件SOC算法标志
#endif

#ifdef BAT_CHANGE_ALGORITHM
static unsigned int cw_convertData(unsigned int ts)
{
	unsigned int i = ts%4096,n = ts/4096;
	unsigned int ret = 65536;

	if(i>=1700){i-=1700;ret=(ret*3)/4;}else{}
	if(i>=1700){i-=1700;ret=(ret*3)/4;}else{}
	if(i>=789){i-=789;ret=(ret*7)/8;}else{}
	if(i>=381){i-=381;ret=(ret*15)/16;}else{}  
	if(i>=188){i-=188;ret=(ret*31)/32;}else{}
	if(i>=188){i-=188;ret=(ret*31)/32;}else{}
	if(i>=93){i-=93;ret=(ret*61)/64;}else{}
	if(i>=46){i-=46;ret=(ret*127)/128;}else{}
	if(i>=23){i-=23;ret=(ret*255)/256;}else{}
	if(i>=11){i-=11;ret=(ret*511)/512;}else{}
	if(i>=6){i-=6;ret=(ret*1023)/1024;}else{}
	if(i>=3){i-=3;ret=(ret*2047)/2048;}else{} 
	if(i>=3){i-=3;ret=(ret*2047)/2048;}else{} 

	return ret>>n; 
}

/**
 * @brief	判断新旧电池以确认是否需要算法，只有旧电池才需要算法
 * @param	cw_bat:	cw_battery结构体指针
 * @param	SOC_NEW
 * @param	SOC_OLD
 * @retval	2: 旧电池，1: 新电池
 */
static int AlgNeed(int SOC_NEW, int SOC_OLD)
{
	printk("Chaman num = %d SOC_NEW = %d   SOC_OLD = %d \n", __LINE__ ,SOC_NEW, SOC_OLD);
	if(SOC_NEW - SOC_OLD > -20 && SOC_NEW - SOC_OLD < 20){
		return 2; // this is old battery
	}else{
		return 1; // this is new battery
	}
}

static int cw_algorithm(int real_capacity)
{
	struct file *file = NULL;
	struct cw_store st;
	struct inode *inode;
	mm_segment_t old_fs;
	int fileresult;
	int vmSOC; 
	unsigned int utemp,utemp1;
	long timeNow;
	int count = 0;
	static unsigned long timeNow_last  = -1;
	long timeChanged = 0;
	long timeChanged_rtc = 0;
	struct timestamp ts;		//时间戳

	static int count_fail=0;
	static int SOC_Dvalue = 0;
	static int Time_real3 = 0;
	static int Time_Dvalue = 0;
	static int Join_Fast_Close_SOC = 0;
	struct timestamp ktime_ts;	//内核时间戳？
	long suspend_time = 0;
	static unsigned long suspend_time_last  = 0;
	long suspend_time_changed = 0;
	static int count_time=0;
	static int mtk_init_vol = -10;
	static int return_vmSOC = 0;

#ifdef USE_MTK_INIT_VOL
	if(mtk_init_vol == -10 && g_mtk_init_vol != -10){
		mtk_init_vol = g_mtk_init_vol;
		mtk_init_vol = mtk_init_vol - 65;
		printk("Chaman %s %d mtk_init_vol = %d !\n", __FILE__, __LINE__, mtk_init_vol);
	}
	if(mtk_init_vol == -10){
		printk("Chaman check mtk init soc is not be saved! why??\n");
		return real_capacity;
	}
	printk("Chaman %s %d mtk_init_vol = %d !\n", __FILE__, __LINE__, mtk_init_vol);
#endif

	timeNow = get_seconds();
	vmSOC = real_capacity;

	/**
	 * 内核层读写应用层文件，使用filp_open函数
	 * https://blog.csdn.net/kylin_fire_zeng/article/details/44778155
	 * 本工程目录下也有该文章的.md文件
	 * 有时候需要在Linux kernel--大多是在需要调试的驱动程序--中读写文件数据。
	 * 在kernel中操作文件没有标准库可用，需要利用kernel的一些函数，这些函数主要有：
	 * 	filp_open()
	 * 	filp_close(),
	 * 	vfs_read()
	 * 	vfs_write()，
	 * 	set_fs()，
	 * 	get_fs()
	 * 	等，
	 * 	调用VFS（虚拟文件系统）的函数
	 */
	//在内核中创建一个文件并支持读写，权限0644
	//	file = filp_open(FILE_PATH,O_RDWR|O_CREAT,0644);
	file = filp_open("/data/lastsoc",O_RDWR|O_CREAT,0644);

	//检查文件创建是否成功
	if(IS_ERR(file))
	{
#if FG_CW2015_DEBUG
		FG_CW2015_ERR(" error occured while opening file %s,exiting...\n",FILE_PATH);
#endif
		return real_capacity;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS); 
	inode = file->f_dentry->d_inode;

	if((long)(inode->i_size)<(long)sizeof(st))
	{
		if(count_fail < 2)
		{
			count_fail++;
			filp_close(file,NULL);
			return real_capacity;
		}
		st.bts = timeNow;
		st.OldSOC = real_capacity;
		st.DetSOC = 0;
		st.AlRunFlag = -1; 
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("cw2015_file_test  file size error!\n");
#endif
	}
	else
	{
		count_fail=0;
		file->f_pos = 0;
		vfs_read(file,(char*)&st,sizeof(st),&file->f_pos);

#if FG_CW2015_DEBUG
		FG_CW2015_ERR(" success opening file, file_path=%s \n", FILE_PATH);
#endif
	}

	/**
	 * Linux时间子系统之三：时间的维护者：timekeeper
	 * https://www.cnblogs.com/arnoldlu/p/7078250.html
	 * timekeeper提供了一系列的接口用于获取各种时间信息。
		void getboottime(struct timestamp *ts);    获取系统启动时刻的实时时间
		void get_monotonic_boottime(struct timestamp *ts);     获取系统启动以来所经过的时间，包含休眠时间
		ktime_t ktime_get_boottime(void);   获取系统启动以来所经过的c时间，包含休眠时间，返回ktime类型
		ktime_t ktime_get(void);    获取系统启动以来所经过的c时间，不包含休眠时间，返回ktime类型
		void ktime_get_ts(struct timestamp *ts) ;   获取系统启动以来所经过的c时间，不包含休眠时间，返回timestamp结构
		unsigned long get_seconds(void);    返回xtime中的秒计数值
		struct timestamp current_kernel_time(void);    返回内核最后一次更新的xtime时间，不累计最后一次更新至今clocksource的计数值
		void getnstimeofday(struct timestamp *ts);    获取当前时间，返回timestamp结构
		void do_gettimeofday(struct timeval *tv);    获取当前时间，返回timeval结构

	 * Linux系统休眠（System Suspend）
	 */

	get_monotonic_boottime(&ts);				//获取系统启动以来所经过的时间，包含休眠时间
	ktime_get_ts(&ktime_ts);					//获取系统启动以来所经过的c时间，不包含休眠时间
	suspend_time = ts.tv_sec - ktime_ts.tv_sec;	//休眠时间 = 时间戳 - 内核时间戳
	if(timeNow_last != -1 && ts.tv_sec > DEVICE_RUN_TIME_FIX_VALUE)
	{
		/* 龟龟？*/
		//暂停时间变化量 = 暂停时间 - 上一次暂停时间
		//时间变化rtc 	 = 当前时间 - 上一次当前时间
		//时间变化 		 = 当前时间 - 上一次当前时间 - 暂停时间变化量
		suspend_time_changed = suspend_time - suspend_time_last;
		timeChanged_rtc 	 = timeNow 		- timeNow_last;
		timeChanged 		 = timeNow 		- timeNow_last - suspend_time_changed;

		printk(KERN_INFO "[FW_2015]suspend_time_changed = \t%ld,timeChanged_rtc = \t%ld, timeChanged = \t%ld\n",
				suspend_time_changed, timeChanged_rtc, timeChanged);

		if(timeChanged < -60 || timeChanged > 60)
		{
			st.bts = st.bts + timeChanged_rtc; // 时间戳加上RTC记录的充电时间
#if FG_CW2015_DEBUG
			FG_CW2015_ERR(" 1 st.bts = \t%ld\n", st.bts);
#endif
		}
	}
	timeNow_last = timeNow;				//上一次当前时间 = 当前时间
	suspend_time_last = suspend_time;	//上一次暂停时间 = 暂停时间

	if(((st.bts) < 0) || (st.OldSOC > 100) || (st.OldSOC < 0) || (st.DetSOC < -1)) 
	{
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("cw2015_file_test  reading file error!\n"); 
		FG_CW2015_ERR("cw2015_file_test  st.bts = %ld st.OldSOC = %d st.DetSOC = %d st.AlRunFlag = %d  vmSOC = %d  2015SOC=%d\n",st.bts,st.OldSOC,st.DetSOC,st.AlRunFlag,vmSOC,real_capacity); 
#endif
		st.bts = timeNow;
		st.OldSOC = real_capacity;
		st.DetSOC = 0;
		st.AlRunFlag = -1; 
	}


	if(PowerResetFlag == 1)
	{
		PowerResetFlag = -1;
#ifdef USE_MTK_INIT_VOL

		/**
		 * mtk_init_vol 是全局变量 g_mtk_init_vol 的值，
		 * battery_meter.c 中
		 * g_mtk_init_vol = gFG_voltage;
		 * ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &g_booting_vbat);
		 * gFG_voltage = g_booting_vbat;
		 * 推测 mtk_init_vol 记录了开机时的电池电压，且仅开机时被赋值一次
		 * 以下if-else应该是按照“电压-SOC曲线”写的
		 */
		if(mtk_init_vol > 4372){				// mtk_init_vol 锂电池 4.372V 电量 (100 - real_capacity)%
			st.DetSOC = 100 - real_capacity;
		}else if(mtk_init_vol > 4349){
			st.DetSOC = 98 - real_capacity;
		}else if(mtk_init_vol > 4325){
			st.DetSOC = 96 - real_capacity;
		}else if(mtk_init_vol > 4302){
			st.DetSOC = 94 - real_capacity;
		}else if(mtk_init_vol > 4278){
			st.DetSOC = 92 - real_capacity;
		}else if(mtk_init_vol > 4255){
			st.DetSOC = 90 - real_capacity;
		}else if(mtk_init_vol > 4233){
			st.DetSOC = 88 - real_capacity;
		}else if(mtk_init_vol > 4211){
			st.DetSOC = 86 - real_capacity;
		}else if(mtk_init_vol > 4189){
			st.DetSOC = 84 - real_capacity;
		}else if(mtk_init_vol > 4168){
			st.DetSOC = 82 - real_capacity;
		}else if(mtk_init_vol > 4147){
			st.DetSOC = 80 - real_capacity;
		}else if(mtk_init_vol > 4126){
			st.DetSOC = 78 - real_capacity;
		}else if(mtk_init_vol > 4106){
			st.DetSOC = 76 - real_capacity;
		}else if(mtk_init_vol > 4089){
			st.DetSOC = 74 - real_capacity;
		}else if(mtk_init_vol > 4071){
			st.DetSOC = 72 - real_capacity;
		}else if(mtk_init_vol > 4048){
			st.DetSOC = 70 - real_capacity;
		}else if(mtk_init_vol > 4024){
			st.DetSOC = 68 - real_capacity;
		}else if(mtk_init_vol > 4001){
			st.DetSOC = 66 - real_capacity;
		}else if(mtk_init_vol > 3977){
			st.DetSOC = 64 - real_capacity;
		}else if(mtk_init_vol > 3965){
			st.DetSOC = 62 - real_capacity;
		}else if(mtk_init_vol > 3953){
			st.DetSOC = 60 - real_capacity;
		}else if(mtk_init_vol > 3936){
			st.DetSOC = 58 - real_capacity;
		}else if(mtk_init_vol > 3919){
			st.DetSOC = 56 - real_capacity;
		}else if(mtk_init_vol > 3901){
			st.DetSOC = 54 - real_capacity;
		}else if(mtk_init_vol > 3882){
			st.DetSOC = 52 - real_capacity;
		}else if(mtk_init_vol > 3869){
			st.DetSOC = 50 - real_capacity;
		}else if(mtk_init_vol > 3857){
			st.DetSOC = 48 - real_capacity;
		}else if(mtk_init_vol > 3846){
			st.DetSOC = 46 - real_capacity;
		}else if(mtk_init_vol > 3835){
			st.DetSOC = 44 - real_capacity;
		}else if(mtk_init_vol > 3827){
			st.DetSOC = 42 - real_capacity;
		}else if(mtk_init_vol > 3818){
			st.DetSOC = 40 - real_capacity;
		}else if(mtk_init_vol > 3811){
			st.DetSOC = 38 - real_capacity;
		}else if(mtk_init_vol > 3804){
			st.DetSOC = 36 - real_capacity;
		}else if(mtk_init_vol > 3797){
			st.DetSOC = 34 - real_capacity;
		}else if(mtk_init_vol > 3790){
			st.DetSOC = 32 - real_capacity;
		}else if(mtk_init_vol > 3786){
			st.DetSOC = 30 - real_capacity;
		}else if(mtk_init_vol > 3781){
			st.DetSOC = 28 - real_capacity;
		}else if(mtk_init_vol > 3775){
			st.DetSOC = 26 - real_capacity;
		}else if(mtk_init_vol > 3770){
			st.DetSOC = 24 - real_capacity;
		}else if(mtk_init_vol > 3762){
			st.DetSOC = 22 - real_capacity;
		}else if(mtk_init_vol > 3753){
			st.DetSOC = 20 - real_capacity;
		}else if(mtk_init_vol > 3742){
			st.DetSOC = 18 - real_capacity;
		}else if(mtk_init_vol > 3731){
			st.DetSOC = 16 - real_capacity;
		}else if(mtk_init_vol > 3715){
			st.DetSOC = 14 - real_capacity;
		}else if(mtk_init_vol > 3699){
			st.DetSOC = 12 - real_capacity;
		}else if(mtk_init_vol > 3694){
			st.DetSOC = 10 - real_capacity;
		}else if(mtk_init_vol > 3689){
			st.DetSOC = 8 - real_capacity;
		}else if(mtk_init_vol > 3681){
			st.DetSOC = 6 - real_capacity;
		}else if(mtk_init_vol > 3673){
			st.DetSOC = 4 - real_capacity;
		}else if(mtk_init_vol > 3660){		// mtk_init_vol 锂电池 3.660V，电量 (3 - real_capacity)%
			st.DetSOC = 3 - real_capacity;
		}else{
			st.DetSOC = 1;
		}
#else 
		/**
		 * 新旧电池的检查
		 * 每次上电cw2015都需要由主机写入电池建模信息，
		 * 而该建模信息来自于新电池，cw2015无法断电记忆
		 * cw2015无法自行跟踪电池衰减
		 *
		 * 手机使用得越久，电池最大容量会就衰减得越多，
		 * 新电池的建模信息会越来越不适用旧电池
		 * 这个驱动搞了个旧电池容量补偿算法
		 * 首先第一步要判断旧电池的衰减情况
		 * 以下if-else应该是按照“容量-SOC曲线”写的
		 * 根据cw2015计算的“real_capacity实际容量”设置DetSOC
		 */
		if(hmi_battery_version==2){			//德赛电池
			if(real_capacity == 0){
				st.DetSOC = 1;
			}else if(real_capacity == 1){
				st.DetSOC = 3;
			}else if(real_capacity == 2){
				st.DetSOC = 10;
			}else if(real_capacity == 3){
				st.DetSOC = 19;
			}else if(real_capacity == 4){
				st.DetSOC = 20;
			}else if(real_capacity == 5){
				st.DetSOC = 22;
			}else if(real_capacity < 14){
				st.DetSOC = 23;
			}else if(real_capacity < 21){
				st.DetSOC = 26;
			}else if(real_capacity < 26){
				st.DetSOC = 25;
			}else if(real_capacity < 31){
				st.DetSOC = 23;
			}else if(real_capacity < 36){
				st.DetSOC = 22;
			}else if(real_capacity < 41){
				st.DetSOC = 20;
			}else if(real_capacity < 51){
				st.DetSOC = 16;
			}else if(real_capacity < 61){
				st.DetSOC = 9;
			}else if(real_capacity < 71){
				st.DetSOC = 7;
			}else if(real_capacity < 81){
				st.DetSOC = 8;
			}else if(real_capacity < 88){
				st.DetSOC = 9;
			}else if(real_capacity < 94){
				st.DetSOC = 6;
			}else if(real_capacity <= 100){		//若 94% <= 实际容量 <= 100%
				vmSOC = 100;
				st.DetSOC = 100 - real_capacity;
			}
		}
#endif

		//只有旧电池才需要算法
		if(AlgNeed(
				cw_bat,
				st.DetSOC + real_capacity,		// SOC_NEW
				st.OldSOC						// SOC_OLD
				) == 2)
		{
			st.DetSOC = st.OldSOC - real_capacity + 1;
#if FG_CW2015_DEBUG
			FG_CW2015_ERR("st.DetDoc=%d\n", st.DetSOC);
#endif
		}

		// 旧电池才需要算法，将算法执行标志置为1
		st.AlRunFlag = 1;
		st.bts = timeNow;					// powerResetFlag = 1，那么时间戳为当前时间
		vmSOC = real_capacity + st.DetSOC;	// powerResetFlag = 1，电量 = 实际容量 + 检测容量
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("cw2015_file_test  PowerResetFlag == 1!\n");
#endif
	}
	//如果 Join_Fast_Close_SOC = 1，并且需要算法
	else if(Join_Fast_Close_SOC && (st.AlRunFlag > 0))
	{
		if(timeNow >= (Time_real3 + Time_Dvalue)){
			vmSOC = st.OldSOC - 1;
			Time_real3 = timeNow;
		}
		else{
			vmSOC = st.OldSOC;
		}
		if (vmSOC == real_capacity)
		{
			st.AlRunFlag = -1;
#if FG_CW2015_DEBUG
			FG_CW2015_ERR("cw2015_file_test  algriothm end of decrease acceleration\n");
#endif
		}
	}
	//如果 Join_Fast_Close_SOC = 0，需要算法，且 st.DetSOC != 0
	else if(((st.AlRunFlag) >0)&&((st.DetSOC) != 0))
	{
		get_monotonic_boottime(&ts);
		if(real_capacity < 1 &&							//
				cw_bat->charger_mode == 0 &&			//非充电状态
				ts.tv_sec > DEVICE_RUN_TIME_FIX_VALUE	//系统启动时间大于40s
		   ){
			if (SOC_Dvalue == 0){
				SOC_Dvalue = st.OldSOC - real_capacity;	//SOC变化值
				if(SOC_Dvalue == 0)
				{
					st.AlRunFlag = -1;
					// 减加速度算法结束
					printk(KERN_INFO "[FG_CW2015]cw2015_file_test  algriothm end of decrease acceleration[2]\n");
				}
				else
				{
					// 减加速度算法开始
					printk(KERN_INFO "[FG_CW2015]cw2015_file_test  begin of decrease acceleration \n");
					Time_real3 = timeNow;
					if((cw_bat->voltage) < 3480){
						Time_Dvalue = 20/(SOC_Dvalue);
					}
					else{
						Time_Dvalue = 90/(SOC_Dvalue);
					}
					Join_Fast_Close_SOC = 1;
					vmSOC = st.OldSOC;
				}
			}		
		}
		else
		{
			utemp1 = 32768/(st.DetSOC);
			if((st.bts) < timeNow)
				utemp = cw_convertData((timeNow-st.bts));
			else
				utemp = cw_convertData(1);
#if FG_CW2015_DEBUG
			FG_CW2015_ERR("cw2015_file_test  convertdata = %d\n",utemp);
#endif
			if((st.DetSOC)<0)
				vmSOC = real_capacity-(int)((((unsigned int)((st.DetSOC)*(-1))*utemp)+utemp1)/65536);
			else
				vmSOC = real_capacity+(int)((((unsigned int)(st.DetSOC)*utemp)+utemp1)/65536);

			if (vmSOC == real_capacity)
			{
				st.AlRunFlag = -1;
#if FG_CW2015_DEBUG
				FG_CW2015_ERR("cw2015_file_test  algriothm end\n");
#endif
			}
		}
	}
	//如果不需要算法
	else
	{
		st.AlRunFlag = -1;
		st.bts = timeNow;
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("cw2015_file_test  no algriothm\n");
#endif
	}
#if FG_CW2015_DEBUG
	FG_CW2015_ERR("cw2015_file_test debugdata,\t%ld,\t%d,\t%d,\t%d,\t%d,\t%ld,\t%d,\t%d,\t%d\n",timeNow,cw_bat->capacity,cw_bat->voltage,vmSOC,st.DetSOC,st.bts,st.AlRunFlag,real_capacity,st.OldSOC);
#endif
	alg_run_flag = st.AlRunFlag;

	//容量归并到有效范围
	if(vmSOC>100)
		vmSOC = 100;
	else if(vmSOC<0)    
		vmSOC = 0;

	//将st结构体写入文件并关闭，以断电记忆？
	st.OldSOC = vmSOC;
	file->f_pos = 0;
	vfs_write(file,(char*)&st,sizeof(st),&file->f_pos);
	set_fs(old_fs);
	filp_close(file,NULL);
	file = NULL;

	if(return_vmSOC < 5){
		if(return_vmSOC > 0){
			file_sys_state = 2;
			printk("cw2015 return_vmsoc=%d, file_sys_state=%d \n", return_vmSOC, file_sys_state);
		}
		return_vmSOC++;
	}
	return vmSOC;

}
#endif

/**
 * @brief	读1个8bit数据
 */
static int cw_read(uint8_t reg, uint8_t buf[])
{
	int ret = 0;

	ret = FRToSI2C2.Mem_Read(CW2015_ADDR_7BIT << 1, reg, buf, 1);
//	ret = i2c_smbus_read_byte_data(client,reg);
//	if (ret < 0) return ret;
//	else {
//		buf[0] = ret;
//		ret = 0;
//	}

	return ret;
}

/**
 * @brief	写1个8bit数据
 */
static int cw_write(uint8_t reg, uint8_t buf[])
{
	int ret = 0;

//	ret =  i2c_smbus_write_byte_data(client,reg,buf[0]);
	ret = FRToSI2C2.Mem_Write(CW2015_ADDR_7BIT << 1, reg, buf, 1);
	return ret;
}

/**
 * @brief	读1个16bit数据
 */
static int cw_read_word(uint8_t reg, uint8_t * pbuf)
{
	int ret = 0;
	cw_read(reg, pbuf);
	cw_read(reg+1, pbuf+1);
//	ret = FRToSI2C2.readWord(CW2015_ADDR_7BIT << 1, reg, pbuf);
//	ret = FRToSI2C2.readWord(CW2015_ADDR_7BIT << 1, reg, pbuf);
//	data = i2c_smbus_read_word_data(client, reg);
//	buf[0] = data & 0x00FF;			// 00000000,11111111
//	buf[1] = (data & 0xFF00)>>8;	// 11111111,00000000

	return ret;
}

/**
 * @brief	配置cw2015寄存器
 */
static int cw_update_config_info()
{
	int ret;
	uint8_t reg_val;
	int i;
	uint8_t reset_val;

#if FG_CW2015_DEBUG
	FG_CW2015_LOG("func: %s-------\n", __func__);
#endif

#if FG_CW2015_DEBUG
#ifdef CONFIG_CM865_MAINBOARD
	if(hmi_battery_version==2)
		FG_CW2015_LOG("test cw_bat_config_info = 0x%x",config_info_sun[0]);
	else if(hmi_battery_version==3)
		FG_CW2015_LOG("test cw_bat_config_info = 0x%x",config_info_scud[0]);
	else
		FG_CW2015_LOG("test cw_bat_config_info = 0x%x",config_info_cos[0]);      
#else
	if(hmi_battery_version==2)
		FG_CW2015_LOG("test cw_bat_config_info = 0x%x",config_info_des[0]);//liuchao
	else
		FG_CW2015_LOG("test cw_bat_config_info = 0x%x",config_info_cos[0]);//liuchao
#endif
#endif
	/**
	 * 检测cw2015是否处于睡眠模式
	 * 睡眠模式则输出错误信息并终止继续执行本函数
	 * make sure no in sleep mode
	 */
	ret = cw_read(REG_MODE, &reg_val);
#if FG_CW2015_DEBUG
	FG_CW2015_LOG("cw_update_config_info reg_val = 0x%x",reg_val);
#endif
	if (ret < 0)
		return ret;

	reset_val = reg_val;
	if((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP) {
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("Error, device in sleep mode, cannot update battery info\n");
#endif
		return -1;
	}


	/**
	 * 写入新电池模型
	 * update new battery info
	 */
	for (i = 0; i < SIZE_BATINFO; i++) {
			ret = cw_write(REG_BATINFO + i, &config_info_des[i]);
		if (ret < 0)
			return ret;
	}

	/**
	 * 读取写入的模型信息校验
	 * readback & check
	 */
	for (i = 0; i < SIZE_BATINFO; i++)
	{
		ret = cw_read(REG_BATINFO + i, &reg_val);
		if (reg_val != config_info_des[i])
			return -1;
	}

	/**
	 * 写入电池信息后，UFG位写1表示电池信息更新，并设置ATHD报警阈值是0%电量
	 * set cw2015/cw2013 to use new battery info */
	ret = cw_read(REG_CONFIG, &reg_val);
	if (ret < 0)
		return ret;

	reg_val |= CONFIG_UPDATE_FLG;   /* set UPDATE_FLAG , UFG是标志位，用于指示电池信息更新状态。1代表更新 */
	reg_val &= 0x07;                /* clear ATHD */
	reg_val |= ATHD;                /* set ATHD 报警阈值 0% */
	ret = cw_write(REG_CONFIG, &reg_val);
	if (ret < 0)
		return ret;

	/**
	 * 读取以校对一次UFG和ATHD值
	 * check 2015/cw2013 for ATHD & update_flag
	 */
	ret = cw_read(REG_CONFIG, &reg_val);
	if (ret < 0)
		return ret;

	if (!(reg_val & CONFIG_UPDATE_FLG)) {
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("update flag for new battery info have not set..\n");
#endif
	}

	if ((reg_val & 0xf8) != ATHD) {
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("the new ATHD have not set..\n");
#endif
	}


	/**
	 * 向POR位写入1111以等效断电后完全重启cw2015，
	 * 这么说64字节电池信息可以断电保存？？
	 * reset
	 */
	reset_val &= ~(MODE_RESTART);			// MODE_RESTART按位取反得到11110000，再& reset_val
	reg_val = reset_val | MODE_RESTART;		// 0xf : 00001111
	ret = cw_write(REG_MODE, &reg_val);
	if (ret < 0)
		return ret;
	msleep(10);								// 等待cw2015 10ms重启
	ret = cw_write(REG_MODE, &reset_val);	// 恢复MODE寄存器除POR位的值
	if (ret < 0)
		return ret;

#ifdef BAT_CHANGE_ALGORITHM
	PowerResetFlag = 1;						//
#if FG_CW2015_DEBUG
	FG_CW2015_ERR("cw2015_file_test  set PowerResetFlag/n ");
#endif
#endif
	msleep(10);

	return 0;
}

/**
 * 初始化cw2015
 *
 */
static int cw_init()
{
	int ret;
	int i;
	uint8_t reg_val = MODE_SLEEP;

	/**
	 * 写 Sleep 00 以唤醒 cw2015
	 */
	if ((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP) 
	{
		reg_val = MODE_NORMAL;

		ret = cw_write(REG_MODE, &reg_val);
		if (ret < 0)
			return ret;

	}

	ret = cw_read(REG_CONFIG, &reg_val);
	if (ret < 0)
		return ret;

#if FG_CW2015_DEBUG
	FG_CW2015_LOG("the new ATHD have not set reg_val = 0x%x\n",reg_val);
#endif
	if ((reg_val & 0xf8) != ATHD) 
	{
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("the new ATHD have not set\n");
#endif
		reg_val &= 0x07;    /* clear ATHD */
		reg_val |= ATHD;    /* set ATHD */
		ret = cw_write(REG_CONFIG, &reg_val);
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("cw_init 1111\n");
#endif
		if (ret < 0)
			return ret;
	}

	ret = cw_read(REG_CONFIG, &reg_val);
	if (ret < 0)
		return ret;

#if FG_CW2015_DEBUG
	FG_CW2015_LOG("cw_init REG_CONFIG = %d\n",reg_val);
#endif

	/**
	 * 检查 UFG bit 是否为 1
	 * 为 1 表示电池信息被主机写入更新过
	 */
	if (!(reg_val & CONFIG_UPDATE_FLG))
	{
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("update flag for new battery info have not set\n");
#endif
		ret = cw_update_config_info();
		if (ret < 0)
			return ret;
	}
	/**
	 * UFG bit 为 0，则有可能需要主机写入电池信息
	 */
	else
	{
		// 先读取64字节电池信息
		for(i = 0; i < SIZE_BATINFO; i++) 
		{ 
			ret = cw_read((REG_BATINFO + i), &reg_val);
			if (ret < 0)
				return ret;

			// 若当前字节cw2015里的电池信息与当前程序使用的电池信息不相等，则退出继续读取
			if (config_info_des[i] != reg_val)
				break;
		}
		// 若 i 不等于 64，则重新配置所有寄存器并写入当前程序使用的电池信息
		if (i != SIZE_BATINFO) {
#if FG_CW2015_DEBUG
			FG_CW2015_LOG("update flag for new battery info have not set\n"); 
#endif
			ret = cw_update_config_info();
			if (ret < 0)
				return ret; 
		}
	}

	/**
	 * 30*100ms = 3s
	 * 在3s内读30次 SOC位
	 */
	for (i = 0; i < 30; i++) 
	{
		ret = cw_read(REG_SOC, &reg_val);
		if (ret < 0)
			return ret;

		else if (reg_val <= 0x64) 	// 0x64 = 100 DEC 单位 %，读到SOC <= 100% 则退出继续读取
			break;

		msleep(100);
		if (i > 25)
		{
#if FG_CW2015_DEBUG
			FG_CW2015_ERR("cw2015/cw2013 input unvalid power error\n");
#endif
		}

	}
	//30次读完了SOC还是没有<=100%，让cw2015休眠
	if (i >=30)
	{
		reg_val = MODE_SLEEP;
		ret = cw_write(REG_MODE, &reg_val);
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("cw2015/cw2013 input unvalid power error_2\n");	// 输入未验证的电源错误
#endif
		return -1;
	}
	CW2015_test_init=1;

	return 0;
}

/**
 * @brief 	在开始充电时更新cw_bat的时间相关的成员
 * 			run_time_charge_start、sleep_time_charge_start
 */
static void cw_update_time_member_charge_start()
{
	struct timestamp ts;
	int new_run_time;
	int new_sleep_time;

	ktime_get_ts(&ts);				//获取系统启动以来所经过的c时间，不包含休眠时间
	new_run_time = ts.tv_sec;

	get_monotonic_boottime(&ts); 	//获取系统启动以来所经过的时间，包含休眠时间
	new_sleep_time = ts.tv_sec - new_run_time;

	cw_bat->run_time_charge_start = new_run_time;		//
	cw_bat->sleep_time_charge_start = new_sleep_time; 	//
}

static void cw_update_time_member_capacity_change()
{
	struct timestamp ts;
	int new_run_time;
	int new_sleep_time;

	ktime_get_ts(&ts);
	new_run_time = ts.tv_sec;

	get_monotonic_boottime(&ts);
	new_sleep_time = ts.tv_sec - new_run_time;

	cw_bat->run_time_capacity_change = new_run_time;
	cw_bat->sleep_time_capacity_change = new_sleep_time; 
}


static int cw_quickstart()
{
	int ret = 0;
	uint8_t reg_val = MODE_QUICK_START;

	ret = cw_write(REG_MODE, &reg_val);
	if(ret < 0) {
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("Error quick start1\n");
#endif
		return ret;
	}

	reg_val = MODE_NORMAL;

	ret = cw_write(REG_MODE, &reg_val);
	if(ret < 0) {
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("Error quick start2\n");
#endif
		return ret;
	}
	return 1;
}

/**
 * 获取电量，被rk_bat_update_capacity调用
 */
static int cw_get_capacity()
{
	int cw_capacity;
	int ret;
	uint8_t reg_val[2];

	struct timestamp ts;
	long new_run_time;
	long new_sleep_time;
	long capacity_or_aconline_time;
	int allow_change;
	int allow_capacity;
	static int if_quickstart = 0;
	static int jump_flag =0;
	static int reset_loop =0;
	int charge_time;
	uint8_t reset_val;
	int loop =0;
	static int count_time=0;
	static int count_time_sum=0;
	static int count_real_capacity=0;
	uint8_t count_real_sum = 0;

	/**
	 * 读SOC 16bit寄存器
	 * 高八位为百分比，及小数位单位为1%的1/256，小数位除以256加上高八位最终输出百分比。
	 */
	ret = cw_read_word(REG_SOC, reg_val);
	if (ret < 0)
		return ret;

#if FG_CW2015_DEBUG
	FG_CW2015_LOG("cw_get_capacity cw_capacity_0 = %d,cw_capacity_1 = %d\n",reg_val[0],reg_val[1]);
#endif
	cw_capacity = reg_val[0];
	if ((cw_capacity < 0) || (cw_capacity > 100)) {
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("get cw_capacity error; cw_capacity = %d\n", cw_capacity);
#endif
		reset_loop++;

		if (reset_loop >5) {
			reset_val = MODE_SLEEP;
			ret = cw_write(REG_MODE, &reset_val);
			if (ret < 0)
				return ret;
			reset_val = MODE_NORMAL;
			msleep(10);
			ret = cw_write(REG_MODE, &reset_val);
			if (ret < 0)
				return ret;

			ret = cw_init();
			if (ret) 
				return ret;
			reset_loop =0;
		}

		return cw_capacity;
	} else {
		reset_loop =0;
	}

#if 0  ///ODG
#if FG_CW2015_DEBUG
	if (cw_capacity == 0) 
		FG_CW2015_LOG("the cw201x capacity is 0 !!!!!!!, funciton: %s, line: %d\n", __func__, __LINE__);
	else
		FG_CW2015_LOG("the cw201x capacity is %d, funciton: %s\n", cw_capacity, __func__);
#endif

	//如果使用软件SOC算法

#ifdef  BAT_CHANGE_ALGORITHM
	if( g_platform_boot_mode == 8 )
	{
		PowerResetFlag = -1; 
		count_real_sum = 26;
	}else{
		count_real_sum = 5;
	}
	cw_capacity = cw_algorithm(cw_bat,cw_capacity);
#endif

	ktime_get_ts(&ts);
	new_run_time = ts.tv_sec;

	get_monotonic_boottime(&ts);
	new_sleep_time = ts.tv_sec - new_run_time;
#if FG_CW2015_DEBUG
	FG_CW2015_LOG("cw_get_capacity cw_bat->charger_mode = %d\n",cw_bat->charger_mode);
#endif
	//count_time == 20s  

		if(count_real_capacity <= count_real_sum) {
			count_real_capacity++;
#if FG_CW2015_DEBUG
			FG_CW2015_LOG("count_real_capacity = %d\n",cw_bat->charger_mode);
#endif
		}

#ifdef CONFIG_CM865_MAINBOARD //add by longcheer_liml_2015_10_12
	if(Charger_enable_Flag==0)
	{
		if ((cw_bat->charger_mode == 0) && (cw_capacity > cw_bat->capacity)&&(cw_capacity < (cw_bat->capacity+20))&&(count_real_capacity>= count_real_sum ))
		{             // modify battery level swing
			if (!(cw_capacity == 0 && cw_bat->capacity <= 2))
		{
			cw_capacity = cw_bat->capacity;
		}
	}
	}else{
		if (
#ifdef CHARGING_NO_DOWN_CAP //liuchao
		((cw_bat->charger_mode > 0) && (cw_capacity <= (cw_bat->capacity - 1)) && (cw_capacity > (cw_bat->capacity - 9)))
		||
#endif
		((cw_bat->charger_mode == 0) && (cw_capacity > cw_bat->capacity)&&(cw_capacity < (cw_bat->capacity+20))&&(count_real_capacity>= count_real_sum ) ))
		{             // modify battery level swing
			if (!(cw_capacity == 0 && cw_bat->capacity <= 2))
		{
			cw_capacity = cw_bat->capacity;
		}
		}
	}

#else
	if (
#ifdef CHARGING_NO_DOWN_CAP //liuchao
		((cw_bat->charger_mode > 0) && (cw_capacity <= (cw_bat->capacity - 1)) && (cw_capacity > (cw_bat->capacity - 9))) ||
#endif
		((cw_bat->charger_mode == 0) && (cw_capacity > cw_bat->capacity)&&(cw_capacity < (cw_bat->capacity+20))&&(count_real_capacity>= count_real_sum ) )) {             // modify battery level swing
		if (!(cw_capacity == 0 && cw_bat->capacity <= 2)) 
		{		
			cw_capacity = cw_bat->capacity;
		}
	}
#endif
	// 避免充不满
	// avoid no charge full
	if ((cw_bat->charger_mode > 0) && (cw_capacity >= 95) && (cw_capacity <= cw_bat->capacity)) {
		capacity_or_aconline_time = (cw_bat->sleep_time_capacity_change > cw_bat->sleep_time_charge_start) ? cw_bat->sleep_time_capacity_change : cw_bat->sleep_time_charge_start;
		capacity_or_aconline_time += (cw_bat->run_time_capacity_change > cw_bat->run_time_charge_start) ? cw_bat->run_time_capacity_change : cw_bat->run_time_charge_start;
		allow_change = (new_sleep_time + new_run_time - capacity_or_aconline_time) / BATTERY_UP_MAX_CHANGE;
		if (allow_change > 0) {
			allow_capacity = cw_bat->capacity + allow_change; 
			cw_capacity = (allow_capacity <= 100) ? allow_capacity : 100;
			jump_flag =1;
		} else if (cw_capacity <= cw_bat->capacity) {
			cw_capacity = cw_bat->capacity; 
		}

	}		 

	// 避免电池电量跳到 CW_BAT
	// avoid battery level jump to CW_BAT
	else if ((cw_bat->charger_mode == 0) && (cw_capacity <= cw_bat->capacity ) && (cw_capacity >= 90) && (jump_flag == 1)) {
		capacity_or_aconline_time = (cw_bat->sleep_time_capacity_change > cw_bat->sleep_time_charge_start) ? cw_bat->sleep_time_capacity_change : cw_bat->sleep_time_charge_start;
		capacity_or_aconline_time += (cw_bat->run_time_capacity_change > cw_bat->run_time_charge_start) ? cw_bat->run_time_capacity_change : cw_bat->run_time_charge_start;
		allow_change = (new_sleep_time + new_run_time - capacity_or_aconline_time) / BATTERY_DOWN_CHANGE;
		if (allow_change > 0) {
			allow_capacity = cw_bat->capacity - allow_change; 
			if (cw_capacity >= allow_capacity) {
				jump_flag =0;
			}
			else{
				cw_capacity = (allow_capacity <= 100) ? allow_capacity : 100;
			}
		} else if (cw_capacity <= cw_bat->capacity) {
			cw_capacity = cw_bat->capacity;
		}
	}

	// 避免电池电量瞬间从超过 2% 跳到 0%
	// avoid battery level jump to 0% at a moment from more than 2%
	if ((cw_capacity == 0) && (cw_bat->capacity > 1)) {
		allow_change = ((new_run_time - cw_bat->run_time_capacity_change) / BATTERY_DOWN_MIN_CHANGE_RUN);
		allow_change += ((new_sleep_time - cw_bat->sleep_time_capacity_change) / BATTERY_DOWN_MIN_CHANGE_SLEEP);

		allow_capacity = cw_bat->capacity - allow_change;
		cw_capacity = (allow_capacity >= cw_capacity) ? allow_capacity: cw_capacity;
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("report GGIC POR happened\n");
#endif
		reset_val = MODE_SLEEP;               
		ret = cw_write(REG_MODE, &reset_val);
		if (ret < 0)
			return ret;
		reset_val = MODE_NORMAL;
		msleep(10);
		ret = cw_write(REG_MODE, &reset_val);
		if (ret < 0)
			return ret;

		ret = cw_init();
		if (ret) 
			return ret;	 
	}

	if((cw_bat->charger_mode > 0) &&(cw_capacity == 0))
	{		  
		charge_time = new_sleep_time + new_run_time - cw_bat->sleep_time_charge_start - cw_bat->run_time_charge_start;
		if ((charge_time > BATTERY_DOWN_MAX_CHANGE_RUN_AC_ONLINE) && (if_quickstart == 0)) {
			reset_val = MODE_SLEEP;               
			ret = cw_write(REG_MODE, &reset_val);
			if (ret < 0)
				return ret;
			reset_val = MODE_NORMAL;
			msleep(10);
			ret = cw_write(REG_MODE, &reset_val);
			if (ret < 0)
				return ret;

			ret = cw_init();
			if (ret) 
				return ret;
#if FG_CW2015_DEBUG
			FG_CW2015_LOG("report battery capacity still 0 if in changing\n");
#endif
			if_quickstart = 1;
		}
	} else if ((if_quickstart == 1)&&(cw_bat->charger_mode == 0)) {
		if_quickstart = 0;
	}

#ifdef SYSTEM_SHUTDOWN_VOLTAGE
	// cw201x 电压低于 SYSTEM_SHUTDOWN_VOLTAGE！
	if ((cw_bat->charger_mode == 0) && (cw_capacity <= 20) && (cw_bat->voltage <= SYSTEM_SHUTDOWN_VOLTAGE))
	{
		if (if_quickstart == 10)
		{

			allow_change = ((new_run_time - cw_bat->run_time_capacity_change) / BATTERY_DOWN_MIN_CHANGE_RUN);
			allow_change += ((new_sleep_time - cw_bat->sleep_time_capacity_change) / BATTERY_DOWN_MIN_CHANGE_SLEEP);

			allow_capacity = cw_bat->capacity - allow_change;
			cw_capacity = (allow_capacity >= 0) ? allow_capacity: 0;

			if (cw_capacity < 1)
			{
				cw_quickstart();
				if_quickstart = 12;
				cw_capacity = 0;
			}
		} else if (if_quickstart <= 10)
			if_quickstart =if_quickstart + 2;
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("the cw201x voltage is less than SYSTEM_SHUTDOWN_VOLTAGE !!!!!!!, funciton: %s, line: %d\n", __func__, __LINE__);
#endif
	}
	else if ((cw_bat->charger_mode > 0)&& (if_quickstart <= 12)) {
		if_quickstart = 0;
	}
#endif
#endif	//ODG
	return cw_capacity;
}

int cw2015_check = 0;

/**
 * 获取电池电压
 */
static int cw_get_vol()
{
	int ret;
	uint8_t reg_val[2];
	uint16_t value16, value16_1, value16_2, value16_3;
	int voltage;
#if FG_CW2015_DEBUG
	FG_CW2015_LOG("cw_get_vol \n");
#endif

	ret = cw_read_word(REG_VCELL, reg_val);
	if (ret < 0)
	{
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("cw_get_vol 1111\n");
#endif
		return ret;
	}
	value16 = (reg_val[0] << 8) + reg_val[1];

	ret = cw_read_word(REG_VCELL, reg_val);
	if (ret < 0)
	{
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("cw_get_vol 2222\n");
#endif
		return ret;
	}
	value16_1 = (reg_val[0] << 8) + reg_val[1];

	ret = cw_read_word(REG_VCELL, reg_val);
	if (ret < 0)
	{
#if FG_CW2015_DEBUG
		FG_CW2015_LOG("cw_get_vol 3333\n");
#endif
		return ret;
	}
	value16_2 = (reg_val[0] << 8) + reg_val[1];

	if(value16 > value16_1)
	{	 
		value16_3 = value16;
		value16 = value16_1;
		value16_1 = value16_3;
	}

	if(value16_1 > value16_2)
	{
		value16_3 =value16_1;
		value16_1 =value16_2;
		value16_2 =value16_3;
	}

	if(value16 >value16_1)
	{	 
		value16_3 =value16;
		value16 =value16_1;
		value16_1 =value16_3;
	}		

	voltage = value16_1 * 312 / 1024;
	//voltage = voltage * 1000;
#if FG_CW2015_DEBUG
	FG_CW2015_LOG("cw_get_vol 4444 voltage = %d\n",voltage);
#endif
	if(voltage ==0)
		cw2015_check++;
	return voltage;
}

#ifdef BAT_LOW_INTERRUPT
/**
 * 获取低电量中断标记
 */
static int cw_get_alt()
{
	int ret = 0;
	uint8_t reg_val;
	uint8_t value8 = 0;
	int alrt;

	ret = cw_read(REG_RRT_ALERT, &reg_val);
	if (ret < 0)
		return ret;
	value8 = reg_val;
	alrt = value8 >> 7;

	value8 = value8 & 0x7f;
	reg_val = value8;
	ret = cw_write(REG_RRT_ALERT, &reg_val);
	if(ret < 0) {
#if FG_CW2015_DEBUG
		FG_CW2015_ERR( "Error clear ALRT\n");
#endif
		return ret;
	}
	return alrt;
}
#endif

static int cw_get_time_to_empty()
{
	int ret;
	uint8_t reg_val;
	uint16_t value16;

	ret = cw_read(REG_RRT_ALERT, &reg_val);
	if (ret < 0)
		return ret;

	value16 = reg_val;

	ret = cw_read(REG_RRT_ALERT + 1, &reg_val);
	if (ret < 0)
		return ret;

	value16 = ((value16 << 8) + reg_val) & 0x1fff;
	return value16;
}

/**
 * 更新电量，被 cw_bat_work 调用
 */
static void rk_bat_update_capacity()
{
	int cw_capacity;
#ifdef BAT_CHANGE_ALGORITHM
	cw_capacity = cw_get_capacity();
#if FG_CW2015_DEBUG
	FG_CW2015_ERR("cw2015_file_test userdata,	%ld,	%d,	%d\n",get_seconds(),cw_capacity,cw_bat->voltage);
#endif
#else
	cw_capacity = cw_get_capacity();
#endif
	if ((cw_capacity >= 0) && (cw_capacity <= 100) && (cw_bat->capacity != cw_capacity)) {
		cw_bat->capacity = cw_capacity;
		cw_bat->bat_change = 1;
		cw_update_time_member_capacity_change();

#if FG_CW2015_DEBUG
		if (cw_bat->capacity == 0)
			FG_CW2015_LOG("report battery capacity 0 and will shutdown if no changing\n");
#endif
	}
#if FG_CW2015_DEBUG
	FG_CW2015_LOG("rk_bat_update_capacity cw_capacity = %d\n",cw_bat->capacity);
#endif
}

static void rk_bat_update_vol()
{
	int ret;

	ret = cw_get_vol();
	if ((ret >= 0) && (cw_bat->voltage != ret)) {
		cw_bat->voltage = ret;
		cw_bat->bat_change = 1;
	}
}

static void rk_bat_update_status()
{
	int status;

	if (cw_bat->charger_mode > 0) {
		if (cw_bat->capacity >= 100) 
			status=POWER_SUPPLY_STATUS_FULL;
		else
			status=POWER_SUPPLY_STATUS_CHARGING;
	} else {
		status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	if (cw_bat->status != status) {
		cw_bat->status = status;
		cw_bat->bat_change = 1;
	} 
}

static void rk_bat_update_time_to_empty()
{
	int ret;

	ret = cw_get_time_to_empty();
	if ((ret >= 0) && (cw_bat->time_to_empty != ret)) {
		cw_bat->time_to_empty = ret;
		cw_bat->bat_change = 1;
	}
}

/**
 * @brief	cw_bat->charger_mode 获取 USB 的充电状态
 */
static int get_usb_charge_state()
{
	int usb_status = 0;

#if FG_CW2015_DEBUG
	FG_CW2015_LOG("get_usb_charge_state FG_charging_type = %d\n",FG_charging_type);
#endif
	//USB无供电
	if(FG_charging_status == CHARGER_UNKNOWN)
	{
		usb_status = 0;
		cw_bat->charger_mode = 0;
	}
	//接入USB Host取电
	else if(FG_charging_type==STANDARD_HOST)
	{
		usb_status = 1;
		cw_bat->charger_mode = USB_CHARGER_MODE;
	}
	//接入AC充电头的USB充电
	else
	{
		usb_status = 2;
		cw_bat->charger_mode = AC_CHARGER_MODE;
	}
#if FG_CW2015_DEBUG
	FG_CW2015_LOG("get_usb_charge_state usb_status = %d,FG_charging_status = %d\n",usb_status,FG_charging_status);
#endif

	return usb_status;
}

/**
 * @brief	检测usb在线状态
 */
static int rk_usb_update_online()
{
	int ret = 0;
	int usb_status = 0;

#if FG_CW2015_DEBUG
	FG_CW2015_LOG("rk_usb_update_online FG_charging_status = %d\n", FG_charging_status);
#endif

	/**
	 * usb_status 值为 0、1、2，分别反映USB无供电，接入USB Host取电，接入AC充电头的USB充电
	 */
	usb_status = get_usb_charge_state();
	// USB无供电
	if (usb_status == 2) {
		if (cw_bat->charger_mode != AC_CHARGER_MODE) {
			cw_bat->charger_mode = AC_CHARGER_MODE;
			ret = 1;
		}
		if (cw_bat->usb_online != 1) {
			cw_bat->usb_online = 1;
			cw_update_time_member_charge_start();
		}
	}
	// 接入USB Host取电
	else if (usb_status == 1) {
		if (cw_bat->charger_mode != USB_CHARGER_MODE) {
			cw_bat->charger_mode = USB_CHARGER_MODE;
			ret = 1;
		}
		if (cw_bat->usb_online != 1) {
			cw_bat->usb_online = 1;
			cw_update_time_member_charge_start();
		}
	}
	// 接入AC充电头的USB充电
	else if (usb_status == 0 && cw_bat->usb_online != 0) {
		cw_bat->charger_mode = 0;
		cw_update_time_member_charge_start();
		cw_bat->usb_online = 0;
		ret = 1;
	}

	return ret;
}
/**
 * @brief	任务函数
 */
static void threadCwBatUpdate(void *ctx)
{
    (void) ctx;
	int ret;
	int count_real_capacity = 0;		//本任务的执行次数

	TickType_t xLastWakeTime;
	TickType_t xFrequency = 1000;	//1s调度一次
	/* 获取当前的系统时间 */
	xLastWakeTime = xTaskGetTickCount();

	for(;;) {
	#if FG_CW2015_DEBUG
		FG_CW2015_FUN();
	#endif
		printk("cw_bat_work\n");

		//检测USB充电状态以更新cw_bat的时间相关的成员
		ret = rk_usb_update_online();
		if (cw_bat->usb_online == 1)
			ret = rk_usb_update_online();

		//更新电量
		rk_bat_update_capacity();
		//更新电压
		rk_bat_update_vol();

		//更新全局变量电量和电压
		g_cw2015_capacity = cw_bat->capacity;
		g_cw2015_vol = cw_bat->voltage;

		printf("[cwBatTask] vol = %d, cap = %d\r\n",cw_bat->voltage, cw_bat->capacity);
		if (cw_bat->bat_change) {
			cw_bat->bat_change = 0;
		}

		/**
		 * count_real_capacity<30或开机阶段1000ms调度本任务函数，否则改为8000ms调度
		 */
		if(count_real_capacity < 30 && g_platform_boot_mode == 8) {
			xFrequency = 1000;
			count_real_capacity++;
		}else
		{
			xFrequency = cw_work_freq;
		}
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

/*----------------------------------------------------------------------------*/

int cw_bat_init()
{
	int ret;
	int irq;
	int irq_flags;
	int loop = 0;

	file_sys_state = 1;

	if (!cw_bat) {
#if FG_CW2015_DEBUG
		FG_CW2015_ERR("fail to access cw_bat\n");
#endif
		return -1;
	}

	cw_bat->plat_data = &cw_bat_platdata;
	ret = cw_init();

	if (ret) 
		return ret;
	cw_bat->dc_online = 0;
	cw_bat->usb_online = 0;
	cw_bat->charger_mode = 0;
	cw_bat->capacity = 1;
	cw_bat->voltage = 0;
	cw_bat->status = 0;
	cw_bat->time_to_empty = 0;
	cw_bat->bat_change = 0;

	cw_update_time_member_capacity_change();
	cw_update_time_member_charge_start();

	//创建线程
    cwBatTaskHandle = osThreadNew(threadCwBatUpdate, nullptr, &cwBatTask_attributes);

    return 0;
}
