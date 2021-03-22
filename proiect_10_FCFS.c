#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#define NR_THREADS 5
int timp_total;
int idle_time;
int wait_t;
typedef struct 
  {
    pthread_t id;//id-ul threadului
    int timp_sosire;
    int status;//0 sau 1 dupa caz
    int timp_executie;

    int timp_initial;
    sem_t run;//are un semafor propriu
} thread;

typedef struct {
      
      int nr_threads;
      int size_q;
      thread* q[101] ;//coada de prioritati
      thread* threads[101];//vector de threaduri
      thread* running;//threadul care ruleaza in acel moment

      //sem_t end;//semafor
} scheduler;
scheduler s;

void *thread_routine(void *args)
{
    
    thread *t;

    t = (thread *)args;
    if( sem_wait(&t->run)) // se blocheaza thread-ul si va astepta sa fie deblocat
    {
         perror(NULL);
         return errno;
    }
    while( t->timp_executie > 0 )   

              {               
                t->timp_executie--; 
               }
    timp_total = timp_total + t->timp_initial;
    printf("\n------------------------\n");        
   printf("Arrival Time: %d\n",t->timp_sosire);
   printf("Completion Time:%d\n",timp_total +idle_time);
   printf("Burst Time: %d\n", t->timp_initial);
   printf("Turn Around Time:%d\n",timp_total +idle_time - t->timp_sosire);
   printf("Waiting Time:%d\n",timp_total +idle_time - t->timp_sosire - t->timp_initial);
   wait_t = wait_t + timp_total +idle_time - t->timp_sosire - t->timp_initial;
   //printf( "Timpurile: sosire:%d....initial:%d....dupa algoritm:%d \n",t->timp_sosire,t->timp_initial,t->timp_executie);
   update_scheduler();



}
void init_thread()
{ 
  thread* t;
  t = malloc(sizeof(thread)); 

  t->timp_sosire = rand() %10;
  t->timp_executie = rand() %10 +1; //sa nu aiba timp de executie 0
  t->status =0;
  t->timp_initial = t->timp_executie;
//cream structura de thread
//ii initializam semaforul, va pleca cu S=0
   
  if(sem_init(&t->run, 0, 0)){  
       perror(NULL);
       return errno;
  }
  

  s.threads[s.nr_threads++]=t; 
  s.q[s.size_q++]=t;
  
  if( pthread_create(& (t->id), NULL, thread_routine, (void *)t)){  
     perror(NULL);
       return errno;
  }
}

void start_thread(thread *t)
{

  if(sem_post(&t->run)){
    perror(NULL);
    return errno;
  }
  t->status = 1;
  
}

void update_scheduler() 
{
 thread*  next;
 thread* current;
 
    current = s.running;

    if (current == NULL) {

         int i,j;
         thread* aux;
         for(i = 0; i<s.size_q-1; i++)
          for(j=i+1; j< s.size_q; j++)
           if( s.q[i]->timp_sosire < s.q[j]->timp_sosire) //ordonam desc pt a putea lua de la final timpul cel mai mic
            {  aux = s.q[i];
              s.q[i] = s.q[j];
              s.q[j] = aux;
            }  
        
        next = s.q[--s.size_q ];   // luam th cu cel mai mic timp
        s.running = next;

        idle_time = next->timp_sosire; // procesorul sta degeaba pana cand incepe primul th
        start_thread(next);
      
      
    }
    else
   { 
        int i ;
       current->status=1; // procesul este terminat
      if(s.size_q >0)
        {
       next = s.q[--s.size_q ];
       if ( next->timp_sosire > timp_total +idle_time) 
         idle_time += next->timp_sosire - timp_total-idle_time;
          
      s.running = next; 
      start_thread(next);
    }
      
      }
 }

void destroy_thread(thread *t)
{
  
  if(sem_destroy(&t->run)){  //distrugem semafor    
         perror(NULL);
         return errno;
        }


  free(t);  //eliberam memoria threadului
}
void s_end( )
{
  
  int i;
  for (i = 0; i < s.nr_threads; i++) {  //asteptam threadurile
    if( pthread_join(s.threads[i]->id, NULL))
    {
      perror(NULL);
      return errno;
    }
    
  }

  for (i = 0; i < s.nr_threads; i++)
    destroy_thread(s.threads[i]);
}


int main()
{  int i;
   
   s.size_q = 0;
   s.nr_threads = 0;
   s.running = NULL;

   for (i=0;i<NR_THREADS;i++)
    { 
      init_thread();
    }
    update_scheduler(); 
 
  s_end(); 
  printf("\n=========================\n");
  printf(" In timpul rularii, idle time-ul este %d\n", idle_time);
  printf(" Timpul total in care ruleaza procesele este %d\n", timp_total); 
  printf(" Timpul mediu de astptare:%f\n", (double)wait_t / s.nr_threads);

}
