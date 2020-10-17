/*
 * MemoryManage.c
 *
 *  Created on: 2020年2月22日
 *      Author: LuYonglei
 */
#include "MemoryManage.h"

static uint8_t Heap[TOTAL_HEAP_SIZE]; //编译器生成可用堆，堆的大小为TOTAL_HEAP_SIZE Bytes

static MCB heapStart; //堆的起始
static MCB *heapEnd; //堆的末尾
static size_t usableBytesRemaining; //当前剩余可用字节数
static size_t usableEverMinRemaining; //历史最小剩余可用字节数
static size_t adjustedHeapSize; //经过对齐调整后的堆的大小
static size_t blockAllocatedBit; //用一个位来标记内存块是否被分配

static size_t MCB_size = sizeof(MCB); //内存控制块大小
static void los_heap_init(void); //系统堆初始化函数，只在第一次动态分配内存时调用
static void los_insert_block_into_usable_list(MCB *toBeInsertedBlock); //把一块内存插入空闲内存链表

void* los_malloc(size_t wantedSize) {
	//动态分配一块内存，内存大小为 wantedSize Bytes
	MCB *currentBlock;
	MCB *previousBlock;
	MCB *newBlock;
	void *addressReturn = NULL;
	los_suspend_all_tasks(); //调度器锁打开
	{
		//如果是第一次分配内存就调用堆初始化函数
		if (heapEnd == NULL)
			los_heap_init();
		if (wantedSize > 0) {
			wantedSize += MCB_size;
			//如果需要进行内存对齐
			if ((wantedSize & BYTE_ALIGNMENT_MASK) != 0)
				wantedSize = (wantedSize + BYTE_ALIGNMENT)
						& (~BYTE_ALIGNMENT_MASK);
		}
		//此处判断wantedSize是否超出限制
		if ((wantedSize & blockAllocatedBit) == 0) {
			//若没有超出限制
			if ((wantedSize > 0) && (wantedSize <= usableBytesRemaining)) {
				//如果申请的内存大小大于0,且系统剩余内存可以满足分配
				previousBlock = &heapStart;
				currentBlock = heapStart.nextUsableBlock;
				while ((currentBlock->blockSize < wantedSize)
						&& (currentBlock->nextUsableBlock != NULL)) {
					previousBlock = currentBlock;
					currentBlock = currentBlock->nextUsableBlock;
				}
				if (currentBlock != heapEnd) {
					addressReturn =
							(void*) ((size_t) (previousBlock->nextUsableBlock)
									+ MCB_size);
					//将当前空闲内存块从内存空闲链表移除
					previousBlock->nextUsableBlock =
							currentBlock->nextUsableBlock;
					if ((currentBlock->blockSize - wantedSize) > MIN_BLOCK_SIZE) {
						//如果这个内存块剩余的部分还可以再利用，就创建新的空闲内存块
						newBlock = (MCB*) ((size_t) currentBlock + wantedSize);
						//更新新的内存块的大小
						newBlock->blockSize = currentBlock->blockSize
								- wantedSize;
						//改变原来的内存块的大小
						currentBlock->blockSize = wantedSize;
						//将新的空闲内存块插入到空闲内存块链表中去
						los_insert_block_into_usable_list(newBlock);
					}
					//更新剩余内存总大小
					usableBytesRemaining -= currentBlock->blockSize;
					//如果当前剩余内存总大小小于历史剩余内存总大小的最小值，就更新历史值
					if (usableBytesRemaining < usableEverMinRemaining)
						usableEverMinRemaining = usableBytesRemaining;
					//此处将内存块的最高位设置为1,标记内存块已经被申请使用
					currentBlock->blockSize |= blockAllocatedBit;
					currentBlock->nextUsableBlock = NULL;
				}
			}
		} else {
			//此处申请内存过大,超出size_t/2
		}
		//此处做trace操作
	}
	los_resume_all_tasks();	//调度器锁关闭
	return addressReturn;
}

void los_free(void *addressToBeFree) {
	uint8_t *baseAddressToBeFree = (uint8_t*) addressToBeFree;
	MCB *blockToBeFree;
	//判断释放的地址是否为空
	if (baseAddressToBeFree != NULL) {
		//经过偏移计算出内存控制块的基地址
		baseAddressToBeFree -= MCB_size;
		blockToBeFree = (MCB*) baseAddressToBeFree;
		//判断内存快是否已经被分配使用，如果是就是放内存块
		if ((blockToBeFree->blockSize & blockAllocatedBit) != 0) {
			if (blockToBeFree->nextUsableBlock == NULL) {
				//将内存块标识为空闲
				blockToBeFree->blockSize &= ~blockAllocatedBit;
				los_suspend_all_tasks();
				{
					usableBytesRemaining += blockToBeFree->blockSize;
					//此处做trace操作
					los_insert_block_into_usable_list(blockToBeFree);
				}
				los_resume_all_tasks();
			}
		}
	}
}

