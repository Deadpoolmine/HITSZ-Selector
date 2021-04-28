#include "utils.h"

#define SM_TEMP_S_POS         5000
#define SM_TEMP_R_POS         6000
#define SM_POS                7000

typedef enum ENUM_MergeType
{
    JOIN,
    INTER,
    UNION,
    DIFF
} ENUM_MERGETYPE;

typedef struct mergerOptions
{
    uINT uiAttrNumS;
    uINT uiAttrNumR;
    ENUM_MERGETYPE mergerType;
    bool bIsOptimise;               /* 是否开启I/O优化器 */
} mergerOptions_t;


uINT sortMerge(mergerOptions_t mergerOptions, uINT uiDBLKLowNumS, uINT uiDBLKHighNumS, uINT uiDBLKLowNumR, uINT uiDBLKHighNumR,uINT *puiNum,  pBuffer pBuf);
uINT intersection(uINT uiDBLKLowNumS, uINT uiDBLKHighNumS, uINT uiDBLKLowNumR, uINT uiDBLKHighNumR, pBuffer pBuf);