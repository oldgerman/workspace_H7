## FATFS R0.15 适配 CubeMX R0.12C

## 适配 ffconf.h

新建 `ffconf_r0.15_by_r013c.h` 解决

## 适配 用户同步函数

在 CubeMX 自动生成的 Middlewares/Third_Party/FatFs/option/syscall.c 中有

```c
/* R0.13C 的4个用户同步函数 */
int ff_cre_syncobj ( BYTE vol, _SYNC_t *sobj )
int ff_del_syncobj ( _SYNC_t sobj )
int ff_req_grant ( _SYNC_t sobj )
void ff_rel_grant ( _SYNC_t sobj )

/* ff malloc 和 free 默认不使能 */
#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
void* ff_memalloc ( UINT msize )
void ff_memfree ( void* mblock )
#endif
```

但 R0.15 版本这几个函数都没了，在ff15/documents/updates.html中有以下说明：

> User provided synchronization functions, `ff_cre_syncobj`, `ff_del_syncobj`, `ff_req_grant` and `ff_rel_grant`, needed when `FF_FS_REENTRANT` are replaced with `ff_mutex_create`, `ff_mutex_delete`, `ff_mutex_take` and `ff_mutex_give` respectively. For example, see `ffsystem.c`.
>
> 翻译：
>
> 用户提供的同步函数，`ff_cre_syncobj`、`ff_del_syncobj`、`ff_req_grant` 和 `ff_rel_grant`。当使能 `FF_FS_REENTRANT` 时，需要分别替换为 `ff_mutex_create`、`ff_mutex_delete`、`ff_mutex_take` 和 `ff_mutex_give`。示例请参见 `ffsystem.c`

ok，看看 ffsystem.c ，cubeMX 我配置为 _FS_REENTRANT 对应使能 FF_FS_REENTRANT，我需要修改此文件的 4个 同步函数 调用 Middlewares/Third_Party/FatFs/option/syscall.c 中的四个同步函数 

```c
#if FF_FS_REENTRANT	/* Mutal exclusion */
/*------------------------------------------------------------------------*/
/* Definitions of Mutex                                                   */
/*------------------------------------------------------------------------*/

#define OS_TYPE	0	/* 0:Win32, 1:uITRON4.0, 2:uC/OS-II, 3:FreeRTOS, 4:CMSIS-RTOS */
```

ffsystem.c 中使能 OS_TYPE 4 是 CMSIS-RTOS V1的，但我的工程是 V2的，这样  ff_mutex_create 中的写法编译就会报错，需要修改如下：

```c
int ff_mutex_create (	/* Returns 1:Function succeeded or 0:Could not create the mutex */
	int vol				/* Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) */
)
{
    ...
#elif OS_TYPE == 4	/* CMSIS-RTOS */
#if (osCMSIS < 0x20000U)
	osMutexDef(cmsis_os_mutex);
	Mutex[vol] = osMutexCreate(osMutex(cmsis_os_mutex));
#else
    Mutex[vol] = osMutexNew(NULL);
#endif
	return (int)(Mutex[vol] != NULL);
#endif
}
```

ff15 中的 ffunicode 与 cubemx自动生成的  Middlewares/Third_Party/FatFs/option/cc936.c 中重复定义，在过滤器中排除 cc936.c 即可