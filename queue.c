#include"queue.h"
#include"stdio.h"

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila
int queue_size (queue_t *queue) {
    if(queue == NULL){
        return 0;
    }
    
    queue_t *inicial = queue;
    queue_t *atual = queue->next;

    int tam = 1;
    while(atual != NULL && atual != inicial) {
        tam++;
        atual = atual->next;
    }


    return tam;
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir
void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    printf("%s", name);
    
    if(queue == NULL){
        return;
    }

    queue_t *inicial = NULL;
    queue_t *atual = queue;
    while(atual != NULL && atual != inicial) {
        print_elem(atual);
        if(inicial == NULL){
            inicial = atual;
        }
        atual = atual->next;
    }
}

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
// Retorno: 0 se sucesso, <0 se ocorreu algum erro
int queue_append (queue_t **queue, queue_t *elem) {
    if(queue == NULL) {
        fprintf(stderr, "A fila ainda não existe !");
        return -1;
    }

    if(elem == NULL){
        fprintf(stderr, "O elemento a ser adicionado não existe !");
        return -1;
    }

    if(elem->next != NULL || elem->prev != NULL){
        fprintf(stderr, "O elemento não deve pertencer a outra fila !");
        return -1;
    }

    queue_t *primeiro = *queue;
    // Fila vazia
    if(primeiro == NULL){
        printf("Adiciona primeiro %p\n", &elem);
        *queue = elem;

        elem->next = elem;
        elem->prev = elem;
        
        return 0;
    }

    queue_t *ultimo = (*queue)->prev;

    // Reoganiza os ponteiros
    primeiro->prev = elem;
    ultimo->next = elem;
    elem->prev = ultimo;
    elem->next = primeiro;

    return 0;
}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_remove (queue_t **queue, queue_t *elem) {
    printf("removendo...\n");
    if(queue == NULL) {
        fprintf(stderr, "A fila ainda não existe !");
        return -1;
    }

    if(queue_size(*queue) <= 0){
        fprintf(stderr, "A fila não pode estar vazia !");
        return -1;   
    }

    if(elem == NULL){
        fprintf(stderr, "O elemento a ser removido não existe !");
        return -1;
    }

    queue_t *inicial = NULL;
    queue_t *atual = *queue;
    while(atual != NULL && atual != inicial && atual != elem) {
        if(inicial == NULL){
            inicial = atual;
        }
        atual = atual->next;
    }

    if(atual == inicial){
        fprintf(stderr, "O elemento a ser removido não existe nesta fila !\n");
        return -1;
    }

    if(atual->next == atual && atual->prev == atual){
        *queue = NULL;
        atual->next = NULL;
        atual->prev = NULL;

        return 0;   
    }
    *queue = atual->next;
    atual->prev->next = atual->next;
    atual->next->prev = atual->prev;

    atual->next = NULL;
    atual->prev = NULL;

    return 0;   
}