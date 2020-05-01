/***********************  server.c  *******************************
* Programmer:   Geoffrey Perez
*
* Course:   CSCI 4354
*
* Date:   April 28, 2019
*
* Assignment:  Program #4
*
* Environment:  Windows 10 using Putty terminal
*
* Purpose:  Asks for number of clients, a time quantum, number of frames and frame size
*           and checks to see if memory is available before putting client into ready queue
*           or else the job is put into hold until memory is available. The server then performs round-robin
*           scheduling and calculates and reports back the completion time, frames required, frames assigned 
*           and fragmentation until the hold and ready queues are empty.
*           
*                   
*
* Input:   Number of clients, time quantum, frame size, frame number
*
* Output:  No output (output reported on client side)
*           
*
* Time :
*        Estimated:  Design     Coding     Testing
*                      3          3          5
*
*        Actual:     Design     Coding     Testing
*                      5          3          15       
*
* Algorithm:
*       prompt for & get number of clients, TQ, frame size & framecount
*       create commonFIFO
*       while i<0 to number of clients
*          read from commonFIFO
*          create privateFIFO
*          create PCB
*          put data in PCB and enqueue into Hold
*     do 
*       {
*        Use checkMemoryRequirements to dequeue and check head of Hold
*          if memoryRequired is available
*             enqueue into Ready & reduce available frames by amount requested
*          if memory request > total memory
*             send error code back to client
*          if memory not currently available
*             enqueue back into Hold
*          return available frames to main function
*        Dequeue from ready and place into running PCB
*          for index = time quantum amount
*            {
*             decrement burst time and increment clock 
*               if burst time is = 0
*                {
*                write fragmentation, frames assigned, error code and completion time back to client
*                increase available frames by number of frames assigned after process is done running
*                close privateFIFO
*                unlink privateFIFO
*                }
*               else enqueue back into ready
*            }
*       } while ready or hold queues are not empty
*       
*     
*
********************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

typedef struct pcb {  /*structure for PCBs */

        char str[20];
        int burst;
	    int memRequest;
} PCB;

typedef struct node{  /*Nodes stored in the linked list*/

        struct pcb elements;
        struct node *next;

} Node;

typedef struct queue{ /*A struct facilitates passing a queue as an argument*/

        Node *head;       /*Pointer to the first node holding the queue's data*/
        Node *tail;       /*Pointer to the last node holding the queue's data*/
        int sz;           /*Number of nodes in the queue*/

} Queue;



int size( Queue *Q )
{
        return Q->sz;
}


int isEmpty( Queue *Q )
{
        if( Q->sz == 0 ) return 1;
        return 0;
}

void enqueue( Queue *Q, struct pcb elements){

        Node *v = (Node*)malloc(sizeof(Node));/*Allocate memory for the Node*/
		
        if( !v )
		{		
                printf("ERROR: Insufficient memory\n");
                return;
        }

        v->elements = elements;

        v->next = NULL;

        if( isEmpty(Q) ) Q->head = v;

        else Q->tail->next = v;

        Q->tail = v;

       Q->sz++;

}


PCB dequeue( Queue *Q ){

        PCB temp;
        Node *oldHead;

        if( isEmpty(Q) )
		{
                printf("ERROR: Queue is empty\n");
                return temp;

        }

        oldHead = Q->head;
        temp = Q->head->elements;
        Q->head = Q->head->next;
        free(oldHead);
        Q->sz--;

        return temp;

}



PCB first( Queue *Q ){

        PCB temp;
        if( isEmpty(Q) ){

                return temp;

        }
        temp = Q->head->elements;
        return temp;

}

void destroyQueue( Queue *Q ){

        while( !isEmpty(Q) ) dequeue(Q);

}


/*A different visit function must be used for each different datatype.*/

/*The name of the appropriate visit function is passed as an argument */

/*to traverseQueue.                                                   */

void visitNode( PCB elements ){

        printf("PCB name: %s\n",elements.str);
        printf("Burst time: %d\n",elements.burst);
        printf("\n");

}


void traverseQueue( Queue *Q ){

        Node *current = Q->head;

        while( current ){

                visitNode(current->elements);

                current = current->next;

        }

        printf("\n");

}

//peak at head of queue
PCB peak(Queue *Q) {
  PCB tmp;
  tmp = Q->head->elements;
  return tmp;
}


