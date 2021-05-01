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

    uINT                    uiDBLKNumLastR;            /* R与S第一个可比块 - 优化器 */

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
    GENERATE_TIPS("merge前先利用TPMMS对关系R排序");
    dSetGlobNextBLKNum(SM_TEMP_R_POS);
    tpmms(querySelector, uiDBLKLowNumR, uiDBLKHighNumR, TRUE, pBuf);

    querySelector.uiAttrNum = mergerOptions.uiAttrNumS;
    querySelector.uiBasePos = SM_TEMP_S_POS;
    GENERATE_TIPS("merge前先利用TPMMS对关系S排序");
    dSetGlobNextBLKNum(SM_TEMP_S_POS);
    tpmms(querySelector, uiDBLKLowNumS, uiDBLKHighNumS, TRUE, pBuf);        /* 升序排列 */

    dSetGlobNextBLKNum(SM_POS);
    INIT_WRITE_BLK();
    INIT_IO_COUNTER();
    uiDBLKNumLastR = 0;

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

            bCanBreak  = FALSE;                 /* 是否读S的下一条记录 */
            bHasSame   = FALSE;

            for (size_t j = 0; j < TABLE_R_NBLK; j++)
            {
                uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK + j;
                if(uiDBLKNumR < uiDBLKNumLastR){                            /* 直接跳到第一个可比块 */
                    continue;
                }
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
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);

                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordR, pBuf);
                            uiWriteCurIndex++;
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
                        }
                        else if(uiKeyR > uiKeyS){                                        /* R排了序，可以直接跳过了 */
                            bCanBreak = TRUE;
                        }
                        else if(uiKeyR < uiKeyS){
                            if(mergerOptions.bIsOptimise)
                                uiDBLKNumLastR = uiDBLKNumR;
                        }
                    }
                        break;
                    case INTER:{
                        if(recordR.attr1 == recordS.attr1 && recordR.attr2 == recordS.attr2){
                            uiOpCnt++;

                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                            uiWriteCurIndex++;
                            
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
                            bCanBreak = TRUE;
                        }
                        else if(uiKeyR > uiKeyS) {
                            bCanBreak = TRUE;
                        }
                        else if(uiKeyR < uiKeyS){
                            if(mergerOptions.bIsOptimise)
                                uiDBLKNumLastR = uiDBLKNumR;
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
                        else if(uiKeyR < uiKeyS){
                            if(mergerOptions.bIsOptimise)
                                uiDBLKNumLastR = uiDBLKNumR;
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
                
                    WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
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
                
                WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
                
                freeBlockInBuffer(puBlkR, pBuf);
            }
        }
    }
    
    if(uiWriteCurIndex != 0) {
        uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);
        *puiNum = *puiNum + 1; 
#ifdef  OUTPUT_DETAIL_ON
        printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);
#endif //  OUTPUT_DETAIL_ON
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }
    DISPLAY_IO_CNT();
    //checkBuffer(pBuf);
    return uiOpCnt;
}


/**
 * @brief 两趟扫描算法，实现集合二元操作
 * 
 * @param mergerOptions 二元操作选项
 * @param uiDBLKLowNumS 关系S低磁盘块号
 * @param uiDBLKHighNumS 关系S高磁盘块号
 * @param uiDBLKLowNumR 关系R低磁盘块号
 * @param uiDBLKHighNumR 关系R高磁盘块号
 * @param puiNum 总共产生的块号指针
 * @param pBuf 内存缓冲区
 * @return uINT 
 */
