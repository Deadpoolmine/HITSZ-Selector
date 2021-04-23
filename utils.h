#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "extmem.h"
#include "conio.h"

#ifndef UTILS
#define UTILS

/* 显示相关 */
#define TIPS_ERROR      "!!![ERROR] ->->-> "

/* 宏变量相关 */
#define INVALID_ATTR    -1 

#define TRUE            1
#define FALSE           0
#define BUF_NO_ERROR    0
#define BUF_INDEX_ERROR 1
#define BUF_BBNUM_ERROR 2



/* BUF相关 */
#define BUF_NBLK            (512 / 64)
#define BUF_NTAG            8
#define BUF_SZ              520

/* DISK相关 */
#define DISK_NBLK           48
#define DISK_BLK_PER_SZ     64

/* BLK相关 */
#define BLK_NRECORD         7

/* 关系相关 */
#define TABLE_R_NBLK        16
#define TABLE_R_NRECORD     (TABLE_R_NBLK * BLK_NRECORD)
#define TABLE_S_NBLK        32       
#define TABLE_S_NRECORD     (TABLE_S_NBLK * BLK_NRECORD)      


typedef struct record
{
    int attr1;
    int attr2;
} record_t;


typedef unsigned int    uINT;
typedef char *          pChar;
typedef unsigned char * puChar;
typedef Buffer *        pBuffer;
typedef int             bool;
typedef int             bError;

/* buftool.c */
/**
 * @brief 获取 Buffer BLK中 data的起始位置，因为需要跳过Valid字节
 * 
 */
#define     GET_BUF_DATA(pBuf, uiBBLKNum) (pBuf->data + (uiBBLKNum - 1) * (DISK_BLK_PER_SZ + 1) + 1)

void        checkTables(pBuffer pBuf);
void        checkBuffer(pBuffer pBuf);
uINT        bConvertBLKAddr2Num(puChar puBlk, pBuffer pBuf);
record_t    bGetBLKRecord(uINT uiBBLKNum, uINT uiIndex, pBuffer pBuf);
bError      bSetBLKRecord(uINT uiBBLKNum, uINT uiIndex, record_t record, pBuffer pBuf);
uINT        bSetBLKNextBLK(uINT uiBBLKNum, uINT uiDBLKNextNum, pBuffer pBuf);
void        bClearBLK(uINT uiBBLKNum, pBuffer pBuf);

/* disktool.c */
void        initDTool();
uINT        dGetBLKNextGlobNum();
uINT        dResetGlobNextBLKNum();
void        dSetGlobNextBLKNum(uINT uiDBLKNum);
uINT        dWriteBLK(uINT uiBBLKNum, uINT uiNum, pBuffer pBuf);

#endif // !UTILS

