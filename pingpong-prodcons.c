// GRR20190485 Gustavo Henrique da Silva Barbosa


#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#include <time.h>

int fila[3];
int tamanho_fila = 0;

task_t produtor_1, produtor_2, produtor_3;
task_t consumidor_1, consumidor_2;
semaphore_t  s_item, s_buffer, s_vaga;

int consome_item() {
    if(tamanho_fila <= 0){
        return -1;
    }

    int item = fila[0];
    for(int i = 0; i < tamanho_fila - 1; i++) {
        fila[i] = fila[i+1];
    }

    tamanho_fila--;
    return item;
}

void adiciona_item(int item) {
    fila[tamanho_fila] = item;
    tamanho_fila++;
}

void produtor(void * arg) {
   while (1){
      task_sleep(1000);
      int item = rand() % 99;
      sem_down(&s_vaga);
      sem_down(&s_buffer);
      adiciona_item(item);
      sem_up(&s_buffer);
      sem_up(&s_item);
      printf("%s produziu %d\n", (char *) arg, item);
   }

  task_exit (0) ;
}

void consumidor(void * arg) {
   while (1) {
      sem_down(&s_item);
      sem_down(&s_buffer);
      int item = consome_item();
      sem_up (&s_buffer);
      sem_up (&s_vaga);

      if(item > 0){
        printf("          %s consumiu %d\n", (char *) arg, item);
      }
      task_sleep (1000);
   }

   task_exit (0) ;
}



int main (int argc, char *argv[])
{
  srand(time(NULL)); 
  printf ("main: inicio\n") ;

  ppos_init () ;

  // inicializa semáforo em 0 (bloqueado)
  sem_create(&s_item, 5);
  sem_create(&s_buffer, 1) ;
  sem_create(&s_vaga, 0) ;


  // cria as tarefas
  task_create (&consumidor_1, consumidor, "c1");

  task_create (&produtor_1, produtor, "p1");
  task_create (&produtor_2, produtor, "p2");
  task_create (&produtor_3, produtor, "p3");

  task_create (&consumidor_2, consumidor, "c2");

  // aguarda as tarefas encerrarem
  task_join (&produtor_1) ;
  task_join (&produtor_2) ;
  task_join (&produtor_3) ;

  // destroi o semáforo
  sem_destroy (&s_item);
  sem_destroy (&s_buffer);
  sem_destroy (&s_vaga);

  task_exit (0) ;

  exit (0) ;
}