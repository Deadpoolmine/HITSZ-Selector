#include "selector.h"


#define GET_CUR_DBLK_NUM(basePos, segmentIndex) basePos + segmentIndex * uiNumPerSegment \
                                                + puiDNextIndex[segmentIndex]

#define GET_CUR_BBLK_NUM(segmentIndex)          uiBaseSegmentBBLKNum + segmentIndex

#define GET_NEXT_RECORD(segmentIndex)           puiBNextIndex[segmentIndex]++
#define GET_CUR_RECORD_INDEX(segmentIndex)      puiBNextIndex[segmentIndex]
#define RESET_CUR_RECORD(segmentIndex)          puiBNextIndex[segmentIndex] = 0

#define GET_NEXT_DBLK(segmentIndex)             puiDNextIndex[segmentIndex]++
#define GET_CUR_DBLK_INDEX(segmentIndex)        puiDNextIndex[segmentIndex]
#define RESET_CUR_DBLK(segmentIndex)            puiDNextIndex[segmentIndex] = 0

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

    
    DISPLAY_TIPS("基于线性搜索的选择算法 - S.C = 50");
    uiNum               = uiDBLKHighNum - uiDBLKLowNum + 1;
    uiRecordNum         = 0;
    INIT_IO_COUNTER();
    INIT_WRITE_BLK();

    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk = readBlockFromDisk(uiDBLKLowNum + i, pBuf);              /* 读一块至内存 */
        uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
#ifdef OUTPUT_ON
        printf("读入数据块 %ld\n", uiDBLKLowNum + i);
#endif // OUTPUT_ON
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            WRITE_TILL_BLK_FILL(printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1));
            switch (querySelector.uiAttrNum)
            {
            case 1:{
                if(record.attr1 == querySelector.uiValue){
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex,record, pBuf);
                    uiWriteCurIndex++;     
                    uiRecordNum++;
#ifdef OUTPUT_ON
                    DISPLAY_RECORD(record);      
#endif // OUTPUT_ON
                }
            }
                break;
            case 2:{
                if(record.attr2 == querySelector.uiValue){
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex,record, pBuf);
                    uiWriteCurIndex++;
                    uiRecordNum++;
#ifdef OUTPUT_ON
                    DISPLAY_RECORD(record);      
#endif // OUTPUT_ON
                }
            }
                break;
            default:
                break;
            }   
        }
        freeBlockInBuffer(puBlk, pBuf);                                 /* 清除内存中读入的数据 */
    }
    if(uiWriteCurIndex != 0) {
        uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);
#ifdef OUTPUT_ON
        printf(FONT_COLOR_RED "结果写入磁盘块 %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);
#endif // OUTPUT_ON
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }
    
#ifdef OUTPUT_ON
    printf("满足条件的元组共 %ld 个\n", uiRecordNum);
    DISPLAY_IO_CNT();
#endif // OUTPUT_ON
}

/**
 * @brief 二阶段多路排序检测Compare BLK来判断是否完成
 * 
 * @param uiCompareBBLKNum Compare BLK在Buffer中的序号
 * @param uiSegment 分成了几路？几组？
 * @param pBuf 内存缓冲区
 * @return true 完成
 * @return false 未完成 
 */
bool __tpmmsCheckIsOver(uINT uiCompareBBLKNum, uINT uiSegment, pBuffer pBuf){
    bool        bIsOver = TRUE;
    record_t    record;
    for (uINT uiIndex = 0; uiIndex < uiSegment; uiIndex++)
    {
        record = bGetBLKRecord(uiCompareBBLKNum, uiIndex, pBuf);
        if(record.attr1 != INVALID_ATTR || record.attr2 != INVALID_ATTR){
            bIsOver = FALSE;
            break;
        }
    }
    return bIsOver;
}

/**
 * @brief 根据升序或者降序选择最大值或最小值所在的组号
 * 
 * @param uiCompareBBLKNum Compare BLK号
 * @param uiSegment 分组
 * @param uiAttrNum 关键字
 * @param bIsAscend 是否升序
 * @param pBuf 内存缓冲区
 * @return uINT 组好
 */
