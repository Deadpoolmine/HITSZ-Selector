#include "selector.h"

/**
 * @brief 线性搜索
 * 
 * @param querySelector 搜索器 
 * @param uiDBLKLowNum 磁盘上低块号
 * @param uiDBLKHighNum 地盘上高块号
 * @param pBuf 内存缓冲区
 */
void linearSearch(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, pBuffer pBuf) {
    uINT        uiBBLKNum;
    uINT        uiNum;
    
    puChar      puBlk;
    puChar      puWriteBlk;
    uINT        uiWriteBBLKNum;
    uINT        uiWriteCurIndex;

    record_t    record;

    uINT        uiRecordNum;
    uINT        uiKey;

    
    DISPLAY_TIPS(querySelector.cTips);
    uiNum               = uiDBLKHighNum - uiDBLKLowNum + 1;
    uiRecordNum         = 0;
    INIT_IO_COUNTER();
    INIT_WRITE_BLK();

    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk = readBlockFromDisk(uiDBLKLowNum + i, pBuf);              /* 读一块至内存 */
        uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
#ifdef  OUTPUT_DETAIL_ON
        printf("读入数据块 %ld\n", uiDBLKLowNum + i);
#endif //  OUTPUT_DETAIL_ON
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            WRITE_TILL_BLK_FILL(NULL);
            uiKey = getKeyAttr(record, querySelector.uiAttrNum);
            
            if(querySelector.uiValue == uiKey){
                bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex,record, pBuf);
                uiWriteCurIndex++;     
                uiRecordNum++;
                DISPLAY_RECORD(record); 
            }
        }
        freeBlockInBuffer(puBlk, pBuf);                                 /* 清除内存中读入的数据 */
    }
    if(uiWriteCurIndex != 0) {
        uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);
#ifdef  OUTPUT_DETAIL_ON
        printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);
#endif //  OUTPUT_DETAIL_ON
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }
    
#ifdef OUTPUT_DETAIL_ON
    printf("满足条件的元组共 %ld 个\n", uiRecordNum);
#endif // OUTPUT_DETAIL_ON
    DISPLAY_IO_CNT();
}

/**
 * @brief 两阶段多路归并排序
 * 
 * @param querySelector 搜索器 
 * @param uiDBLKLowNum 磁盘上低块号
 * @param uiDBLKHighNum 地盘上高块号
 * @param pBuf 内存缓冲区
 */
void tpmms(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, bool bIsAscend, pBuffer pBuf){
    uINT        uiNum;

    uINT        uiSegment;
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
    uINT        uiMaxMinSegmentIndex;

    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    uiBaseSegmentBBLKNum = -1;
    
    DISPLAY_TIPS(querySelector.cTips);
    INIT_IO_COUNTER();
    if(uiNum < BUF_NBLK * BUF_NBLK){                                /* 两阶段二路归并排序 */
        uiNumPerSegment = BUF_NBLK;                                 /* 每组8个块 */
        uiSegment = uiNum / uiNumPerSegment;                        /* 分成uiSegment组 */
        
        if(uiSegment > BLK_NRECORD){
            printf(TIPS_ERROR "[tpmms: Compare BLK can't hold #%d segments]\n", uiSegment);
            return;    
        }

        for (size_t i = 0; i < uiSegment; i++)                      /* 第一趟扫描 */
        {
            uiDBLKNum = uiDBLKLowNum + i * uiNumPerSegment;         /* 该组在磁盘中的位置 */
            for (size_t j = 0; j < uiNumPerSegment; j++)
            {
                readBlockFromDisk(uiDBLKNum + j, pBuf);
            }
            sortInBuf(1, uiNumPerSegment, bIsAscend, querySelector.uiAttrNum, pBuf);
            dWriteBLK(1, uiNumPerSegment, pBuf);
        }
        

        puCompareBlk     = getNewBlockInBuffer(pBuf);
        uiCompareBBLKNum = bConvertBLKAddr2Num(puCompareBlk, pBuf);
        bClearBLK(uiCompareBBLKNum, pBuf);

        INIT_WRITE_BLK();

        puiBNextIndex = (uINT *)malloc(uiSegment * sizeof(uINT));
        puiDNextIndex = (uINT *)malloc(uiSegment * sizeof(uINT));
        for (uINT uiSegmentIndex = 0; uiSegmentIndex < uiSegment; uiSegmentIndex++)
        {
            RESET_CUR_RECORD(uiSegmentIndex);
            RESET_CUR_DBLK(uiSegmentIndex);
        }

        
        for (uINT uiSegmentIndex = 0; uiSegmentIndex < uiSegment; uiSegmentIndex++)                                       /* 初始每个分组都读入一块至内存 */
        {
            uiDBLKNum = GET_CUR_DBLK_NUM(querySelector.uiBasePos, uiSegmentIndex);
            puBlk     = readBlockFromDisk(uiDBLKNum, pBuf);  
            uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
            
            if(uiBaseSegmentBBLKNum == -1){
                uiBaseSegmentBBLKNum = uiBBLKNum;
            }
            
            record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiSegmentIndex), pBuf);
            bSetBLKRecord(uiCompareBBLKNum, uiSegmentIndex, record, pBuf);              /* 初始化Compare BLK */
        }
        

        while (!tpmmCheckIsOver(uiCompareBBLKNum, uiSegment, pBuf))                  /* 判断TPMMS是否结束 */
        {
            WRITE_TILL_BLK_FILL(NULL);
                                                                                        /* 选择最小值所在Segment的索引 */
            uiMaxMinSegmentIndex = tpmmSelectMaxMinIndex(uiCompareBBLKNum, uiSegment, querySelector.uiAttrNum, bIsAscend, pBuf);
            if((bIsAscend && uiMaxMinSegmentIndex != INT_MAX) || 
               (!bIsAscend && uiMaxMinSegmentIndex != INT_MIN)){                        /* 如果存在 */
                record = bGetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, pBuf);   /* 获取该索引对应的record */
                
                bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, record, pBuf);           /* 写入Write BLK中 */
                uiWriteCurIndex++;                                                      /* 写入指针++ */
                tpmmShiftSegmentRecord(querySelector.uiBasePos, uiCompareBBLKNum, uiBaseSegmentBBLKNum, uiMaxMinSegmentIndex, 
                                       puiBNextIndex, puiDNextIndex, uiNumPerSegment, pBuf);
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
}


