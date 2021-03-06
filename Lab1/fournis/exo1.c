/*
*********************************************************************************************************
*                                                 uC/OS-II
*                                          The Real-Time Kernel
*                                               PORT Windows
*
*
*                                     Arnaud Desault, Fr�d�ric Fortier
*                                  �cole Polytechnique de Montreal, QC, CANADA
*                                                  01/2018
*
* File : exo1.c
*
*********************************************************************************************************
*/

// Main include of �C-II
#include "includes.h"
/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE       16384            // Size of each task's stacks (# of WORDs)

#define TASK1_PRIO   		5				 // Defining Priority of each task
#define TASK2_PRIO   		3
#define TASK3_PRIO   		7
#define TASK4_PRIO   		8
#define TASK5_PRIO   		9

#define TASK1_ID    		1                // Defining Id of each task
#define TASK2_ID    		2
#define TASK3_ID    		3
#define TASK4_ID    		3
#define TASK5_ID    		3

/*
*********************************************************************************************************
*                                          SHARED  VARIABLES
*********************************************************************************************************
*/

OS_STK           Task1Stk[TASK_STK_SIZE];	//Stack of each task
OS_STK           Task2Stk[TASK_STK_SIZE];
OS_STK           Task3Stk[TASK_STK_SIZE];
OS_STK           Task4Stk[TASK_STK_SIZE];
OS_STK           Task5Stk[TASK_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void    Task1(void *data);
void    Task2(void *data);
void    Task3(void *data);
void    Task4(void *data);
void    Task5(void *data);

/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main(void)
{
	UBYTE err = OS_NO_ERR;
	OSInit();
	err |= OSTaskCreate(&Task1, (NULL), &Task1Stk[TASK_STK_SIZE - 1], TASK1_PRIO);
	err |= OSTaskCreate(&Task2, (NULL), &Task2Stk[TASK_STK_SIZE - 1], TASK2_PRIO);
	err |= OSTaskCreate(&Task3, (NULL), &Task3Stk[TASK_STK_SIZE - 1], TASK3_PRIO);
	err |= OSTaskCreate(&Task4, (NULL), &Task4Stk[TASK_STK_SIZE - 1], TASK4_PRIO);
	err |= OSTaskCreate(&Task5, (NULL), &Task5Stk[TASK_STK_SIZE - 1], TASK5_PRIO);
	OSStart();

	errMsg(err, "Error");

	return;
}

/*
*********************************************************************************************************
*                                            TASK FUNCTIONS
*********************************************************************************************************
*/


void Task1(void* data)
{
	while (1)
	{
		printf("are an \n");
		OSTimeDly(10000);
	}
}

void Task2(void* data)
{
	while (1)
	{
		printf("Task priorities \n");
		OSTimeDly(10000);
	}
}

void Task3(void* data)
{
	while (1)
	{
		printf("important \n");
		OSTimeDly(10000);
	}
}

void Task4(void* data)
{
	while (1)
	{
		printf("feature \n");
		OSTimeDly(10000);
	}
}

void Task5(void* data)
{
	while (1)
	{
		printf("of MicroC-II ! \n");
		OSTimeDly(10000);
	}
}

