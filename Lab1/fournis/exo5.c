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
* File : exo5.c
*
*********************************************************************************************************
*/

// Main include of µC-II
#include "includes.h"
/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE       16384            // Size of each task's stacks (# of WORDs)

#define ROBOT_A_PRIO   		8				 // Defining Priority of each task
#define ROBOT_B_PRIO   		9
#define CONTROLLER_PRIO     22

/*
*********************************************************************************************************
*                                             VARIABLES
*********************************************************************************************************
*/

OS_STK           prepRobotAStk[TASK_STK_SIZE];	//Stack of each task
OS_STK           prepRobotBStk[TASK_STK_SIZE];
OS_STK           controllerStk[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                           SHARED  VARIABLES
*********************************************************************************************************
*/

OS_EVENT* queue_controller_to_A;
void     *commMsgControllerToA[10];
OS_EVENT* queue_A_to_B;
void     *commMsgAToB[10];
OS_EVENT* mutex;

volatile int total_item_count = 0;
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
*                                             STRUCTURES
*********************************************************************************************************
*/

typedef struct work_data {
	int work_data_a;
	int work_data_b;
} work_data;

/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main(void)
{
	UBYTE err = OS_NO_ERR;
	OSInit();
	queue_controller_to_A = OSQCreate(&commMsgControllerToA[0], 10);
	queue_A_to_B = OSQCreate(&commMsgAToB[0], 10);
	mutex = OSMutexCreate(7, &err);
	errMsg(err, "Error");
	err |= OSTaskCreate(&controller, (NULL), &controllerStk[TASK_STK_SIZE - 1], CONTROLLER_PRIO);
	err |= OSTaskCreate(&robotA, (NULL), &prepRobotAStk[TASK_STK_SIZE - 1], ROBOT_A_PRIO);
	err |= OSTaskCreate(&robotB, (NULL), &prepRobotBStk[TASK_STK_SIZE - 1], ROBOT_B_PRIO);
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

	printf("ROBOT A @ %d : DEBUT.\n", OSTimeGet() - startTime);
	int itemCountRobotA;
	while (1)
	{
		work_data *data = (work_data*)OSQPend(queue_controller_to_A, 0, &err);
		errMsg(err, "Error");

		itemCountRobotA = data->work_data_a;

		err = OSQPost(queue_A_to_B, data);
		errMsg(err, "Error");


		OSMutexPend(mutex, 0, &err);
		errMsg(err, "Error");
		printf("mutex A IN ");
		writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCountRobotA);
		err = OSMutexPost(mutex);
		errMsg(err, "Error");
		printf("mutex A OUT ");


		int counter = 0;
		while (counter < itemCountRobotA * 1000) { counter++; }
		printf("ROBOT A COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCountRobotA, OSTimeGet() - startTime);

		orderNumber++;
	}
}

void robotB(void* data)
{
	INT8U err;
	int startTime = 0;
	int orderNumber = 1;
	printf("ROBOT B @ %d : DEBUT. \n", OSTimeGet() - startTime);
	int itemCountRobotB;
	while (1)
	{
		work_data *data = (work_data*)OSQPend(queue_A_to_B, 0, &err);
		errMsg(err, "Error");

		itemCountRobotB = data->work_data_b;
		free(data);

		OSMutexPend(mutex, 0, &err);
		errMsg(err, "Error");
		printf("mutex B IN ");

		writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCountRobotB);
		err = OSMutexPost(mutex);
		errMsg(err, "Error");
		printf("mutex B out ");


		int counter = 0;
		while (counter < itemCountRobotB * 1000) { counter++; }
		printf("ROBOT B COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCountRobotB, OSTimeGet() - startTime);

		orderNumber++;
	}
}

void controller(void* data)
{
	INT8U err;
	int startTime = 0;
	int randomTime = 0;
	work_data* workData;
	work_data* workData2;

	printf("TACHE CONTROLLER @ %d : DEBUT. \n", OSTimeGet() - startTime);

	for (int i = 1; i < 11; i++)
	{
		//Création d'une commande
		workData = malloc(sizeof(work_data*));
		workData2 = malloc(sizeof(work_data*));

		workData->work_data_a = (rand() % 8 + 3) * 10;
		workData->work_data_b = (rand() % 8 + 6) * 10;

		printf("TACHE CONTROLLER @ %d : COMMANDE #%d. \n prep time A = %d, prep time B = %d\n", OSTimeGet() - startTime, i, workData->work_data_a, workData->work_data_b);

		// A completer
		err = OSQPost(queue_controller_to_A, workData);
		memcpy(workData2, workData, sizeof(work_data*));
		err = OSQPost(queue_controller_to_A, workData2);

		errMsg(err, "Error");

		// Délai aléatoire avant nouvelle commande
		randomTime = (rand() % 9 + 5) * 4;
		OSTimeDly(randomTime);
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