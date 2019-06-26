#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

#define ACK_parity 4
#define NACK_parity 5

#define ACK_hamming 3
#define NACK_hamming 4

int test_0(char* string, msg r) {

	// low, high si mid sunt folosite pentru ghicirea numarului
	int res, low, high, mid;
	char* token;

	// in a pun raspunsul meu
	char* a[MSGSIZE];
	int i = 0, j = 0, k = 0, n;

	// caut dupa cuvinte cheie in mesaj
	while ((token = strsep(&string, " ")) != NULL) {

		j++;

		// caut dupa word
		// deci stiu ca urmatorul cuvant e cel pe care trebuie sa il trimit
		if (!strcmp(token, "word")) {
			j = i;
			i++;

		}

		if (j == i) {
			a[k++] = token;

		}

		// aici caut intervalele intre care se afla numarul
		// si folosesc cautarea binara pentru a ghici
		if (!strncmp(token, "random", 6)) {
			char* pch, *pEnd;
			pch = strchr(r.payload,'[');

			// am gasit marginea inferioara
			low = r.payload[pch - r.payload + 1] - '0';

			// am gasit marginea superioara
			high = (int) strtol(pch + 4, &pEnd, 10);

			// calculez mijlocul
			mid = (low + high) / 2;

			// pun numarul intr-un sir de caractere si il trimit
			sprintf(r.payload, "%d", mid);
			r.len = strlen(r.payload) + 1;
			res = send_message(&r);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}


		}

		// daca clientul imi trimite smaller
		// modific marginea superioara
		if (!strncmp(r.payload, "smaller", 7)) {
			high = mid - 1;
			
			// recalculez mijloc
			mid = (low + high) / 2;

			// trimit raspuns
			sprintf(r.payload, "%d", mid);
			r.len = strlen(r.payload) + 1;
			res = send_message(&r);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
		}

		// daca clientul imi trimite smaller
		// modific marginea inferioara
		if (!strncmp(r.payload, "bigger", 6)) {
			low = mid + 1;
			
			// recalculez mijloc
			mid = (low + high) / 2;

			// trimit raspuns
			sprintf(r.payload,"%d", mid);
			r.len = strlen(r.payload) + 1;
			res = send_message(&r);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
		}

	}

	// aici trimit toate raspunsurile gasite dupa cuvantul cheie "word"
	for (n = 0; n < k; n++){

		// trimit cate un raspuns, fiecare aflandu-se in vectorul a
		sprintf(r.payload,"%s", a[n]);
		r.len = strlen(r.payload) + 1;
		res = send_message(&r);

		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}

	}
	return 0;
}


int test_parity(char* string, msg r) {

	int res, low, high, mid;
	char* token;

	// in a pun raspunsul meu
	char* a[MSGSIZE];
	int i = 0, j = 0, k = 0, n;

	char buf[sizeof(int) * 3 + 2];

	msg t;

	// la fel ca mai sus, caut dupa cuvantul cheie word
	// si trimit un numar in functie de ce raspuns imi trimite clientul
	while ((token = strsep(&string, " ")) != NULL) {
		j++;

		if (!strcmp(token, "word")) {
			j = i;
			i++;

		}

		if (j == i) {
			a[k++] = token;
		}

		if (!strncmp(token, "random", 6)) {
			char* pch, *pEnd;

			pch = strchr(r.payload,'[');

			low = r.payload[pch - r.payload + 1] - '0';
			high = (int) strtol(pch + 4, &pEnd, 10);

			mid = (low + high) / 2;

			// pun numarul intr-un sir de caractere
			int nr = sprintf(buf, "%d", mid);

			int bit = 0;

			// calculez paritatea lui
			for (i = 0; i < nr; i++){
				for (j = 0; j < 8; j++) {
					bit ^= (buf[i] >> j) & 1;
				}
			}
			
			// pun paritatea + numar in payload si trimit
			sprintf(t.payload, "%d%d", bit, mid);
			t.len = strlen(t.payload) + 1;
			res = send_message(&t);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
		} else if (!strncmp(r.payload + 1, "smaller", 7)) {
			high = mid - 1;
			
			mid = (low + high) / 2;

			// pun numarul intr-un sir de caractere
			int nr = sprintf(buf, "%d", mid);

			int bit = 0;

			// calculez paritatea lui
			for (i = 0; i < nr; i++){
				for (j = 0; j < 8; j++) {
					bit ^= (buf[i] >> j) & 1;
				}
			}

			// pun paritatea + numar in payload si trimit
			sprintf(t.payload, "%d%d", bit, mid);
			t.len = strlen(t.payload) + 1;
			res = send_message(&t);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
		} else if (!strncmp(r.payload + 1, "bigger", 6)) {
			low = mid + 1;

			mid = (low + high) / 2;

			// pun numarul intr-un sir de caractere
			int nr = sprintf(buf, "%d", mid);

			int bit = 0;

			// calculez paritatea lui
			for (i = 0; i < nr; i++){
				for (j = 0; j < 8; j++) {
					bit ^= (buf[i] >> j) & 1;
				}
			}
			
			// pun paritatea + numar in payload si trimit
			sprintf(t.payload, "%d%d", bit, mid);
			t.len = strlen(t.payload) + 1;
			res = send_message(&t);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
		}

	}

	// daca primesc NACK, trimit din nou raspuns
	if (r.len == 5) {

		res = send_message(&t);

		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}

	}

	// aici trimit toate raspunsurile gasite dupa cuvantul cheie "word"
	for (n = 0; n < k; n++) {

		int bit = 0;

		// calculez paritatea raspunsului
		for (i = 0; i < strlen(a[n]); i++) {
			for (j = 0; j < 8; j++) {
				bit ^= (a[n][i] >> j) & 1;
			}
		}

		// trimit raspunsul impreuna cu paritatea
		sprintf(t.payload, "%d%s", bit, a[n]);
		t.len = strlen(t.payload) + 1;
		res = send_message(&t);

		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}

	}
	return 0;
}

