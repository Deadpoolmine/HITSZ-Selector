#include "utils.h"
#include "windows.h"

uINT _G_DBLKNextAvailableNum;

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

uINT dGetBLKNextGlobNum(){
    return _G_DBLKNextAvailableNum++;
}

uINT dResetGlobNextBLKNum(){
    initDTool();
}

void dSetGlobNextBLKNum(uINT uiDBLKNum){
    _G_DBLKNextAvailableNum = uiDBLKNum;
}


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
        puBBlk = GET_BUF_DATA(pBuf, uiBBLKNum + i);
        writeBlockToDisk(puBBlk, uiDBLKNextNum, pBuf);
    }

    return _G_DBLKNextAvailableNum;
}