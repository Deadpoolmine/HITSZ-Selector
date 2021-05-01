#include "utils.h"
/**
 * @brief 两趟扫描算法，检测Compare BLK中是否存在重复记录
 * 
 * @param uiCompareBBLKNum Compare BLK在Buffer中的序号
 * @param uiSegment 分成了几路？几组？
 * @param puiSegmentIndexi 第一条记录
 * @param puiSegmentIndexj 与第一条记录重复的记录
 * @param pBuf 内存缓冲区
 * @return true 存在
 * @return false 不存在 
 */
bool tpmmCheckIfSame(uINT uiCompareBBLKNum, uINT uiSegment, uINT* puiSegmentIndexi,uINT* puiSegmentIndexj, pBuffer pBuf){
    bool        bHasSame = FALSE;
    record_t    recordi;
    record_t    recordj;
    for (uINT uiIndexi = 0; uiIndexi < uiSegment - 1; uiIndexi++)
    {
        recordi = bGetBLKRecord(uiCompareBBLKNum, uiIndexi, pBuf);
        if(recordi.attr1 != INVALID_ATTR || recordi.attr2 != INVALID_ATTR ){
            for (uINT uiIndexj = uiIndexi + 1; uiIndexj < uiSegment; uiIndexj++)
            {
                recordj = bGetBLKRecord(uiCompareBBLKNum, uiIndexj, pBuf);
                if(recordi.attr1 == recordj.attr1 && recordi.attr2 == recordj.attr2){
                    *puiSegmentIndexi = uiIndexi;
                    *puiSegmentIndexj = uiIndexj;
                    bHasSame = TRUE;
                    break;
                }
            }
        }
    }
    return bHasSame;
}
/**
 * @brief 根据升序或者降序选择最大值或最小值所在的组号，若给定属性字段相同，则比较另一属性字段值
 * 
 * @param uiCompareBBLKNum Compare BLK号
 * @param uiSegment 分组
 * @param uiAttrNum 关键字
 * @param bIsAscend 是否升序
 * @param pBuf 内存缓冲区
 * @return uINT 组好
 */
uINT tpmmSelectMaxMinIndex(uINT uiCompareBBLKNum, uINT uiSegment, uINT uiAttrNum, bool bIsAscend, pBuffer pBuf){
    int         iKeyMaxMin;
    int         iAnotherKeyMaxMin;
    uINT        uiKey;
    uINT        uiAnotherKey;
    uINT        uiMaxMinIndex;
    record_t    record;
    
    if(bIsAscend){
        iKeyMaxMin    = INT_MAX;
        iAnotherKeyMaxMin = INT_MAX;
    }
    else {
        iKeyMaxMin    = INT_MIN;
        iAnotherKeyMaxMin = INT_MIN;
    }
    for (uINT uiIndex = 0; uiIndex < uiSegment; uiIndex++)
    {
        record = bGetBLKRecord(uiCompareBBLKNum, uiIndex, pBuf);
        if(record.attr1 != INVALID_ATTR && record.attr2 != INVALID_ATTR){
            uiKey = getKeyAttr(record, uiAttrNum);
            uiAnotherKey = getAnotherKeyAttr(record, uiAttrNum);
            if(bIsAscend){
                if((int)uiKey < iKeyMaxMin){
                    iKeyMaxMin = uiKey;
                    uiMaxMinIndex = uiIndex;
                }
                if((int)uiKey == iKeyMaxMin) {
                    if((int)uiAnotherKey < iAnotherKeyMaxMin){
                        iAnotherKeyMaxMin = uiAnotherKey;
                        uiMaxMinIndex = uiIndex;
                    }
                }
            }
            else {
                if((int)uiKey > iKeyMaxMin){
                    iKeyMaxMin = uiKey;
                    uiMaxMinIndex = uiIndex;
                }
                if((int)uiKey == iKeyMaxMin) {
                    if((int)uiAnotherKey > iAnotherKeyMaxMin){
                        iAnotherKeyMaxMin = uiAnotherKey;
                        uiMaxMinIndex = uiIndex;
                    }
                }
            }
        }
    }
    return uiMaxMinIndex;
}
/**
 * @brief 两趟扫描算法，移动组uiSegmentIndex指针
 * 
 * @param uiDBasePos 该组在Disk上的首磁盘块号
 * @param uiCompareBBLKNum CompareBlk在Buffer中的编号
 * @param uiBaseSegmentBBLKNum 跳过CompareBlk和WriteBlk的基地址
 * @param uiSegmentIndex 组号
 * @param puiBNextIndex Buffer组指针 POUT
 * @param puiDNextIndex Disk组指针  PIN
 * @param uiNumPerSegment 每组由多少个块组成
 * @param pBuf 内存缓冲区
 */
void tpmmShiftSegmentRecord(uINT uiDBasePos, uINT uiCompareBBLKNum, uINT uiBaseSegmentBBLKNum, uINT uiSegmentIndex, uINT* puiBNextIndex, uINT* puiDNextIndex, uINT uiNumPerSegment, pBuffer pBuf){    
    puChar      puBlk;
    uINT        uiBBLKNum;

    uINT        uiDBLKNum;
    record_t    record;

    GET_NEXT_RECORD(uiSegmentIndex);                                  /* 该Segment的读指针++ */
    if(GET_CUR_RECORD_INDEX(uiSegmentIndex) != BLK_NRECORD){          /* 若没有读完 */
                                                                            /* 直接将下一个值送入Compare BLK中 */
        uiBBLKNum = GET_CUR_BBLK_NUM(uiSegmentIndex);
        record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiSegmentIndex), pBuf);
        bSetBLKRecord(uiCompareBBLKNum, uiSegmentIndex, record, pBuf);
    }
    else {                                                                  /* 否则 */
                                                                            /* 释放该Segment读入的块 */
        uiBBLKNum =  GET_CUR_BBLK_NUM(uiSegmentIndex);
        freeBlockInBuffer(GET_BUF_DATA(pBuf, uiBBLKNum), pBuf);
        GET_NEXT_DBLK(uiSegmentIndex);
                                                                            /* 读入该Segment的下一块 */
        if(GET_CUR_DBLK_INDEX(uiSegmentIndex) != uiNumPerSegment){          /* 若该Segment具有下一块 */
            uiDBLKNum = GET_CUR_DBLK_NUM(uiDBasePos, uiSegmentIndex);
            puBlk = readBlockFromDisk(uiDBLKNum, pBuf);
            RESET_CUR_RECORD(uiSegmentIndex);   
            uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
            record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiSegmentIndex), pBuf);
        }
        else {                                                  
            record.attr1 = INVALID_ATTR;
            record.attr2 = INVALID_ATTR;
        }

        bSetBLKRecord(uiCompareBBLKNum, uiSegmentIndex, record, pBuf);
    }
}

/**
 * @brief 两趟扫描算法，检测Compare BLK来判断是否完成
 * 
 * @param uiCompareBBLKNum Compare BLK在Buffer中的序号
 * @param uiSegment 分成了几路？几组？
 * @param pBuf 内存缓冲区
 * @return true 完成
 * @return false 未完成 
 */
bool tpmmCheckIsOver(uINT uiCompareBBLKNum, uINT uiSegment, pBuffer pBuf){
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