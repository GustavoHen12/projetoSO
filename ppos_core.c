// GRR20190485 Gustavo Henrique da Silva Barbosa

#include<stdio.h>
#include<stdlib.h>
#include"ppos.h"

task_t *MAIN_TASK;
task_t *ACTUAL_TASK;
int LAST_ID = 0;

int new_task_id () {
    int last = LAST_ID;
    LAST_ID = LAST_ID + 1;
    debug_print("new task id: %d\n", LAST_ID);
    return last;
}

void setActualTask(task_t *task){
    ACTUAL_TASK = task;
}

int task_init(task_t *task, ucontext_t *context) {
    task->context = *context;
    task->id = new_task_id();
    task->status = READY;
    task->preemptable = 0;

    return 0;
}

/*
* Esta função inicializa as estruturas internas do SO. Por enquanto, conterá apenas algumas 
* inicializações de variáveis e a seguinte instrução, que desativa o buffer utilizado pela 
* função printf, para evitar condições de disputa que podem ocorrer nesse buffer ao usar as 
* funções de troca de contexto:
*/
void ppos_init (){
    debug_print("Iniciando estruturas...\n");
    // Desativa buffer printf
    setvbuf (stdout, 0, _IONBF, 0) ;

    // Inicia task principal
    MAIN_TASK = malloc(sizeof(task_t));
    if(!MAIN_TASK){
        perror ("Erro: não foi possível alocar task !") ;
        return;
    }

    getcontext(&(MAIN_TASK->context));
    MAIN_TASK->id = new_task_id();
    MAIN_TASK->status = RUNNING;
    MAIN_TASK->preemptable = 0;

    setActualTask(MAIN_TASK);
    debug_print("Finalizou estruturas\n");
}

/*
* task: estrutura que referencia a tarefa criada
* start_routine: função que será executada pela tarefa
* arg: parâmetro a passar para a tarefa que está sendo criada
* retorno: o ID (>0) da tarefa criada ou um valor negativo, se houver erro
* Atenção: deve ser previsto um descritor de tarefa que aponte para o programa 
* principal (que exercerá a mesma função da variável ContextMain no programa contexts.c).
*/
int task_create (task_t *task, void (*start_routine)(void *),  void *arg) {
    debug_print("Criando tarefa...\n");
    
    // Cria contexto
    ucontext_t context;
    char *stack ;

    getcontext (&context) ;

    stack = malloc (STACKSIZE) ;
    if (stack) {
        context.uc_stack.ss_sp = stack ;
        context.uc_stack.ss_size = STACKSIZE ;
        context.uc_stack.ss_flags = 0 ;
        context.uc_link = 0 ;
    } else {
        perror ("Erro na criação da pilha: ") ;
        return -1;
    }
    makecontext (&context, (void*)(*start_routine), 1, arg) ;

    // Cria tarefa
    task->context = context;
    task->id = new_task_id();
    task->status = READY;
    task->preemptable = 0;

    debug_print("Tarefa %d criada\n", task->id);
    return 0;
}

/*
* task: tarefa que irá assumir o processador
* retorno: valor negativo se houver erro, ou zero
* Esta é a operação básica de troca de contexto, que encapsula a função swapcontext.
* Ela será chamada sempre que for necessária uma troca de contexto.
*/
int task_switch (task_t *task) {
    debug_print("Alterando tarefas...\n");
    // Verificacoes
    debug_print("Contexts: %p %p\n", &(ACTUAL_TASK->context), &(task->context));
    task_t *atual = ACTUAL_TASK;
    ACTUAL_TASK = task;
    atual->status = SUSPENDED;
    ACTUAL_TASK->status = RUNNING;
     
    swapcontext(&(atual->context), &(task->context));
    debug_print("Tarefas alteradas\n");
    return 0;
}

/*
* exit_code : código de término devolvido pela tarefa corrente
* (ignorar este parâmetro por enquanto, pois ele somente será usado mais tarde).
* Quando uma tarefa encerra, o controle deve retornar à tarefa main. Esta chamada 
* será implementada usando task_switch.
*/
void task_exit (int exit_code) {
    task_switch(MAIN_TASK);
}

/*
* retorno: Identificador numérico (ID) da tarefa corrente, que deverá ser 0 para main, 
* ou um valor positivo para as demais tarefas. Esse identificador é único: não devem existir
* duas tarefas com o mesmo ID.
*/
int task_id () {
    //verificacoes
    return ACTUAL_TASK->id;
}

