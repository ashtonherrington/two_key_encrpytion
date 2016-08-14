#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>


//function provides easier way to handle errors with 1 line instead of 2
void error(const char *msg)
{
    perror(msg);
    exit(1);
}


int main(int argc, char *argv[])
{
     //new variables declared to store socket numbers, port number and the length of the client socket
     int sockfd, newsockfd, portno;
     socklen_t clilen;

     //large buffer declared with 150000 size
     char buffer[150000];

     //socket address structures for both the server and the client are declared
     struct sockaddr_in serv_addr, cli_addr;

     //integer n declared to hold the return value of the write functions
     int n;

     //if there are less than 2 arguments present an error is provided about lack of port
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     //the sockfd variable is set equal to the standard socket function parameters
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
     //im sure I used this for something at some point, not clear what anymore, probably antiquated code
     int res = 0;

     //if the sockfd number is equal to -1, an error is declared and program ends
     if (sockfd < 0) 
        error("ERROR opening socket");

     //the server address variable is filled with 0s
     bzero((char *) &serv_addr, sizeof(serv_addr));
     //the port number is set equal to that of the first argument
     portno = atoi(argv[1]);

     //server address parameters filled 
     serv_addr.sin_family = AF_INET; //declares the addressing type to be TCP
     serv_addr.sin_addr.s_addr = INADDR_ANY; //accepts any incoming address
     serv_addr.sin_port = htons(portno); //converts the portno to Big Endian 

     //the socket is now bound to a port and error declared if binding fails
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     //the program then begins to listen on this socket for incoming connections
     listen(sockfd,5);
     //clilen it set equal to the size of the client address
     clilen = sizeof(cli_addr);
     
     //process id declared to store return value of the fork function
     int pid;

     //ongoing loop allowing for children to be branched off the program
     while(1){
	
	//the newsockfd is set equal to the return value of an incoming connection
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	//if the value of newsockfd is -1 failure is declared and program exits
	if (newsockfd < 0) 
	     error("ERROR on accept");
	
	//program forks upon acepting a new connection
	pid = fork();
        
	//if the fork fails the program ends with an error code
        if(pid < 0){
	   perror("Error on fork\n");
	   exit(1);
	}	

	//otherwise, if we are now in the child process we preform the programs actions
        if(pid == 0){ 
	   
	   //first the sockfd is closed by this program so it is not open in multiple versions of it
	   close(sockfd);
	   
	   //the buffer is then filled with null termiantors
	   bzero(buffer,150000);
	   
	   //an initial read from the buffer with max size value leaves bytes read in this variable
	   int bytesRead = read(newsockfd, buffer, 150000);
	   
           int t;
	   for(t=0; t<9; t++){
	      if(buffer[t] == '~'){
	         printf("Server error: invalid connection attempted\n");
		 exit(2);
	      }	   
	   } 

	   //as the only contents that should be numbers are the packet size, the entire buffer is atoi to generate a parse number
	   int parseNumber = atoi(buffer);
	  
	   //integer accumulator to hold the number of characters taken up by the parseNumber and its newline character in the buffer 
	   int preChars = 0;
	   while(buffer[preChars] != '\n'){
	      preChars++;
	   }
	   preChars++;

	   //the amount of total bytes to read is set equal to the prechars plus two times the packet length (both packets sent have same # characters)
	   int bytesToRead = preChars + (2 * parseNumber);

	   //as long as the bytes read are not equal to the bytes to read this runs
	   while(bytesToRead != bytesRead){
	   
	      //integer variable to collect the amount of bytes read on this runthrough
	      int readThisTime;

	      //as long as the read function does not fail
	      do{
		 //read starts from the offset of bytes already read, and the size to read is determined by difference of ToRead to already read
		 readThisTime = read(newsockfd, buffer + bytesRead, (bytesToRead - bytesRead));
	      }while((readThisTime == -1) && (errno == EINTR));
	      
	      //if there is an actual error the program exits with error code 1
	      if(readThisTime == -1){
		 exit(1);
	      }
	      //the accumulator adds the amount read this iteration to the total bytesRead
	      bytesRead += readThisTime;
	   }

	   //arrays to hold the passed message and secret key are declared
	   char message[parseNumber];
	   char key[parseNumber];
	   
	   //for loop fills key and message arrays with contents of buffer shifted to their repsective length
	   int a;
	   for(a=0; a<parseNumber; a++){
	      //the key array will start preChars subscripts to the right of 0
	      key[a] = buffer[preChars + a];
	      //the message array will start preChars + parseNumber subscripts to the right of 0
	      message[a] = buffer[preChars + a + parseNumber];
	   }
	   //manually ensure that both of the arrays end with a null terminator (otherwise the first has a character in final subscript)
	   message[parseNumber-1] = '\0';
	   key[parseNumber-1] = '\0';

	   //the length of each of these is stored, this was done for testing but is valuable to keep for adjusting the program
	   int mLen = strlen(message);
	   int kLen = strlen(key);
	   
	   //these integers are declared to store the steps required to encode the message using the cipher key 
	   int d,e,f,g;

	   //the cipher array is declared to store the message converted in to a ciphered message
	   char cipher[parseNumber];

	   //for loop itereates from 0 to parsenumber encoding each of the subscripts
	   for(a=0; a<parseNumber; a++){
	      if(message[a] != '\0'){
		 //the first integer variable is set equal to that of the message subscript minus 65 (normalizes A-Z to 0-26)
		 d =  message[a] - 65;
		 //space is ascii 32, so minus 65 yields negative, it is substituted instead as 26 (follows Z)
		 if(d == -33){
		    d = 26;
		 }
		 //the variable e is then taken from the current subscript of the key array
		 e = key[a] - 65;
		 //again if the current subscript is a space it is reoganized to 27 to keep it seqential
		 if(e == -33){
		    e = 26;
		 }
		 //f is set equal to the sum of the key and text characters
		 f = d + e;
		 //g is set equal to the mod 27 of g (27 due to the addition of the space character)
		 g = f % 27;
		 //65 is then added to restore g back to its proper character alignment
		 g += 65;
		 //in the case where g is equal to [, this is spacebar so restored to proper value
		 if(g == 91){
		    g = 32;
		 }
		 cipher[a] = g;
	      }
	      else{
		 cipher[a] = '\0';
	      } 
	   }
	   
	   //the final subscript is given a null terminator
	   cipher[parseNumber-1] = '\0'; 

	   //this statement needs revamped, atm just tells the client it is encoding
	   n = write(newsockfd, cipher, (parseNumber-1));
	   if (n < 0) error("ERROR writing to socket");
	   
	   //program exits upon completion
	   exit(0);   
	}
	else{

	   //if we are in the parent, close the newsockfd socket down so new connections can be made
	   close(newsockfd);
	}
     }
     
     //this should never be reached but added for OCD purposes
     return 0; 
}
