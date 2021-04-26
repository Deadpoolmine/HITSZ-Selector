/*
 * extmem.c
 * Zhaonian Zou
 * Harbin Institute of Technology
 * Jun 22, 2011
 */

#include <stdlib.h>
#include <stdio.h>
#include<string.h>
#include "extmem.h"

/**
 * @brief 初始化Buffer
 * 
 * @param bufSize Buffer的大小
 * @param blkSize 块大小
 * @param buf Buffer数据结构
 * @return Buffer* 
 */
Buffer *initBuffer(size_t bufSize, size_t blkSize, Buffer *buf)
{
    int i;

    buf->numIO = 0;
    buf->bufSize = bufSize;
    buf->blkSize = blkSize;
    buf->numAllBlk = bufSize / (blkSize + 1);
    buf->numFreeBlk = buf->numAllBlk;
    buf->data = (unsigned char*)malloc(bufSize * sizeof(unsigned char));
    
    if (!buf->data)
    {
        perror("Buffer Initialization Failed!\n");
        return NULL;
    }

    memset(buf->data, 0, bufSize * sizeof(unsigned char));
    return buf;
}

/**
 * @brief 清除Buffer中的内容
 * 
 * @param buf Buffer
 */
void freeBuffer(Buffer *buf)
{
    free(buf->data);
}

/**
 * @brief 在Buffer中开辟一个新的块供写入
 * 
 * @param buf 内存缓冲区
 * @return unsigned char*   该Block的跳过valid位的地址 
 */
unsigned char *getNewBlockInBuffer(Buffer *buf)
{
    unsigned char *blkPtr;

    if (buf->numFreeBlk == 0)
    {
        perror("Buffer is full!\n");
        return NULL;
    }

    blkPtr = buf->data;

    while (blkPtr < buf->data + (buf->blkSize + 1) * buf->numAllBlk)
    {
        if (*blkPtr == BLOCK_AVAILABLE)
            break;
        else
            blkPtr += buf->blkSize + 1;
    }

    *blkPtr = BLOCK_UNAVAILABLE;
    buf->numFreeBlk--;
    return blkPtr + 1;
}
/**
 * @brief 根据BLK起始地址，清除Buffer中的该块
 * 
 * @param blk BLK起始地址
 * @param buf 内存缓冲区
 */
void freeBlockInBuffer(unsigned char *blk, Buffer *buf)
{
    *(blk - 1) = BLOCK_AVAILABLE;
    buf->numFreeBlk++;
}

/**
 * @brief 清理磁盘上的一个数据块
 * 
 * @param addr 磁盘数据块块号
 * @return int 
 */
int dropBlockOnDisk(unsigned int addr)
{
    char filename[40];

    sprintf(filename, "data/%d.blk", addr);

    if (remove(filename) == -1)
    {
        perror("Dropping Block Fails!\n");
        return -1;
    }

    return 0;
}
/**
 * @brief 从磁盘上读入一个数据块
 * 
 * @param addr 磁盘数据块块号
 * @param buf 内存缓冲区
 * @return unsigned char*  该Block的跳过valid位的地址 
 */
unsigned char *readBlockFromDisk(unsigned int addr, Buffer *buf)
{
    char filename[40];
    unsigned char *blkPtr, *bytePtr;
    char ch;

    if (buf->numFreeBlk == 0)
    {
        perror("Buffer Overflows!\n");
        return NULL;
    }

    blkPtr = buf->data;

    while (blkPtr < buf->data + (buf->blkSize + 1) * buf->numAllBlk)
    {
        if (*blkPtr == BLOCK_AVAILABLE)
            break;
        else
            blkPtr += buf->blkSize + 1;
    }

    sprintf(filename, "data/%d.blk", addr);
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        perror("Reading Block Failed!\n");
        return NULL;
    }

    *blkPtr = BLOCK_UNAVAILABLE;
    blkPtr++;
    bytePtr = blkPtr;

    while (bytePtr < blkPtr + buf->blkSize)
    {
        ch = fgetc(fp);
        *bytePtr = ch;
        bytePtr++;
    }

    fclose(fp);
    buf->numFreeBlk--;
    buf->numIO++;
    return blkPtr;
}
/**
 * @brief 向磁盘中写入一个数据块，并将内存中的该数据块释放
 * 
 * @param blkPtr 欲写入块起始指针
 * @param addr 目标数据块地址
 * @param buf 内存缓冲区
 * @return int 
 */
int writeBlockToDisk(unsigned char *blkPtr, unsigned int addr, Buffer *buf)
{
    char filename[40];
    unsigned char *bytePtr;

    sprintf(filename, "data/%d.blk", addr);
    FILE *fp = fopen(filename, "w");

    if (!fp)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }

    for (bytePtr = blkPtr; bytePtr < blkPtr + buf->blkSize; bytePtr++)
        fputc((int)(*bytePtr), fp);

    fclose(fp);
    *(blkPtr - 1) = BLOCK_AVAILABLE;//重新将块置为可用，特别要注意这里
    buf->numFreeBlk++;
    buf->numIO++;
    return 0;
}
