#include "utils.h"
#include "windows.h"
#include "selector.h"

uINT _G_DBLKNextAvailableNum;

/**
 * @brief 初始化磁盘工具：
 *        1. 初始化下一个磁盘欲写入的块块号
 * 
 */
void initDTool(){
    const char * sDir = ".\\data";
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;
    size_t fileLen, nameLen;
    char sPath[2048];
    int maxBLKNum = -1;
    puChar fileNameBuf;

    sprintf(sPath, "%s\\*.blk", sDir);
    
    if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        printf("Path not found: [%s]\n", sDir);
        return;
    }

    do
    {
        if(strcmp(fdFile.cFileName, ".") != 0
                && strcmp(fdFile.cFileName, "..") != 0)
        {
            fileLen = strlen(fdFile.cFileName);
            nameLen = fileLen - 4;
            fileNameBuf = (puChar)malloc(fileLen + 1);
            for (size_t i = 0; i < nameLen; i++)
            {
                *(fileNameBuf + i) = fdFile.cFileName[i];
            }
            *(fileNameBuf + nameLen) = '\0';
            if(atoi(fileNameBuf) > maxBLKNum){
                maxBLKNum = atoi(fileNameBuf);
            }

            if(atoi(fileNameBuf) > TABLE_R_NBLK + TABLE_S_NBLK){
                dropBlockOnDisk(atoi(fileNameBuf));    
            }
        }
    }
    while(FindNextFile(hFind, &fdFile)); 
    FindClose(hFind); 
    
    _G_DBLKNextAvailableNum = maxBLKNum + 1;
    
    if(fileNameBuf){
        free(fileNameBuf);
    }
}

/**
 * @brief 获取下一个磁盘欲写入的块块号，并使之 !!增一
 * 
 * @return uINT 重置后的块号
 */
uINT dGetBLKNextGlobNum(){
    return _G_DBLKNextAvailableNum++;
}

/**
 * @brief 重置下一个磁盘欲写入的块块号为当前最大块号后一个
 * 
 * @return uINT 重置后的块号
 */
uINT dResetGlobNextBLKNum(){
    initDTool();
    return _G_DBLKNextAvailableNum;
}

/**
 * @brief 设置下一个磁盘欲写入的块
 * 
 * @param uiDBLKNum 设置下一个欲写入的块的块号 
 */
void dSetGlobNextBLKNum(uINT uiDBLKNum){
    _G_DBLKNextAvailableNum = uiDBLKNum;
}

/**
 * @brief 向磁盘写入Buffer中的数据块，写入后，这些块被释放
 * 
 * @param uiBBLKNum Buffer BLK号
 * @param uiNum 连续写入uiNum个块
 * @param pBuf 内存缓冲区
 * @return uINT 最后一次写入的块号
 */
uINT dWriteBLK(uINT uiBBLKNum, uINT uiNum, pBuffer pBuf){
    uINT    uiDBLKNextNum; 
    uINT    uiRemainNum;
    puChar  puBBlk;

    
    if(uiBBLKNum + uiNum - 1 > BUF_NBLK){
        printf(TIPS_ERROR "[dWriteBLK: uiNum = %d out of bounds = %d]\n", 
               uiNum, (BUF_NBLK - uiBBLKNum + 1));
    }
    
    for (size_t i = 0; i < uiNum; i++)
    {
        uiDBLKNextNum = dGetBLKNextGlobNum();
        bSetBLKNextBLK(uiBBLKNum + i, uiDBLKNextNum + 1, pBuf);
        puBBlk = GET_BUF_DATA(pBuf, uiBBLKNum + i);
        writeBlockToDisk(puBBlk, uiDBLKNextNum, pBuf);
    }

    return _G_DBLKNextAvailableNum;
}

/**
 * @brief 查看磁盘上的块信息
 * 
 * @param uiDBLKLowNum 磁盘上低块号
 * @param uiDBLKHighNum 磁盘上高块号
 * @param pBuf 内存缓冲区
 */
