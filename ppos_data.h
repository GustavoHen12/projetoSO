// GRR20190485 Gustavo Henrique da Silva Barbosa

// From:
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

//
typedef struct execinfo_t
{
  // unsigned int execution_time; 
  int activations;

  unsigned int processor_time;
  unsigned int start_last_run;

  unsigned int creation_time;
  unsigned int kill_time;
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
  execinfo_t execinfo;
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
  // preencher quando necessário
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
  // preencher quando necessário
} mqueue_t ;

#endif
