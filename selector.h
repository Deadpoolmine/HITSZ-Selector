#include "utils.h"


/* 磁盘数据段相关 */
#define LINEAR_SEARCH_POS   100
#define INDEX_FILE_POS      2000
#define INDEX_SEARCH_POS    3000
#define TPMMS_S_POS         1000
#define TPMMS_R_POS         4000

typedef struct querySelector
{
    uINT    uiAttrNum;
    uINT    uiValue;
    /**
     * @brief 用于记录基准地址（tmpps带有中间结果）
     */
    uINT    uiBasePos;
} querySelector_t;


void linearSearch(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, pBuffer pBuf);
void tpmms(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, bool bIsAscend, pBuffer pBuf);
void indexSearch(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, pBuffer pBuf);