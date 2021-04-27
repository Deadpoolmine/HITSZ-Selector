#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "extmem.h"
#include "conio.h"
#include "limits.h"

#ifndef UTILS
#define UTILS

/* 输出开关 */
#define OUTPUT_DETAIL_ON

/* 显示相关 */ 
#define TIPS_ERROR           "!!![ERROR] ->->-> "
#define FONT_COLOR_RED       "\033[31m"
#define FONT_COLOR_GREEN     "\033[32m"
#define FONT_COLOR_BULE      "\033[34m"
#define FONT_COLOR_PURPLE    "\033[35m"
#define FONT_COLOR_DGREEN    "\033[36m"
#define FONT_COLOR_END       "\033[0m"

#define MAX_DISPLAY          115
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

/**
 * @brief 根据属性列数获取记录中的关键字
 * 
 * @param record 记录
 * @param uiAttrNum 属性列数
 * @return uINT 关键字值
 */
static inline uINT getKeyAttr(record_t record, uINT uiAttrNum){
    switch (uiAttrNum)
    {
    case 1:
        return record.attr1;
    case 2:
        return record.attr2;
    default:
        break;
    }
}


/* general */

#define     GENERATE_TIPS(tips)             memset(querySelector.cTips, 0, MAX_DISPLAY);                                            \
                                            strcpy(querySelector.cTips, tips);                                                      \

#define     DISPLAY_TIPS(STATMENT)          {printf("\n");                                                                          \
                                            for(int i = 0; i < MAX_DISPLAY; i++) printf("-");                                       \
                                            printf("\n");                                                                           \
                                            char temp[MAX_DISPLAY];                                                                 \
                                            sprintf(temp,FONT_COLOR_BULE "func [%s] for %s" FONT_COLOR_END, __func__, STATMENT);    \
                                            int leftLen = (MAX_DISPLAY - strlen(temp)) / 2;                                         \
                                            int rightLen = MAX_DISPLAY - leftLen - strlen(temp);                                    \
                                            for(int i = 0; i < leftLen;i++) printf(" ");                                            \
                                            printf("%s", temp);                                                                     \
                                            for(int i = leftLen + strlen(temp); i < strlen(temp);i++) printf(" ");                  \
                                            printf("\n");                                                                           \
                                            for(int i = 0; i < MAX_DISPLAY; i++) printf("-");                                       \
                                            printf("\n");}     
/**
 * @brief 初始化写入块
 * 
 */
#define     INIT_WRITE_BLK()                puWriteBlk              = getNewBlockInBuffer(pBuf);                                          \
                                            uiWriteBBLKNum          = bConvertBLKAddr2Num(puWriteBlk, pBuf);                              \
                                            uiWriteCurIndex         = 0;                                                                  \
                                            bClearBLK(uiWriteBBLKNum, pBuf);                                                              \

/**
 * @brief 当写入块满，写回磁盘，并执行回调语句STATMENT
 * 
 */
#ifdef OUTPUT_DETAIL_ON
#define     WRITE_TILL_BLK_FILL(STATMENT)   if(uiWriteCurIndex == BLK_NRECORD) {                            /* 写满一个BLK就该写入磁盘了 */ \
                                                uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);   /* 写入后Buffer会被清除 */     \
                                                INIT_WRITE_BLK()                                                                          \
                                                printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);         \
                                                STATMENT;                                                                                 \
                                            }   
#define     INIT_IO_COUNTER()               uINT uiOriginalIOCnt    = pBuf->numIO;

#define     DISPLAY_IO_CNT()                {printf("\n");                                                                          \
                                            char temp[MAX_DISPLAY];                                                                 \
                                            sprintf(temp,FONT_COLOR_DGREEN "func [%s] IO读写共 %ld 次\n" FONT_COLOR_END, __func__, pBuf->numIO - uiOriginalIOCnt);    \
                                            int leftLen = (MAX_DISPLAY - strlen(temp)) / 2;                                         \
                                            int rightLen = MAX_DISPLAY - leftLen - strlen(temp);                                    \
                                            for(int i = 0; i < leftLen;i++) printf(" ");                                            \
                                            printf("%s", temp);                                                                     \
                                            for(int i = leftLen + strlen(temp); i < strlen(temp);i++) printf(" ");                  \
                                            printf("\n");                                                                           \
                                            }                                                        

#define     DISPLAY_RECORD(record)          printf(FONT_COLOR_GREEN "找到记录 (%ld, %ld)\n" FONT_COLOR_END, record.attr1, record.attr2);                                                                                          
#else 
#define     WRITE_TILL_BLK_FILL(STATMENT)   if(uiWriteCurIndex == BLK_NRECORD) {                            /* 写满一个BLK就该写入磁盘了 */ \
                                                uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);   /* 写入后Buffer会被清除 */     \
                                                INIT_WRITE_BLK()                                                                          \
                                                STATMENT;                                                                                 \
                                            }   
#define     INIT_IO_COUNTER()               uINT uiOriginalIOCnt    = pBuf->numIO;

#define     DISPLAY_IO_CNT()                NULL                                                              

#define     DISPLAY_RECORD(record)          NULL
#endif // OUTPUT_ON
                                                                     \

                                            

/* buftool.c */
/**
 * @brief 获取 Buffer BLK中 data的起始位置，因为需要跳过Valid字节
 * 
 */
#define     GET_BUF_DATA(pBuf, uiBBLKNum) (pBuf->data + (uiBBLKNum - 1) * (DISK_BLK_PER_SZ + 1) + 1)

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
void        dCheckBLKs(uINT uiDBLKLowNum,uINT uiDBLKHighNum, pBuffer pBuf);
void        dCheckTables(pBuffer pBuf);
void        dCheckTpmmsS(uINT uiTPMMSRes, pBuffer pBuf);
uINT        dBuildIndexFile(uINT uiDBLKLowNum,uINT uiDBLKHighNum, uINT uiAttrNum, uINT uiGap, uINT *puiNum, pBuffer pBuf);

/* sorttool.c */
void        sortInBuf(uINT uiBBLKLowNum, uINT uiBBLKHighNum, bool bIsAscend, uINT uiKeyCol, pBuffer pBuf);
#endif // !UTILS

