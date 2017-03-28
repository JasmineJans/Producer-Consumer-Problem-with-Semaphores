/*
 Jessica Peretti
 Kaylin Hunter
 Jasmine Jans (submitter)
 
 11/15/2016
 
 This program solves the producer consumer program using linux semaphores.
 
 Compile: gcc asgn6.c
 Run: ./a.out
 
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define SEMKEY 0 
typedef struct sembuf sem_buf;

const int PROD = 0; 
const int CONS = 1;

void producer(int, int, int, sem_buf*, sem_buf*);
void consumer(int, int, int, sem_buf*, sem_buf*);
void criticalSection(int);

void main(int argc, char* argv[])
{
	short int in[2];
	in[0] = 1;
	in[1] = 100;
	in[2] = 0;
	short int out[2];
	 
	sem_buf sem_wait[1], sem_signal[1];

	//set the buffer values for wait
	sem_wait[0].sem_num = 0;
	sem_wait[0].sem_op = -1; //decrement
 	sem_wait[0].sem_flg = SEM_UNDO;   

	//set the buffer values for signal 
	sem_signal[0].sem_num = 0;
	sem_signal[0].sem_op = 1; //increments
	sem_signal[0].sem_flg = SEM_UNDO; 
	
	//create 3 semaphores, semid_mutex, semid_empty, semid_full
	int semid_mutex = semget(SEMKEY, 1, 0777 | IPC_CREAT);
	int semid_empty = semget(SEMKEY, 1, 0777 | IPC_CREAT);
	int semid_full = semget(SEMKEY, 1, 0777 | IPC_CREAT);
	
	//sets the values of the semaphores using given values (1, 100, 0)
	semctl(semid_mutex, 0, SETVAL, 1);
	semctl(semid_empty, 0, SETVAL, 100);
	semctl(semid_full, 0, SETVAL, 0);	

	//store the values of the semaphores in variables
	int mutex, empty, full;
	mutex = semctl(semid_mutex, 0, GETVAL, 0);
	empty = semctl(semid_empty, 0, GETVAL, 0);
	full = semctl(semid_full, 0, GETVAL, 0);
	
	printf("Initial semaphore values: mutex = %d, empty = %d, full = %d\n", mutex, empty, full); 
	
	//Fork a process, child runs consumer, parents runs producer
	int value;
	if ((value = fork()) < 0)
 		printf("Child could not be created\n");
 	else
    if (value == 0) //fork returns 0 to child process, runs child process
   	consumer(semid_mutex, semid_empty, semid_full, sem_wait, sem_signal);
   else
		producer(semid_mutex, semid_empty, semid_full, sem_wait, sem_signal); 
	
	//print the values in the semaphores from the child process/consumer
	if(value == 0)
	{
		mutex = semctl(semid_mutex, 0, GETVAL, 0);
		empty = semctl(semid_empty, 0, GETVAL, 0);
		full = semctl(semid_full, 0, GETVAL, 0);
		printf("Final semaphore values: mutex = %d, empty = %d, full = %d\n", mutex, empty, full);
	
		//clean up the semaphores
		semctl(semid_mutex, 1, IPC_RMID, NULL);
		semctl(semid_empty, 1, IPC_RMID, NULL);
		semctl(semid_full, 1, IPC_RMID, NULL);
	}
}

/*
	"produces" aka decrements the empty and increments the full semaphores
*/
void producer(int semid_mutex, int semid_empty, int semid_full, sem_buf* sem_wait, sem_buf* sem_signal) 
{ 
	int i; 
	for(i = 0; i < 5; i++) 
	{ 
		semop(semid_empty, sem_wait, 1); 
		semop(semid_mutex, sem_wait, 1);
		
		criticalSection(PROD); 
		
		semop(semid_mutex, sem_signal, 1); 
		semop(semid_full, sem_signal, 1); 
		
		sleep(2);
	} 
	
	wait(NULL);
}

/*
	"consumes" aka decrements the full and increments the empty semaphores
*/
void consumer(int semid_mutex, int semid_empty, int semid_full, sem_buf* sem_wait, sem_buf* sem_signal) 
{ 
	int i; 
	for (i = 0; i <5; i++) 
	{ 
		semop(semid_full, sem_wait, 1);
		semop(semid_mutex, sem_wait, 1); 

		criticalSection(CONS); 
		
		semop(semid_mutex, sem_signal, 1); 
		semop(semid_empty, sem_signal, 1); 

		sleep(5);
	} 

}

/*
	critical section that the producer and consumer both access
*/
void criticalSection(int who) 
{  
	if (who == PROD) 
		printf("Producer making an item\n"); 
	else 
		printf("Consumer consuming an item\n"); 
}
