#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//Define Total Tickets, Number of Tickets, Max Booking Amount
#define TOTAL_TICKETS 500
#define NUM_OF_CUSTOMERS 1000
#define MAX_BOOKING 5

//Global variable of total tickets available
int available_tickets = TOTAL_TICKETS;
pthread_mutex_t ticket_mutex;

//Booking Function
void *book(void* tid){
    //Randomize the amount of tickets to buy
    unsigned int seed = time(NULL) ^ pthread_self();
    srand(seed);
    int customer_id = *((int*)tid);

    //Used to simulate randomness/delays that could happen in real time
    usleep(rand() % 50000);
    
    //Lock critical section
    pthread_mutex_lock(&ticket_mutex);
    //Check if there are any available tickets
    if (available_tickets > 0) {
        //Randomize the amount of tickets to buy
        int ticket_count = (rand() % 5)+ 1;
        
        //Check if amount of tickets to buy is greater than available tickets, if so buy the rest of the tickets
        if(ticket_count > available_tickets){
            ticket_count = available_tickets;
        }
        
        //Decrease the amount of tickets
        available_tickets-=ticket_count;

        //Simulate a booked ticket
        printf("Customer %d booked %d ticket(s)! Remaining tickets: %d\n", customer_id, ticket_count, available_tickets);

        //Write to a file, bookings: thread 't' bought 'x' amount of tickets
        FILE *logFile = fopen("bookings.log", "a");
        if (logFile != NULL) {
            fprintf(logFile, "Customer %d booked %d ticket(s)\n", customer_id, ticket_count);
            fclose(logFile);
        }

    } else {
        //Simulate a failure to book a ticket
        printf("Customer %d failed to book a ticket. Sold out!\n", customer_id);
    }
    //Unlock critical section
    pthread_mutex_unlock(&ticket_mutex);

    //Free allocated memory
    free(tid);
    return NULL;
}

int main(){
    //Array of customer threads
    pthread_t customers[NUM_OF_CUSTOMERS];
    //Initialize mutex
    pthread_mutex_init(&ticket_mutex, NULL);
    srand(time(NULL));

    //Print program start, countdown 3 seconds, release the threads
    printf("Ticket Booking System\n----------------------\n");
    printf("Tickets open in\n");
    for (int i = 3; i > 0; i--) {
        printf("%d...\n", i);
        sleep(1);
    }
    printf("Good luck!\n");

    for(int i = 0; i < NUM_OF_CUSTOMERS; i++){
        //Allocate space for a unique integer for each thread
        int* customer_id = malloc(sizeof(int));
        *customer_id = i + 1;
        //Create the threads and run them with the book function
        pthread_create(&customers[i], NULL, book, customer_id);
    }

    for(int i = 0; i < NUM_OF_CUSTOMERS; i++){
        //Wait for all threads to finish
        pthread_join(customers[i], NULL);
    }

    //Destroy mutex
    pthread_mutex_destroy(&ticket_mutex);

    printf("All tickets are sold out. Enjoy the show!\n");

    return 0;
}