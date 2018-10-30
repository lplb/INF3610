/*
 * tourControleAvions.h
 *
 *  Created on: 14 sept. 2018
 *      Author: evter
 */

#ifndef SRC_SIMAVION_H_
#define SRC_SIMAVION_H_

#include <ucos_ii.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE       16384            // Size of each task's stacks (# of WORDs)

#define	VERIFICATION_PRIO	3				// Defining Priority of each task
#define	GENERATION_PRIO		4
#define	STATISTIQUES_PRIO	5
#define	ATTERRISSAGE_PRIO	6
#define	TERMINAL0_PRIO		7
#define	TERMINAL1_PRIO		8
#define	DECOLLAGE_PRIO		9

//Intervalles criteres de retard (en minutes)
#define BORNE_INF_LOW      	0
#define BORNE_SUP_LOW      	19
#define BORNE_INF_MEDIUM   	20
#define BORNE_SUP_MEDIUM  	39
#define BORNE_INF_HIGH     	40
#define BORNE_SUP_HIGH     	60

#define FLAG_TERMINAL0	0x01
#define FLAG_TERMINAL1	0x02

#define MUT_MEMORY_PRIO			0
#define MUT_PRINTF_PRIO			1
#define MUT_DEBORDEMENT_PRIO	2




/*
*********************************************************************************************************
*                                               QUEUES
*********************************************************************************************************
*/
OS_EVENT *Q_atterrissage_high;
void *Q_atterrissage_high_data[3];
OS_EVENT *Q_atterrissage_medium;
void *Q_atterrissage_medium_data[4];
OS_EVENT *Q_atterrissage_low;
void *Q_atterrissage_low_data[6];

OS_EVENT *Q_decollage;
void *Q_decollage_data[10];

/*
*********************************************************************************************************
*                                              MAILBOX
*********************************************************************************************************
*/


OS_EVENT* mboxTerm0;
OS_EVENT* mboxTerm1;


/*
*********************************************************************************************************
*                                              FLAGS
*********************************************************************************************************
*/


OS_FLAG_GRP* flagTerminaux;

/*
*********************************************************************************************************
*                                              SEMAPHORES
*********************************************************************************************************
*/

OS_EVENT* semGeneration;
OS_EVENT* semStats;
OS_EVENT* semVerification;


/*
*********************************************************************************************************
*                                              MUTEX
*********************************************************************************************************
*/

OS_EVENT* mutMemory;
OS_EVENT* mutPrintf;
OS_EVENT* mutDebordement;

/*
*********************************************************************************************************
*                                             ENUMERATIONS
*********************************************************************************************************
*/

enum Aeroport { YUL, YYZ, YVR, PEK, DBX, LGA, HND, LAX, CDG, AMS, NB_AEROPORTS };

/*
*********************************************************************************************************
*                                             STRUCTURES
*********************************************************************************************************
*/

typedef struct Avion {
	int id;
	int retard;
	enum Aeroport origine;
	enum Aeroport destination;
} Avion;

/*
*********************************************************************************************************
*                                             SHARED VAIRABLES
*********************************************************************************************************
*/
bool stopSimDebordement;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void create_application();
int create_tasks();
int create_events();

void	generation(void *data);
void	atterrissage(void *data);
void    terminal(void *data);
void    decollage(void *data);
void	remplirAvion(Avion* avion);
void	statistiques(void *data);
void 	verification(void* data);

void    errMsg(INT8U err, char* errMSg);
void* 	safeMalloc(size_t size);
void	safeFree(void* data);
void 	safePrint(const char* msg, void* arg);


#endif /* SRC_SIMAVION_H_ */
