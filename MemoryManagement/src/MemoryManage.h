/*
 * MemoryManage.h
 *
 *  Created on: 2020年2月22日
 *      Author: LuYonglei
 */

#ifndef SRC_MEMORYMANAGE_H_
#define SRC_MEMORYMANAGE_H_

#include <stdlib.h>
#include <stdint.h>
#include "task.h"

/*      字节对齐数  字节对齐掩码
 *         8        0x0007
 *         4        0x0003
 *         2        0x0001
 *         1        0x0000
 */
#define BYTE_ALIGNMENT 8           //字节对齐数
#define BYTE_ALIGNMENT_MASK 0x0007 //字节对齐掩码
#define TOTAL_HEAP_SIZE 1000       //堆大小
#define MIN_BLOCK_SIZE 32           //最小内存块大小
#define HEAP_BITS_PER_BYTE 8       //堆中每个字节拥有的位数

//定义内存控制块
typedef struct memory_control_block {
	struct memory_control_block *nextUsableBlock; //下一个空闲块的地址
	size_t blockSize; //空闲块的大小
} MCB;

void* los_malloc(size_t wantedSize); //动态分配一块内存，内存大小为 wantedSize Bytes
void los_free(void *addressToBeFree); //释放一块动态分配了的内存
size_t los_get_usable_heap_size(void); //获取当前未分配的内存堆大小
size_t los_get_ever_min_usable_heap_size(void); //获取未分配的内存堆的历史最小值
//以下函数为调试系统所用，实际使用中可删除
void OS_MemoryUsableInfo(void); //获取当前系统内存可分配情况

#endif /* SRC_MEMORYMANAGE_H_ */