//checks and dequeues head of hold queue, if memRequirements are satisfied it gets put
//into ready and runs, otherwise it is enqueued back into hold
int checkMemoryRequirements(Queue *Ready, Queue *Hold, int availableFrames, int numFrames, int frameSize) {
	//struct to client
	struct output {  
	int clockTime;
	int fragmentation;
    int frameNumbers;
	int error;
	}  output;
	
	int i;
	int framesRequired;
	int metMemReq;
	int numInHold = size(Hold);
	int privateFIFO;
	int fragmentation;
	
	PCB checkRequest;
	//check all jobs in hold to see if they can fit in memory
	for(i = 0;i < numInHold;i++)
	{
	  checkRequest = dequeue(Hold);
	  framesRequired = checkRequest.memRequest/frameSize;
	  if (checkRequest.memRequest%frameSize != 0)
	  {
	    fragmentation = checkRequest.memRequest%frameSize;
	  }
	  else
	  {
		fragmentation = 0;
	  }
	  if (fragmentation != 0)
	  {
		  framesRequired++;
	  }
	  //printf("memory requested %d" , checkRequest.memRequest);
	  //printf("Frames required: %d ", framesRequired);
	  //printf("Fragmentation %d ", fragmentation);
	  
	  if(framesRequired > numFrames)
	  {
	    //report back that the job size is too big 
		output.error = 3;
		
	    if ((privateFIFO=open(checkRequest.str, O_WRONLY))<0)
          printf("cant open private FIFO to write");
	  
        write(privateFIFO, &output, sizeof(output));
		continue;
	  }
	  //if it's not greater than available or greater than total 
	  //we can enqueue to ready
	  else if (framesRequired <= availableFrames)
	  {
		availableFrames -= framesRequired;
	    enqueue(Ready,checkRequest);
		//printf("Available frames: %d ", availableFrames);
	  }
	  
	  //check if the framesRequired are only greater than available
	  else
	  {
		enqueue(Hold,checkRequest);
	  }
	}
	return availableFrames;
}
  

main (void) {

  //struct from client
  struct clientInfo {
    char privInfo[20];  // private FIFO name
    int burst;
	int memRequest;
  } input;

 //struct to client
  struct output {  
    int clockTime;
	int fragmentation;
	int frameNumbers;
	int error;
  }  output;


  int commonFIFO;  // common FIFO to read from
  int privateFIFO;  // private FIFO to write to
  int i;
  int TQ;
  int numClients;
  int clock;
  int frameSize;
  int availableFrames;
  int fragmentation;
  int error=0;
  int numFrames;
  int metMemReq;
  int numInHold;
  int TQcount; 
  
 //ready queue
  Queue Ready;
  Ready.head = NULL;
  Ready.tail = NULL;
  Ready.sz = 0;
 //wait queue 
  Queue Hold;
  Hold.head = NULL;
  Hold.tail = NULL;
  Hold.sz = 0;

  printf("Server: Please enter number of clients: ");
  scanf("%d", &numClients);
  printf("Server: Please enter a time quantum: ");
  scanf("%d", &TQ);
  printf("Server: Enter number of frames:  ");
  scanf("%d", &numFrames);
  availableFrames=numFrames;
  printf("Server: Enter frame size:  ");
  scanf("%d", &frameSize);
  

  if ((mkfifo("commonFIFO", 0666)<0 && errno != EEXIST)) {
    perror("cant create Common FIFO to Client");
    exit(-1);
  }

  if((commonFIFO=open("commonFIFO", O_RDONLY))<0)
    printf("can't open fifo for reading");


  i=0;
 
  do {

    read(commonFIFO, &input, sizeof(input));

    if ((mkfifo(input.privInfo, 0666)<0 && errno != EEXIST)) {
      perror("cant create private FIFO");
      exit(-1);
    }
		
    PCB data;
    sprintf(data.str, input.privInfo);
    data.burst = input.burst;
	data.memRequest= input.memRequest;
    enqueue(&Hold,data);
    i+=1;
 
  } while (i<numClients);


  close(commonFIFO);
  unlink("commonFIFO");

  clock = 0;
  PCB running;
  PCB checkRequest;
	
  do 
  {	
	//check if any process fits in memory
	availableFrames = checkMemoryRequirements(&Ready, &Hold, availableFrames, numFrames, frameSize);
	running = dequeue(&Ready);
	for(TQcount = 0;TQcount < TQ; TQcount++)
	{
	  //increment clock
	  clock += 1;
	  //decrement burst time
	  running.burst -= 1;
	  //check if lead process is completed
	  if(running.burst == 0)
	  {
	    //completed so dequeue and write clock and framing info to client
		//send PCB to function and write to client
		output.clockTime = clock;
		int framesRequired = running.memRequest/frameSize;
		//printf("\nframes required: %d ", framesRequired);
		if (running.memRequest%frameSize != 0)
		{
		  fragmentation = frameSize- (running.memRequest%frameSize);
		}
		else fragmentation = 0;
		//printf("\nfragmentation is: %d ", fragmentation);
		if (fragmentation != 0)
		{
			framesRequired++;
		}
		output.error= 1;
		output.fragmentation = fragmentation;
		output.frameNumbers = framesRequired;
		
		if ((privateFIFO=open(running.str, O_WRONLY))<0)
          printf("cant open private FIFO to write");
	  
		write(privateFIFO, &output, sizeof(output));
		close(privateFIFO);
        unlink(running.str);
		
		availableFrames += framesRequired;
		//printf("available frames: %d ", availableFrames);
	    break;
	  }
	  
	}
	if (running.burst > 0)
	{
	  //enqueue back to ready
	  enqueue(&Ready,running);
	}
  } while ((!isEmpty(&Ready)) || (!isEmpty(&Hold)));


}