void dCheckBLKs(uINT uiDBLKLowNum,uINT uiDBLKHighNum, pBuffer pBuf){
    puChar      puBlk;
    uINT        uiNum;

#ifdef OUTPUT_ON
    printf("\n\n块[%ld, %ld]信息记录:\n\n", uiDBLKLowNum, uiDBLKHighNum);
#endif // OUTPUT_ON
    
    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk = readBlockFromDisk(uiDBLKLowNum + i, pBuf);
        printf("[DBLK %d]: ", uiDBLKLowNum + i);
        for (size_t j = 0; j < BLK_NRECORD; j++)
        {
            record_t record = bGetBLKRecord(bConvertBLKAddr2Num(puBlk, pBuf), j, pBuf);
            printf("(%d, %d)\t", record.attr1, record.attr2);
        }
        printf("\n");
        freeBlockInBuffer(puBlk, pBuf);
    }
}
/**
 * @brief 查看关系R和S关系
 * 
 * @param pBuf 内存读取缓冲区
 */
void dCheckTables(pBuffer pBuf) {
    printf("\n------------------------------Table R:------------------------------\n");
    dCheckBLKs(1, TABLE_R_NBLK, pBuf);

    printf("\n------------------------------Table S:------------------------------\n");
    dCheckBLKs(TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, pBuf);
}

/**
 * @brief 查看TPMMS的S关系
 * 
 * @param uiTPMMSRes 传入TPMMS_S_POS
 * @param pBuf 内存缓冲区
 */
void dCheckTpmmsS(uINT uiTPMMSRes, pBuffer pBuf){
    printf("\n------------------------------Table S After TPMMS:------------------------------\n");
    dCheckBLKs(uiTPMMSRes + 32, uiTPMMSRes + 32 + 31, pBuf);
}

/**
 * @brief 检测磁盘块[uiDBLKLowNum, uiDBLKHighNum]基于属性是否排序
 * 
 * @param uiDBLKLowNum 磁盘块低块号
 * @param uiDBLKHighNum 磁盘块高块号
 * @param uiAttrNum 基于属性
 * @return true 已排序
 * @return false 未排序
 */
bool __dCheckIsSort(uINT uiDBLKLowNum,uINT uiDBLKHighNum, uINT uiAttrNum, pBuffer pBuf){
    uINT        uiNum;
    uINT        uiBBLKNum;

    uINT        uiDBLKNum;
    puChar      puBlk;
    
    bool        bIsSort;
    bool        bIsAscend;
    
    
    uINT        uiKey1;
    uINT        uiKey2;
    record_t    record1;
    record_t    record2;

    record1.attr1 = INVALID_ATTR;
    record1.attr2 = INVALID_ATTR;

    record2.attr1 = INVALID_ATTR;
    record2.attr2 = INVALID_ATTR;
    
    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    
    for (size_t i = 0; i < uiNum; i++)
    {
        uiDBLKNum   = i + uiDBLKLowNum;
        puBlk       = readBlockFromDisk(uiDBLKNum, pBuf);
        uiBBLKNum   = bConvertBLKAddr2Num(puBlk, pBuf);
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record1 = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            if(record2.attr1 != INVALID_ATTR && record2.attr2 != INVALID_ATTR){
                uiKey1 = getKeyAttr(record1, uiAttrNum);
                uiKey2 = getKeyAttr(record2, uiAttrNum);
                // switch (uiAttrNum)
                // {
                // case 1:{
                //     uiKey1 = record1.attr1;
                //     uiKey2 = record2.attr1;
                // }
                //     break;
                // case 2:{
                //     uiKey1 = record1.attr2;
                //     uiKey2 = record2.attr2;
                // }
                //     break;
                // default:
                //     break;
                // }
                if(uiKey1 > uiKey2){
                    bIsAscend = TRUE;
                    bIsSort = TRUE;
                }
                else {
                    bIsAscend = FALSE;
                    bIsSort = TRUE;
                }

                if(bIsAscend){
                    if(uiKey1 < uiKey2) {
                        bIsSort = FALSE;
                        break;
                    }
                }
                else {
                    if(uiKey1 > uiKey2) {
                        bIsSort = FALSE;
                        break;
                    }
                }
            }        
            record2 = record1;
        }
        freeBlockInBuffer(puBlk, pBuf);
    }
    return bIsSort;
}
/**
 * @brief 为磁盘块[uiDBLKLowNum, uiDBLKHighNum]建立索引文件
 * 
 * @param uiDBLKLowNum 磁盘块低块号
 * @param uiDBLKHighNum 磁盘块高块号
 * @param uiAttrNum 索引划分基于属性
 * @param uiGap 索引块与索引块间的间距
 * @param puiNum 索引文件占用多少磁盘块
 * @param pBuf 内存缓冲区
 * @return uINT 索引文件起始块号
 */
