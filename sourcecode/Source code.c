#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>



#define MAX_CONNECTIONS 3
#define BUFFER_SIZE 512

int port;
struct sockaddr_in address;
struct sockaddr_in connector;
int current_socket;
int connecting_socket;
socklen_t addr_size;
char *wwwroot;
char *conf_file;
char *currerntLine;
pthread_t tid;
static int total_request;
static int count = 0;
pthread_mutex_t lock_requests_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_size_mtx = PTHREAD_MUTEX_INITIALIZER;

void *function(void *);


FILE *filepointer = NULL;




void increment_req()
{

 pthread_mutex_lock(&lock_requests_mtx);
total_request++;
pthread_mutex_unlock(&lock_requests_mtx);
}


int show_total_requests()
{

return total_request;

}


void createSocket()

{

current_socket = socket(AF_INET, SOCK_STREAM, 0);
if ( current_socket == -1 )

{

perror("Create socket");

exit(-1);

}

 }


void bindSocket()

{

//Bind to the current_socket descriptor and listen to the port in PORT.

address.sin_family = AF_INET;

address.sin_addr.s_addr = INADDR_ANY;

address.sin_port = htons(port);

if ( bind(current_socket, (struct sockaddr *)&address, sizeof(address)) < 0 )

{

perror("Bind to port");

exit(-1);

 }

}


void startListener()

{

 perror("Listen on port");

//Start listening for connections and accept no more than MAX_CONNECTIONS in the Queue.

if ( listen(current_socket, MAX_CONNECTIONS) < 0 )

{

exit(-1);

}

}

 

void acceptConnection()

{

 
addr_size = sizeof(connector);

connecting_socket = accept(current_socket, (struct sockaddr *)&connector, &addr_size);

if ( connecting_socket < 0 )
{
perror("Accepting sockets");

exit(-1);

}
else
{
if(pthread_create(&tid,NULL,&function,(void *)&connecting_socket) < 0)
{
perror("could not create thread");

}
printf("Thread created Successfull\n");
}

}

void *function(void *current_socket)

{
// type casting.

char  buffer[9999];
char *reqline[3], path[99999];
int fd,bytes_read;
int sock = *(int *)current_socket;
int connecting_socket;
 bzero(buffer,9999);

// Storing header in buffers.
char *head = "HTTP/1.1";
char *status_code = " 200 OK\r\n";
char *content_head = "content-type:";
char *server_head = "\r\nserver:207httpd/0.0.1";
char *connect = "\r\n Connection:close";
char *length_head = "\r\ncontent-length: ";
char *newline = "\r\n\r\n";
char contentlength[200];

FILE *fp;
char *filetype;

// recieving request from client.

 if (recv(sock, buffer, 9999,0)<0)
{
printf ("error in recieving...\n");
}

// Checking File Type.
else 
{
filetype="0";
(strstr(buffer,"html"));
filetype="html";


if ((strstr(buffer,"jpg"))>0)
{

filetype = "jpg";
}

}

  // Clients detail.
 
 printf("\nrecieved data from client is %s\n",buffer);
 printf("clients port number is: %d\n",ntohs(connector.sin_port));

//Verifying HTTP Request from client.

reqline[0] = strtok (buffer, " \t\n");

printf("The cut part is %s\n",reqline[0]);
        if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
            reqline[1] = strtok (NULL, " ");
            reqline[2] = strtok (NULL, "\r\n\r\n");
            if ((strncmp( reqline[2], "HTTP/1.1", 8)!=0 ))
            {
                send(sock, "HTTP/1.0 400 Bad Request\n", 25,0);
               printf("Buffer filled upto:%s\n",buffer);
 				pthread_mutex_lock(&lock_requests_mtx);
				count++;
				pthread_mutex_unlock(&lock_requests_mtx);
       			printf("Number of bad request:%d\n",count);
          
            }
            else
            {
                if ( strncmp(reqline[1], "/\0", 2)==0 )
                    reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default.
}
}
// Copies root directory in path.
                strcpy(path, wwwroot);
// Copies and joins them in order.
                strcpy(&path[strlen(wwwroot)], reqline[1]);
                printf("file: %s\n", path);

                if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND.
                {

printf("hilaooo \n");
 fp = fdopen(fd,"r");
int filesize = 0;
fseek(fp,0, SEEK_END);
filesize = ftell(fp);
rewind(fp);

sprintf (contentlength," %i",filesize);

//Concatination of header.

char message[10000];

if(message != NULL)
{
strcpy(message, head);
strcat(message,status_code);
strcat(message,content_head);
strcat(message,filetype);
strcat(message,server_head);
strcat(message,connect);
strcat(message,length_head);
strcat(message,contentlength);
strcat(message,newline);
}



printf("header is %s \n",message);

send(sock,message, strlen(message),0);

increment_req();
printf("\n Request number :%d\n",show_total_requests());

         
         }
printf("file opened with request:%s\n",buffer);

          
      
        if(fp==NULL)
        {
            send(sock, "HTTP/1.0 404 Not Found\n", 23,0);
            printf("File open error");
           pthread_mutex_lock(&lock_requests_mtx);
			count++;
			pthread_mutex_unlock(&lock_requests_mtx);
       printf("Number of bad request:%d\n",count);            
        }   
  
        /* Read data from file and send it */
        while(1)
        {
            /* First read file in chunks of 256 bytes */
           unsigned char data[256]={0};
            int nread = fread(data,1,256,fp);
            printf("Bytes read %d \n", nread);        
  
            /* If read was success, send data. */
            if(nread > 0)

            {
                printf("Sending \n");
                write(sock, data, nread);
              
            }

            
            if (nread < 256)
            {
             
                if (feof(fp))
                    printf("End of file\n");
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }       
  }
               
 close(sock);
   }
 



void start()

{

createSocket();

bindSocket();

startListener();

while ( 1 )

{

acceptConnection();

}

}

void init()
{
char *currentLine = malloc(100);
wwwroot = malloc(100);
conf_file = malloc(100);
conf_file = "http.conf";


filepointer = fopen(conf_file, "r");

if(filepointer == NULL)
{

fprintf(stderr,"cannot open file\n");
exit(1);
}

if (fscanf(filepointer,"%s %s",currentLine,wwwroot) !=2 )
{
fprintf(stderr,"Error in finding config file\n");
exit(1);
}

if (fscanf(filepointer, "%s %i", currentLine, &port) != 2)

// Get default port from configuration file

{

fprintf(stderr, "Error in configuration file on line 2!\n");

exit(1);

}

fclose(filepointer);

free(currentLine);
}

int main(int argc, char* argv[])
{

init();

printf("Server root:\t\t%s\n", wwwroot);
printf("Settings:\n");
printf("Port:\t\t\t%i\n", port);
printf("Configuration file:\t%s\n", conf_file);


start();

return 0;
}

