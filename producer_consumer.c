// Compile with 
// gcc --std=c99 -pthread -g producer_consumer.c -o producer_consumer

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// declare constant variables
#define CAPACITY 50000
#define TOTAL_PRODUCTS 47000

// incremented when 'STOP\n' is input
int stop_count = 0;

// keeps count of index in 80 length buffer
int line_index = 0;

// reads lines from stdin from Input Thread
char *buff1[CAPACITY];

int buff1_count = 0;
int producer1_index = 0;
int consumer1_index = 0;

pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_buff1 = PTHREAD_COND_INITIALIZER;


// shared buffer between Input Thread and Line Seperator Thread
char *buff2[CAPACITY];

int buff2_count = 0;
int producer2_index = 0;
int consumer2_index = 0;

pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_buff2 = PTHREAD_COND_INITIALIZER;

// shared buffer between Plus Sign Thread and Output Thread
char *buff3[CAPACITY];

int buff3_count = 0;
int producer3_index = 0;
int consumer3_index = 0;

pthread_mutex_t mut3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_buff3 = PTHREAD_COND_INITIALIZER;


char *get_stdin(void)
{
	// gets input from user, stores in input, returns input
	char *input;
	input = malloc(50000 * sizeof(char));
	fgets(input, 50000, stdin);
	return input;
}

void put_buff1(char *product)
{

	// lock mutex before altering
	pthread_mutex_lock(&mut1);

	// place product in buffer at designated index
	buff1[producer1_index] = product;
	
	// adjust index and buffer count
	producer1_index++;
	buff1_count++;
	
	// signal buffer is no longer empty
	pthread_cond_signal(&full_buff1);
	
	// unlock mutex after altering
	pthread_mutex_unlock(&mut1);		
}

void put_buff2(char *product)
{
	pthread_mutex_lock(&mut2);

	buff2[producer2_index] = product;

	producer2_index++;
	buff2_count++;
	
	pthread_cond_signal(&full_buff2);
	
	pthread_mutex_unlock(&mut2);		
}


void put_buff3(char *product)
{
	pthread_mutex_lock(&mut3);
	buff3[producer3_index] = product;

	producer3_index++;
	buff3_count++;
	
	pthread_cond_signal(&full_buff3);
	pthread_mutex_unlock(&mut3);		
}

char *get_buff1()
{
	// lock mutex before altering
	pthread_mutex_lock(&mut1);
	// if buffer is empty, wait for buffer to fill
	while (buff1_count == 0)
	{
		pthread_cond_wait(&full_buff1, &mut1);
	}
	// add product to buffer and adjust consumer index and buffer count
	char *product = buff1[consumer1_index];
	consumer1_index++;
	buff1_count--;
	// unlock mutex and return product
	pthread_mutex_unlock(&mut1);
	return product;
}

char *get_buff2()
{
	pthread_mutex_lock(&mut2);
	while (buff2_count == 0)
	{
		pthread_cond_wait(&full_buff2, &mut2);
	}
	char *product = buff2[consumer2_index];
	consumer2_index++;
	buff2_count--;
	pthread_mutex_unlock(&mut2);
	return product;
}

char *get_buff3()
{
	pthread_mutex_lock(&mut3);
	while (buff3_count == 0)
	{
		pthread_cond_wait(&full_buff3, &mut3);
	}
	char *product = buff3[consumer3_index];
	consumer3_index++;
	buff3_count--;
	pthread_mutex_unlock(&mut3);
	return product;
}


void *buff_to_stdout(void *args)
{
	char *line;
	char *eighty_buff;
	char c;
	int line_length = 0;
	
    // allocate size to line and eighty_buff variables
	line = malloc(50000 * sizeof(char)); 	
	eighty_buff = malloc(80 * sizeof(char));
	
	for (size_t i = 0; i < TOTAL_PRODUCTS; i++)
	{
		line = get_buff3();
		line_length = strlen(line);

		// if STOP is present, since we're on the final thread, just terminate thread
		if (strcmp(line, "STOP\n") == 0) {
			return NULL;
		}
		
		/* iterate through all stdin lines, and fill eighty_buff until 80 characters                           */
		/* once eighty_buff is full, write to stdout, flush stdout, clear eighty_buff, and set line_index to 0 */ 
		for (size_t j = 0; j < line_length; j++)
		{
			c = line[j];
			eighty_buff[line_index] = c;
			line_index++;
			if (line_index == 80) {
				puts(eighty_buff);
				fflush(stdout);
				memset(eighty_buff, 0, 80);
				line_index = 0;
			} 
		}
	}
	return NULL;
	
}

void *caret_to_quote(void *args)
{
	char *line;
	int difference;
	int line_length;
	int plus_count = 0;

    // allocate size to line variable
	line = malloc(50000 * sizeof(char));
	
	// iterate through strings in buffer
	for (size_t i = 0; i < TOTAL_PRODUCTS; i++)
	{
		line = get_buff2();
		
		// if STOP is present, add it to the next buffer and terminate thread
		if (strcmp(line, "STOP\n") == 0) {
			put_buff3(line);
			return NULL;
		}
		line_length = strlen(line);
		// iterate through characters of string
		for (size_t j = 0; j < line_length; j++)
		{		
			if (line[j] == '^')
			{	
				// if two carets are together, convert to double quote, and adjust remaining character indexes in the string
				if (j < (line_length-1) && line[j+1] == '^')
				{
					plus_count++;
					difference = line_length - plus_count - j;
					line[j] = '"';
					for (size_t k = 1; k < difference; k++)
					{
						line[j+k] = line[j+k+1];	
					}
				}
			}
		}
		// add null terminator to end of string and add to buff3
		line[strlen(line)-plus_count] = '\0';
		put_buff3(line);
	}
	return NULL;
}


void *newline_to_space(void *args)
{	
	char *line;
	int line_length;

    // allocate size to line variable
	line = malloc(50000 * sizeof(char));

	for (size_t i = 0; i < TOTAL_PRODUCTS; i++)
	{
		line = get_buff1();
		// if STOP is present, add it to the next buffer and terminate thread
		if (strcmp(line, "STOP\n") == 0) {
			put_buff2(line);
			return NULL;
		}
		line_length = strlen(line);
		/* finds the newline character at the end of the string */
		/* and replaces it with a space.                        */
		line[line_length-1] = ' ';
		put_buff2(line);
	}	
	return NULL;
	
}

void *stdin_to_buff(void *args)
{
	char *payload;

    // allocate size to payload variable
	payload = malloc(50000 * sizeof(char));

	/* loop for input until STOP is encountered */
	/* place input in buff1 if valid            */ 
	while (1)
	{
		payload = get_stdin();
		if (strcmp(payload, "STOP\n") == 0) {
			put_buff1(payload);
			break;
		}
		put_buff1(payload);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	// initialize thread variables
	pthread_t input_t, line_sep_t, caret_t, output_t;
	
	// create the 4 threads
	pthread_create(&input_t, NULL, stdin_to_buff, NULL);
	pthread_create(&line_sep_t, NULL, newline_to_space, NULL);
	pthread_create(&caret_t, NULL, caret_to_quote, NULL);	
	pthread_create(&output_t, NULL, buff_to_stdout, NULL);	
	
	// close out the threads on thread return
	pthread_join(input_t, NULL);
	pthread_join(line_sep_t, NULL);
	pthread_join(caret_t, NULL);
	pthread_join(output_t, NULL);
	
	return EXIT_SUCCESS;
}
