/*
*********************************************************************************************************
*                                                 uC/OS-II
*                                          The Real-Time Kernel
*                                               PORT Windows
*
*
*            		          	Arnaud Desaulty, Frederic Fortier, Eva Terriault
*                                  Ecole Polytechnique de Montreal, Qc, CANADA
*                                                  01/2018
*
* File : exo3.c
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

#define ROBOT_A_PRIO   		10				 // Defining Priority of each task
#define ROBOT_B_PRIO   		9
#define CONTROLLER_PRIO     7

#define controller_A_event	0x01
#define A_B_event			0x02
/*
*********************************************************************************************************
*                                             VARIABLES
*********************************************************************************************************
*/

OS_STK           robotAStk[TASK_STK_SIZE];	//Stack of each task
OS_STK           robotBStk[TASK_STK_SIZE];
OS_STK           controllerStk[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                           SHARED  VARIABLES
*********************************************************************************************************
*/
OS_EVENT *mutexItemCount, *mutexPendingCommands;
OS_FLAG_GRP *flagGroup;
volatile int total_item_count = 0;
volatile int pending_commands = 0;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void    robotA(void *data);
void    robotB(void *data);
void    controller(void *data);
void    errMsg(INT8U err, char* errMSg);
int		readCurrentTotalItemCount();
void	writeCurrentTotalItemCount(int qty);

/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main(void)
{
	UBYTE err = OS_NO_ERR;
	OSInit();
	mutexItemCount = OSMutexCreate(8, &err);
	errMsg(err, "Error");
	mutexPendingCommands = OSMutexCreate(4, &err);
	errMsg(err, "Error");
	flagGroup = OSFlagCreate(0, &err);
	errMsg(err, "Error");
	err |= OSTaskCreate(&controller, (NULL), &controllerStk[TASK_STK_SIZE - 1], CONTROLLER_PRIO);
	err |= OSTaskCreate(&robotA, (NULL), &robotAStk[TASK_STK_SIZE - 1], ROBOT_A_PRIO);
	err |= OSTaskCreate(&robotB, (NULL), &robotBStk[TASK_STK_SIZE - 1], ROBOT_B_PRIO);
	OSStart();

	errMsg(err, "Error");
	return;
}

/*
*********************************************************************************************************
*                                            TASK FUNCTIONS
*********************************************************************************************************
*/

void robotA(void* data)
{
	INT8U err;
	int startTime = 0;
	int orderNumber = 1;
	int itemCount;
	printf("ROBOT A @ %d : DEBUT.\n", OSTimeGet() - startTime);
	while (1)
	{
		itemCount = (rand() % 7 + 1) * 10;

		OSFlagPend(flagGroup, controller_A_event, OS_FLAG_WAIT_SET_ALL, 0, &err);
		errMsg(err, "Error");

		OSMutexPend(mutexPendingCommands, 0, &err);
		errMsg(err, "Error");
		pending_commands--;
		if (pending_commands == 0) 
		{
			OSFlagPost(flagGroup, controller_A_event, OS_FLAG_CLR, &err);
			errMsg(err, "Error");
		}
		err = OSMutexPost(mutexPendingCommands);
		errMsg(err, "Error");

		OSFlagPost(flagGroup, A_B_event, OS_FLAG_SET, &err);
		errMsg(err, "Error");
		OSMutexPend(mutexItemCount, 0, &err);
		errMsg(err, "Error");
		writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCount);
		err = OSMutexPost(mutexItemCount);
		errMsg(err, "Error");

		int counter = 0;
		while (counter < itemCount * 1000) { counter++; }
		printf("ROBOT A COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCount, OSTimeGet() - startTime);

		orderNumber++;
	}
}

void robotB(void* data)
{
	INT8U err;
	int startTime = 0;
	int orderNumber = 1;
	int itemCount;
	printf("ROBOT B @ %d : DEBUT. \n", OSTimeGet() - startTime);
	while (1)
	{
		itemCount = (rand() % 6 + 2) * 10;

		OSFlagPend(flagGroup, A_B_event, OS_FLAG_WAIT_SET_ALL, 0, &err);
		errMsg(err, "Error");
		OSFlagPost(flagGroup, A_B_event, OS_FLAG_CLR, &err);
		errMsg(err, "Error");
		OSMutexPend(mutexItemCount, 0, &err);
		errMsg(err, "Error");
		writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCount);
		err = OSMutexPost(mutexItemCount);
		errMsg(err, "Error");

		int counter = 0;
		while (counter < itemCount * 1000) { counter++; }
		printf("ROBOT B COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCount, OSTimeGet() - startTime);

		orderNumber++;
	}
}

void controller(void* data)
{
	INT8U err;
	int startTime = 0;
	int randomTime = 0;
	printf("CONTROLLER @ %d : DEBUT. \n", OSTimeGet() - startTime);
	for (int i = 1; i < 11; i++)
	{
		randomTime = (rand() % 9 + 5) * 10;
		OSTimeDly(randomTime);

		printf("CONTROLLER @ %d : COMMANDE #%d. \n", OSTimeGet() - startTime, i);

		OSMutexPend(mutexPendingCommands, 0, &err);
		errMsg(err, "Error");
		pending_commands++;
		err = OSMutexPost(mutexPendingCommands);
		errMsg(err, "Error");

		OSFlagPost(flagGroup, controller_A_event, OS_FLAG_SET, &err);
		errMsg(err, "Error");

	}
}

int	readCurrentTotalItemCount()
{
	OSTimeDly(2);
	return total_item_count;
}
void writeCurrentTotalItemCount(int newCount)
{
	OSTimeDly(2);
	total_item_count = newCount;
}

void errMsg(INT8U err, char* errMsg)
{
	if (err != OS_ERR_NONE)
	{
		printf(errMsg);
		exit(1);
	}
}