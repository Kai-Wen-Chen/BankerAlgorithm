#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#define NUM_CUSTOMERS 5
#define NUM_RESOURCES 3
int curr[NUM_CUSTOMERS][NUM_RESOURCES];
int max_need[NUM_CUSTOMERS][NUM_RESOURCES];
int finish[NUM_CUSTOMERS];
int max_res[NUM_RESOURCES];
int avail[NUM_RESOURCES];
pthread_mutex_t mutex;

void* Banker(void*);
void Print_state();

int main(){
/*-----Initialize the status--------------------------*/
    pthread_t tid[NUM_CUSTOMERS];
    pthread_attr_t attr;
    int i, j;

    for (i=0;i<NUM_CUSTOMERS;i++){
        finish[i] = 0;
        for (j=0;j<NUM_RESOURCES;j++){
            curr[i][j] = 0;
            max_need[i][j] = 0;
        }
    }
/*-----Get ready for Banker's algorithm---------------*/
    printf("Give the max number of each resources:\n");
    for (i=0;i<NUM_RESOURCES;i++){
        scanf("%d", &max_res[i]);
    }

    printf("\nGive the number of available resources:\n");
    for (i=0;i<NUM_RESOURCES;i++){
        scanf("%d", &avail[i]);
    }

    printf("\nGive the current allocation of each process:\n");
    for (i=0;i<NUM_CUSTOMERS;i++){
        printf("p%d ", i+1);
        for (j=0;j<NUM_RESOURCES;j++){
            scanf("%d", &curr[i][j]);
        }
    }

    //check if curr+avail ?= max_res
    for (j=0;j<NUM_RESOURCES;j++){
        int tmp=0;
        for (i=0;i<NUM_CUSTOMERS;i++){
            tmp += curr[i][j];
        }

        if (avail[j]+tmp != max_res[j]){
            fprintf(stderr, "Wrong setting\n");
            return 0;
        }
    }

    printf("\nGive the max number of resources each process may need:\n");
    for (i=0;i<NUM_CUSTOMERS;i++){
        printf("p%d ", i+1);
        for (j=0;j<NUM_RESOURCES;j++){
            scanf("%d", &max_need[i][j]);
        }
    }

    //check if max_need <= max_res
    for (i=0;i<NUM_CUSTOMERS;i++){
        for (j=0;j<NUM_RESOURCES;j++){
            if (max_need[i][j] > max_res[j]){
                fprintf(stderr, "Wrong setting\n");
                return 0;
            }
        }
    }

    Print_state();
    printf("\nStart the Banker's algorithm\n");
/*-----Start thread-----------------------------------*/
    pthread_attr_init(&attr);
    pthread_mutex_init(&mutex, NULL);

    for (i=0;i<NUM_CUSTOMERS;i++)
        pthread_create(&tid[i], &attr, Banker, (void*)i);
/*-----Finish-----------------------------------------*/
    for (i=0;i<NUM_CUSTOMERS;i++)
        pthread_join(tid[i], NULL);

    printf("\nAll processes have finished\n");
    pthread_exit(0);
    return 0;
}

void* Banker(void* Pid){
    int pid = (int*)Pid;
    int j;
    bool wait = true;

    while (finish[pid] == 0){ //process running
/*-----Check if the bank can satisfy the requisition--*/
        while (wait){
            wait = false;
            for (j=0;j<NUM_RESOURCES;j++){
                int need = max_need[pid][j] - curr[pid][j];
                if (need > avail[j]){
                    wait = true;//has to wait
                }
            }
        }
/*-----Get lock to allocation the resources-----------*/
        pthread_mutex_lock(&mutex);

        printf("\nProcess p%d is allocated with: ", pid+1);
        //allocate the resource
        for (j=0;j<NUM_RESOURCES;j++){
            int need = max_need[pid][j] - curr[pid][j];
            printf("%d ", need);
            curr[pid][j] += need;
            avail[j] -= need;
        }

        //check if the allocation = max_need
        finish[pid] = 1;
        for (j=0;j<NUM_RESOURCES;j++){
            if (max_need[pid][j] != curr[pid][j])
                finish[pid] = 0; //cannot finish
        }

        //if all requisition is satisfied, finish the process
        //release all the resource the process is holding
        if (finish[pid] == 1){
            printf("\np%d has finished\n", pid+1);
            for (j=0;j<NUM_RESOURCES;j++){
                avail[j] += curr[pid][j];
                curr[pid][j] = 0;
                max_need[pid][j] = 0;
            }
        }

        //print the state
        Print_state();
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

void Print_state(){
    int i, j;

    printf("\n---------------------------------");
    printf("\nCurrent available resources: ");
    for (i=0;i<NUM_RESOURCES;i++) printf("%d ", avail[i]);
    printf("\n");

    printf("\nCurrent max_need table:\n");
    for (i=0;i<NUM_CUSTOMERS;i++){
        printf("p%d", i+1);
        for (j=0;j<NUM_RESOURCES;j++){
            printf("%3d", max_need[i][j]);
        }
        printf("\n");
    }

    printf("\nCurrent allocation table:\n");
    for (i=0;i<NUM_CUSTOMERS;i++){
        printf("p%d", i+1);
        for (j=0;j<NUM_RESOURCES;j++){
            printf("%3d", curr[i][j]);
        }
        printf("\n");
    }

    printf("\nCurrent need table:\n");
    for (i=0;i<NUM_CUSTOMERS;i++){
        printf("p%d", i+1);
        for (j=0;j<NUM_RESOURCES;j++){
            printf("%3d", max_need[i][j] - curr[i][j]);
        }
        printf("\n");
    }
    //system("PAUSE");
}
