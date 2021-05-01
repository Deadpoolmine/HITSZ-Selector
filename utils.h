#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "extmem.h"
#include "conio.h"
#include "limits.h"

#ifndef UTILS
#define UTILS
/* ������� */
#define OUTPUT_DETAIL_ON
/* ���Կ��� */
//#define TEST

/* ��ʾ��� */ 
#define TIPS_ERROR           "!!![ERROR] ->->-> "
#define FONT_COLOR_RED       "\033[31m"
#define FONT_COLOR_GREEN     "\033[32m"
#define FONT_COLOR_BULE      "\033[34m"
#define FONT_COLOR_PURPLE    "\033[35m"
#define FONT_COLOR_DGREEN    "\033[36m"
#define FONT_COLOR_END       "\033[0m"

#define MAX_DISPLAY          115
/* �������� */
#define INVALID_ATTR    -1 

#define TRUE            1
#define FALSE           0
#define BUF_NO_ERROR    0
#define BUF_INDEX_ERROR 1
#define BUF_BBNUM_ERROR 2



/* BUF��� */
#define BUF_NBLK            (512 / 64)
#define BUF_NTAG            8
#define BUF_SZ              520

/* DISK��� */
#define DISK_NBLK           48
#define DISK_BLK_PER_SZ     64

/* BLK��� */
#define BLK_NRECORD         7

/* ��ϵ��� */
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
 * @brief ��������������ȡ��¼�еĹؼ���
 * 
 * @param record ��¼
 * @param uiAttrNum ��������
 * @return uINT �ؼ���ֵ
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

static inline uINT getAnotherKeyAttr(record_t record, uINT uiAttrNum){
    return getKeyAttr(record, 2 - uiAttrNum == 1 ? 2 : 1);
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
 * @brief ��ʼ��д���
 * 
 */
#define     INIT_WRITE_BLK()                puWriteBlk              = getNewBlockInBuffer(pBuf);                                          \
                                            uiWriteBBLKNum          = bConvertBLKAddr2Num(puWriteBlk, pBuf);                              \
                                            uiWriteCurIndex         = 0;                                                                  \
                                            bClearBLK(uiWriteBBLKNum, pBuf);                                                              \

/**
 * @brief ��д�������д�ش��̣���ִ�лص����STATMENT
 * 
 */
#ifdef OUTPUT_DETAIL_ON
#define     WRITE_TILL_BLK_FILL(STATMENT)   if(uiWriteCurIndex == BLK_NRECORD) {                            /* д��һ��BLK�͸�д������� */ \
                                                uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);   /* д���Buffer�ᱻ��� */     \
                                                INIT_WRITE_BLK()                                                                          \
                                                printf(FONT_COLOR_RED "���д����̿� %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);         \
                                                STATMENT;                                                                                 \
                                            }   
#define     INIT_IO_COUNTER()               uINT uiOriginalIOCnt    = pBuf->numIO;

#define     DISPLAY_IO_CNT()                {printf("\n");                                                                          \
                                            char temp[MAX_DISPLAY];                                                                 \
                                            sprintf(temp,FONT_COLOR_DGREEN "func [%s] IO��д�� %ld ��\n" FONT_COLOR_END, __func__, pBuf->numIO - uiOriginalIOCnt);    \
                                            int leftLen = (MAX_DISPLAY - strlen(temp)) / 2;                                         \
                                            int rightLen = MAX_DISPLAY - leftLen - strlen(temp);                                    \
                                            for(int i = 0; i < leftLen;i++) printf(" ");                                            \
                                            printf("%s", temp);                                                                     \
                                            for(int i = leftLen + strlen(temp); i < strlen(temp);i++) printf(" ");                  \
                                            printf("\n");                                                                           \
                                            }                                                        

#define     DISPLAY_RECORD(record)          printf(FONT_COLOR_GREEN "�ҵ���¼ (%ld, %ld)\n" FONT_COLOR_END, record.attr1, record.attr2);                                                                                          
#else 
#define     WRITE_TILL_BLK_FILL(STATMENT)   if(uiWriteCurIndex == BLK_NRECORD) {                            /* д��һ��BLK�͸�д������� */ \
                                                uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);   /* д���Buffer�ᱻ��� */     \
                                                INIT_WRITE_BLK()                                                                          \
                                                STATMENT;                                                                                 \
                                            }   
#define     INIT_IO_COUNTER()               uINT uiOriginalIOCnt    = pBuf->numIO;

#define     DISPLAY_IO_CNT()                NULL                                                              

#define     DISPLAY_RECORD(record)          NULL
#endif // OUTPUT_ON
                                                                     \

                                            
/* tpmmtool.c���׶�ɨ�蹤�� */
#define GET_CUR_DBLK_NUM(basePos, segmentIndex) basePos + (segmentIndex) * uiNumPerSegment \
                                                + puiDNextIndex[(segmentIndex)]

#define GET_CUR_BBLK_NUM(segmentIndex)          uiBaseSegmentBBLKNum + segmentIndex

#define GET_NEXT_RECORD(segmentIndex)           puiBNextIndex[segmentIndex]++
#define GET_CUR_RECORD_INDEX(segmentIndex)      puiBNextIndex[segmentIndex]
#define RESET_CUR_RECORD(segmentIndex)          puiBNextIndex[segmentIndex] = 0

#define GET_NEXT_DBLK(segmentIndex)             puiDNextIndex[segmentIndex]++
#define GET_CUR_DBLK_INDEX(segmentIndex)        puiDNextIndex[segmentIndex]
#define RESET_CUR_DBLK(segmentIndex)            puiDNextIndex[segmentIndex] = 0

bool tpmmCheckIfSame(uINT uiCompareBBLKNum, uINT uiSegment, uINT* puiSegmentIndexi,uINT* puiSegmentIndexj, pBuffer pBuf);
uINT tpmmSelectMaxMinIndex(uINT uiCompareBBLKNum, uINT uiSegment, uINT uiAttrNum, bool bIsAscend, pBuffer pBuf);
void tpmmShiftSegmentRecord(uINT uiDBasePos, uINT uiCompareBBLKNum, uINT uiBaseSegmentBBLKNum, uINT uiSegmentIndex, uINT* puiBNextIndex, uINT* puiDNextIndex, uINT uiNumPerSegment, pBuffer pBuf);
bool tpmmCheckIsOver(uINT uiCompareBBLKNum, uINT uiSegment, pBuffer pBuf);

/* buftool.c */
/**
 * @brief ��ȡ Buffer BLK�� data����ʼλ�ã���Ϊ��Ҫ����Valid�ֽ�
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

