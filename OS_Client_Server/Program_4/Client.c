/***********************  client.c  *******************************
* Programmer:   Geoffrey Perez
*
* Course:   CSCI 4354
*
* Date:   April 28, 2019
*
* Assignment:   Program #4
*
* Environment:   Windows 10 using Putty terminal
*
* Purpose:  Get burst time, privateFIFO info, and memory request
*           and send to server. Server sends back completion time,
*           fragmentation amount and # frames assigned or an error
*           for the client to output.
*            
*        
*          
* Input:   Burst time, memory request
*
* Output:  completion time, fragmentation, and frames assigned or error msg
*          
*
* Time :
*        Estimated:  Design     Coding     Testing
*                      1          1          1
*
*        Actual:     Design     Coding     Testing
*                      1          1          2       
*
* Algorithm:
*       create structs to send and receive info to/from client
*       get PID and append it to struct privInfo buffer
*       get user input burst time & memrequest
*       create common fifo
*       write to server
*       create private fifo
*       read private fifo information from server
*       create case/switch to display the correct output
*       close common and private fifo's
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


main (void) {

  struct clientInfo {
    char privInfo[20];  // private FIFO name
    int burst;
	int memRequest;
  } input;

  struct output {
    int clockTime; 
	int fragmentation;	
	int frameNumbers;
	int error;
  }  output;

  int commonFIFO;  // to write to server
  int privateFIFO;  // to read from server
  int i=0;

  // Append pid to "ProcessID_" and store in struct
  snprintf(input.privInfo, sizeof(input.privInfo), "ProcessID_%d", getpid());

  printf("The pid is: %s", input.privInfo);
  printf("\nClient: Please enter burst time: ");
  scanf("%d", &input.burst);
  printf("Client: Please input how many units of memory you'd like to request: ");
  scanf("%d", &input.memRequest);
 
  

//open common fifo for writing
  if ((commonFIFO=open("commonFIFO", O_WRONLY))<0)
    printf("cant open fifo to write");

  write(commonFIFO, &input, sizeof(input));

  sleep(2);
//open private fifo for reading
  if ((privateFIFO=open(input.privInfo, O_RDONLY))<0)
    printf("cant open privateFIFO to read");

  read(privateFIFO, &output, sizeof(output));

  //print outputs
  
switch(output.error)
{
	
case 1:
  printf("\nReceived a completion time of %d from server", output.clockTime);
  printf("\nThe number of frames assigned = %d ", output.frameNumbers);
  printf("\nThe fragmentation was %d ", output.fragmentation);
  break;
  
case 2:
  printf("\nError: the memory request was more than the current available amount of memory");
  break; 
  
case 3:

  printf("\nError: the memory request was more than the total amount of memory");
  break;  
  
}

  close(commonFIFO);
  close(privateFIFO);
}