uINT dBuildIndexFile(uINT uiDBLKLowNum,uINT uiDBLKHighNum, uINT uiAttrNum, uINT uiGap, uINT *puiNum, pBuffer pBuf){
    querySelector_t         querySelector;
    uINT                    uiNum;

    record_t                record;
    record_t                recordIndex;

    puChar                  puWriteBlk;
    uINT                    uiWriteBBLKNum;
    uINT                    uiWriteCurIndex;

    puChar                  puBlk;
    uINT                    uiBBLKNum;
    
    uINT                    BuiBLKPerGap;                    

    DISPLAY_TIPS("为经过TPMMS排序后的关系S建立索引文件\n索引文件位于磁盘块2000");

    if(!__dCheckIsSort(uiDBLKLowNum, uiDBLKHighNum, uiAttrNum, pBuf)){
        dSetGlobNextBLKNum(TPMMS_S_POS);
        querySelector.uiAttrNum = 1;
        querySelector.uiValue = INVALID_ATTR;
        tpmms(querySelector, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, FALSE, pBuf);
    }

    dSetGlobNextBLKNum(INDEX_FILE_POS);

    uiNum           = uiDBLKHighNum - uiDBLKLowNum + 1;
    BuiBLKPerGap    = uiNum / uiGap;    

    puWriteBlk      = getNewBlockInBuffer(pBuf);
    uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
    uiWriteCurIndex = 0;
    bClearBLK(uiWriteBBLKNum, pBuf);

    for (size_t i = 0; i < BuiBLKPerGap; i++)
    {
        if(uiWriteCurIndex == BLK_NRECORD){
            dWriteBLK(uiWriteBBLKNum, 1, pBuf);                                     /* 写入后Buffer会被清除 */
            puWriteBlk      = getNewBlockInBuffer(pBuf);                            /* 重新申请写入块 */
            uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
            bClearBLK(uiWriteBBLKNum, pBuf);
            uiWriteCurIndex = 0;
            *puiNum++;
        }
        puBlk       = readBlockFromDisk(uiDBLKLowNum + i * uiGap, pBuf);
        uiBBLKNum   = bConvertBLKAddr2Num(puBlk, pBuf);
        record      = bGetBLKRecord(uiBBLKNum, 0, pBuf);

        recordIndex.attr1 = getKeyAttr(record, uiAttrNum);

        recordIndex.attr2 = uiDBLKLowNum + i * uiGap;
        bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordIndex, pBuf);
        freeBlockInBuffer(puBlk, pBuf);
        uiWriteCurIndex++;
    }

    if(uiWriteCurIndex != 0) {
        *puiNum++;
        while (uiWriteCurIndex != BLK_NRECORD)
        {
            recordIndex.attr1 = INVALID_ATTR;
            recordIndex.attr2 = INVALID_ATTR;
            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordIndex, pBuf);
            uiWriteCurIndex++;
        }
        dWriteBLK(uiWriteBBLKNum, 1, pBuf);
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }
    
    checkBuffer(pBuf);
}