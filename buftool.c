#include "utils.h"

/**
 * @brief 获取 Buffer BLK中 valid字节
 * 
 */
#define GET_BUF_BLK_VALID(pBuf, uiBBLKNum) *(pBuf->data + (uiBBLKNum - 1) * (DISK_BLK_PER_SZ + 1))

/**
 * @brief 判断uiBBLKNum是否合法
 * 
 * @param uiBBLKNum 不应该超过已经读取到pBuf中的总个数
 * @param pBuf 内存缓冲取
 * @return true 合法
 * @return false 不合法
 */
bool checkBBLKNum(uINT uiBBLKNum, pBuffer pBuf){
    if(uiBBLKNum > (pBuf->numAllBlk - pBuf->numFreeBlk)){
        printf(TIPS_ERROR "[checkBBLKNum: uiBBLKNum = %d > BlkHasBeenRead = %d]\n", 
               uiBBLKNum, (pBuf->numAllBlk - pBuf->numFreeBlk));
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief 判断Buffer BLK内部索引是否合法
 * 
 * @param uiIndex 取值应该为 0 ~ 6
 * @return true 
 * @return false 
 */
bool checkBIndex(uINT uiIndex){
    if (uiIndex < 0 || uiIndex > 6)
    {
        printf(TIPS_ERROR "[checkBIndex: uiIndex = %d Out of Bounds]\n", uiIndex);
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief 查看Buffer状态
 * 
 * @param pBuf 缓冲区 
 */
void checkBuffer(pBuffer pBuf){
    printf("buffer.blkSize: %d\n",pBuf->blkSize);
    printf("buffer.bufSize: %d\n",pBuf->bufSize);
    printf("buffer.numAllBlk: %d\n",pBuf->numAllBlk);
    printf("buffer.numFreeBlk: %d\n",pBuf->numFreeBlk);
    printf("buffer.numIO: %d\n",pBuf->numIO);
}
/**
 * @brief 将Buffer中的地址转换为相应Buffer中的BLK号
 * 
 * @param puBlk Buffer中的地址，可通过getNewBufferBLK得到，也
 * @param pBuf 
 * @return uINT Buffer内BLK号 
 */
uINT bConvertBLKAddr2Num(puChar puBlk, pBuffer pBuf){
    return ((puBlk - pBuf->data) / (DISK_BLK_PER_SZ + 1)) + 1;
}

/**
 * @brief 获取Buffer BLK中的一条记录。（注：一行数据，不一定是记录，大小为 8B，即两个INT）
 * 
 * @param uiBBLKNum Buffer中的BLK号
 * @param uiIndex 相应Buffer BLK内部索引（只有0 ~ 6可用，剩余为块指针）
 * @param pBuf 内存缓冲区
 * @return record_t 记录，8B
 */
record_t bGetBLKRecord(uINT uiBBLKNum, uINT uiIndex, pBuffer pBuf){
    char               temp[sizeof(int) + 1];
    record_t           record;
    
    memset(temp, 0, sizeof(int) + 1);
    record.attr1 = INVALID_ATTR;
    record.attr2 = INVALID_ATTR;
    
    if(!checkBBLKNum(uiBBLKNum, pBuf)){
        return record;
    }

    if(!checkBIndex(uiIndex)){
        return record;
    }    

    for (size_t i = 0; i < sizeof(int); i++)
    {
        temp[i] = *(GET_BUF_DATA(pBuf, uiBBLKNum) + uiIndex * 8 + i);
    }
    record.attr1 = atoi(temp);

    for (size_t i = 0; i < sizeof(int); i++)
    {
        temp[i] = *(GET_BUF_DATA(pBuf, uiBBLKNum) + uiIndex * 8 + 4 + i);
    }
    record.attr2 = atoi(temp);
    return record;
}

/**
 * @brief 向Buffer BLK中的uiIndex地方写入record
 * 
 * @param uiBBLKNum Buffer BLK号
 * @param uiIndex Buffer BLK内部索引
 * @param record 要写入的记录
 * @param pBuf 内存缓冲区
 * @return bError 分别是索引错误、内存块号错误以及无错误
 */
bError bSetBLKRecord(uINT uiBBLKNum, uINT uiIndex, record_t record, pBuffer pBuf){
    char              temp[sizeof(int) + 1];
    
    memset(temp, 0, sizeof(int) + 1);
    
    if(!checkBBLKNum(uiBBLKNum, pBuf)){
        return BUF_BBNUM_ERROR;
    }

    if(!checkBIndex(uiIndex)){
        return BUF_INDEX_ERROR;
    }    

    itoa(record.attr1, temp, 10);
    for (size_t i = 0; i < sizeof(int); i++)
    {
        *(GET_BUF_DATA(pBuf, uiBBLKNum) + uiIndex * 8 + i) = temp[i];
    }

    itoa(record.attr2, temp, 10);
    for (size_t i = 0; i < sizeof(int); i++)
    {
        *(GET_BUF_DATA(pBuf, uiBBLKNum) + uiIndex * 8 + 4 + i) = temp[i];
    }
    return BUF_NO_ERROR;
}
/**
 * @brief 向Buffer BLK中写入下一块的值
 * 
 * @param uiBBLKNum Buffer BLK号
 * @return uINT 下一块的值
 */
uINT bSetBLKNextBLK(uINT uiBBLKNum, uINT uiDBLKNextNum, pBuffer pBuf){
    char    temp[sizeof(int) + 1];

    itoa(uiDBLKNextNum, temp, 10);
    for (size_t i = 0; i < sizeof(int); i++)
    {
        *(GET_BUF_DATA(pBuf, uiBBLKNum) + BLK_NRECORD * 8 + i) = temp[i];
    }
    return uiDBLKNextNum;
}

/**
 * @brief 查看关系R和S关系
 * 
 * @param pBuf 内存读取缓冲区
 */
void checkTables(pBuffer pBuf) {
    puChar blk;
    printf("\n------------------------------Table R:------------------------------\n");
    for (size_t i = 0; i < TABLE_R_NBLK; i++)
    {
        blk = readBlockFromDisk(i + 1, pBuf);
        printf("[BLK %d]: ", i + 1);
        for (size_t j = 0; j < BLK_NRECORD; j++)
        {
            record_t record = bGetBLKRecord(bConvertBLKAddr2Num(blk, pBuf), j, pBuf);
            printf("(%d, %d)\t", record.attr1, record.attr2);
        }
        printf("\n");
        freeBlockInBuffer(blk, pBuf);
    }

    printf("\n------------------------------Table S:------------------------------\n");
    for (size_t i = 0; i < TABLE_S_NBLK; i++)
    {
        blk = readBlockFromDisk(TABLE_R_NBLK + i + 1, pBuf);
        printf("[BLK %d]: ", TABLE_R_NBLK + i + 1);
        for (size_t j = 0; j < BLK_NRECORD; j++)
        {
            record_t record = bGetBLKRecord(bConvertBLKAddr2Num(blk, pBuf), j, pBuf);
            printf("(%d, %d)\t", record.attr1, record.attr2);
        }
        printf("\n");
        freeBlockInBuffer(blk, pBuf);
    }
}

/**
 * @brief 清除Buffer BLK中的内容（全部置为0）
 * 
 * @param uiBBLKNum
 * @param pBuf 
 */
void bClearBLK(uINT uiBBLKNum, pBuffer pBuf){

    if(!checkBBLKNum(uiBBLKNum, pBuf)){
        return;
    }

    memset(GET_BUF_DATA(pBuf, uiBBLKNum), 0, DISK_BLK_PER_SZ);
}