/*
 * task.h
 *
 *  Created on: 2020年2月22日
 *      Author: LuYonglei
 */

#ifndef SRC_TASK_H_
#define SRC_TASK_H_
#include "list.h"

typedef
typedef struct task_control_block{
	volatile
};


void los_suspend_all_tasks(); //调度器锁开启，禁止任务调度
void los_resume_all_tasks(); //调度器锁关闭
#endif /* SRC_TASK_H_ */


