#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4       // Número de hilos
#define NUM_OPERATIONS 100000 // Número de operaciones por hilo

struct list_node_s {
    int data;
    struct list_node_s* next;
};

// Read-write lock for the entire list
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

int Insert(int value, struct list_node_s** head_pp);
void Print(struct list_node_s* head_p);
int Member(int value, struct list_node_s* head_p);
int Delete(int value, struct list_node_s** head_pp);
void Free_list(struct list_node_s** head_pp);
int Is_empty(struct list_node_s* head_p);
char Get_command(void);
int Get_value(void);

// Function to handle thread operations
void* thread_operations(void* arg);

struct list_node_s* head_p = NULL; // Start with empty list

int main(void) {
    pthread_t threads[NUM_THREADS];
   
    // Initialize read-write lock
    pthread_rwlock_init(&rwlock, NULL);
   // Variables para medir el tiempo
   struct timespec start, finish;
   double elapsed;

   // Obtener el tiempo inicial
   clock_gettime(CLOCK_MONOTONIC, &start);
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_operations, (void*)&head_p);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
   // Obtener el tiempo final
   clock_gettime(CLOCK_MONOTONIC, &finish);
    // Free the list
    Free_list(&head_p);

    // Destroy the read-write lock
    pthread_rwlock_destroy(&rwlock);
    // Calcular el tiempo transcurrido
   elapsed = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1e9;
   printf("Tiempo total de ejecución: %.9f segundos\n", elapsed);



    return 0;
}

void* thread_operations(void* arg) {
    struct list_node_s** head_pp = (struct list_node_s**)arg;
    int value;

    // Perform 100000 operations
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        // 99.9% Member
        if (rand() % 1000 < 999) {
            value = rand() % 1000; // Random value
            pthread_rwlock_rdlock(&rwlock);
            Member(value, *head_pp);
            pthread_rwlock_unlock(&rwlock);
        }
        // 0.05% Insert
        else if (rand() % 1000 < 5) {
            value = rand() % 1000; // Random value
            pthread_rwlock_wrlock(&rwlock);
            Insert(value, head_pp);
            pthread_rwlock_unlock(&rwlock);
        }
        // 0.05% Delete
        else {
            value = rand() % 1000; // Random value
            pthread_rwlock_wrlock(&rwlock);
            Delete(value, head_pp);
            pthread_rwlock_unlock(&rwlock);
        }
    }
    return NULL;
}

// Insert function with read-write lock
int Insert(int value, struct list_node_s** head_pp) {
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;
    struct list_node_s* temp_p;

    while (curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p == NULL || curr_p->data > value) {
        temp_p = malloc(sizeof(struct list_node_s));
        temp_p->data = value;
        temp_p->next = curr_p;
        if (pred_p == NULL)
            *head_pp = temp_p;
        else
            pred_p->next = temp_p;
        return 1;
    } else {
        //printf("%d is already in the list\n", value);
        return 0;
    }
}

// Print function
void Print(struct list_node_s* head_p) {
    struct list_node_s* curr_p;

    printf("list = ");
    curr_p = head_p;
    while (curr_p != NULL) {
        printf("%d ", curr_p->data);
        curr_p = curr_p->next;
    }
    printf("\n");
}

// Member function with read-write lock
int Member(int value, struct list_node_s* head_p) {
    struct list_node_s* curr_p;

    curr_p = head_p;
    while (curr_p != NULL && curr_p->data < value)
        curr_p = curr_p->next;

    if (curr_p == NULL || curr_p->data > value) {
        //printf("%d is not in the list\n", value);
        return 0;
    } else {
        //printf("%d is in the list\n", value);
        return 1;
    }
}

// Delete function with read-write lock
int Delete(int value, struct list_node_s** head_pp) {
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;

    while (curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p != NULL && curr_p->data == value) {
        if (pred_p == NULL) {
            *head_pp = curr_p->next;
            free(curr_p);
        } else {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1;
    } else {
        //printf("%d is not in the list\n", value);
        return 0;
    }
}

// Free the list
void Free_list(struct list_node_s** head_pp) {
    struct list_node_s* curr_p;
    struct list_node_s* succ_p;

    if (Is_empty(*head_pp)) return;
    curr_p = *head_pp; 
    succ_p = curr_p->next;
    while (succ_p != NULL) {
        free(curr_p);
        curr_p = succ_p;
        succ_p = curr_p->next;
    }
    free(curr_p);
    *head_pp = NULL;
}

// Check if the list is empty
int Is_empty(struct list_node_s* head_p) {
    return head_p == NULL;
}

// Get command
char Get_command(void) {
    char c;
    printf("Please enter a command:  ");
    scanf(" %c", &c);
    return c;
}

// Get value
int Get_value(void) {
    int val;
    printf("Please enter a value:  ");
    scanf("%d", &val);
    return val;
}
