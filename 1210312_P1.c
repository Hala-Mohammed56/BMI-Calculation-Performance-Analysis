//Name:Hala Mohammed - ID:1210312 - Sec:1
//Project #1
//*********************************************************************


//included standard C libraries and POSIX libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>

//Maximum line length
#define MAX_LINE_LENGTH 150


//declared global variables used in functions
int SizeOfData = 0;
double Sum1 = 0.0;
double Sum2 = 0.0;
int Count = 0;


//initialize a mutex variable for thread for concurrent execution
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


//define a node for a single linked list, store the data
struct Node {
    char gender[10];
    double height;
    double weight;
    struct Node *next;
};


// Function prototypes
void readTheData(struct Node **head);
double calculate_BMI(double weight, double height);
void NaiveApproach(struct Node **head);
void MultiprocessingApproach(struct Node **head, int numProcesses);
void *TheProcess(void *arg);
void MultithreadingApproach(struct Node **head, int numThreads);
void InsertAtEnd(struct Node **head, char gender[], double height, double weight);
struct Node *createNewNode(char gender[], double height, double weight);
void printList(struct Node* head);


//insert the node in the LL
void InsertAtEnd(struct Node **head, char gender[], double height, double weight) {
    struct Node *newNode = createNewNode(gender, height, weight);
    if (*head == NULL) {
        *head = newNode;
    } else {
        struct Node *p = *head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = newNode;
    }
}


//create a new LL node
struct Node *createNewNode(char gender[], double height, double weight) {
    struct Node* NewNode = (struct Node*)malloc(sizeof(struct Node));
    strcpy(NewNode->gender, gender);
    NewNode->height = height;
    NewNode->weight = weight;
    NewNode->next = NULL;
    return NewNode;
}


