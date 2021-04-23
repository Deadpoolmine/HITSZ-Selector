#include "utils.h"
#include "windows.h"

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
 * @brief 向磁盘写入Buffer中的数据块
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