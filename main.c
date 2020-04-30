#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "buffer.h"

//todo try to remove item_count, use buffer_count instead

#define TRUE 1
buffer_item buffer[BUFFER_SIZE];    // Buffer of size set in buffer.h
sem_t empty;                        // For acquiring the empty lock
sem_t full;                         // For acquiring the full lock
pthread_mutex_t mutex;              // Mutex lock for critical section
pthread_t pTID;              		// Producer thread id
pthread_t cTID;                     // Consumer thread id
pthread_attr_t attr;                // Thread attributes
int buffer_count;                   // Counter to keep track of the number of items in the buffer
int main_sleep;                     // Main thread sleep time
int item_count;                     // Keeps track of the last element in the buffer


// Purpose: Displays the header before running the simulation.
// PARAMETER: None
// Logic: Simply prints a series of statements to the screen.
void print_header()
{
	printf("\n====================================================\n");
	printf("Michael Tobin and Travis Vensel\n");
	printf("CS433 - Operating Systems\n");
	printf("Assignment 4 - Producer Consumer problem\n");
	printf("Simulates a given number of producers and consumers.\n");
	printf("====================================================\n\n");
};


// PURPOSE: Function to initialize the mutual exclusion object mutex, along with the empty and full
//          semaphores.
// PARAMETER: None
// Logic: todo
void initialize()
{
	// Create the mutex lock
	pthread_mutex_init(&mutex, NULL);

	// Create the full semaphore
	sem_init(&full, 0, 0);

	// Create the empty semaphore
	sem_init(&empty, 0, BUFFER_SIZE);

	// Initialize attributes and counters
	pthread_attr_init(&attr);
	buffer_count = 0;
	item_count = 0;
};


// PURPOSE: Displays the content of the shared buffer
// PARAMETER: A copy of the current buffer_count
// Logic: todo
void displayBuffer(int buff_ctr)
{
	printf("The current content of the buffer is [ ");
	fflush(stdout);

	if (buff_ctr == 0)
	{
		printf("empty");
		fflush(stdout);
	}

	for (int x = 0; x < buff_ctr; x++)
	{
		if (buff_ctr != 0)
		{
			printf("%d", buffer[x]);
			fflush(stdout);

			if (x + 1 != buff_ctr)
			{
				printf(", ");
				fflush(stdout);
			}
		}
	}
	printf(" ]\n\n");
};


// PURPOSE: Buffer operation used by the Producer thread
//          to insert an item into the buffer.
// PARAMETER: buffer_item
// Logic: todo
// Returns: - 0 if successful
//   - Otherwise -1 indicating an error
int insertItem(buffer_item item)
{
	if (buffer_count < BUFFER_SIZE)
	{
		// Insert item into buffer
		buffer[buffer_count] = item;
		buffer_count++;

		item_count = buffer_count;

		// Print out the contents of the shared buffer
		printf("Item %d inserted by a producer. \n", item_count);
		displayBuffer(buffer_count);

		return 0;
	}

	else
	{
		return -1;
	}
}


// PURPOSE: Buffer operation used by the Consumer thread to
//          remove an item from the buffer.
// PARAMETER: buffer_item
// Logic: RETURNS: returns 0 if successful. returns -1 if not. todo
int removeItem(buffer_item *item)
{
	if (buffer_count > 0)
	{
		*item = buffer[(buffer_count - 1)];
		item_count = buffer_count;
		buffer_count--;

		// Print out the contents of the shared buffer
		printf("Item %d removed by a consumer. \n", item_count);
		displayBuffer(buffer_count);
		return 0;
	} else
	{
		// Error buffer empty
		return -1;
	}

	item_count = item_count - 1; //todo is this line necessary? its not functioning
}


// PURPOSE: The Producer thread will alternate between sleeping for
//           a random period of time and inserting a random integer
//           into the buffer.
// PARAMETER: todo
// Logic: todo
void *producer(void *param)
{
	buffer_item item;
	while (TRUE)
	{
		// Sleep for a random period of time between 1 and main sleep time
		int randSleep = rand() % main_sleep + 1;
		sleep(randSleep);

		// Before entering critical section, wait for the empty signal then acquire the mutex lock
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);

		// Produce a random number between 1 and 1000
		item = rand() % 1000 + 1;

		// Add item to the buffer if there is no error
		if (insertItem(item))
		{
			printf("Producer error: Buffer is full.\n\n");
		}

		// Leaving critical section so release the mutex lock and signal full
		pthread_mutex_unlock(&mutex);
		sem_post(&full);
	}
}


// PURPOSE: The Consumer will also sleep for a random period of time
//          and, upon awakening, the consumer will attempt to
//          remove an item from the buffer.
// PARAMETER: todo
// Logic: todo
void *consumer(void *param)
{
	buffer_item item;
	while (TRUE)
	{
		// Sleep for a random period of time between 1 and main sleep time
		int randomSleep = rand() % main_sleep + 1;
		sleep(randomSleep);

		// Wait for the producer to signal full then acquire mutex lock
		sem_wait(&full);
		pthread_mutex_lock(&mutex);

		// Remove item from buffer if no error
		if (removeItem(&item))
		{
			printf("Consumer error: Buffer is empty.\n\n");
		}

		// Done with critical section so release mutex lock and signal empty
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
	}
}


/*   char *argv[] - contains the 4 parameters being passed from command line:
 *      0. File name.
 *      1. How long the main thread sleep before terminating.
 *      2. The number of producer threads.
 *      3. The number of consumer threads.
 */
// PURPOSE: Runs the Producer-Consumer simulation.
// PARAMETER: argc is the number of parameters (should be 4).
//			  argv holds these values:
// 				- The 1st is the filename (automatic).
// 				- The 2nd is time (in seconds) for main to sleep provided by user.
// 				- The 3rd is the number of producer threads to create provided by user.
// 				- The 4th is the number of consumer threads to create provided by user.
// Logic: todo
int main(int argc, char *argv[])
{
	// Check validity of command line arguments
	if (argc != 4)
	{
		printf("Error: Not enough arguments were entered.\n");
		return -1;
	}

	if (atoi(argv[1]) < 0 || atoi(argv[2]) < 0 || atoi(argv[3]) < 0)
	{
		printf("Error: Entries must be greater than or equal to 0");
		return -1;
	}

	// Store arguments
	main_sleep = atoi(argv[1]);
	int num_produce_threads = atoi(argv[2]);
	int num_consume_threads = atoi(argv[3]);

	print_header();

	// Display values entered by user
	printf("Program name is %s", argv[0]);
	printf("\nTime to sleep before terminating = %d ", main_sleep);
	printf("\nNumber of producer threads = %d ", num_produce_threads);
	printf("\nNumber of consumer threads = %d ", num_consume_threads);
	printf("\n\n");

	// Initialize semaphores, muxtex, attributes, and counters
	initialize();

	// Create producer threads
	int i, j;

	for (i = 0; i < num_produce_threads; i++)
	{
		pthread_create(&pTID, &attr, producer, NULL);
	}

	// Create consumer threads
	for (j = 0; j < num_consume_threads; j++)
	{
		pthread_create(&cTID, &attr, consumer, NULL);
	}

	// Sleep for the amount of time that the user specified in seconds
	sleep(main_sleep);

	return 0;
}

