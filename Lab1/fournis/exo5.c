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

#define ROBOT_A1_PRIO   		8				 // Defining Priority of each task
#define ROBOT_B1_PRIO   		10
#define ROBOT_A2_PRIO			9
#define ROBOT_B2_PRIO			11
#define CONTROLLER_PRIO			22

/*
*********************************************************************************************************
*                                             VARIABLES
*********************************************************************************************************
*/

OS_STK           prepRobotA1Stk[TASK_STK_SIZE];	
OS_STK           prepRobotB1Stk[TASK_STK_SIZE];
OS_STK           prepRobotA2Stk[TASK_STK_SIZE];
OS_STK           prepRobotB2Stk[TASK_STK_SIZE];
OS_STK           controllerStk[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                           SHARED  VARIABLES
*********************************************************************************************************
*/

OS_EVENT* queue_controller_to_A1;
void     *commMsgControllerToA1[10];
OS_EVENT* queue_A1_to_B1;
void     *commMsgA1ToB1[10];

OS_EVENT* queue_controller_to_A2;
void     *commMsgControllerToA2[10];
OS_EVENT* queue_A2_to_B2;
void     *commMsgA2ToB2[10];

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
#define FIRST_SET "firstSet"
#define SECOND_SET "secondSet"
void main(void)
{
	UBYTE err = OS_NO_ERR;
	OSInit();

	queue_controller_to_A1 = OSQCreate(&commMsgControllerToA1[0], 10);
	queue_A1_to_B1 = OSQCreate(&commMsgA1ToB1[0], 10);

	queue_controller_to_A2 = OSQCreate(&commMsgControllerToA2[0], 10);
	queue_A2_to_B2 = OSQCreate(&commMsgA2ToB2[0], 10);


	mutex = OSMutexCreate(7, &err);

	char pdata1[20] = FIRST_SET;
	char pdata2[20] = SECOND_SET;
	errMsg(err, "Error");
	err |= OSTaskCreate(&controller, (NULL), &controllerStk[TASK_STK_SIZE - 1], CONTROLLER_PRIO);
	err |= OSTaskCreate(&robotA, (void*)&pdata1, &prepRobotA1Stk[TASK_STK_SIZE - 1], ROBOT_A1_PRIO);
	err |= OSTaskCreate(&robotB, (void*)&pdata1, &prepRobotB1Stk[TASK_STK_SIZE - 1], ROBOT_B1_PRIO);
	err |= OSTaskCreate(&robotA, (void*)&pdata2, &prepRobotA2Stk[TASK_STK_SIZE - 1], ROBOT_A2_PRIO);
	err |= OSTaskCreate(&robotB, (void*)&pdata2, &prepRobotB2Stk[TASK_STK_SIZE - 1], ROBOT_B2_PRIO);
	errMsg(err, "Error");

	OSStart();


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
	char *set = (char*)data;
	printf("ROBOT A @ %d : DEBUT.\n", OSTimeGet() - startTime);
	int itemCountRobotA;
	BOOLEAN finished = FALSE;
	while (!finished)
	{
		work_data *workData;
		if (strcmp(set, FIRST_SET) == 0) {
			workData = (work_data*)OSQPend(queue_controller_to_A1, 0, &err);
			errMsg(err, "Error");
			itemCountRobotA = workData->work_data_a;
			err = OSQPost(queue_A1_to_B1, workData);
			errMsg(err, "Error");

		}
		else if (strcmp(set, SECOND_SET) == 0) {
			workData = (work_data*)OSQPend(queue_controller_to_A2, 0, &err);
			errMsg(err, "Error");

			itemCountRobotA = workData->work_data_a;

			err = OSQPost(queue_A2_to_B2, workData);
			errMsg(err, "Error");

		}
		else {
			printf("ERROR: No valid set found.\n");
			return;
		}

		if (workData->work_data_a == -1) {
			finished = TRUE;
		}
		else {
			OSMutexPend(mutex, 0, &err);
			errMsg(err, "Error");
			writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCountRobotA);
			err = OSMutexPost(mutex);
			errMsg(err, "Error");


			int counter = 0;
			while (counter < itemCountRobotA * 1000) { counter++; }
			printf("ROBOT A COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCountRobotA, OSTimeGet() - startTime);

			orderNumber++;
		}
	}
	printf("ROBOT A @ %d : FIN.\n", OSTimeGet() - startTime);
}

void robotB(void* data)
{
	INT8U err;
	int startTime = 0;
	int orderNumber = 1;
	printf("ROBOT B @ %d : DEBUT. \n", OSTimeGet() - startTime);
	int itemCountRobotB;
	char *set = (char*)data;
	BOOLEAN finished = FALSE;
	while (!finished)
	{
		work_data *workData;
		if (strcmp(set, FIRST_SET) == 0) {
			workData = (work_data*)OSQPend(queue_A1_to_B1, 0, &err);
			errMsg(err, "Error");

		}
		else if (strcmp(set, SECOND_SET) == 0) {
			workData = (work_data*)OSQPend(queue_A2_to_B2, 0, &err);
			errMsg(err, "Error");
	
		}
		else {
			printf("ERROR: No valid set found.\n");
			return;
		}
	
		if (workData->work_data_b == -1) {
			finished = TRUE;
		}
		else {
			itemCountRobotB = workData->work_data_b;

			OSMutexPend(mutex, 0, &err);
			errMsg(err, "Error");

			writeCurrentTotalItemCount(readCurrentTotalItemCount() + itemCountRobotB);
			err = OSMutexPost(mutex);
			errMsg(err, "Error");


			int counter = 0;
			while (counter < itemCountRobotB * 1000) { counter++; }
			printf("ROBOT B COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCountRobotB, OSTimeGet() - startTime);

			orderNumber++;
		}

		free(workData);
		
	}
	printf("ROBOT B @ %d : FIN.\n", OSTimeGet() - startTime);
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
		workData = malloc(sizeof(work_data));
		workData2 = malloc(sizeof(work_data));

		workData->work_data_a = (rand() % 8 + 3) * 10;
		workData->work_data_b = (rand() % 8 + 6) * 10;
		workData2->work_data_a = (rand() % 8 + 3) * 10;
		workData2->work_data_b = (rand() % 8 + 6) * 10;



		printf("TACHE CONTROLLER @ %d : COMMANDE #%d. \n prep time A = %d, prep time B = %d\n", OSTimeGet() - startTime, i, workData->work_data_a, workData->work_data_b);

		// A completer
		err = OSQPost(queue_controller_to_A1, workData);
		err = OSQPost(queue_controller_to_A2, workData2);

		errMsg(err, "Error");

		// Délai aléatoire avant nouvelle commande
		randomTime = (rand() % 9 + 5) * 4;
		OSTimeDly(randomTime);
	}
	workData = malloc(sizeof(work_data));
	workData2 = malloc(sizeof(work_data));

	workData->work_data_a = -1;
	workData->work_data_b = -1;
	workData2->work_data_a = -1;
	workData2->work_data_b = -1;

	err = OSQPost(queue_controller_to_A1, workData);
	err = OSQPost(queue_controller_to_A2, workData2);

	errMsg(err, "Error");
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