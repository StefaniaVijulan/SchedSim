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
int finish;

typedef struct 
  {
    pthread_t id;//id-ul threadului
    int timp_sosire;
    int status;//0 sau 1 sau 2 dupa caz
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

void insertie(thread * t)
{ 
  int i, j;
  i=0;
  while ( i< s.size_q && s.q[i]->timp_executie > t->timp_executie)
      i++;
  
  for ( j = s.size_q; j>i; j--)
     s.q[j] = s.q [j-1];
   s.size_q++;
   t->status =2;// este introdus in coada
   s.q[i]=t; 
}

void update_scheduler() 
{
 thread*  next;
 thread* current;
 
    current = s.running;

    if (current == NULL) {
         
       int timp_minim= 11, i;
       for(i=0; i< s.nr_threads;i++)
        if( s.threads[i]->timp_sosire < timp_minim)
            timp_minim = s.threads[i]->timp_sosire;

       for(i=0; i< s.nr_threads;i++)

        if( s.threads[i]->timp_sosire == timp_minim)
          insertie ( s.threads[i]);
  
        next = s.q[--s.size_q ];   // ordona dupa timp_sosire--->q

        idle_time = next ->timp_sosire; 
        s.running = next;

        start_thread(next);
           
    }
    else
   { 
      
       int i ;
       current->status=1; // procesul este terminat
       finish ++;
       for(i=0; i<s.nr_threads;i++)
         if( s.threads[i]->status == 0 && s.threads[i]->timp_sosire <= timp_total+idle_time)
             insertie(s.threads[i]);

        int m= 100;
        thread *aux;
        printf("finish %d \n", finish);
         if(finish < s.nr_threads && s.size_q ==0)  // 6 .....10
            {for( i=0;i< s.nr_threads;i++)
              if( s.threads[i]->status ==0 && s.threads[i]->timp_sosire < m) // timpul in care sta procesorul degeaba
                { m = s.threads[i]->timp_sosire;
                  aux= s.threads[i]; }
               insertie(aux);
    
             }

      if(s.size_q >0)
        {
       next = s.q[--s.size_q ];  
       if( next->timp_sosire - timp_total >0)  
              idle_time += next->timp_sosire - timp_total;
       s.running = next; 
      start_thread(next);}
      
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
      //printf("%d\n",i);

    }
    update_scheduler(); 
 
  s_end();   
  printf("\n=========================\n");
  printf(" In timpul rularii, idle time-ul este %d\n", idle_time);
  printf(" Timpul total in care ruleaza procesele este %d\n", timp_total); 
  printf(" Timpul mediu de astptare:%f\n", (double)wait_t / s.nr_threads);

}
