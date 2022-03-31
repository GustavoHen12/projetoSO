// GRR20190485 Gustavo Henrique da Silva Barbosa

#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include"ppos.h"
#include"queue.h"

/* ============ VARIAVEIS GLOBAIS ============ */

task_t *MAIN_TASK; // Tarefa principal
task_t *ACTUAL_TASK; // Tarefa em execução no momento
task_t *DISPATCHER_TASK; // Tarefa para o dispatcher
task_t *USER_TASKS; // Fila de tarefas do usuário
task_t *SUSPENDED_TASKS; // Fila de tarefas do usuário

int LAST_ID = 0; // Varável para controle dos ids das tarefas

// Status das tarefas
short READY = 1;
short RUNNING = 2;
short SUSPENDED = 3;
short FINISHED = 4;

// Variaveis para preempção e compartilhamento de tempo
struct sigaction action ; // Estrutura que define um tratador de sinal para interrupcao de tempo
struct itimerval timer; // Estrutura de inicialização to timer
int QUANTUM_INTERVAL = 20; // Número de ticks em um quantum
int QUANTUM_COUNTER = 20; // Contador do quantum

unsigned int CURRENT_TIME = 0; // Variável com tempo atual desde o começo da execução em ms

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
    MAIN_TASK->preemptable = 1;
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


void awake_tasks (task_t **queue) {
    // Percorre a lista
    while(queue_size((queue_t *) *queue) > 0) {
        // Coloca a tarefa na fila de prontos e retira da fila
        if(task_resume(*queue, queue) < 0){
            break;
        }
    }
}

void task_suspend (task_t **queue) {
    // Retira tarefa da fila de prontas
    int result = queue_remove((queue_t **) &USER_TASKS, (queue_t *) ACTUAL_TASK);
    if(result < 0) {
        fprintf(stderr,"Não foi possível remover a tarefa atual da fila fila !\n") ;
        return;
    }

    // Coloca como suspensa
    ACTUAL_TASK->status = SUSPENDED;

    // Adiciona na fila queue
    queue_append((queue_t **) queue, (queue_t *) ACTUAL_TASK);

    // Coloca a tarefa do Dispather como corrente
    DISPATCHER_TASK->status = RUNNING;

    // Volta para o dispatcher
    task_switch(DISPATCHER_TASK);

}

int task_resume (task_t * task, task_t **queue) {
    // Retira tarefa da fila queue
    int result = queue_remove((queue_t **) queue, (queue_t *) task);
    if(result < 0) {
        fprintf(stderr,"Não foi possível remover a tarefa da fila fila !\n") ;
        return -1;
    }

    // Coloca como pronta
    ACTUAL_TASK->status = READY;

    // Adiciona na fila queue
    queue_append((queue_t **) &USER_TASKS, (queue_t *) task);

    return 0;
}


/* ============ FUNCOES PRINCIPAIS ============ */

int task_join (task_t *task) {
    if(queue_find((queue_t**) &USER_TASKS, (queue_t *) task)){
        // Suspende tarefa
        task_suspend(&(task->tasks_waiting));

        // Retorna o exit code
        return (task->execinfo).exit_code;
    }

    return -3;
}

/*
* Description: Trata as interrupções de tempo
*   Incrementa varável que armazena tempo atual
*   Se for um fim de quantum e não for uma tarefa do sistema gera um task_yeld
* Args:
* Return:
*/
void timer_interruption_handler (int signum) {
    CURRENT_TIME++;
    if((!ACTUAL_TASK->system_task) && (--QUANTUM_COUNTER <= 0)) {
        QUANTUM_COUNTER = 20;
        task_yield();
    }
}

/*
* Description: Cria as estruturas para tratar interrupção e inicia temporizador
* Args:
* Return:
*/
int init_time_interruption (int interval_usec) {
    // Inicia ação que será gerada para tratar a interrupção
    action.sa_handler = timer_interruption_handler ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0) {
        fprintf(stderr,"Erro: não foi possível inciar sigaction!\n") ;
        return 0;
    }

    // Configura valor do temporizador
    timer.it_value.tv_usec = interval_usec;
    timer.it_value.tv_sec  = 0;
    timer.it_interval.tv_usec = interval_usec;
    timer.it_interval.tv_sec  = 0;

    // Arma temporizador
    if (setitimer (ITIMER_REAL, &timer, 0) < 0) {
        fprintf(stderr,"Erro: não foi possível armar temporizador setitimer!\n") ;
        return 0;
    }

    return 1;
}

/*
* Description: Retorna o intervalo em ms entre o inicio da execução e agora
* Args:
* Return:
*/
unsigned int systime () {
    return CURRENT_TIME;
}

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
               awake_tasks(&(next_task->tasks_waiting));
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
    DISPATCHER_TASK->system_task = 1;
    
    // Zera fila de execucao e adiciona tarefa principal
    USER_TASKS = NULL;
    
    int result = queue_append((queue_t **)&USER_TASKS, (queue_t *) MAIN_TASK);
    if(result < 0){
        fprintf(stderr,"Não foi possível adicionar a tarefa na fila !\n") ;
        return;
    }

    // Inicia interrupção de tempo
    if(!init_time_interruption(1000)){
        fprintf(stderr,"Erro: não foi possível iniciar as interrupções por tempo !\n") ;
        return;
    }

    // Passa controle para o dispatcher
    task_yield();

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
    task->preemptable = 1;
    task->next = NULL;
    task->prev = NULL;
    task->system_task = 0;

    // Inicia campos da estrutura de tempo
    task->execinfo.creation_time = systime();
    task->execinfo.processor_time = 0;
    task->execinfo.activations = 0;

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

    // Contabiliza o tempo que a tarefa antiga ficou em execucao
    old_task->execinfo.processor_time += (systime() - old_task->execinfo.start_last_run);

    // Altera variavel com o tempo incial da execução da tarefa atual
    task->execinfo.start_last_run = systime();

    // Incrementa o número de ativações da tarefa
    task->execinfo.activations++;

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
    
    // Seta o momento final de execução e imprime dados de execução
    old_task->execinfo.kill_time = systime();
    unsigned int execution_time = old_task->execinfo.kill_time - old_task->execinfo.creation_time;
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", old_task->id, execution_time, old_task->execinfo.processor_time, old_task->execinfo.activations);
    
    // Salva o exit_code
    (old_task->execinfo).exit_code = exit_code;

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