char* coded_byte(char byte) {
	// primesc un octet si il codific cu ajutorul a doi octeti(12 biti codificati)
	char* encoded = (char *) calloc(2, sizeof(char));

	// extrag bitii de date din octet
	char D1 = (byte >> 7) & 1, D2 = (byte >> 6) & 1;
	char D3 = (byte >> 5) & 1, D4 = (byte >> 4) & 1;
	char D5 = (byte >> 3) & 1, D6 = (byte >> 2) & 1;
	char D7 = (byte >> 1) & 1, D8 = (byte >> 0) & 1;

	// calculez bitii de paritate
	char P1 = D1 ^ D2 ^ D4 ^ D5 ^ D7;
	char P2 = D1 ^ D3 ^ D4 ^ D6 ^ D7;
	char P4 = D2 ^ D3 ^ D4 ^ D8;
	char P8 = D5 ^ D6 ^ D7 ^ D8;

	// pun bitii de date si de paritate pe pozitiile respective in cei doi octeti
	encoded[0] |= (P1 << 3) | (P2 << 2) | (D1 << 1) | (P4 << 0);
	encoded[1] |= (D2 << 7) | (D3 << 6) | (D4 << 5) | (P8 << 4) | (D5 << 3) | (D6 << 2) | (D7 << 1) | (D8 << 0);

	// returnez cei doi bytes
	return encoded;
}

char* encoded_message(char* message) {
	int i;

	// mesajul codificat va avea lungime dubla
	char* encoded_message = (char *) calloc(2 * strlen(message) + 2, sizeof(char));

	// fiecare octet din mesaj il codific in doi octeti si ii pun in encoded_message
	for (i = 0; i <= strlen(message); i++) {
		encoded_message[2 * i] = coded_byte(message[i])[0];
		encoded_message[2 * i + 1] = coded_byte(message[i])[1];
	}

	// returnez mesajul codificat
	return encoded_message;
}


