#include "utils.h"

/* 磁盘数据段相关 */
#define LINEAR_SEARCH_RES   100

typedef struct querySelector
{
    uINT    uiColNum;
    uINT    uiValue;
} querySelector_t;


void linearSearch(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, pBuffer pBuf);