uINT  tpmm(mergerOptions_t mergerOptions, uINT uiDBLKLowNumS, uINT uiDBLKHighNumS, uINT uiDBLKLowNumR, uINT uiDBLKHighNumR, uINT *puiNum, pBuffer pBuf){
    uINT        uiNumS;
    uINT        uiNumR;

    uINT        uiSegmentS;
    uINT        uiSegmentR;

    uINT        uiSegmentIndexi;
    uINT        uiSegmentIndexj;

    uINT        uiNumPerSegment;

    
    
    uINT        uiBaseSegmentBBLKNum;
    
    puChar      puBlk;
    uINT        uiBBLKNum;

    uINT        uiDBLKNum;

    
    puChar      puCompareBlk;
    uINT        uiCompareBBLKNum;

    uINT*       puiBNextIndex;                                      /* 组 Buffer Record指针*/
    uINT*       puiDNextIndex;                                      /* 组 磁盘读指针 */
    
    puChar      puWriteBlk;
    uINT        uiWriteBBLKNum;
    uINT        uiWriteCurIndex;                                    /* 写入 指针 */

    record_t    record;
    record_t    recordi;
    record_t    recordj;
    bool        bHasSame;
    uINT        uiMaxMinSegmentIndex;

    uINT        uiOpCnt;

    switch (mergerOptions.mergerType)
    {
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

    uiOpCnt = 0;
    uiNumS = uiDBLKHighNumS - uiDBLKLowNumS + 1;
    uiNumR = uiDBLKHighNumR - uiDBLKLowNumR + 1;

    uiBaseSegmentBBLKNum = -1;
    
    INIT_IO_COUNTER();
    if(uiNumS < BUF_NBLK * BUF_NBLK){                               /* 两阶段二路归并排序 */
        uiNumPerSegment = BUF_NBLK;                                 /* 每组8个块 */
        uiSegmentS = uiNumS / uiNumPerSegment;                      /* 分成uiSegmentS组 */
        uiSegmentR = uiNumR / uiNumPerSegment;
        
        if(uiSegmentS + uiSegmentR > BLK_NRECORD){
            printf(TIPS_ERROR "[tpmm: Compare BLK can't hold #%d segments]\n", uiSegmentS);
            return uiOpCnt;    
        }

        dSetGlobNextBLKNum(SM_TEMP_S_POS);
        for (size_t i = 0; i < uiSegmentS; i++)                      /* 第一趟扫描 */
        {
            uiDBLKNum = uiDBLKLowNumS + i * uiNumPerSegment;         /* S中该组在磁盘中的位置 */
            for (size_t j = 0; j < uiNumPerSegment; j++)
            {
                readBlockFromDisk(uiDBLKNum + j, pBuf);
            }
            sortInBuf(1, uiNumPerSegment, TRUE, 1, pBuf);
            dWriteBLK(1, uiNumPerSegment, pBuf);
        }

        for (size_t i = 0; i < uiSegmentR; i++)                      /* 第一趟扫描 */
        {
            uiDBLKNum = uiDBLKLowNumR + i * uiNumPerSegment;         /* R中该组在磁盘中的位置 */
            for (size_t j = 0; j < uiNumPerSegment; j++)
            {
                readBlockFromDisk(uiDBLKNum + j, pBuf);
            }
            sortInBuf(1, uiNumPerSegment, TRUE, 1, pBuf);
            dWriteBLK(1, uiNumPerSegment, pBuf);
        }

        dSetGlobNextBLKNum(SM_POS);
        puCompareBlk     = getNewBlockInBuffer(pBuf);
        uiCompareBBLKNum = bConvertBLKAddr2Num(puCompareBlk, pBuf);
        bClearBLK(uiCompareBBLKNum, pBuf);

        INIT_WRITE_BLK();

        puiBNextIndex = (uINT *)malloc((uiSegmentS + uiSegmentR) * sizeof(uINT));
        puiDNextIndex = (uINT *)malloc((uiSegmentS + uiSegmentR) * sizeof(uINT));
        for (uINT uiSegmentIndex = 0; uiSegmentIndex < uiSegmentS + uiSegmentR; uiSegmentIndex++)
        {
            RESET_CUR_RECORD(uiSegmentIndex);
            RESET_CUR_DBLK(uiSegmentIndex);
        }

        for (uINT uiSegmentIndex = 0; uiSegmentIndex < uiSegmentS + uiSegmentR; uiSegmentIndex++)                                       /* 初始每个分组都读入一块至内存 */
        {
            uiDBLKNum = GET_CUR_DBLK_NUM(SM_TEMP_S_POS, uiSegmentIndex);
            puBlk     = readBlockFromDisk(uiDBLKNum, pBuf);  
            uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
            
            if(uiBaseSegmentBBLKNum == -1){
                uiBaseSegmentBBLKNum = uiBBLKNum;
            }
            
            record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiSegmentIndex), pBuf);
            bSetBLKRecord(uiCompareBBLKNum, uiSegmentIndex, record, pBuf);              /* 初始化Compare BLK */
        }

        while (!tpmmCheckIsOver(uiCompareBBLKNum, uiSegmentS + uiSegmentR, pBuf))     /* 判断TPMMS是否结束 */
        {
            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
            bHasSame = tpmmCheckIfSame(uiCompareBBLKNum, uiSegmentR + uiSegmentS, &uiSegmentIndexi, &uiSegmentIndexj, pBuf);
            if(bHasSame){                                                                        /* 如果存在 */
                recordi = bGetBLKRecord(uiCompareBBLKNum, uiSegmentIndexi, pBuf);                /* 获取该索引对应的record */
                recordj = bGetBLKRecord(uiCompareBBLKNum, uiSegmentIndexj, pBuf);
                if(uiSegmentIndexi + 1 <= uiSegmentS && uiSegmentIndexj + 1 > uiSegmentS){        /* uiSegemnti属于关系S，uiSegemnti属于关系R*/
                        
                    if(mergerOptions.mergerType == INTER || mergerOptions.mergerType == UNION){
                        bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordi, pBuf);            /* 写入Write BLK中 */
                        uiWriteCurIndex++;    
                        uiOpCnt++;  
                        
                    }
                    tpmmShiftSegmentRecord(SM_TEMP_S_POS, uiCompareBBLKNum, uiBaseSegmentBBLKNum,uiSegmentIndexi, puiBNextIndex, 
                                                 puiDNextIndex, uiNumPerSegment, pBuf);
                    tpmmShiftSegmentRecord(SM_TEMP_S_POS,uiCompareBBLKNum, uiBaseSegmentBBLKNum, uiSegmentIndexj, puiBNextIndex, 
                                                puiDNextIndex, uiNumPerSegment, pBuf);
                }
                else {
                    tpmmShiftSegmentRecord(SM_TEMP_S_POS, uiCompareBBLKNum, uiBaseSegmentBBLKNum, uiSegmentIndexj, puiBNextIndex, 
                                             puiDNextIndex, uiNumPerSegment, pBuf);
                }
            }
            else {
                uiMaxMinSegmentIndex = tpmmSelectMaxMinIndex(uiCompareBBLKNum, uiSegmentS + uiSegmentR, 1, TRUE, pBuf);
                if(uiMaxMinSegmentIndex != INT_MAX){                           /* 如果存在 */
                    record = bGetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, pBuf);   /* 获取该索引对应的record */
                    if(mergerOptions.mergerType == UNION){
                        bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, record, pBuf);            /* 写入Write BLK中 */
                        uiOpCnt++;
                        uiWriteCurIndex++;
                    }
                    if(mergerOptions.mergerType == DIFF){
                        if(uiMaxMinSegmentIndex + 1 <= uiSegmentS){
                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, record, pBuf);            /* 写入Write BLK中 */
                            uiOpCnt++;
                            uiWriteCurIndex++;
                        }
                    }
                    tpmmShiftSegmentRecord(SM_TEMP_S_POS,uiCompareBBLKNum, uiBaseSegmentBBLKNum, uiMaxMinSegmentIndex, puiBNextIndex, 
                                             puiDNextIndex, uiNumPerSegment, pBuf);
                }
            }
        }
        
        if(uiWriteCurIndex != 0) {
            dWriteBLK(uiWriteBBLKNum, 1, pBuf);
        }
        else {
            freeBlockInBuffer(puWriteBlk, pBuf);
        }

        freeBlockInBuffer(puCompareBlk, pBuf);

        free(puiBNextIndex);
        free(puiDNextIndex);
    }
    DISPLAY_IO_CNT();
    return uiOpCnt;
}