int test_hamming(char* string, msg r){
	int res, low, high, mid;

	msg t;

	char* token;

	// in a pun raspunsul meu
	char* a[MSGSIZE];

	int i = 0, j = 0, k = 0, n;

	// ca si in celelalte functii de mai sus, caut raspunsul dupa cuvantul cheie "word"
	// ce fac in plus, este sa trimit raspunsul codificat, in loc de cel normal
	while ((token = strsep(&string, " ")) != NULL) {

		j++;
		if (!strcmp(token, "word")) {
			j = i;
			i++;

		}

		if (j == i) {
			a[k++] = token;

		}

		if (!strncmp(token, "random", 6)) {
			char* pch, *pEnd;
			pch = strchr(r.payload,'[');
			low = r.payload[pch - r.payload + 1] - '0';

			high = (int) strtol(pch + 4, &pEnd, 10);

			mid = (low + high) / 2;

			// lungimea numarului, excluzand '\0'
			int len = sprintf(t.payload, "%d", mid);

			// pun in payload numarul codificat, dar codific si '\0'
			// de aceea lungimea este 2 * len + 2
			memcpy(r.payload, encoded_message(t.payload), 2 * len + 2);
			r.len = 2 * len + 2;

			// trimit raspunsul codificat
			res = send_message(&r);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}


		}

		if (!strncmp(r.payload, "smaller", 7)) {
			high = mid - 1;

			mid = (low + high) / 2;

			// lungimea numarului, excluzand '\0'
			int len = sprintf(t.payload,"%d", mid);

			// pun in payload numarul codificat, dar codific si '\0'
			// de aceea lungimea este 2 * len + 2
			memcpy(r.payload, encoded_message(t.payload), 2 * len + 2);
			r.len = 2 * len + 2;

			// trimit raspunsul codificat
			res = send_message(&r);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
		}

		if (!strncmp(r.payload, "bigger", 6)) {
			low = mid + 1;

			mid = (low + high) / 2;

			// lungimea numarului, excluzand '\0'
			int len = sprintf(t.payload, "%d", mid);

			// pun in payload numarul codificat, dar codific si '\0'
			// de aceea lungimea este 2 * len + 2
			memcpy(r.payload, encoded_message(t.payload), 2 * len + 2);
			r.len = 2 * len + 2;

			// trimit raspunsul codificat
			res = send_message(&r);

			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
		}

	}

	for (n = 0; n < k; n++) {

		// lungimea raspunsului, excluzand '\0'
		int len = sprintf(t.payload, "%s", a[n]);

		// pun in payload raspunsul codificat, dar codific si '\0'
		// de aceea lungimea este 2 * len + 2
		memcpy(r.payload, encoded_message(t.payload), 2 * len + 2);
		r.len = 2 * len + 2;

		// trimit raspunsul codificat
		res = send_message(&r);

		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}

	}
	return 0;
}

char* decoded_message(msg r) {
	// decodific mesajul trimis de client
	// la final iau bitii de date din fiecare doi octeti, si ii pun in cate un octet
	// astfel refac mesajul
	int i;

	// mesajul decodificat are lungimea len / 2
	char* decoded_message = (char *) calloc(r.len / 2, sizeof(char));

	// pentru fiecare octet din mesaj
	for (i = 0; i < r.len - 1; i = i + 2) {

		// suma care indica bit-ul eronat dintre cei 12 biti
		int sum = 0;

		// extrag bitii de date si de paritate din fiecare doi octeti consecutivi
		// e.g. 0 si 1, 2 si 3 etc.
		char sent_P4 = (r.payload[i] >> 0) & 1;
		char sent_D1 = (r.payload[i] >> 1) & 1;
		char sent_P2 = (r.payload[i] >> 2) & 1;
		char sent_P1 = (r.payload[i] >> 3) & 1;

		char sent_D8 = (r.payload[i + 1] >> 0) & 1;
		char sent_D7 = (r.payload[i + 1] >> 1) & 1;
		char sent_D6 = (r.payload[i + 1] >> 2) & 1;
		char sent_D5 = (r.payload[i + 1] >> 3) & 1;
		char sent_P8 = (r.payload[i + 1] >> 4) & 1;
		char sent_D4 = (r.payload[i + 1] >> 5) & 1;
		char sent_D3 = (r.payload[i + 1] >> 6) & 1;
		char sent_D2 = (r.payload[i + 1] >> 7) & 1;

		// calculez bitii mei de paritate in functie de bitii de date primiti
		char calculated_P1 = sent_D1 ^ sent_D2 ^ sent_D4 ^ sent_D5 ^ sent_D7;
		char calculated_P2 = sent_D1 ^ sent_D3 ^ sent_D4 ^ sent_D6 ^ sent_D7;
		char calculated_P4 = sent_D2 ^ sent_D3 ^ sent_D4 ^ sent_D8;
		char calculated_P8 = sent_D5 ^ sent_D6 ^ sent_D7 ^ sent_D8;

		// daca gasesc diferente intre ei, adun la suma indicii corespunzatori
		if ((calculated_P1 & 1) != (sent_P1 & 1)) {
			sum += 1;
		}
		if ((calculated_P2 & 1) != (sent_P2 & 1)) {
			sum += 2;
		}
		if ((calculated_P4 & 1) != (sent_P4 & 1)) {
			sum += 4;
		}
		if ((calculated_P8 & 1) != (sent_P8 & 1)) {
			sum += 8;
		}

		// daca exista erori, le corectez
		if (sum != 0) {
			// modific direct in cei doi octeti
			// daca sum + 3 < 8, atunci bit-ul se alfa in primul octet
			// adun 3 pentru ca 4 biti sunt nefolositi
			if ((sum + 3) < 8) {
				r.payload[i] ^= (128 >> (sum + 3));
			} else if ((sum + 3) >= 8) {
				// alfel se afla in al doilea, si fac % 8
				r.payload[i + 1] ^= (128 >> (sum + 3) % 8);
			}
		}

		// extrag bitii de date din nou(recorectati, daca e cazul)
		sent_D8 = (r.payload[i + 1] >> 0) & 1;
		sent_D7 = (r.payload[i + 1] >> 1) & 1;
		sent_D6 = (r.payload[i + 1] >> 2) & 1;
		sent_D5 = (r.payload[i + 1] >> 3) & 1;
		sent_D4 = (r.payload[i + 1] >> 5) & 1;
		sent_D3 = (r.payload[i + 1] >> 6) & 1;
		sent_D2 = (r.payload[i + 1] >> 7) & 1;
		sent_D1 = (r.payload[i] >> 1) & 1;

		// si ii pun in octetul aflat la pozitia i/2 din decoded_message 
		decoded_message[i / 2] |= (sent_D1 << 7) | (sent_D2 << 6) | (sent_D3 << 5) | (sent_D4 << 4) | (sent_D5 << 3) | (sent_D6 << 2) | (sent_D7 << 1) | (sent_D8 << 0);
	}

	// returnez mesajul decodificat
	return decoded_message;
}


