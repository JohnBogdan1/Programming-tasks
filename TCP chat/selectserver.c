#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>	/* O_RDWR */
#include <errno.h>	/* perror() */

#define BUFLEN 256

void error(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]) {

	int sockfd, newsockfd, portno, clilen;

	// buffer-ul prin care sunt transmise mesajele
	char buffer[BUFLEN];

	struct sockaddr_in serv_addr, cli_addr;
	int n, i, j, r;

	// cele doua fisiere de configurare ale serverului 
	FILE *users_config_file, *static_shares_config_file;

	fd_set read_fds; // multimea de citire folosita in select()
	fd_set tmp_fds; // multime folosita temporar
	int fdmax; // valoare maxima file descriptor din multimea read_fds

	if (argc < 3) {
		fprintf(stderr,"Usage : %s port\n", argv[0]);
		exit(1);
	}

	// golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	// deschidere socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	// portul
	portno = atoi(argv[1]);

	// setez valorile necesare
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
	serv_addr.sin_port = htons(portno);

	// fac bind
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
		error("ERROR on binding");

	// deschid cele doua fisiere de configurare
	users_config_file = fopen(argv[2], "r");
	static_shares_config_file = fopen(argv[3], "r");

	if (users_config_file == NULL || static_shares_config_file == NULL)
		error("Nu pot deschide un fisier");

	// ma pozitionez la inceputul lor
	rewind(users_config_file);
	rewind(static_shares_config_file);

	// citesc numarul N care imi da numarul de intrari
	int N;
	r = fscanf(users_config_file, "%d", &N);

	// ascult maxim N clienti pe socket-ul inactiv
	listen(sockfd, N);

	//adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	// am un vector in care retin numarul de incercari de logare 
	// pentru fiecare client
	int client_attempts[N], eroare = 0;

	// folosit pentru a deschide un director
	DIR *dp = NULL;

	// cei doi vectori de string-uri pentru getuserlist si getfilelist
	char *list = NULL;
	char *flist = NULL;

	// bucla infinita
	while (1) {

		// setez tmp_fds la multimea de citire
		tmp_fds = read_fds;

		// fac select
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1)
			error("ERROR in select");

		for(i = 0; i <= fdmax; i++) {

			// daca descriptorul este setat
			if (FD_ISSET(i, &tmp_fds)) {

				if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					}
					else {
						// adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) {
							fdmax = newsockfd;
						}
						// pentru fiecare client, initializez numarul de login-uri consecutive la 0
						client_attempts[i % N] = 0;
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				} else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					// actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {

						if (n == 0) {
							// conexiunea s-a inchis
							printf("selectserver: socket %d hung up\n", i);
						} else {
							error("ERROR in recv");
						}
						close(i); 
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul

					} else { //recv intoarce > 0
						printf ("Am primit de la clientul de pe socketul %d, mesajul: %s\n", i, buffer);

						// folosite pentru a citi date din buffer/fisier
						char read_username[24], read_password[24], action[24], username[24], password[24];
						memset(username, 0, 24);
						memset(password, 0, 24);
						memset(action, 0, 24);
						memset(read_username, 0, 24);
						memset(read_password, 0, 24);

						rewind(users_config_file);
						rewind(static_shares_config_file);

						// citesc numarul de intrari
						r = fscanf(users_config_file, "%d", &N);

						// citesc din buffer actiunea/comanda data de client
						r = sscanf(buffer, "%s", action);

						int ok = 0;

						if (!strcmp(action, "login")) { // daca comanda este login

							// citesc ce username si parola mi-au fost trimise
							r = sscanf(buffer, "%s %s %s", action, username, password);
							int j;

							// parcurg fisierul users_config pentru a verifica 
							// daca datele sunt corecte
							// daca sunt corecte, trimit clientului un mesaj 
							// care nu este afisat, pentru a stii ca este logat
							for (j = 0; j < N; j++) {
								r = fscanf(users_config_file, "%s %s", read_username, read_password);
								if (!strcmp(username, read_username) && !strcmp(password, read_password)) {
									// fac send catre client
									ok = 1;
									char authenticated_user[40];
									strcpy(authenticated_user, "AUTHENTICATED ");
									strcat(authenticated_user, username);

									int k = send(i, authenticated_user, BUFLEN - 1, 0);
									if (k < 0)
										error("ERROR send");
									break;
								}
							}

							// altfel, ori ii trimit ca a incercat de 3 ori 
							// consecutiv, ori ii zic ca datele sunt gresite
							// daca este brute-force, opresc conexiunea
							if (ok == 0) {

								client_attempts[i % N]++;
								if (client_attempts[i % N] == 3) {

									// resetez numarul de incercari pentru acest client
									client_attempts[i % N] = 0;
									strcpy(buffer, "-8 Brute-force detectat");
									int k = send(i, buffer, BUFLEN - 1, 0);

									printf("selectserver: socket %d hung up\n", i);
									if (k < 0)
										error("ERROR send");

									memset(buffer, 0, BUFLEN);
									close(i);
									FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul
								} else {
									strcpy(buffer, "-3 User/parola gresita");
									int k = send(i, buffer, BUFLEN - 1, 0);
									if (k < 0)
										error("ERROR send");
								}
							}
						} else if (!strcmp(action, "logout")) {
							// in acest caz in client se va reseta prompt-ul
							// in server, resetez numarul de incercari de logare
							memset(buffer, 0, BUFLEN);
							client_attempts[i % N] = 0;
						} else if (!strcmp(action, "getuserlist")) {

							// resetez numarul de incercari de logare
							client_attempts[i % N] = 0;

							int j;
							char read_username[24], read_password[24];
							memset(read_username, 0, 24);
							memset(read_password, 0, 24);

							memset(buffer, 0, BUFLEN);

							rewind(users_config_file);
							rewind(static_shares_config_file);

							// citesc numarul de intrari
							r = fscanf(users_config_file, "%d", &N);

							// aloc un string de string-uri in care voi pune userii
							int size = 1024;
							list = malloc(size * sizeof(char *));
							int len_contor = 0;

							for (j = 0; j < N; j++) {

								// citesc fiecare nume si parola
								r = fscanf(users_config_file, "%s %s", read_username, read_password);

								// adaug dimensiunea numelui + 1 de fiecare data
								len_contor += strlen(read_username) + 1;

								char *new = NULL;

								// daca dimensuinea este depasita, realoc
								if (len_contor > size ) {
									size = size * 2;
									if (!new) {
										printf("ERROR\n");
									} else {
										flist = new;
									}
								}

								// adaug in string
								strcat(list, read_username);
								strcat(list, "\n");
							}

							// trimit clientului dimensiunea string-ului
							// el o sa imi trimita un mesaj, 
							// iar eu o sa ii trimit string-ul
							char string_size[20];
							memset(string_size, 0, 20);

							sprintf(string_size, "%d", (int)strlen(list) + 1);

							int k = send(i, string_size, 20, 0);
							if (k < 0)
								error("ERROR send");

						} else if (!strcmp(action, "getfilelist")) {

							// resetez numarul de incercari de logare
							client_attempts[i % N] = 0;

							char dir_name[24];
							memset(dir_name, 0, 24);

							// citesc numele directorului utilizatorului
							r = sscanf(buffer, "%s %s", action, dir_name);

							struct dirent *dptr = NULL;

							rewind(static_shares_config_file);

							rewind(users_config_file);

							// numarul de intrari
							int N;
							r = fscanf(users_config_file, "%d", &N);

							memset(username, 0, 24);
							memset(password, 0, 24);

							int ok = 0;

							// verific daca numele exista
							while ( (r = fscanf(users_config_file, "%s %s", username, password) ) == 2) {
								if (!strncmp(dir_name, username, strlen(username))) {
									ok = 1;
									break;
								}
							}

							// daca exista, trimit clientului fisierele
							if (ok == 1) {

								struct stat st;

								// deschid directorul utilizatorului
								if( NULL == (dp = opendir(dir_name)) ) {
									printf("\n Cannot open directory [%s]\n", dir_name);
									exit(1);
								} else {

									char path[44];
									memset(path, 0, 44);

									// un string de string-uri in care voi adauga toate datele
									int size = 1024;
									flist = malloc(size * sizeof (char *));
									int len_contor = 0;

									// citesc fisierele din director
									while( NULL != (dptr = readdir(dp)) ) {

										// fara fisierele directorul parinte si cel curent
										if (strcmp(dptr -> d_name, ".") && strcmp(dptr -> d_name, "..")) {

											// adaug la len_contor dimensiunile datelor de fiecare data
											len_contor += strlen(dptr -> d_name);

											char attribute[7];
											memset(attribute, 0, 7);

											char file_name[80];

											memset(username, 0, 24);
											memset(file_name, 0, 80);
											int ok = 0;

											rewind(static_shares_config_file);

											r = fscanf(static_shares_config_file, "%d", &N);

											// citesc fiecare nume:fisier din fisierul de configurare
											while ( (r = fscanf(static_shares_config_file, "%[^:]:%[^\n]", username, file_name) ) == 2) {

												// daca numele fisierului curent se afla in fisierul de configurare
												// atunci este SHARED, altfel este PRIVATE
												if (!strcmp(dptr -> d_name, file_name)) {
													ok = 1;
													len_contor += strlen("SHARED");
													strcpy(attribute, "SHARED");
													break;
												}
											}

											if (ok == 0) {
												len_contor += strlen("PRIVATE");
												strcpy(attribute, "PRIVATE");
											}

											// pentru a afla dimensiunea fisierului folosesc functia stat
											// dau path-ul catre fisier ca parametru
											strcpy(path, "./");
											strcat(path, dir_name);
											strcat(path, "/");
											strcat(path, dptr -> d_name);

											stat(path, &st);

											char file_size[20];
											memset(file_size, 0, 20);

											// pun dimensiunea fisierului intr-un string
											sprintf(file_size, "%d", (int)st.st_size);

											// adaug la contor dimensiunea string-ului
											// plus patru pentru separatori
											len_contor += strlen(file_size) + 4;

											char *new = NULL;

											// daca am depasit dimensiunea, realoc
											if (len_contor > size) {
												size = size * 2;
												new = realloc(flist, size * sizeof(char *));
												if (!new) {
													printf("ERROR\n");
												} else {
													flist = new;
												}
											}

											// adaug la string datele necesare
											strcat(flist, dptr -> d_name);
											strcat(flist, " ");
											strcat(flist, file_size);
											strcat(flist, " bytes ");
											strcat(flist, attribute);
											strcat(flist, "\n");
										}
									}

									char string_size[20];
									memset(string_size, 0, 20);

									// trimit dimensiunea la client
									// el va trimite un mesaj, 
									// dupa care serverul va trimite string-ul
									sprintf(string_size, "%d", (int)strlen(flist) + 1);

									int k = send(i, string_size, 20, 0);
									if (k < 0)
										error("ERROR send");

								}
							} else { // altfel, trimit eroare
								strcpy(buffer, "-11 Utilizator inexistent");
								int k = send(i, buffer, BUFLEN - 1, 0);
								if (k < 0)
									error("ERROR send");
								eroare = 1;
							}
						} else if (!strcmp(action, "users")) { // mesajul trimis de client 
																//cand se cere lista de useri
							int k = send(i, list, strlen(list), 0);
							if (k < 0)
								error("ERROR send");
							list = NULL;
							free(list);
						} else if (!strcmp(action, "files")) { // mesajul trimis de client cand
																// se cere lista de fisiere

							if (eroare == 0) {
								int k = send(i, flist, strlen(flist), 0);
								if (k < 0)
									error("ERROR send");
							} else { // daca nu exista user-ul -> eroare
								eroare = 0;
								strcpy(buffer, "-11 Utilizator inexistent");
								int k = send(i, buffer, BUFLEN - 1, 0);
								if (k < 0)
									error("ERROR send");
							}

							flist = NULL;
							free(flist);
						} else if (!strcmp(action, "quit")) { // clientul doreste sa iasa

							// resetez numarul de incercari de logare(nu e nevoie)
							client_attempts[i % N] = 0;
							memset(buffer, 0, BUFLEN);
							close(i); // inchid socket-ul
							FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul
						} else { // in acest caz avem o comanda gresita de la client
							client_attempts[i % N] = 0;
							memset(buffer, 0, BUFLEN);
							strcpy(buffer, "error");

							// trimit clientului un mesaj in acest caz
							// acesta va afisa la terminal un mesaj corespunzator
							int k = send(i, buffer, BUFLEN - 1, 0);
							if (k < 0)
								error("ERROR send");
						}
					}
				} 
			}
		}
	}

	// inchid socket-ul
	close(sockfd);

	// inchid fisierele
	fclose(users_config_file);
	fclose(static_shares_config_file);
	
	return 0; 
}