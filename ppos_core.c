// GRR20190485 Gustavo Henrique da Silva Barbosa

#include<stdio.h>
#include<stdlib.h>
#include"ppos.h"
#include"queue.h"

/* ============ VARIAVEIS GLOBAIS ============ */

task_t *MAIN_TASK; // Tarefa principal
task_t *ACTUAL_TASK; // Tarefa em execução no momento
task_t *DISPATCHER_TASK; // Tarefa para o dispatcher
task_t *USER_TASKS; // Fila de tarefas do usuário

int LAST_ID = 0; // Varável para controle dos ids das tarefas

// Status das tarefas
short READY = 1;
short RUNNING = 2;
short SUSPENDED = 3;
short FINISHED = 4;

/* ============ FUNCOES AUXILIARES ============ */

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
* Description: Inicia tarefa principal
* Args:
* Return:
*/
void init_main_task(){
    // Aloca espaço para task principal
    MAIN_TASK = malloc(sizeof(task_t));
    if(!MAIN_TASK){
        fprintf(stderr,"Erro: não foi possível alocar task !") ;
        return;
    }
    // Inicia os valores da task principal 
    getcontext(&(MAIN_TASK->context));
    MAIN_TASK->id = new_task_id();
    MAIN_TASK->status = RUNNING;
    MAIN_TASK->preemptable = 0;
    // Coloca task principal como atual
    ACTUAL_TASK = MAIN_TASK;
}

/*
* Description: Finaliza uma tarefa que já foi executada
* Args: 
*   task: Tarefa a ser finalizada
* Return:
*/
void end_task (task_t *task){
    // Remove task da fila
    queue_remove((queue_t **) &USER_TASKS, (queue_t *) task);
    
    // Libera dados da task
    free(task->context.uc_stack.ss_sp);
    task->context.uc_stack.ss_size = 0;
    task->context.uc_stack.ss_flags = 0;
    task->id = -1;
}

/* ============ FUNCOES ============ */

/*
* Description: Altera a prioridade estática da tarefa e inicia a prioridade dinâmica 
*   com a mesma prioridade. Se a tarefa não existir ou se o valor de prioridade 
*   estiver fora do limite retorna sem alterar a prioridadeinicia a prioridade dinâmica com a mesma prioridade.
* Args:
* Return:
*/
void task_setprio (task_t *task, int prio) {
    if(prio < -20 || prio > 20){
        return;
    }

    // Verifica qual sera a tarefa alterada
    task_t * target_task = task == NULL ? ACTUAL_TASK : task;
    
    // Altera a prioridade
    target_task->static_prio = prio;
    target_task->dynamic_prio = prio;
}

/*
* Description: Verifica prioridade estatica da tarefa
* Args:
* Return: Se tak for null, prioridade da tarefa atual, caso contrário prioridade da tarefa
*/
int task_getprio (task_t *task) {
    return task == NULL ? ACTUAL_TASK->static_prio : task->static_prio;
}

/*
* Description: Busca pela tarefa com maior prioridade dinamica. Caso exista mais de uma com
* a mesma prioridade, a primeira da fila será a escolhida
* Args:
* Return: Retorna a tarefa
*/
task_t *get_priority_task(task_t *queue){
    task_t* temp = queue; // Elemento a ser comparado
    task_t* priority = queue; // Elemento com maior prioridade
    int size = queue_size((queue_t*) queue);
    
    for(int i = 0; i < size; i++){
        // Verifica se o valor da prioridade do elemento atual é estritamente
        // menor que a do com maior prioridade
        if(priority->dynamic_prio > temp->dynamic_prio) {
            priority = temp;
        }

        temp = temp->next;
    }

    return priority;
}

/*
* Description: Função responsável por determinar qual a próxima tarefa que será executada
* Args:
* Return: Tarefa prioritário (com base na prioridade dinâmica) da lista
*/
task_t *scheduler () {
    if(!USER_TASKS){
        return NULL;
    }

    // Busca a tarefa prioritária da fila para execução
    task_t *next = get_priority_task(USER_TASKS);

    if(next != NULL){

        // Percorre a fila de tarefas envelhecendo elas
        task_t* temp = USER_TASKS;
        int size = queue_size((queue_t*) USER_TASKS);
        for(int i = 0; i < size; i++){
            // A única que não sofre de envelhecimento é a tarefa que será executada
            if(temp != next){
                temp->dynamic_prio = temp->dynamic_prio -1;
            }
            temp = temp->next;
        }

        // Reseta prioridade dinamica da tarefa a ser executada
        next->dynamic_prio = task_getprio(next);
    }

    // Retorna próxima tarefa
    return next;
}

/*
* Description: Função executada na tarefa do dispatcher, escolhe as tarefas que serão executas
*   após a execução retorna para main
* Args:
* Return:
*/
void dispatcher () {
    debug_print("Inicio dispatcher...\n");

   // Enquanto houver tarefas a serem executadas
   while(queue_size((queue_t *) USER_TASKS) > 0){
       // Escolhe a próxima tarefa a executar
       task_t *next_task = scheduler();

       // Se houver próxima tarefa
       if (next_task != NULL) {
           // Coloca em execução
           debug_print("Próxima tarefa: %d\n", next_task->id);
           task_switch(next_task);

           // Verifica status de saida
           if(next_task->status == FINISHED){
               end_task(next_task);
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
        fprintf(stderr,"Erro: não foi possível alocar task !") ;
        return;
    }
    task_create (DISPATCHER_TASK, dispatcher, NULL) ;
    
    // Zera fila de execucao
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
        fprintf(stderr,"Erro na criação da pilha !\n") ;
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

    // Adiciona a tarefa na fila de tarefa
    int result = queue_append((queue_t **)&USER_TASKS, (queue_t *) task);
    if(result < 0){
        fprintf(stderr,"Não foi possível adicionar a tarefa na fila !\n") ;
        return -1;
    }

    // Seta a prioridade da tarefa
    task_setprio(task, 0);

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

    if(task == NULL){
        fprintf(stderr, "A fila de tarefas ainda não existe !\n");
        return -1;
    }

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
        fprintf(stderr,"Tarefa atual não iniciada ! \n") ;
        return -1;
    }

    return ACTUAL_TASK->id;
}