// GRR20190485 Gustavo Henrique da Silva Barbosa

#include<stdio.h>
#include<stdlib.h>
#include"ppos.h"
#include"queue.h"

task_t *MAIN_TASK;
task_t *ACTUAL_TASK;

int LAST_ID = 0;

short READY = 1;
short RUNNING = 2;
short SUSPENDED = 3;
short FINISHED = 4;

void print_task (void *ptr)
{
   task_t *elem = ptr ;

   if (!elem)
      return ;

   printf ("%d", elem->id) ;
}


/* ============ P3 ============ */
task_t *DISPATCHER_TASK;
task_t *USER_TASKS;

/*
* Description: Função de gerenciamento, utilizada pela tarefa dispatcher
* Args:
* Return:
*/
task_t *scheduler () {
    if(!USER_TASKS){
        return NULL;
    }

    task_t *next = USER_TASKS;
    USER_TASKS = USER_TASKS->next;

    return next;
}

void end_task (task_t *task){
    queue_remove((queue_t **) &USER_TASKS, (queue_t *) task);
    
    //libera dados
    free(task->context.uc_stack.ss_sp);
    task->context.uc_stack.ss_size = 0;
    task->context.uc_stack.ss_flags = 0;

    task->id = -1;
}

void dispatcher () {
    debug_print("Inicio dispatcher...\n");
    //queue_print("Fila: ", (queue_t *) USER_TASKS,  print_task);

   // encerra a tarefa dispatcher
   while(queue_size((queue_t *) USER_TASKS) > 0){
       // escolhe a próxima tarefa a executar
       task_t *next_task = scheduler();

       if (next_task != NULL) {
           debug_print("Próxima tarefa: %d\n", next_task->id);
           task_switch(next_task);
            
           short status = next_task->status;
           if(status == FINISHED){
               end_task(next_task);
               // queue_print("Fila: ", (queue_t *) USER_TASKS,  print_task);
           }
       }
   }

   debug_print("Fim dispatcher...\n");
   task_exit(0);
}

/*
* Description: Função altera o contexto para o dispatcher
* Args:
* Return:
*/
void task_yield () {
    // Coloca a tarefa atual como suspensa
    ACTUAL_TASK->status = SUSPENDED;

    // Coloca a tarefa do Dispather como corrente
    DISPATCHER_TASK->status = RUNNING;

    // Altera tasks
    task_switch(DISPATCHER_TASK);
}



/* ============ FIM P3 ============ */

/*
* Description: Retorna um novo id para tarefa de forma sequencial
* Args:
* Return: novo ID para tarefa
*/
int new_task_id () {
    int last = LAST_ID;
    LAST_ID = LAST_ID + 1;
    debug_print("new task id: %d\n", last);
    return last;
}

/*
* Description: Retorna um novo id para tarefa de forma sequencial
* Args:
* Return: novo ID para tarefa
*/
void set_actual_task(task_t *task){
    ACTUAL_TASK = task;
}

/*
* Description: Inicia tarefa principal
* Args:
* Return:
*/
void init_main_task(){
    // Aloca espaço para task principal
    MAIN_TASK = malloc(sizeof(task_t));
    if(!MAIN_TASK){
        perror ("Erro: não foi possível alocar task !") ;
        return;
    }
    // Inicia os valores da task principal 
    getcontext(&(MAIN_TASK->context));
    MAIN_TASK->id = new_task_id();
    MAIN_TASK->status = RUNNING;
    MAIN_TASK->preemptable = 0;
    // Coloca task principal como atual
    set_actual_task(MAIN_TASK);
}

/*Fim funcoes auxiliares*/

/*
* Description: Inicia as estruturas necessarias para o ping pong os
* Args:
* Return:
*/
void ppos_init (){
    debug_print("Iniciando estruturas...\n");
    // Desativa buffer printf
    setvbuf (stdout, 0, _IONBF, 0) ;

    // Inicia task principal
    init_main_task();

    // Cria dispatcher
    DISPATCHER_TASK = malloc(sizeof(task_t));
    if(!DISPATCHER_TASK){
        perror ("Erro: não foi possível alocar task !") ;
        return;
    }
    task_create (DISPATCHER_TASK, dispatcher, NULL) ;
    
    USER_TASKS = NULL;
    debug_print("Finalizou estruturas\n");
}

