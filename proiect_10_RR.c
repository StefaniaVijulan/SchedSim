#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define NR_THREADS 3
int count;
int wait_t;
int timp_total;
typedef struct 
  {
    pthread_t id;//id-ul threadului
    
    int status;
    int aux_q;
    int timp_executie;
    int creste;
    int timp_initial;

    sem_t run;//are un semafor propriu
} thread;

typedef struct {
      int quantum;//pt round robin
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
   
start_waiting:

    if( sem_wait(&t->run))
    {
    	   perror(NULL);
    	   return errno;
    }
    
    t->aux_q = 0;
    
    t->creste =0;
    while( t->aux_q < s.quantum && t->timp_executie > 0 )   

              {               
                t->timp_executie--; 
                t->creste++;
                t->aux_q ++;
              }
           
          timp_total = timp_total + t->creste;
          update_scheduler();
         
     if( t->timp_executie > 0) // nu s - terminat procesul
       goto start_waiting;
    
   printf("\n------------------------\n");        
   printf("Arrival Time: %d\n",0);
   printf("Completion Time:%d\n",timp_total );
   printf("Burst Time: %d\n", t->timp_initial);
   printf("Turn Around Time:%d\n",timp_total);
   printf("Waiting Time:%d\n",timp_total- t->timp_initial);
   wait_t = wait_t + timp_total - t->timp_initial; 

}

void init_thread()
{ 
	thread* t;
	t = malloc(sizeof(thread)); 
	t->timp_executie = rand() %10 +1; //sa nu aiba timp de executie 0
	t->status =0;
 	t->timp_initial = t->timp_executie;
  
//cream structura de thread
//ii initializam semaforul, va pleca cu S=0
   
  if(sem_init(&t->run, 0, 0)){   //
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
        	next = s.q[0];   // ordona dupa timp_sosire--->q
		s.running = next;
		start_thread(next);
	  }
	  else
	  {
	    if( current->timp_executie == 0) //procesul  s-a terminat
	        {   
	    	int i = 0;
       
	    	for(i = 0; i < s.size_q-1; i++)
	    		s.q[i] = s.q[i+1];   //il scoatem din coada
	    	

	    	current->status = 0;
      s.size_q--;  
    
	    	if(s.size_q >0) //
	    	{ next = s.q[0];

	    	s.running = next; 

	    	start_thread(next); } 

	    } 
	    else
	    {    //procesul mai are de executat dar s a terminat cuanta
        printf(" threadul cu timpul%d\n", current ->timp_initial);
	    	if( current->aux_q == s.quantum)
	    	{ 
          int i=0;
        
	    	for(i=0;i<s.size_q-1;i++)
	    		s.q[i] = s.q[i+1];

        	s.q[s.size_q-1]= current; // il punem la finalul cozii      
     
	      current->status=0;
	      next = s.q[0];
	     	s.running = next; 
	    	start_thread(next);
	    	}
	    }
	 }
 }
  
//}
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
   
   s.quantum = 3;
   s.size_q = 0;
   s.nr_threads = 0;
   s.running = NULL;

   for (i=0; i<NR_THREADS; i++)
    { 
      init_thread();
      

    }
    update_scheduler();
  
 
  s_end();  
    printf("\n=========================\n");
 
  printf(" Timpul total in care ruleaza procesele este %d\n", timp_total); 
  printf(" Timpul mediu de astptare:%f\n", (double)wait_t / s.nr_threads); 

}
