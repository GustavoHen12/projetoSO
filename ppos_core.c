// GRR20190485 Gustavo Henrique da Silva Barbosa

#include<stdio.h>
#include<stdlib.h>
#include"ppos.h"

task_t *MAIN_TASK;
task_t *ACTUAL_TASK;
int LAST_ID = 0;

/* ============ P2 ============
Todo: revisar e comentar
*/
/*
* Description: Retorna um novo id para tarefa de forma sequencial
* Args:
* Return: novo ID para tarefa
*/
int new_task_id () {
    int last = LAST_ID;
    LAST_ID = LAST_ID + 1;
    debug_print("new task id: %d\n", LAST_ID);
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
    
    // Coloca o status da tarefa atual como suspensa
    // E a da atual tarefa como executando 
    old_task->status = SUSPENDED;
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
    task_switch(MAIN_TASK);
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

/* ============ P3 ============
Todo: 
*/

void task_yield () {

}