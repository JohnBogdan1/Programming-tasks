package implementation;

/**
 * Fiecare litera are o codificare de la 1 la 68(dimensiunea alfabetului
 * folosit), aceste codificari respectand ordinea caracterelor din ASCII.
 * 
 * @author Johnny
 *
 */
public class Codifications {

	private char letter;

	/**
	 * Constructor care initializeaza atributul letter.
	 * 
	 * @param c
	 *            caracterul care este dat ca parametru functiei.
	 */
	public Codifications(char c) {
		this.letter = c;
	}

	/**
	 * 
	 * @return o codificare(intreg) a caracterului.
	 */
	public char encode() {
		switch (this.letter) {
		case '!':
			return 1;
		case '(':
			return 2;
		case ')':
			return 3;
		case '-':
			return 4;
		case '0':
			return 5;
		case '1':
			return 6;
		case '2':
			return 7;
		case '3':
			return 8;
		case '4':
			return 9;
		case '5':
			return 10;
		case '6':
			return 11;
		case '7':
			return 12;
		case '8':
			return 13;
		case '9':
			return 14;
		case '?':
			return 15;
		case 'A':
			return 16;
		case 'B':
			return 17;
		case 'C':
			return 18;
		case 'D':
			return 19;
		case 'E':
			return 20;
		case 'F':
			return 21;
		case 'G':
			return 22;
		case 'H':
			return 23;
		case 'I':
			return 24;
		case 'J':
			return 25;
		case 'K':
			return 26;
		case 'L':
			return 27;
		case 'M':
			return 28;
		case 'N':
			return 29;
		case 'O':
			return 30;
		case 'P':
			return 31;
		case 'Q':
			return 32;
		case 'R':
			return 33;
		case 'S':
			return 34;
		case 'T':
			return 35;
		case 'U':
			return 36;
		case 'V':
			return 37;
		case 'W':
			return 38;
		case 'X':
			return 39;
		case 'Y':
			return 40;
		case 'Z':
			return 41;
		case '_':
			return 42;
		case 'a':
			return 43;
		case 'b':
			return 44;
		case 'c':
			return 45;
		case 'd':
			return 46;
		case 'e':
			return 47;
		case 'f':
			return 48;
		case 'g':
			return 49;
		case 'h':
			return 50;
		case 'i':
			return 51;
		case 'j':
			return 52;
		case 'k':
			return 53;
		case 'l':
			return 54;
		case 'm':
			return 55;
		case 'n':
			return 56;
		case 'o':
			return 57;
		case 'p':
			return 58;
		case 'q':
			return 59;
		case 'r':
			return 60;
		case 's':
			return 61;
		case 't':
			return 62;
		case 'u':
			return 63;
		case 'v':
			return 64;
		case 'w':
			return 65;
		case 'x':
			return 66;
		case 'y':
			return 67;
		case 'z':
			return 68;
		default:
			return 0;

		}
	}
}
