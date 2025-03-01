#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

//Define Total Tickets, Number of Tickets, Max Booking Amount
#define TOTAL_TICKETS 500
#define NUM_OF_CUSTOMERS 1000
#define MAX_BOOKING 5
#define TIMEOUT 2 //2 seconds

//Global variable of total tickets available
int available_tickets = TOTAL_TICKETS;
pthread_mutex_t ticket_hold_mutex;
pthread_mutex_t payment_mutex;
pthread_mutex_t logging_mutex;

//Booking Function
void *book(void* tid){
    //Randomize the amount of tickets to buy
    unsigned int seed = time(NULL) ^ pthread_self();
    srand(seed);

    //Customer ID
    int customer_id = *((int*)tid);
    free(tid);

    //Used to simulate randomness/delays that could happen in real time
    usleep(rand() % 50000);
    
    //lock_acquired signals if locks have been acquired
    int lock_acquired = 0;
    struct timespec ts;

    //While locks are not acquired
    while(!lock_acquired){
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += TIMEOUT;
        
        //Try to first acquire ticket_hold_mutex lock, second acquire payment_mutex lock,
        //Wait for 2 seconds before "timing out" mechanism, release ticket_hold_mutex lock if waiting too long to acquire payment_mutex lock
        if(pthread_mutex_timedlock(&ticket_hold_mutex, &ts) == 0){
            if(pthread_mutex_timedlock(&payment_mutex, &ts) == 0){
                lock_acquired = 1;
            }
            else{
                pthread_mutex_unlock(&ticket_hold_mutex);
            }
        }

        //Check if locks have been acquired
        //Print possibile deadlock warning and try again after 1 second
        if (!lock_acquired) {
            printf("Customer %d detected possible deadlock! Retrying...\n", customer_id);
            usleep(1000000);
        }
    }
   
    
    //Check if there are any available tickets
    if (available_tickets > 0) {
        //Randomize the amount of tickets to buy
        int ticket_count = (rand() % 5)+ 1;
        
        //Check if amount of tickets to buy is greater than available tickets, if so buy the rest of the tickets
        //Simulate "holding" the tickets for this customer
        if(ticket_count > available_tickets){
            ticket_count = available_tickets;
        }
        
        //Decrease the amount of tickets
        available_tickets-=ticket_count;

        //Simulate a booked ticket
        printf("Customer %d booked %d ticket(s)! Remaining tickets: %d\n", customer_id, ticket_count, available_tickets);

        //Write to a file, bookings: thread 't' bought 'x' amount of tickets
        pthread_mutex_lock(&logging_mutex);
        FILE *logFile = fopen("bookings.log", "a");
        if (logFile != NULL) {
            fprintf(logFile, "Customer %d booked %d ticket(s)\n", customer_id, ticket_count);
            fclose(logFile);
        }
        pthread_mutex_unlock(&logging_mutex);

    } 
    else {
        //Simulate a failure to book a ticket
        printf("Customer %d failed to book a ticket. Sold out!\n", customer_id);
    }
    
    //Unlock critical section
    pthread_mutex_unlock(&ticket_hold_mutex);
    pthread_mutex_unlock(&payment_mutex);

    return NULL;
}

int main(){
    //Array of customer threads
    pthread_t customers[NUM_OF_CUSTOMERS];
    //Initialize mutexes
    pthread_mutex_init(&ticket_hold_mutex, NULL);
    pthread_mutex_init(&payment_mutex, NULL);
    pthread_mutex_init(&logging_mutex, NULL);
    srand(time(NULL));

    //Print program start, countdown 3 seconds, release the threads
    printf("Ticket Booking System\nDEADLOCK SIMULATION\n----------------------\n");
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

    //Destroy mutexes
    pthread_mutex_destroy(&ticket_hold_mutex);
    pthread_mutex_destroy(&payment_mutex);
    pthread_mutex_destroy(&logging_mutex);

    printf("All tickets are sold out. Enjoy the show!\n");

    return 0;
}