#include<sys/wait.h>
#include<sys/socket.h>
#include<signal.h>
#include<ctype.h>          
#include<arpa/inet.h>
#include<netdb.h>
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>


#define PORT 2000
#define LUNGIME 128

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    /* Declararea Variabilelor */
    int sockfd, nsockfd;
    char revbuf[LUNGIME]; 
    struct sockaddr_in server_addr;

   if(argc!=2)
  {printf("Nu ati dat niciun fisier\n",argv[0]);
   exit(1);
   }   

 /* Crearea descriptorului de socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	 {
       		 fprintf(stderr, "Eroare la crearea socket-ului!	(errno = %d)\n",errno);
       		 exit(1);
   	 }

    /* Completam structura adresei*/
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(PORT); 
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); 
    bzero(&(server_addr.sin_zero), 8);

    /* ne conectam la server */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Eroare la conectarea la server! (errno = %d)\n",errno);
        exit(1);
    }
    else
        printf("[Client] Clientul s-a conectat la portul %d!\n", PORT);

    /*trimitem serverului*/
   char buffer[LUNGIME];  
   
   printf("[Client] Trimitem fisierul %s catre Server... \n",argv[1]);

    FILE *f = fopen(argv[1], "r");
    	if(f == NULL) 
	{
        printf("Fisierul nu a fost gasit.\n");
        exit(1);
   	 }
    bzero(buffer, LUNGIME); 
    int block_trim,i=1; 
    while((block_trim= fread(buffer, sizeof(char), LUNGIME, f)) > 0) 
       {
        printf("Block-ul %d contine %d octeti\n",i,block_trim);
        if(send(sockfd, buffer, block_trim, 0) < 0) 
          {
            fprintf(stderr, "Eroare la trimiterea block-urilor (errno = %d)\n", errno);
            exit(1);
          }
        bzero(buffer, LUNGIME);
        i++;
       }
    close (sockfd);
    printf("[Client]Inchidem conexiunea\n");
    return (0);
}