int main(int argc, char** argv)
{
	msg r, t;
	int i, res, rez;

	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);

	for (i = 0; i < COUNT; i++) {

		/* wait for message */
		res = recv_message(&r);

		// in functie de test execut operatiile respective
		// test_0 folosesc atat la primul task cat si la al doilea
		if (argv[1] == NULL) {
			res = test_0(r.payload, r);
		} else if (!strcmp(argv[1], "ack")) {

			// daca primesc ACK, nu fac nimic
			// altfel trimit ACK + raspuns
			if (strncmp(r.payload, "ACK", 3)) {

				// trimit ACK
				sprintf(t.payload, "%s", "ACK");
				t.len = strlen(t.payload) + 1;
				rez = send_message(&t);

				if (rez < 0) {
					perror("[RECEIVER] Send ACK error. Exiting.\n");
					return -1;
				}

				// trimit raspuns
				res = test_0(r.payload, r);
			}
		} else if (!strncmp(argv[1], "parity", 6)) {

			// daca primesc ACK/NACK nu fac nimic
			// altfel verific mesajul trimis
			if (r.len != ACK_parity && r.len != NACK_parity) {

				int bit = 0;
				int k, j;

				// extrag bit-ul de paritate trimis de client
				int input_bit = (r.payload[0] >> 0) & 1;

				// calculez paritatea mesajului primit
				for (k = 1; k < r.len; k++) {
					for (j = 0; j < 8; j++) {
						bit ^= (r.payload[k] >> j) & 1;
					}
				}

				// daca paritatile sunt diferite, trimit NACK
				if (input_bit != bit) {

					sprintf(t.payload,"%s", "NACK");
					t.len = strlen(t.payload) + 1;

					rez = send_message(&t);

					if (rez < 0) {
						perror("[RECEIVER] Send ACK error. Exiting.\n");
						return -1;
					}
				} else if (input_bit == bit) {

					// altfel trimit ACK
					sprintf(t.payload,"%s", "ACK");
					t.len = strlen(t.payload) + 1;
					rez = send_message(&t);

					if (rez < 0) {
						perror("[RECEIVER] Send ACK error. Exiting.\n");
						return -1;
					}

					// si trimit si raspunsul(care include paritate)
					res = test_parity(r.payload + 1, r);

				}
			} else if (r.len == NACK_parity) {
				// acopar si acest caz
				// daca primesc NACK trimit din nou
				res = test_parity(r.payload, r);
			}
		} else if (!strncmp(argv[1], "hamming", 7)) {

			// daca primesc ACK/NACK nu fac nimic
			// altfel verific mesajul trimis
			// detectez erorile si le corectez(daca exista)
			if (r.len != ACK_hamming && r.len != NACK_hamming) {

				// trimit ACK
				sprintf(t.payload, "%s", "ACK");
				t.len = strlen(t.payload) + 1;
				rez = send_message(&t);

				if (rez < 0) {
					perror("[RECEIVER] Send ACK error. Exiting.\n");
					return -1;
				}

				// decodific mesajul(si il corectez eventual)
				char* message = decoded_message(r);

				// trimit raspuns la acest mesaj
				sprintf(r.payload, "%s", message);
				r.len = strlen(message) + 1;
				res = test_hamming(r.payload, r);
			}
		}

		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}

	}

	printf("[RECEIVER] Finished receiving..\n");

	return 0;
}
