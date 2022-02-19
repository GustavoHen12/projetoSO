// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Demonstração das funções POSIX de troca de contexto (ucontext.h).

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define _XOPEN_SOURCE 600	/* para compilar no MacOS */

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

ucontext_t ContextPing, ContextPong, ContextMain ;

/*
Funções de contexto:

@ Recebe um contexto A e B, salva o contexto atual em A e Inicia o contexto B
@ uso: durante a execução de um contexto, salvo o contexto na variavel que corresponde ao se
e coloco o pŕoximo contexto em execução. Durante a execução do contexto Proximo, realizo a 
mesma tarefa, entretanto colocando o contexto atual para rodar e salvando o contexto Proximo
swapcontext (&Atual, &Proximo) ;

@ Pega o contexto atual e salva na variável
getcontext (&Atual) ;

@ Altera os valores internos de um contexto
@ Usado para adicionar uma função em uso no contexto. O 1 sinaliza a quantidade de parâmetros e
após ele é recebido os parâmetros
makecontext (&ContextPing, (void*)(*BodyPing), 1, "    Ping") ;

--
ContextPing.uc_stack.ss_sp = stack ; // Aponta para nova stack
ContextPing.uc_stack.ss_size = STACKSIZE ; // Tamanho da stack
ContextPing.uc_stack.ss_flags = 0 ; // A stack esta desativada
ContextPing.uc_link = 0 ; // The uc_link member is used to determine the context that shall be resumed when the context being modified by makecontext() returns. The application shall ensure that the uc_link member is initialized prior to the call to makecontext().
*/
/*****************************************************/

void BodyPing (void * arg)
{
   int i ;

   printf ("%s: inicio\n", (char *) arg) ;

   for (i=0; i<4; i++)
   {
      printf ("%s: %d\n", (char *) arg, i) ;
      printf("\n ---- swapcontext ping -> pong ---- \n");
      swapcontext (&ContextPing, &ContextPong) ;
   }
   printf ("%s: fim\n", (char *) arg) ;

   swapcontext (&ContextPing, &ContextMain) ;
}

/*****************************************************/

void BodyPong (void * arg)
{
   int i ;

   printf ("%s: inicio\n", (char *) arg) ;

   for (i=0; i<4; i++)
   {
      printf ("%s: %d\n", (char *) arg, i) ;
      printf("\n ---- swapcontext pong -> ping ---- \n");
      swapcontext (&ContextPong, &ContextPing) ;
   }
   printf ("%s: fim\n", (char *) arg) ;

   swapcontext (&ContextPong, &ContextMain) ;
}

/*****************************************************/

int main (int argc, char *argv[])
{
   char *stack ;

   printf ("main: inicio\n") ;

   getcontext (&ContextPing) ;

   stack = malloc (STACKSIZE) ;
   if (stack)
   {
      ContextPing.uc_stack.ss_sp = stack ;
      ContextPing.uc_stack.ss_size = STACKSIZE ;
      ContextPing.uc_stack.ss_flags = 0 ;
      ContextPing.uc_link = 0 ;
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
   }
   printf("Cria ping... \n");
   makecontext (&ContextPing, (void*)(*BodyPing), 1, "    Ping") ;

   getcontext (&ContextPong) ;

   stack = malloc (STACKSIZE) ;
   if (stack)
   {
      ContextPong.uc_stack.ss_sp = stack ;
      ContextPong.uc_stack.ss_size = STACKSIZE ;
      ContextPong.uc_stack.ss_flags = 0 ;
      ContextPong.uc_link = 0 ;
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
   }
   printf("Cria pong... \n");
   makecontext (&ContextPong, (void*)(*BodyPong), 1, "        Pong") ;

   swapcontext (&ContextMain, &ContextPing) ;
   swapcontext (&ContextMain, &ContextPong) ;

   printf ("main: fim\n") ;

   exit (0) ;
}
