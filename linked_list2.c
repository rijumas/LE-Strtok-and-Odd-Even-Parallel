#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4       // Número de hilos
#define NUM_OPERATIONS 100000 // Número de operaciones por hilo

// Definir la estructura del nodo de la lista enlazada
struct list_node_s {
   int    data;
   struct list_node_s* next;
   pthread_mutex_t mutex;   // Agregar un mutex en cada nodo
};

// Declaración de las funciones
int  Insert(int value, struct list_node_s** head_p);
void Print(struct list_node_s* head_p);
int  Member(int value, struct list_node_s* head_p);
int  Delete(int value, struct list_node_s** head_p);
void Free_list(struct list_node_s** head_p);
int  Is_empty(struct list_node_s* head_p);
void* Thread_work(void* rank); // Trabajo de los hilos

struct list_node_s* head_p = NULL;  // Lista enlazada global

/*-----------------------------------------------------------------*/
int main(void) {
   long thread;
   pthread_t thread_handles[NUM_THREADS];

   // Variables para medir el tiempo
   struct timespec start, finish;
   double elapsed;

   // Obtener el tiempo inicial
   clock_gettime(CLOCK_MONOTONIC, &start);

   // Crear los hilos
   for (thread = 0; thread < NUM_THREADS; thread++) {
      pthread_create(&thread_handles[thread], NULL, Thread_work, (void*) thread);
   }

   // Esperar a que los hilos terminen
   for (thread = 0; thread < NUM_THREADS; thread++) {
      pthread_join(thread_handles[thread], NULL);
   }

   // Obtener el tiempo final
   clock_gettime(CLOCK_MONOTONIC, &finish);

   Free_list(&head_p);

   // Calcular el tiempo transcurrido
   elapsed = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1e9;
   printf("Tiempo total de ejecución: %.9f segundos\n", elapsed);

   return 0;
}

/*-----------------------------------------------------------------*/
/* Trabajo de los hilos: Cada hilo hará 100,000 operaciones */
void* Thread_work(void* rank) {
   long my_rank = (long) rank;
   int operations = NUM_OPERATIONS;
   int i;

   for (i = 0; i < operations; i++) {
      int random_value = rand() % 1000; // Generar un número aleatorio entre 0 y 999
      
      if (random_value < 999) { // 99.9% para la operación Member
         Member(my_rank * random_value, head_p);
      } else if (random_value == 999) { // 0.05% para la operación Insert
         Insert(my_rank * random_value, &head_p);
      } else { // 0.05% para la operación Delete
         Delete(my_rank * random_value, &head_p);
      }
   }

   return NULL;
}

/*-----------------------------------------------------------------*/
/* Función de inserción */
int Insert(int value, struct list_node_s** head_pp) {
   struct list_node_s* curr_p = *head_pp;
   struct list_node_s* pred_p = NULL;
   struct list_node_s* temp_p;

   // Bloquear nodos a medida que se recorren
   while (curr_p != NULL && curr_p->data < value) {
      pthread_mutex_lock(&(curr_p->mutex));  // Bloquear el nodo actual
      if (pred_p != NULL) {
         pthread_mutex_unlock(&(pred_p->mutex));  // Desbloquear el nodo anterior
      }
      pred_p = curr_p;
      curr_p = curr_p->next;
   }

   // Crear e insertar un nuevo nodo
   temp_p = malloc(sizeof(struct list_node_s));
   temp_p->data = value;
   temp_p->next = curr_p;
   pthread_mutex_init(&(temp_p->mutex), NULL); // Inicializar el mutex del nuevo nodo

   // Inserción en la lista
   if (curr_p == NULL || curr_p->data > value) {
      if (pred_p == NULL) {
         *head_pp = temp_p;  // Inserción en la cabeza
      } else {
         pred_p->next = temp_p;  // Inserción en medio
      }
   } else {
      //printf("%d ya está en la lista\n", value);
      free(temp_p); // Liberar el nuevo nodo si ya está en la lista
   }

   // Desbloquear el nodo anterior y el actual si existen
   if (pred_p != NULL) pthread_mutex_unlock(&(pred_p->mutex));
   if (curr_p != NULL) pthread_mutex_unlock(&(curr_p->mutex));

   return 1;
}

/*-----------------------------------------------------------------*/
/* Función de búsqueda */
int Member(int value, struct list_node_s* head_p) {
   struct list_node_s* curr_p = head_p;

   // Buscar el valor en la lista
   while (curr_p != NULL) {
      pthread_mutex_lock(&(curr_p->mutex));  // Bloquear el nodo actual

      if (curr_p->data == value) {
         //printf("%d está en la lista\n", value);
         pthread_mutex_unlock(&(curr_p->mutex));  // Desbloquear el nodo actual
         return 1;
      }

      struct list_node_s* next_p = curr_p->next;
      pthread_mutex_unlock(&(curr_p->mutex));  // Desbloquear el nodo actual

      curr_p = next_p;  // Avanzar al siguiente nodo
   }

   //printf("%d no está en la lista\n", value);
   return 0;
}

/*-----------------------------------------------------------------*/
/* Función de eliminación */
int Delete(int value, struct list_node_s** head_pp) {
   struct list_node_s* curr_p = *head_pp;
   struct list_node_s* pred_p = NULL;

   // Bloquear nodos a medida que se recorren
   while (curr_p != NULL) {
      pthread_mutex_lock(&(curr_p->mutex));  // Bloquear el nodo actual

      if (curr_p->data == value) {
         // Eliminar el nodo
         if (pred_p == NULL) {
            *head_pp = curr_p->next;  // Eliminar la cabeza
         } else {
            pred_p->next = curr_p->next;  // Eliminar un nodo intermedio
         }

         // Destruir el mutex del nodo eliminado y liberar la memoria
         pthread_mutex_destroy(&(curr_p->mutex)); // Destruir el mutex
         free(curr_p);

         // Desbloquear el nodo anterior antes de salir
         if (pred_p != NULL) pthread_mutex_unlock(&(pred_p->mutex));
         return 1;
      }

      // Guardar el nodo anterior
      if (pred_p != NULL) {
         pthread_mutex_unlock(&(pred_p->mutex));  // Desbloquear el nodo anterior
      }
      pred_p = curr_p;
      curr_p = curr_p->next;  // Avanzar al siguiente nodo
   }

   //printf("%d no está en la lista\n", value);
   pthread_mutex_unlock(&(curr_p->mutex));  // Desbloquear el nodo actual
   return 0;
}

/*-----------------------------------------------------------------*/
/* Función para liberar la lista */
void Free_list(struct list_node_s** head_pp) {
   struct list_node_s* curr_p;
   struct list_node_s* succ_p;

   if (Is_empty(*head_pp)) return;
   curr_p = *head_pp; 
   succ_p = curr_p->next;
   while (succ_p != NULL) {
      pthread_mutex_destroy(&(curr_p->mutex)); // Destruir el mutex
      free(curr_p);
      curr_p = succ_p;
      succ_p = curr_p->next;
   }
   pthread_mutex_destroy(&(curr_p->mutex)); // Destruir el mutex del último nodo
   free(curr_p);
   *head_pp = NULL;
}

/*-----------------------------------------------------------------*/
/* Determinar si la lista está vacía */
int Is_empty(struct list_node_s* head_p) {
   return head_p == NULL;
}
