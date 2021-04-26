#include "utils.h"
#include "selector.h"
#include "merger.h"

#define TEST


int main(int argc, char const *argv[])
{
    Buffer              buf;
    pBuffer             pBuf;
    querySelector_t     querySelector;
    uINT                uiOpCnt; 
    uINT                uiNum; 
    initDTool();
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    pBuf = &buf;

#ifdef OUTPUT_ON
    system("cls");
#endif // OUTPUT_ON

    dSetGlobNextBLKNum(LINEAR_SEARCH_POS);
    querySelector.uiAttrNum = 1;                /* 选择第一个属性，也就是S.C */
    querySelector.uiValue   = 50;               /* S.C = 50 */
    querySelector.uiBasePos = LINEAR_SEARCH_POS;
    linearSearch(querySelector, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, pBuf);      
    dCheckBLKs(100, 101, pBuf);

    /* TPMMS FOR S */
    dSetGlobNextBLKNum(TPMMS_S_POS);
    querySelector.uiAttrNum = 1;
    querySelector.uiValue   = INVALID_ATTR;
    querySelector.uiBasePos = TPMMS_S_POS;
    tpmms(querySelector, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, FALSE, pBuf);
    dCheckBLKs(TPMMS_S_POS + TABLE_S_NBLK, TPMMS_S_POS + TABLE_S_NBLK + TABLE_S_NBLK - 1, pBuf);

    /* TPMMS FOR R */
    dSetGlobNextBLKNum(TPMMS_R_POS);
    querySelector.uiAttrNum = 1;
    querySelector.uiValue   = INVALID_ATTR;
    querySelector.uiBasePos = TPMMS_R_POS; 
    tpmms(querySelector, 1, TABLE_R_NBLK, FALSE, pBuf);
    dCheckBLKs(TPMMS_R_POS + TABLE_R_NBLK, TPMMS_R_POS + TABLE_R_NBLK + TABLE_R_NBLK - 1, pBuf);

    /* Build Index File */
    uiNum = 0;
    dSetGlobNextBLKNum(INDEX_FILE_POS);
    dBuildIndexFile(TPMMS_S_POS + 32, TPMMS_S_POS + 32 + 31, 1, 8, &uiNum, pBuf);
    dCheckBLKs(INDEX_FILE_POS, INDEX_FILE_POS + uiNum, pBuf);

    /* Index Search */
    dSetGlobNextBLKNum(INDEX_SEARCH_POS);
    querySelector.uiAttrNum = 1;             /* 选择第一个属性，也就是S.C */
    querySelector.uiValue   = 50;              /* S.C = 50 */
    querySelector.uiBasePos = INDEX_SEARCH_POS;
    indexSearch(querySelector, INDEX_FILE_POS, INDEX_FILE_POS + uiNum, pBuf);
    dCheckBLKs(3000, 3001, pBuf);
    
    /* 连接操作 */
    uiNum = 0;
    mergerOptions_t mergerOptions;
    mergerOptions.uiAttrNumR = 1;
    mergerOptions.uiAttrNumS = 1;
    mergerOptions.mergerType = JOIN;
    uiOpCnt = sortMerge(mergerOptions, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, 1, TABLE_R_NBLK, &uiNum, pBuf);
    dCheckBLKs(SM_POS, SM_POS + uiNum - 1, pBuf);
    printf("总共连接: %ld 次\n", uiOpCnt);

    /* 交操作 */
    uiNum = 0;
    mergerOptions.uiAttrNumR = 1;
    mergerOptions.uiAttrNumS = 1;
    mergerOptions.mergerType = INTER;
    uiOpCnt = sortMerge(mergerOptions, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, 1, TABLE_R_NBLK, &uiNum, pBuf);
    dCheckBLKs(SM_POS, SM_POS + uiNum - 1, pBuf);
    printf("S和R的交集共 %ld 个元组\n", uiOpCnt);


    /* 并操作 */
    uiNum = 0;
    mergerOptions.uiAttrNumR = 1;
    mergerOptions.uiAttrNumS = 1;
    mergerOptions.mergerType = UNION;
    uiOpCnt = sortMerge(mergerOptions, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, 1, TABLE_R_NBLK, &uiNum, pBuf);
    dCheckBLKs(SM_POS, SM_POS + uiNum - 1, pBuf);
    printf("S和R的并集共 %ld 个元组\n", uiOpCnt);


    /* 差操作 */
    uiNum = 0;
    mergerOptions.uiAttrNumR = 1;
    mergerOptions.uiAttrNumS = 1;
    mergerOptions.mergerType = DIFF;
    uiOpCnt = sortMerge(mergerOptions, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, 1, TABLE_R_NBLK, &uiNum, pBuf);
    dCheckBLKs(SM_POS, SM_POS + uiNum - 1, pBuf);
    printf("S和R的差集共 %ld 个元组\n", uiOpCnt);
    
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
    
    // uINT uiNum = 0;
    // mergerOptions_t mergerOptions;
    // mergerOptions.uiAttrNumR = 1;
    // mergerOptions.uiAttrNumS = 1;
    // mergerOptions.mergerType = DIFF;
    // uINT cnt = sortMerge(mergerOptions, 17, 48, 1, 16, &uiNum, pBuf);
    // dCheckBLKs(SM_POS, SM_POS + uiNum - 1, pBuf);
    // printf("CNT: %d\n", cnt);

    printf("\n\n输入任意键以结束...\n\n");
    getch();
    return 0;
}
