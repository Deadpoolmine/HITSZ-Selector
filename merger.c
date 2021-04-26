#include "selector.h"
#include "merger.h"


/**
 * @brief 基于排序的两趟算法 
 * 
 * @param mergerOptions 连接选项
 * @param uiDBLKLowNumS 集合S的低块号
 * @param uiDBLKHighNumS 集合S的高块号
 * @param uiDBLKLowNumR 集合R的低块号
 * @param uiDBLKHighNumR 集合R的高块号
 * @param pBuf 内存缓冲区
 * @return uINT 元素个数 - 交、并、差 均为元组，连接为连接数
 */
uINT sortMerge(mergerOptions_t mergerOptions, uINT uiDBLKLowNumS, uINT uiDBLKHighNumS, uINT uiDBLKLowNumR, uINT uiDBLKHighNumR, uINT *puiNum, pBuffer pBuf){
    querySelector_t         querySelector;
    
    puChar                  puWriteBlk;
    uINT                    uiWriteBBLKNum;
    uINT                    uiWriteCurIndex;

    uINT                    uiDBLKNumS;
    uINT                    uiDBLKNumR;

    puChar                  puBlkS;
    uINT                    uiBBLKNumS;
    record_t                recordS;
    uINT                    uiKeyS;

    puChar                  puBlkR;
    uINT                    uiBBLKNumR;
    record_t                recordR;
    uINT                    uiKeyR;

    bool                    bCanBreak;
    bool                    bHasSame;
    uINT                    uiOpCnt;
    record_t                record;

    switch (mergerOptions.mergerType)
    {
    case JOIN:
        DISPLAY_TIPS("基于排序的连接算法");
        break;
    case UNION:
        DISPLAY_TIPS("基于排序的集合并运算");
        break;
    case INTER:
        DISPLAY_TIPS("基于排序的集合交运算");
        break;
    case DIFF:
        DISPLAY_TIPS("基于排序的集合差运算");
        break; 
    default:
        break;
    }

    dResetGlobNextBLKNum();
    *puiNum = 0;
    uiOpCnt = 0;
    querySelector.uiAttrNum = mergerOptions.uiAttrNumR;
    querySelector.uiBasePos = SM_TEMP_R_POS;
    dSetGlobNextBLKNum(SM_TEMP_R_POS);
    tpmms(querySelector, uiDBLKLowNumR, uiDBLKHighNumR, TRUE, pBuf);

    querySelector.uiAttrNum = mergerOptions.uiAttrNumS;
    querySelector.uiBasePos = SM_TEMP_S_POS;
    dSetGlobNextBLKNum(SM_TEMP_S_POS);
    tpmms(querySelector, uiDBLKLowNumS, uiDBLKHighNumS, TRUE, pBuf);        /* 升序排列 */

    dSetGlobNextBLKNum(SM_POS);
    INIT_WRITE_BLK();

    uiDBLKNumS = SM_TEMP_S_POS + TABLE_S_NBLK;
    uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK;
    
    // printf("--------\n");
    // dCheckBLKs(uiDBLKNumS, uiDBLKNumS + 31, pBuf);
    // printf("--------\n");
    // dCheckBLKs(uiDBLKNumR, uiDBLKNumR + 15, pBuf);

    for (size_t i = 0; i < TABLE_S_NBLK; i++)
    {
        uiDBLKNumS = SM_TEMP_S_POS + TABLE_S_NBLK + i;
        for (uINT uiIndexS = 0; uiIndexS < BLK_NRECORD; uiIndexS++)
        {
            puBlkS     = readBlockFromDisk(uiDBLKNumS, pBuf);
            uiBBLKNumS = bConvertBLKAddr2Num(puBlkS, pBuf);
            recordS    = bGetBLKRecord(uiBBLKNumS, uiIndexS, pBuf);
            uiKeyS     = getKeyAttr(recordS, mergerOptions.uiAttrNumS);

            bCanBreak  = FALSE;
            bHasSame   = FALSE;

            for (size_t j = 0; j < TABLE_R_NBLK; j++)
            {
                uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK + j;
                for (uINT uiIndexR = 0; uiIndexR < BLK_NRECORD; uiIndexR++)
                {
                    puBlkR      = readBlockFromDisk(uiDBLKNumR, pBuf);
                    uiBBLKNumR  = bConvertBLKAddr2Num(puBlkR, pBuf);
                    recordR     = bGetBLKRecord(uiBBLKNumR, uiIndexR, pBuf);
                    uiKeyR      = getKeyAttr(recordR, mergerOptions.uiAttrNumS);
                    switch (mergerOptions.mergerType)
                    {
                    case JOIN:{
                        if(uiKeyR == uiKeyS) {
                            uiOpCnt++;
                            
                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                            uiWriteCurIndex++;
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1; 
                                                printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1));

                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordR, pBuf);
                            uiWriteCurIndex++;
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1; 
                                                printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1));
                        }
                        else if(uiKeyR > uiKeyS){                                        /* R排了序，可以直接跳过了 */
                            bCanBreak = TRUE;
                        }
                    }
                        break;
                    case INTER:{
                        if(recordR.attr1 == recordS.attr1 && recordR.attr2 == recordS.attr2){
                            uiOpCnt++;

                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                            uiWriteCurIndex++;
                            
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1; 
                                                printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1));
                            bCanBreak = TRUE;
                        }
                        else if(uiKeyR > uiKeyS) {
                            bCanBreak = TRUE;
                        }
                    }
                        break;
                    case UNION:
                    case DIFF: {
                        if((recordR.attr1 == recordS.attr1 && recordR.attr2 == recordS.attr2)){
                            bHasSame    = TRUE;
                            bCanBreak   = TRUE;
                        }
                        else if(uiKeyR > uiKeyS){
                            bCanBreak   = TRUE;
                        }
                    }
                        break;
                    default:
                        break;
                    }
                    
                    freeBlockInBuffer(puBlkR, pBuf);
                    if(bCanBreak) {
                        break;
                    }
                }
                if(bCanBreak){
                    break;
                }
            }
            if(!bHasSame){
                switch (mergerOptions.mergerType)
                {
                case UNION:
                case DIFF:{
                    uiOpCnt++;
                
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                    uiWriteCurIndex++;
                
                    WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1; 
                                        printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1));
                }
                    break;
                default:
                    break;
                }
            }
            freeBlockInBuffer(puBlkS, pBuf);                                 /* 清除内存中读入的数据 */    
        }   
    }
    if(mergerOptions.mergerType == UNION) {
        for (size_t i = 0; i < TABLE_R_NBLK; i++)
        {
            uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK + i;
            for (uINT uiIndexR = 0; uiIndexR < BLK_NRECORD; uiIndexR++)
            {
                puBlkR      = readBlockFromDisk(uiDBLKNumR, pBuf);
                uiBBLKNumR  = bConvertBLKAddr2Num(puBlkR, pBuf);
                recordR     = bGetBLKRecord(uiBBLKNumR, uiIndexR, pBuf);

                uiOpCnt++;
                bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordR, pBuf);
                uiWriteCurIndex++;
                
                WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1; 
                                    printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1));
                
                freeBlockInBuffer(puBlkR, pBuf);
            }
        }
    }
    
    if(uiWriteCurIndex != 0) {
        uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);
        *puiNum = *puiNum + 1; 
        printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }

    checkBuffer(pBuf);
    return uiOpCnt;
}
