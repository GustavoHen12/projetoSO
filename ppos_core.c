// GRR20190485 Gustavo Henrique da Silva Barbosa

#include<stdio.h>
#include"ppos.h"

/*
* Esta função inicializa as estruturas internas do SO. Por enquanto, conterá apenas algumas 
* inicializações de variáveis e a seguinte instrução, que desativa o buffer utilizado pela 
* função printf, para evitar condições de disputa que podem ocorrer nesse buffer ao usar as 
* funções de troca de contexto:
*/
void ppos_init (){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
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
    return 0;
}

/*
* task: tarefa que irá assumir o processador
* retorno: valor negativo se houver erro, ou zero
* Esta é a operação básica de troca de contexto, que encapsula a função swapcontext.
* Ela será chamada sempre que for necessária uma troca de contexto.
*/
int task_switch (task_t *task) {
    return 0;
}

/*
* exit_code : código de término devolvido pela tarefa corrente
* (ignorar este parâmetro por enquanto, pois ele somente será usado mais tarde).
* Quando uma tarefa encerra, o controle deve retornar à tarefa main. Esta chamada 
* será implementada usando task_switch.
*/
void task_exit (int exit_code) {

}

/*
* retorno: Identificador numérico (ID) da tarefa corrente, que deverá ser 0 para main, 
* ou um valor positivo para as demais tarefas. Esse identificador é único: não devem existir
* duas tarefas com o mesmo ID.
*/
int task_id () {
    return 0;
}