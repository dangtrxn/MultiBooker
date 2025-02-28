#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

//Number of random values to generate
#define NUM_VALUES 25 

int main() {
    //File descriptors for the pipe
    //fd[0] = read end, fd[1] = write end
    int fd[2];
    pid_t pid;
    
    if (pipe(fd) == -1) {
        perror("Pipe failed");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (pid > 0) {  
        //Parent/Producer Process
        //Close unused read end
        close(fd[0]);
        srand(time(NULL));

        printf("Producer: Generating numbers...\n");
        int numbers[NUM_VALUES];

        for (int i = 0; i < NUM_VALUES; i++) {
            //Generate a random number and print
            numbers[i] = rand() % 100;  
            printf("%d ", numbers[i]); 
        }
        printf("\n");

        //Write to pipe and close write end
        write(fd[1], numbers, sizeof(numbers)); 
        close(fd[1]);
    } 
    else {  
        //Child/Consumer Process
        //Close unused write end
        close(fd[1]);
        int numbers[NUM_VALUES];

        //Read from pipe and close read end
        read(fd[0], numbers, sizeof(numbers)); 
        close(fd[0]);

        //Calculate product of received numbers
        int sum = 0;
        printf("Consumer: Received numbers:\n");
        for (int i = 0; i < NUM_VALUES; i++) {
            printf("%d ", numbers[i]);
            sum += numbers[i];
        }
        printf("\nTotal sum: %d\n", sum);
    }

    return 0;
}
