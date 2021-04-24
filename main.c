#include "utils.h"
#include "selector.h"
#include "merger.h"

#define TEST

int main(int argc, char const *argv[])
{
    Buffer              buf;
    pBuffer             pBuf;
    querySelector_t     querySelector;

    initDTool();
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    pBuf = &buf;

#ifndef TEST
    /* LinearSearch */
    dSetGlobNextBLKNum(LINEAR_SEARCH_POS);
    querySelector.uiAttrNum = 1;            /* 选择第一个属性，也就是S.C */
    querySelector.uiValue = 50;             /* S.C = 50 */
    linearSearch(querySelector, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, pBuf);      
    dCheckBLKs(100, 101, pBuf);
    
    /* TPMMS FOR S */
    dSetGlobNextBLKNum(TPMMS_S_POS);
    querySelector.uiAttrNum = 1;
    querySelector.uiValue = INVALID_ATTR;
    tpmms(querySelector, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, FALSE, pBuf);
    dCheckBLKs(TPMMS_S_POS + TABLE_S_NBLK, TPMMS_S_POS + TABLE_S_NBLK + TABLE_S_NBLK - 1, pBuf);

    /* TPMMS FOR R */
    dSetGlobNextBLKNum(TPMMS_R_POS);
    querySelector.uiAttrNum = 1;
    querySelector.uiValue = INVALID_ATTR;
    tpmms(querySelector, 1, TABLE_R_NBLK, FALSE, pBuf);
    dCheckBLKs(TPMMS_R_POS + TABLE_R_NBLK, TPMMS_R_POS + TABLE_R_NBLK + TABLE_R_NBLK - 1, pBuf);

    /* Build Index File */
    uINT uiNum = 0;
    dSetGlobNextBLKNum(INDEX_FILE_POS);
    dBuildIndexFile(TPMMS_S_POS + 32, TPMMS_S_POS + 32 + 31, 1, 8, &uiNum, pBuf);
    printf("\n");
    dCheckBLKs(2000, 2000, pBuf);
    
    /* Index Search */
    dSetGlobNextBLKNum(INDEX_SEARCH_POS);
    querySelector.uiAttrNum = 1;             /* 选择第一个属性，也就是S.C */
    querySelector.uiValue = 50;             /* S.C = 50 */
    indexSearch(querySelector, 2000, 2000, pBuf);
    dCheckBLKs(3000, 3001, pBuf);
#endif // !TEST
    // printf("-----------------------------------------------Table S-----------------------------------------------\n");
    // dCheckBLKs(TPMMS_S_POS + TABLE_S_NBLK, TPMMS_S_POS + TABLE_S_NBLK + TABLE_S_NBLK - 1, pBuf);
    // printf("-----------------------------------------------Table R-----------------------------------------------\n");
    // dCheckBLKs(TPMMS_R_POS + TABLE_R_NBLK, TPMMS_R_POS + TABLE_R_NBLK + TABLE_R_NBLK - 1, pBuf);
    
    uINT uiNum = 0;
    mergerOptions_t mergerOptions;
    mergerOptions.uiAttrNumR = 1;
    mergerOptions.uiAttrNumS = 1;
    mergerOptions.mergerType = DIFF;
    uINT cnt = sortMerge(mergerOptions, 17, 48, 1, 16, &uiNum, pBuf);
    dCheckBLKs(SM_POS, SM_POS + uiNum - 1, pBuf);
    printf("CNT: %d\n", cnt);
    
    // dCheckTpmmsS(TPMMS_S_RES, pBuf);
    
    // checkTables(pBuf);
    
    // dCheckTables(pBuf);
    // for (size_t i = 0; i < 8; i++)
    // {
    //     readBlockFromDisk(1 + i, pBuf);
    // }
    // dSetGlobNextBLKNum(1000);
    // sortInBuf(1, 8, FALSE, 1, pBuf);
    // dWriteBLK(1, 8, pBuf);
    // dCheckBLKs(1000, 1007, pBuf);

    // record_t record;
    // record.attr1 = 1;
    // record.attr2 = 3;

    // puChar puBlk = getNewBlockInBuffer(pBuf);
    // uINT uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
    // bClearBLK(uiBBLKNum, pBuf);
    // bSetBLKRecord(uiBBLKNum, 0, record, pBuf);
    // uINT uiDBLKNextNum = dWriteBLK(uiBBLKNum, 1, pBuf);
    // printf("写入磁盘：%d\n", uiDBLKNextNum - 1);

    

    printf("\n\n输入任意键以结束...\n\n");
    getch();
    return 0;
}