/*
* Description: Inicia os valores de uma nova tarefa
* Args:
*   task: ponteiro para nova tarefa a ser criada
*   start_routine: funcao executada pela tarefa
*   arg: argumentos de start_routine
* Return: Em caso de erro retorna um valor negativo, caso contrário o ID da tarefa criada
*/
int task_create (task_t *task, void (*start_routine)(void *),  void *arg) {
    debug_print("Criando tarefa...\n");
    
    // Cria contexto
    ucontext_t context;
    char *stack ;

    // Pega contexto atual
    getcontext (&context) ;
    // Aloca pilha
    stack = malloc (STACKSIZE) ;
    // Se conseguiu alocar, inicia campos do contexo criado
    if (stack) {
        context.uc_stack.ss_sp = stack ;
        context.uc_stack.ss_size = STACKSIZE ;
        context.uc_stack.ss_flags = 0 ;
        context.uc_link = 0 ;
    } else {
        perror ("Erro na criação da pilha: ") ;
        return -1;
    }
    // Cria contexto com funcao e argumentos passados
    makecontext (&context, (void*)(*start_routine), 1, arg) ;

    // Preenche os campos da tarefa
    task->context = context;
    task->id = new_task_id();
    task->status = READY;
    task->preemptable = 0;
    task->next = NULL;
    task->prev = NULL;


    int result = queue_append((queue_t **)&USER_TASKS, (queue_t *) task);
    if(result < 0){
        return -1;
    }

    // debug_print("Tarefa adicionada a fila \n");
    // queue_print("Fila: ", (queue_t *) USER_TASKS,  print_task);

    debug_print("Tarefa %d criada\n", task->id);
    return task->id;
}

/*
* Description: Sai da tarefa atual (principal) e inicia a tarefa task
* Args:
*   task: tarefa que ira se tornar a atual e assumir o processador
* Return: 0 quando sucesso, -1 se houver erro
*/
int task_switch (task_t *task) {
    debug_print("Alterando tarefas...\n");
    // TODO: Verificacoes
    debug_print("Contexts: %p %p\n", &(ACTUAL_TASK->context), &(task->context));
    
    // Armazena tarefa atual
    task_t *old_task = ACTUAL_TASK;
    // Faz a tarefa atual apontar para a nova tarefa
    ACTUAL_TASK = task;
    
    // Se a tarefa anterior possui status como executando
    // coloca o status dela como suspensa
    if(old_task->status == RUNNING) {
        old_task->status = SUSPENDED;
    }

    // Altera status da atual tarefa para executando 
    task->status = RUNNING;
    
    // Troca contextos
    swapcontext(&(old_task->context), &(task->context));
    debug_print("Tarefas alteradas\n");
    return 0;
}

/*
* Description: Sai da tarefa atual e inicia a tarefa principal
* Args:
*   exit_code:
* Return:
*/
void task_exit (int exit_code) {
    debug_print("Finalizando tarefa...\n");
    
    // Armazena tarefa atual
    task_t *old_task = ACTUAL_TASK;
    task_t *next_task = (ACTUAL_TASK == DISPATCHER_TASK) ? MAIN_TASK : DISPATCHER_TASK;

    // Coloca status da tarefa a ser finalizada como encerrada
    old_task->status = FINISHED;

    // Altera status da proxima tarefa como rodando
    next_task->status = RUNNING;
    
    // Troca contextos
    task_switch(next_task);
    debug_print("Tarefas finalizadas\n");
}

/*
* Description: Retorna id da tarefa atual
* Args:
* Return: id da tarefa atual ou -1 se houver erro
*/
int task_id () {
    if(ACTUAL_TASK == NULL){
        perror ("Tarefa atual não iniciada !") ;
        return -1;
    }

    return ACTUAL_TASK->id;
}