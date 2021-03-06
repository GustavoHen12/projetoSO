// GRR20190485 Gustavo Henrique da Silva Barbosa

// From:
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

// Estrutura que aramzena informações a respeito
// da execução da tarefa
typedef struct execinfo_t
{
  int activations; // Número de ativiações

  unsigned int processor_time; // Tempo total que ficou em execução
  unsigned int start_last_run; // Momento (ms) que iniciou a execução na última vez

  unsigned int creation_time; // Momento (ms) que a tarefa foi criada
  unsigned int kill_time; // Momento (ms) que a tarefa finalizou a execução

  int exit_code; // Código de saida
} execinfo_t ;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  short preemptable ;			// pode ser preemptada?
  int static_prio, dynamic_prio;		// prioridade estática e dinamica da tarefa (-20 a +20) 
  int system_task;		// Flag para identificar se tarefa do sistema ou do usuário
  execinfo_t execinfo;		// Informações da execução da tarefa
  struct task_t *tasks_waiting; // Fila com as tarefas que estão esperando a task ser concluida
  int awake_time; // Tempo que a tarefa deve dormir
} task_t ;

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

// Macro para print quando executado em modo debug
#ifdef DEBUG
#define debug_print(...) do{ printf( __VA_ARGS__ ); } while(0)
#else
#define debug_print(...) do{ } while (0)
#endif


// estrutura que define um semáforo
typedef struct
{
  int counter; // Contador do semáforo
  task_t *queue; // Fila de tarefas do semaforo

  int lock; // Para evitar condicoes de disputa
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // semaforos do buffer
  semaphore_t s_buffer, s_cons, s_prod;

  // informacoes da fila
  int max_msgs;
  int msg_size;
  int quant_msg;

  // buffer circular
  void *buff; // buffer de mensagens
  int start, end;
} mqueue_t ;

#endif