//read the data from the CSV file and add it to the LL
void readTheData(struct Node **head) {
    FILE* file = fopen("/home/halamohammed/bmi.csv", "r");
    if (file == NULL) {
        printf("Error!\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    if (fgets(line, sizeof(line), file) == NULL) {
        printf("Error while reading !\n");
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char gender[10];
        double height, weight;
        sscanf(line, "%9[^,],%lf,%lf", gender, &height, &weight);//Split the line into the required data
        InsertAtEnd(head, gender, height, weight);
    }

    fclose(file);
}


//calculate the Body Mass Index
double calculate_BMI(double weight, double height) {
    double heightInMeters = height / 100;
    double BMI = weight / pow(heightInMeters, 2);
    return BMI;
}


//naive approach to calculate the avg bmi for the LL
void NaiveApproach(struct Node **head) {
    struct timeval start, end; //track execution time
    gettimeofday(&start, NULL);//get start time

    readTheData(head);

    double sum = 0.0;
    struct Node *current = *head;

    // Iterate in each node
    while (current != NULL) {
        double BMI = calculate_BMI(current->weight, current->height);
        SizeOfData++;
        sum += BMI;
        current = current->next;
    }

    if (SizeOfData > 0) {
        double AvgBMI = sum / SizeOfData;
        printf("Naive Approach: Average BMI = %.2f\n",AvgBMI);
    } else {
        printf("No data in The LL\n");
    }

    gettimeofday(&end, NULL);//get end time
    // Calculate execution time
    double ExecutionTime = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Naive Approach Execution Time is: %.6f seconds\n", ExecutionTime);
}


//calculate the sum of bmi values for a single process
void *TheProcess(void *arg) {
    struct Node *subset = (struct Node *)arg;
    double sum = 0.0;
    int count = 0;

    // Iterate for each nodes in the subset
    while (subset != NULL) {
        double BMI = calculate_BMI(subset->weight, subset->height);
        sum += BMI;
        count++;
        subset = subset->next;
    }

    pthread_mutex_lock(&mutex);// Obtain a mutex lock to modify the shared variables
    Sum1 += sum;
    SizeOfData += count;
    pthread_mutex_unlock(&mutex);// Release mutex lock
    pthread_exit(NULL); // Exit thread
}


//multiprocessing approach to calculate avg of bmi values using multiple processes
void MultiprocessingApproach(struct Node **head, int numProcesses) {
    struct timeval start, end;//track execution time
    gettimeofday(&start, NULL);//get start time

    readTheData(head);

    int TotalNodes = 0;
    struct Node *current = *head;
    while (current != NULL) {
        TotalNodes++;
        current = current->next;
    }

    // Calculate chunk size
    int ChunkSize = TotalNodes / numProcesses;
    struct Node *startChunk = *head;

    double totalSum = 0.0;
    int totalCount = 0;
    pid_t pids[numProcesses];//process IDs
    int pipefd[numProcesses][2]; // Array of pipes for communication 2 for(read,write)

    // Iterate for each process
    for (int i = 0; i < numProcesses; i++) {
        struct Node *endChunk = startChunk;//end of the chunk
        for (int j = 0; j < ChunkSize - 1 && endChunk != NULL; j++) {
            endChunk = endChunk->next;
        }

        // Verify if there are remaining nodes to be processed
        if (endChunk != NULL) {
            struct Node *NextNode = endChunk->next;
            endChunk->next = NULL;

            // Create a communication pipe
            if (pipe(pipefd[i]) == -1) {
                printf("Error creating pipe\n");
                exit(EXIT_FAILURE);
            }


            pid_t pid = fork(); // Create a child process
            if (pid == 0) {
                // Child process
                double sum = 0.0;
                int count = 0;
                struct Node *current = startChunk;
                // Iterate for each nodes in the chunk
                while (current != NULL) {
                    double BMI = calculate_BMI(current->weight, current->height);
                    sum += BMI;
                    count++;
                    current = current->next;
                }

                // Send the sum to the pipe
                close(pipefd[i][0]); // Close the read
                write(pipefd[i][1], &sum, sizeof(double));
                close(pipefd[i][1]); // Close the write

                exit(0);
            } else if (pid > 0) {// Parent process
                pids[i] = pid;
                startChunk = NextNode;
            } else {
                fprintf(stderr, "Error creating process\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Await completion of all child processes
    for (int i = 0; i < numProcesses; i++) {
        int PipeStatus;
        waitpid(pids[i], &PipeStatus, 0);
        if (WIFEXITED(PipeStatus)) {
            // Child process exited
        } else {
            printf("Error in Process #%d\n", pids[i]);
        }

        // Read the sum from the pipe
        close(pipefd[i][1]); // Close the write
        double sum;
        read(pipefd[i][0], &sum, sizeof(double));
        close(pipefd[i][0]); // Close the read
        totalSum += sum;
    }

    // Calculate average BMI
    double avgBMI;
    if (TotalNodes > 0) {
       avgBMI = totalSum / TotalNodes;
    } else {
       avgBMI = 0.0;
    }


    gettimeofday(&end, NULL);//get end time
    // Calculate execution time
    double ExecutionTime = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Multiprocessing Approach: Average BMI = %.2f\n",avgBMI);
    printf("Multiprocessing Approach Execution Time: %.6f seconds\n", ExecutionTime);
}


//calculate the sum of bmi values for a single thread
void *TheThread(void *arg) {
    // Convert the argument to a pointer of type struct Node
    struct Node *Current = (struct Node *)arg;
    double sum = 0.0;
    int count = 0;

    // Calculate sum and count for the thread
    while (Current != NULL) {
        double BMI = calculate_BMI(Current->weight, Current->height);
        sum += BMI;
        count++;
        Current = Current->next;
    }

    // Update global values
    pthread_mutex_lock(&mutex);
    Sum2 += sum;
    Count += count;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);// Exit the thread
}


//multithreading approach to calculate the avg of bmi values using multiple threads
void MultithreadingApproach(struct Node **head, int numThreads) {
    struct timeval start, end;//track execution time
    gettimeofday(&start, NULL);//get start time
    
    // Divide the data into subsets and create the threads
    pthread_t threads[numThreads];
    struct Node *Current = *head;
    int totalNodes = 0;

    //total number of nodes
    while (Current != NULL) {
        totalNodes++;
        Current = Current->next;
    }

    // Calculate chunk size for each thread
    int chunkSize = totalNodes / numThreads;
    int remaining = totalNodes % numThreads;


    pthread_mutex_init(&mutex, NULL);

    //Executing threads
    Current = *head;
    for (int i = 0; i < numThreads && Current != NULL; i++) {
        // Modify the chunk size for the remaining nodes
        int CurrentChunkSize;
        if (i < remaining) {
           CurrentChunkSize = chunkSize + 1;
        } else {
           CurrentChunkSize = chunkSize;
        }


        // Create the thread
        pthread_create(&threads[i], NULL, TheThread, (void *)Current);

        // Move current pointer to the next chunk
        for (int j = 0; j < CurrentChunkSize && Current != NULL; j++) {
            Current = Current->next;
        }
    }

   // Await completion of all threads
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);// Release the mutex

    // Calculate average BMI
    double avgBMI;
    if (Count > 0) {
       avgBMI = Sum2 / Count;
    } else {
       avgBMI = 0.0;
    }
    gettimeofday(&end, NULL);//get end time
    // Calculate execution time
    double ExecutionTime = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Multithreading Approach: Average BMI = %.2f\n", avgBMI);
    printf("Multithreading  Approach Execution Time: %.6f seconds\n", ExecutionTime);
}



int main() {
    struct Node *head = NULL;
    printf("Operating System - Project#1\n");
    printf("************************************************************\n");
    // Testing Naive Approach
    printf("Performing test using a naive approach:\n");
    NaiveApproach(&head);
    printf("\n");

    // Testing Multithreading Approach
    printf("Performing test using a Multithreading Approach:\n");
    printf("Using 1 thread(s):\n");
    MultithreadingApproach(&head, 1);
    printf("Using 2 thread(s):\n");
    MultithreadingApproach(&head, 2);
    printf("Using 3 thread(s):\n");
    MultithreadingApproach(&head, 3);
    printf("Using 4 thread(s):\n");
    MultithreadingApproach(&head, 4);
    printf("\n");

    // Testing Multiprocessing Approach
    printf("Performing test using a Multiprocessing Approach:\n");
    printf("Using 1 core(s):\n");
    MultiprocessingApproach(&head, 1);
    printf("Using 2 core(s):\n");
    MultiprocessingApproach(&head, 2);
    printf("Using 3 core(s):\n");
    MultiprocessingApproach(&head, 3);
    printf("Using 4 core(s):\n");
    MultiprocessingApproach(&head, 4);
    printf("\n");

    // Free memory allocated
    while (head != NULL) {
        struct Node* temp = head;
        head = head->next;
        free(temp);
    }
    return 0;
}


