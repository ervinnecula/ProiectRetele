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


#define PORT 2000
#define NRCLIENTI 5
#define LUNGIME 128

char * conv_addr (struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy (str, inet_ntoa (address.sin_addr));	
  /* portul utilizat de client */
  bzero (port, 7);
  sprintf (port, ":%d", ntohs (address.sin_port));	
  strcat (str, port);
  return (str);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main ()
{
    /* Declararea Variabilelor */
    int server_fd, client_fd; 
    fd_set readfds; /* multimea descriptorilor de citire*/
    fd_set actfds; /*multimea descriptorilor activi*/
    int fd;    /* descriptor folosit pentru 
		parcurgerea listelor de descriptori */
    int nfds;	/* numarul maxim de descriptori */
    int len; /* lungimea structurii sockaddr_in*/
    int optval=1; /* optiune folosita pentru setsockopt()*/ 
    struct timeval tv;	/* structura de timp pentru select() */
  
    struct sockaddr_in client_addr; /* client addr */
    struct sockaddr_in server_addr; /* server addr */
    char recvbuff[LUNGIME]; // Receiver buffer

    /* Crearea socket-ului */
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) 
	{
        fprintf(stderr, "Eroare la crearea socket-ului (errno = %d)\n", errno);
        exit(1);
        }
    else 
        printf("[Server] Socket creat cu succes.\n");

    /*setam pentru socket optiunea SO_REUSEADDR */ 
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

    /* pregatim structurile de date */
    bzero (&server_addr, sizeof (server_addr));

    /* completam structura de date */  
    server_addr.sin_family = AF_INET; // Protocol Family
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(PORT); // Port number
   

    /* bind la port */
    if( bind(server_fd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr)) == -1 ) {
        fprintf(stderr, "Eroare la bind (errno = %d)\n", errno);
        exit(1);
    }
   
    /* serverul asculta */
    if(listen(server_fd,5) == -1) {
        fprintf(stderr, "Eroare la listen (errno = %d)\n", errno);
        exit(1);
    }
   
   /* completam multimea de descriptori de citire */
   FD_ZERO (&actfds);		/* initial, multimea este vida 		*/
   FD_SET (server_fd, &actfds);		/* includem in multime 		socketul creat */

   tv.tv_sec = 1;		/* se va astepta un timp de 1 		sec. */
   tv.tv_usec = 0;
  
  /* valoarea maxima a descriptorilor folositi */
  nfds = server_fd;

  printf ("[Server] Asteptam la portul %d...\n", PORT);
  fflush (stdout);
        
  /* servim in mod concurent clientii... */
  while (1)
  {
    /* ajustam multimea descriptorilor activi (efectiv utilizati) */
      bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));


    /* apelul select() */
      if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
	{
	   fprintf(stderr, "Eroare la select (errno = %d)\n", errno);
	   exit(1);
	}

      /* vedem daca e pregatit socketul pentru a-i accepta pe clienti*/
      if (FD_ISSET (server_fd, &readfds))
	{
	  /* pregatirea structurii client */
	  len = sizeof (client_addr);
	  bzero (&client_addr, sizeof (client_addr));
 
        /* a venit un client, acceptam conexiunea */
        
        if ((client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &len)) == -1) 
	{
            fprintf(stderr, "Eroare la accept (errno = %d)\n", errno);
            exit(1);
        }
        if (nfds < client_fd) /* ajusteaza valoarea maximului */
            nfds = client_fd;
            
	  /* includem in lista de descriptori activi si acest socket */
	  FD_SET (client_fd, &actfds);

	  printf("[Server]S-a conectat clientul cu descriptorul %d, de la adresa %s \n",client_fd,conv_addr (client_addr));
	  fflush (stdout);
	}
 /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
      for (fd = 0; fd <= nfds; fd++)	/* parcurgem multimea de descriptori */
    {
      /* este un socket de citire pregatit? */
      if (fd != server_fd && FD_ISSET (fd, &readfds))
        {	
          if (primeste(fd))
	    {
	     printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
	     fflush (stdout);
	     close (fd);		/* inchidem conexiunea cu clientul */
	     FD_CLR (fd, &actfds);/* scoatem si din multime */
	  
	    }
        }
    }	/* for */
    }	/* while */
}/* main */

int primeste(int fd)
{ char recvbuff[LUNGIME];
         char *path_fis_prim = "/home/ervin/Desktop/Proiect/primit.txt";
        FILE *f = fopen(path_fis_prim, "a+");
        if(f == NULL)
            printf("Eroare la crearea fisierului primit de la client\n", path_fis_prim);
        else {
            bzero(recvbuff, LUNGIME); 
            int block_prim = 0;
            int i=1;
            while((block_prim = recv(fd, recvbuff, LUNGIME, 0)) > 0)
	    {    printf("Block-ul %d contine %d octeti\n",i,block_prim);

                int nr_oct_scris = fwrite(recvbuff, sizeof(char), block_prim, f);

                if(nr_oct_scris < block_prim)
                    error("Am primit mai putini octeti decat trebuia.\n");
                bzero(recvbuff, LUNGIME);
                i++;
	    }
            
            if(block_prim < 0)
	      {
                if (errno == EAGAIN)
                    printf("recv() timed out.\n");
                else {
                    fprintf(stderr, "Eroarea %d la recv()\n", errno);
                    exit(1);
                    }
              }
              fclose(f); 
	      printf("Am trimis primit fisierul cu succes!\n");
	    }


}