/**
 * @brief 基于索引的线性搜索 
 * 
 * @param querySelector 搜索器
 * @param uiDBLKLowNum 索引文件的低块号
 * @param uiDBLKHighNum 索引文件的高块号
 * @param pBuf 内存缓冲区
 */
void indexSearch(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, pBuffer pBuf){
    puChar      puBlk;
    uINT        uiBBLKNum;

    uINT        uiNum;

    uINT        uiDBLKHighBound;
    uINT        uiDBLKLowBound;

    record_t    record1;
    record_t    record2;
    uINT        uiKey;

    DISPLAY_TIPS(querySelector.cTips);
    INIT_IO_COUNTER();
    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    

    record1.attr1 = INVALID_ATTR;
    record1.attr2 = INVALID_ATTR;
    
    record2.attr1 = INVALID_ATTR;
    record2.attr2 = INVALID_ATTR;
    
    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk       = readBlockFromDisk(uiDBLKLowNum + i, pBuf);
        uiBBLKNum   = bConvertBLKAddr2Num(puBlk, pBuf);
#ifdef  OUTPUT_DETAIL_ON
        printf(FONT_COLOR_PURPLE "读入索引块 %ld\n" FONT_COLOR_END, uiDBLKLowNum + i);
#endif //  OUTPUT_DETAIL_ON
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record1 = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            if(uiIndex == BLK_NRECORD - 1 && i == uiNum - 1){
                uiDBLKHighBound = TPMMS_S_POS + 32 + 31;
                uiDBLKLowBound  = record1.attr2;                  /* 索引文件Record (Attr, BlockPtr) */
                break;
            }
            else {
                if(record2.attr1 != INVALID_ATTR && record2.attr2 != INVALID_ATTR){
                    uiKey = querySelector.uiValue;
                    if(abs(uiKey - record1.attr1) <= abs(record2.attr1 - record1.attr1)){
                        uiDBLKHighBound = record1.attr2;
                        uiDBLKLowBound = record2.attr2;
                        break;
                    }
                }
                record2.attr1 = record1.attr1;
                record2.attr2 = record1.attr2;
            }
        }
        freeBlockInBuffer(puBlk, pBuf);
    }

#ifdef  OUTPUT_DETAIL_ON
        printf(FONT_COLOR_GREEN "找到边界块 [%ld, %ld]\n" FONT_COLOR_END, uiDBLKLowBound, uiDBLKHighBound);
#endif //  OUTPUT_DETAIL_ON
    GENERATE_TIPS("利用索引文件找到的边界结果进行线性搜索 - S.C = 50")
    linearSearch(querySelector, uiDBLKLowBound, uiDBLKHighBound, pBuf);
    DISPLAY_IO_CNT();
}