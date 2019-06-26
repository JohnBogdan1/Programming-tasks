package io;

import java.io.File;
import java.io.IOException;
import java.util.Scanner;

/**
 * @author Johnny
 */
public final class Parser {
	/**
	 * Functia readFromFile, citeste un fisier si ii intoarce continutul sub
	 * forma de String.
	 * 
	 * @param path
	 *            numele caii catre fisier.
	 * @return un string.
	 * @throws IOException
	 */
	public static String readFromFile(String path) throws IOException {

		String input = "";

		Scanner in = new Scanner(new File(path));

		/*
		 * Adaug fiecare linie din fisier la un string si adaug un '\n' la
		 * sfarsit.
		 */
		while (in.hasNextLine()) {
			input += in.nextLine() + "\n";
		}

		// Inchid fisierul.
		in.close();

		return input;
	}
}