uINT __tpmmsSelectMaxMinIndex(uINT uiCompareBBLKNum, uINT uiSegment, uINT uiAttrNum, bool bIsAscend, pBuffer pBuf){
    int         iKeyMaxMin;
    uINT        uiKey;
    uINT        uiMaxMinIndex;
    record_t    record;
    if(bIsAscend){
        iKeyMaxMin    = INT_MAX;
    }
    else {
        iKeyMaxMin    = INT_MIN;
    }
    for (uINT uiIndex = 0; uiIndex < uiSegment; uiIndex++)
    {
        record = bGetBLKRecord(uiCompareBBLKNum, uiIndex, pBuf);
        if(record.attr1 != INVALID_ATTR && record.attr2 != INVALID_ATTR){
            uiKey = getKeyAttr(record, uiAttrNum);
            if(bIsAscend){
                if((int)uiKey < iKeyMaxMin){
                    iKeyMaxMin = uiKey;
                    uiMaxMinIndex = uiIndex;
                }
            }
            else {
                if((int)uiKey > iKeyMaxMin){
                    iKeyMaxMin = uiKey;
                    uiMaxMinIndex = uiIndex;
                }
            }
        }
    }
    return uiMaxMinIndex;
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
        

        while (!__tpmmsCheckIsOver(uiCompareBBLKNum, uiSegment, pBuf))                  /* 判断TPMMS是否结束 */
        {
            WRITE_TILL_BLK_FILL(NULL);
                                                                                        /* 选择最小值所在Segment的索引 */
            uiMaxMinSegmentIndex = __tpmmsSelectMaxMinIndex(uiCompareBBLKNum, uiSegment, querySelector.uiAttrNum, bIsAscend, pBuf);
            if((bIsAscend && uiMaxMinSegmentIndex != INT_MAX) || 
               (!bIsAscend && uiMaxMinSegmentIndex != INT_MIN)){                        /* 如果存在 */
                record = bGetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, pBuf);   /* 获取该索引对应的record */
                
                bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, record, pBuf);           /* 写入Write BLK中 */
                uiWriteCurIndex++;                                                      /* 写入指针++ */

                GET_NEXT_RECORD(uiMaxMinSegmentIndex);                                  /* 该Segment的读指针++ */
                if(GET_CUR_RECORD_INDEX(uiMaxMinSegmentIndex) != BLK_NRECORD){          /* 若没有读完 */
                                                                                        /* 直接将下一个值送入Compare BLK中 */
                    uiBBLKNum = GET_CUR_BBLK_NUM(uiMaxMinSegmentIndex);
                    record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiMaxMinSegmentIndex), pBuf);
                    bSetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, record, pBuf);
                }
                else {                                                                  /* 否则 */
                                                                                        /* 释放该Segment读入的块 */
                    uiBBLKNum =  GET_CUR_BBLK_NUM(uiMaxMinSegmentIndex);
                    freeBlockInBuffer(GET_BUF_DATA(pBuf, uiBBLKNum), pBuf);
                    GET_NEXT_DBLK(uiMaxMinSegmentIndex);
                                                                                        /* 读入该Segment的下一块 */
                    if(GET_CUR_DBLK_INDEX(uiMaxMinSegmentIndex) != uiNumPerSegment){    /* 若该Segment具有下一块 */
                        uiDBLKNum = GET_CUR_DBLK_NUM(querySelector.uiBasePos, uiMaxMinSegmentIndex);
                        puBlk = readBlockFromDisk(uiDBLKNum, pBuf);
                        RESET_CUR_RECORD(uiMaxMinSegmentIndex);   
                        uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
                        record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiMaxMinSegmentIndex), pBuf);
                    }
                    else {                                                  
                        record.attr1 = INVALID_ATTR;
                        record.attr2 = INVALID_ATTR;
                    }

                    bSetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, record, pBuf);
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
    
    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    

    record1.attr1 = INVALID_ATTR;
    record1.attr2 = INVALID_ATTR;
    
    record2.attr1 = INVALID_ATTR;
    record2.attr2 = INVALID_ATTR;
    
    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk       = readBlockFromDisk(uiDBLKLowNum + i, pBuf);
        uiBBLKNum   = bConvertBLKAddr2Num(puBlk, pBuf);
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
    linearSearch(querySelector, uiDBLKLowBound, uiDBLKHighBound, pBuf);
}