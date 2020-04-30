#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "buffer.h"


#define TRUE 1
buffer_item buffer[BUFFER_SIZE];    // Buffer of size set in buffer.h
sem_t empty;                        // Semaphore for empty lock
sem_t full;                         // Semaphore for full lock
pthread_mutex_t mutex;              // Mutex lock for critical section
pthread_t pTID;              		// Producer thread id
pthread_t cTID;                     // Consumer thread id
pthread_attr_t attr;                // Thread attributes
int buffer_count;                   // Counter to keep track of the number of items in the buffer
int main_sleep;                     // Main thread sleep time


// Purpose: Displays the header before running the simulation.
// PARAMETER: None
// LOGIC: Simply prints a series of statements to the screen.
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
// LOGIC: Creates the mutex, empty and full semaphores, attribute, and a counter for the buffer.
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
};


// PURPOSE: Displays the content of the shared buffer
// PARAMETER: A copy of the current buffer_count
// LOGIC: If the buffer is not empty, loops through until it reaches the end.
void displayBuffer(int buff_ctr)
{
	printf("The current content of the buffer is [ ");
	fflush(stdout); // Needed for lines that done end in a new-line character.

	if (buff_ctr == 0)
	{
		printf("empty");
		fflush(stdout);// Needed for lines that done end in a new-line character.
	}

	for (int x = 0; x < buff_ctr; x++)
	{
		if (buff_ctr != 0)
		{
			printf("%d", buffer[x]);
			fflush(stdout);// Needed for lines that done end in a new-line character.

			if (x + 1 != buff_ctr)
			{
				printf(", ");
				fflush(stdout);// Needed for lines that done end in a new-line character.
			}
		}
	}
	printf(" ]\n\n");
};


// PURPOSE: Buffer operation used by the Producer thread
//          to insert an item into the buffer.
// PARAMETER: buffer_item
// LOGIC: If there is room in the buffer, add the item, print it and return 0.
//		  If there is not room, just return -1.
int insertItem(buffer_item item)
{
	if (buffer_count < BUFFER_SIZE)
	{
		// Insert item into buffer and add it to the counter
		buffer[buffer_count] = item;
		buffer_count++;

		// Print the confirmation and the contents of the buffer
		printf("Item %d inserted by a producer. \n", item);
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
// PARAMETER: Pointer to the item to be removed from the buffer.
// LOGIC: If the buffer is not empty, decrement the counter so that the element is not counted.
// The item is not actually removed from the buffer, but rather ignored in printing
int removeItem(buffer_item *item)
{
	if (buffer_count > 0)
	{
		*item = buffer[0];

		// Print out the contents of the shared buffer
		printf("Item %d removed by a consumer. \n", buffer[0]);

		for (int i=0; i<buffer_count;i++)
		{
			buffer[i]=buffer[i+1];
		}
		buffer_count--;
		displayBuffer(buffer_count);

		return 0;
	}

	else
	{
		// Error buffer empty
		return -1;
	}
}


// PURPOSE: The Producer thread will alternate between sleeping for
//           a random period of time and inserting a random integer
//           into the buffer.
// PARAMETER: None
// LOGIC: Sleeps for a random time that is less than main's sleep time, then wait for empty and mutex lock.
// Then pick a random number and insert it into the buffer. If that fails, print an error. Then release the lock
// and post a full semaphore.
void *producer(void *param)
{
	buffer_item item;
	while (TRUE)
	{
		// Sleep for a random period of time less than 1 second
		usleep(rand()%1000000);

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
// PARAMETER: None
// LOGIC: Sleeps for a random time that is less than main's sleep time, then wait for empty and mutex lock.
// Attempt to remove an item from the buffer; if fails print an error. Release the mutex and post a full semaphore.
void *consumer(void *param)
{
	buffer_item item;
	while (TRUE)
	{
		// Sleep for a random period of time less than 1 second
		usleep(rand()%1000000);

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


// PURPOSE: Runs the Producer-Consumer simulation.
// PARAMETER: argc (argument counter) is the number of parameters pointed to by argv (should be 4 for this program).
//			  argv (argument vector) holds pointers to these strings:
// 				- The 1st is the filename (automatic).
// 				- The 2nd is time (in seconds) for main to sleep provided by user.
// 				- The 3rd is the number of producer threads to create provided by user.
// 				- The 4th is the number of consumer threads to create provided by user.
// LOGIC: Check the number of arguments, if good, convert them from strings to ints and store them. Print the header,
// then print out the users input for confirmation and reminder to the user. Initialize the mutex, semaphore, etc. then
// create the number of processes that the user requested. Sleep for the users chosen amount of time while the processes
// run and then terminate.
int main(int argc, char *argv[])
{
	// Ensure that the user has supplied enough arguments on the command line
	if (argc != 4)
	{
		printf("Error: Not enough arguments were entered.\n");
		return -1;
	}

	// Ensure that the three arguments provided are valid for our uses (non-negative)
	if (atoi(argv[1]) < 0 || atoi(argv[2]) < 0 || atoi(argv[3]) < 0)
	{
		printf("Error: Entries must be greater than or equal to 0");
		exit(-1);
	}

	// Store arguments
	main_sleep = atoi(argv[1]);
	int num_produce_threads = atoi(argv[2]);
	int num_consume_threads = atoi(argv[3]);

	print_header();

	// Display values entered by user
	printf("Program name is: %s", argv[0]);
	printf("\nBuffer size is: %d", BUFFER_SIZE);
	printf("\nTime to sleep before terminating = %d ", main_sleep);
	printf("\nNumber of producer threads = %d ", num_produce_threads);
	printf("\nNumber of consumer threads = %d ", num_consume_threads);
	printf("\n\n");

	// Initialize mutex, semaphores, attributes, and counters
	initialize();

	// Create producer threads
	for (int i = 0; i < num_produce_threads; i++)
	{
		pthread_create(&pTID, &attr, producer, NULL);
	}

	// Create consumer threads
	for (int j = 0; j < num_consume_threads; j++)
	{
		pthread_create(&cTID, &attr, consumer, NULL);
	}

	// Sleep for the amount of time that the user specified in seconds
	sleep(main_sleep);

	return 0;
}

