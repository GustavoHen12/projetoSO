// GRR20190485 Gustavo Henrique da Silva Barbosa

#include<stdio.h>
#include"queue.h"

int queue_size (queue_t *queue) {
    // Se a fila não existe retorna tamanho 0
    if(queue == NULL){
        return 0;
    }
    
    // Inicial é o primeiro elemento
    // Vai iterando sobre atual, até ele ser igual ao inicial
    // Retorna o tamanho
    queue_t *inicial = queue;
    queue_t *atual = queue->next;
    int tam = 1;
    while(atual != NULL && atual != inicial) {
        tam++;
        atual = atual->next;
    }

    return tam;
}


void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    // Imprime a primeira parte
    printf("%s [", name);

    // Se a fila não existe retorna
    if(queue == NULL){
        printf("]\n");
        return;
    }

    // Itera sobre a fila imprimindo os elementos
    queue_t *inicial = NULL;
    queue_t *atual = queue;
    while(atual != NULL && atual != inicial) {
        // Se for o primeiro elemento, inicia variavel inicial
        // Caso contrario imprime o espaço entre os elementos
        if(inicial == NULL){
            inicial = atual;
        } else {    
           printf(" ");
        }
        
        print_elem(atual);
        
        atual = atual->next;
    }
    printf("]\n");
}


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

    // Se o primeiro elemento for NULL a fila está vazia
    // Adiciona elemento e retorna
    if(primeiro == NULL){
        *queue = elem;

        elem->next = elem;
        elem->prev = elem;
        
        return 0;
    }

    // Recebe ultimo elemento da fila
    queue_t *ultimo = (*queue)->prev;

    // Reoganiza os ponteiros
    // Novo elemento irá após o último
    primeiro->prev = elem;
    ultimo->next = elem;
    elem->prev = ultimo;
    elem->next = primeiro;

    return 0;
}


int queue_remove (queue_t **queue, queue_t *elem) {
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

    // Itera sobre a fila até o final dela ou encontrar o elemento a ser removido
    queue_t *inicial = NULL;
    queue_t *atual = *queue;
    while(atual != NULL && atual != inicial && atual != elem) {
        if(inicial == NULL){
            inicial = atual;
        }
        atual = atual->next;
    }
    // Verifica se chegou até o final da fila sem encontrar o elemento
    if(atual == inicial){
        fprintf(stderr, "O elemento a ser removido não existe nesta fila !\n");
        return -1;
    }

    // Se existe apenas um elemento na fila e este deve ser removido
    if(atual->next == atual && atual->prev == atual){
        // Faz fila apontar para NULL e organiza ponteiros do elemento removido
        *queue = NULL;
        atual->next = NULL;
        atual->prev = NULL;

        return 0;   
    }

    // Se o elemento removido for o primeiro da fila
    // a fila deve passar a apontar para o segundo
    if(*queue == atual){
        *queue = atual->next;
    }

    // Reorganizza os ponteiros da fila
    atual->prev->next = atual->next;
    atual->next->prev = atual->prev;
    
    // Remove elemento
    atual->next = NULL;
    atual->prev = NULL;

    return 0;   
}

int queue_find (queue_t **queue, queue_t *elem) {
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

    // Itera sobre a fila até o final dela ou encontrar o elemento a ser removido
    queue_t *inicial = NULL;
    queue_t *atual = *queue;
    while(atual != NULL && atual != inicial && atual != elem) {
        if(inicial == NULL){
            inicial = atual;
        }
        atual = atual->next;
    }

    // Verifica se chegou até o final da fila sem encontrar o elemento
    if(atual == inicial){
        return 0;
    }

    return 1;   
}