static void los_heap_init(void) {
	//系统堆的初始化函数
	MCB *firstUsableBlock;
	uint8_t *alignedHeapBaseAddress;
	size_t totalHeapSize = TOTAL_HEAP_SIZE;
	//进行内存对齐操作
	size_t newHeapBaseAdress = (size_t) Heap; //获取堆数祖的地址
	if ((newHeapBaseAdress & BYTE_ALIGNMENT_MASK) != 0) {
		newHeapBaseAdress = (newHeapBaseAdress + BYTE_ALIGNMENT)
				& (~BYTE_ALIGNMENT_MASK);
		//内存对齐后,堆的大小发生变化
		totalHeapSize -= newHeapBaseAdress - (size_t) Heap;
	}
	alignedHeapBaseAddress = (uint8_t*) newHeapBaseAdress;
	//初始化空闲链表头部
	heapStart.nextUsableBlock = (MCB*) alignedHeapBaseAddress;
	heapStart.blockSize = (size_t) 0;
	//初始化链表尾部
	newHeapBaseAdress = ((size_t) alignedHeapBaseAddress) + totalHeapSize; //获取堆尾部地址
	newHeapBaseAdress -= MCB_size;
	newHeapBaseAdress &= ~((size_t) BYTE_ALIGNMENT_MASK); //完成heapEnd分配并进行内存对齐后的堆尾部地址
	//初始化链表尾部
	heapEnd = (MCB*) newHeapBaseAdress;
	heapEnd->nextUsableBlock = NULL;
	heapEnd->blockSize = (size_t) 0;
	//将当前所有内存插入空闲内存块链表
	firstUsableBlock = (MCB*) alignedHeapBaseAddress;
	firstUsableBlock->blockSize = newHeapBaseAdress - (size_t) firstUsableBlock;
	firstUsableBlock->nextUsableBlock = heapEnd;
	//更新统计变量
	adjustedHeapSize = firstUsableBlock->blockSize;	//内存对齐后系统管理的堆的大小
	usableEverMinRemaining = firstUsableBlock->blockSize;
	usableBytesRemaining = firstUsableBlock->blockSize;
	//内存块分配标志设置
	blockAllocatedBit = ((size_t) 1)
			<< ((sizeof(size_t) * HEAP_BITS_PER_BYTE) - 1);
}

static void los_insert_block_into_usable_list(MCB *toBeInsertedBlock) {
	//把一块内存插入空闲内存链表
	MCB *blockIterator;
	uint8_t *tempAddress;
	//首先找到一个和toBeInsertedBlock相邻的前一个空闲内存
	blockIterator = &heapStart;
	while ((blockIterator->nextUsableBlock) < toBeInsertedBlock)
		blockIterator = blockIterator->nextUsableBlock;
	tempAddress = (uint8_t*) blockIterator;
	//如果前一个内存的尾部恰好是toBeInsertedBlock的头部，那么这两块空闲内存是连续的，可以合并
	if ((tempAddress + blockIterator->blockSize)
			== (uint8_t*) toBeInsertedBlock) {
		//将toBeInsertedBlock合并到blockIterator中
		blockIterator->blockSize += toBeInsertedBlock->blockSize;
		toBeInsertedBlock = blockIterator;
	}
	//判断是否和后面的空闲内存相邻
	tempAddress = (uint8_t*) toBeInsertedBlock;
	if ((tempAddress + toBeInsertedBlock->blockSize)
			== (uint8_t*) blockIterator->nextUsableBlock) {
		if (blockIterator->nextUsableBlock != heapEnd) {
			//如果后面的内存块的不是heapEnd
			//将后面的内存块合入toBeInsertedBlock
			toBeInsertedBlock->blockSize +=
					blockIterator->nextUsableBlock->blockSize;
			toBeInsertedBlock->nextUsableBlock =
					blockIterator->nextUsableBlock->nextUsableBlock;
		} else {
			//如果后面的内存块的是heapEnd，则只改变指针
			toBeInsertedBlock->nextUsableBlock = heapEnd;
		}
	} else {
		//如果和后面的内存块不相邻，就只能插入链表了
		toBeInsertedBlock->nextUsableBlock = blockIterator->nextUsableBlock;
	}
	//判断前面是否已经合并，如果没有合并就要更新链表
	if (toBeInsertedBlock != blockIterator) {
		blockIterator->nextUsableBlock = toBeInsertedBlock;
	}
}

size_t los_get_usable_heap_size(void) {
	return usableBytesRemaining;
}

size_t los_get_ever_min_usable_heap_size(void) {
	return usableEverMinRemaining;
}

void OS_MemoryUsableInfo(void) {
	MCB *start = &heapStart;
	MCB *end = heapEnd;
	start = start->nextUsableBlock;
	printf(
			"------------------------------------------------------------------\n");
	printf("当前可用内存大小为：%d\n", usableBytesRemaining);
	printf("历史最小可用大小：%d\n", usableEverMinRemaining);
	while (start != end) {
		printf("空闲内存块地址：%p ，空闲内存块大小 %d ，空闲内存块结束地址：%p\n", (size_t) start,
				start->blockSize, ((size_t) start) + start->blockSize);
		start = start->nextUsableBlock;
	}
	printf(
			"------------------------------------------------------------------\n");
}

