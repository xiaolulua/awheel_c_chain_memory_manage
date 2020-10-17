/*
 * main.c
 *
 *  Created on: 2020年2月21日
 *      Author: LuYonglei
 */
#include <stdio.h>
#include <stdlib.h>

#include "MemoryManage.h"
//
//size_t malloc_size_aligned(size_t size) {
//	//申请内存字节数对齐
//	//malloc一块内存时，需要对传入的size做字节对齐处理
//	//举例:size为21,那么字节对齐后应该是24
//	//计算过程:size=21+(8-(21对8求余))
////	size += (BYTE_ALIGNMENT - (size & BYTE_ALIGNMENT_MASK));
//	if ((size & BYTE_ALIGNMENT_MASK) != 0)
//		size = (size + BYTE_ALIGNMENT) & (~BYTE_ALIGNMENT_MASK);
//	return size;
//}

//void heap_init_size(size_t base_address, size_t total_heap_size) {
//	//申请堆空间基地址对齐
//	//系统初始化的时候需要创建堆空间
//	//原理和malloc_size_aligned()类似
//	size_t new_base_address = base_address;
//	if ((base_address & BYTE_ALIGNMENT_MASK) != 0) {
//		new_base_address = (base_address + BYTE_ALIGNMENT)
//				& (~BYTE_ALIGNMENT_MASK);
//		size_t decrease_size = new_base_address - base_address;
//		total_heap_size = total_heap_size - decrease_size;
//	}
//	printf("对齐后起始地址：%d 对齐后可用大小：%d\n", new_base_address, total_heap_size);
//
//	size_t end_address = (new_base_address + total_heap_size)
//			& (~BYTE_ALIGNMENT_MASK);
//	printf("对齐后末尾地址：%d ", end_address);
//}
int main(int argc, char **argv) {
//	heap_init_size(16, 15);
//	OS_HeapInit();
//	printf("%d",memory_block_size*8);
	void *p1 = los_malloc(8);
//	OS_MemoryFree(p1);
	OS_MemoryUsableInfo();



//	OS_MemoryFree(p1);
	void *p2 = los_malloc(16);
	OS_MemoryUsableInfo();

	los_free(p1);
	OS_MemoryUsableInfo();
	los_free(p2);
	OS_MemoryUsableInfo();



}

