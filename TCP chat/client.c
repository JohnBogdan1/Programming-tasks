#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFLEN 256

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, fdmax;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	fd_set fs_read, fs_temp;

	// golesc multimea de descriptori de citire (fs_read) si multimea fs_temp
	FD_ZERO(&fs_read);
	FD_ZERO(&fs_temp);

	// buffer-ul prin care se trimit mesaje
	char buffer[BUFLEN];

	if (argc < 3) {
		fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
		exit(0);
	}

	// deschid socket-ul
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	// setez valorile necesare(adresa IP, port)
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &serv_addr.sin_addr);

	// adaug in set sockfd si 0
	FD_SET(sockfd, &fs_read);
	FD_SET(0, &fs_read);

	// conctare pe acel socket la server
	if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");  

	// descriptorul cu valoare maxima
	fdmax = sockfd;  
	int i = 0;

	FILE *fp = NULL;

	// creez log-file pentru client
	char id[24], filename[24];

	strcpy(filename, "client-");
	sprintf(id, "%d", getpid());

	strcat(filename, id);
	strcat(filename, ".log");

	// il deschid pentru a scrie comenzile si raspunsurile
	fp = fopen(filename, "w");

	if (fp == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}

	int authenticated = 0, r, dimension = BUFLEN;

	// buffere folosite pentru a salva datele
	char command_recv[24], username[24], command[24];

	// loop infinit
	while(1) {

		for (i = 0; i <= fdmax; i++) {
			fs_temp = fs_read;

			// fac select
			if (select(fdmax + 1, &fs_temp, NULL, NULL, NULL) == -1) {
				error("ERROR in select");
			}

			// verific daca descriptorul este din set
			if (FD_ISSET(0, &fs_temp)) {

				// intializez buffer-ul
				memset(buffer, 0 , BUFLEN);

				//citesc de la tastatura
				fgets(buffer, BUFLEN - 1, stdin);
				if (n < 0)
					error("Error reading from stdin");
				else {

					// scriu in fisier comanda trimisa
					memset(command, 0 , 24);

					// citesc comanda si mai jos o sa o scriu in fisier
					// impreuna cu raspunsuri
					r = sscanf(buffer, "%s", command);

					// daca clientul este autentificat si a dat o comanda de login
					// afisez eroare
					if (authenticated == 1 && !strcmp(command, "login")) {
						printf("-2 Sesiune deja deschisa\n\n");
						fprintf(fp, "%s", username);
						fprintf(fp, "> ");
						fprintf(fp, "%s", buffer);
						fprintf(fp, "-2 Sesiune deja deschisa\n\n");
					} else { // altfel afisez prompt-ul corespunzator
						if (authenticated == 1) {
							fprintf(fp, "%s", username);
							fprintf(fp, "> ");
						} else {
							fprintf(fp, "$ ");
						}

						// daca comanda este logout, resetez prompt-ul
						// daca clientul nu este autentificat, afisez eroare
						// nu mai trimit la server
						// altfel notific serverul si afisez comanda
						if (!strcmp(command, "logout")) {
							if (authenticated == 0) {
								printf("-1 Clientul nu e autentificat\n\n");
								fprintf(fp, "%s", buffer);
								fprintf(fp, "-1 Clientul nu e autentificat\n\n");
							} else {
								authenticated = 0;
								send(sockfd, buffer, strlen(buffer), 0);
								printf("\n");
								fprintf(fp, "%s\n", buffer);
							}
						} else if (!strcmp(command, "getuserlist")) {
							// clientul cere server-ului lista de utilizatori inregistrati

							// daca clientul nu este autentificat, afisez eroare
							// nu mai trimit la server
							if (authenticated == 0) {
								printf("-1 Clientul nu e autentificat\n\n");
								fprintf(fp, "%s", buffer);
								fprintf(fp, "-1 Clientul nu e autentificat\n\n");
							} else { // altfel afisez lista

								// afisez comanda in fisier
								fprintf(fp, "%s", buffer);

								// trimit comanda la server
								send(sockfd, buffer, strlen(buffer), 0);

								// primesc lungimea mesajului care contine userii
								recv(sockfd, buffer, BUFLEN, 0);

								char number[24];
								memset(number, 0, 24);

								r = sscanf(buffer, "%s", number);

								dimension = atoi(number);
								if (dimension <= 0)
									dimension = BUFLEN;
								
								// notific serverul ca am primt dimensiunea
								send(sockfd, "users", strlen("users"), 0);
							}
						} else if (!strcmp(command, "quit")) {

							// scriu comanda in fisier
							fprintf(fp, "%s", buffer);

							// clientul notifica serverul
							send(sockfd, buffer, strlen(buffer), 0);

							// clientul iese
							exit(0);
						} else {

							// scriu comanda
							fprintf(fp, "%s", buffer);

							// o trimit la server
							send(sockfd, buffer, strlen(buffer), 0);

							// daca comanda cere lista de fisiere
							// fac la fel ca la getuserlist
							if (!strcmp(command, "getfilelist")) {
								recv(sockfd, buffer, BUFLEN, 0);

								char number[24];
								memset(number, 0, 24);

								r = sscanf(buffer, "%s", number);

								dimension = atoi(number);

								if (dimension <= 0)
									dimension = BUFLEN;
								
								send(sockfd, "files", strlen("files"), 0);
							}
						}
					}
				}
			}

			// verific daca descriptorul este din set
			if (FD_ISSET(sockfd, &fs_temp)) {

				// in acest buffer primesc lista de useri si de fisiere
				char recv_buff[dimension];

				if (!strcmp(command, "getfilelist")) {
					memset(recv_buff, 0 , dimension);

					// primesc lista de fisiere de la server
					n = recv(sockfd, recv_buff, dimension, 0);

				} else if (!strcmp(command, "getuserlist")) {
					memset(recv_buff, 0 , dimension);

					// primesc lista de useri de la server
					n = recv(sockfd, recv_buff, dimension, 0);

				} else { // primesc orice altceva
					memset(buffer, 0 , BUFLEN);
					n = recv(sockfd, buffer, BUFLEN, 0);
				}

				if (n < 0) {
					error("Error receive");
				} else if (n > 0) {

					// "comanda" de la server
					// folosita pentru a sti ce sa primesc
					r = sscanf(buffer, "%s", command_recv);

					// inseamna ca clientul s-a autentificat cu succes
					// scriu in fisier si afisez User>
					if (!strcmp(command_recv, "AUTHENTICATED")) {
						r = sscanf(buffer, "%s %s", command_recv, username);
						authenticated = 1;
						printf("%s", username);
						printf(">\n\n");
						fprintf(fp, "%s", username);
						fprintf(fp, ">\n\n");
					} else if (!strcmp(command, "getuserlist")) { // afisez lista de useri
						printf("%s\n", recv_buff);
						fprintf(fp, "%s\n", recv_buff);
					} else if (!strcmp(command, "getfilelist")) { // afisez lista de fisiere
						char error_no[3];
						memset(error_no, 0, 3);
						r = sscanf(recv_buff, "%s", error_no);
						// in caz de eroare, afisez eroarea
						if (!strcmp(error_no, "-11")) {
							printf("%s\n\n", recv_buff);
							fprintf(fp, "%s\n\n", recv_buff);
						} else {
							printf("%s\n", recv_buff);
							fprintf(fp, "%s\n", recv_buff);
						}
					} else if (!strcmp(command_recv, "error")) { // comanda gresita de la client
						printf("Comanda eronata\n\n");
						fprintf(fp, "Comanda eronata\n\n");
					} else { // orice altceva primit de la server
						printf("%s\n\n", buffer);
						fprintf(fp, "%s\n\n", buffer);

						// daca serverul a trimis acest mesaj
						// conexiunea cu clientul este intrerupta
						if (!strcmp(buffer,"-8 Brute-force detectat"))
							exit(0);
					}
				}
			}
		}
	}

	// inchid fisierul
	fclose(fp);
	return 0;
}