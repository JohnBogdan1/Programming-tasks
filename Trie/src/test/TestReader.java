package test;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Citeste datele din fisier.
 * 
 * @author Stefan
 */
public class TestReader {

	private String[] words;
	private Command[] firstCommands;
	private Command[] secondCommands;

	/**
	 * 
	 * @param fileName
	 *            numele fisierului de intrare(un string).
	 * @exception FileNotFoundException
	 *                daca nu este gasit fisierul.
	 */
	public TestReader(String fileName) {
		try {
			Scanner in = new Scanner(new File(fileName));
			String text = in.nextLine();
			words = text.split(" ");
			firstCommands = parseCommands(in);
			secondCommands = parseCommands(in);
		} catch (FileNotFoundException ex) {
			Logger.getLogger(TestReader.class.getName()).log(Level.SEVERE, null, ex);
		}
	}

	/**
	 * @see Scanner
	 * @see #parseCommand(Scanner)
	 * @param s
	 *            un obiect Scanner.
	 * @return un vector de obiecte de tipul Command.
	 */
	private Command[] parseCommands(Scanner s) {
		int n = s.nextInt();
		Command[] commands = new Command[n];
		for (int i = 0; i < n; i++) {
			commands[i] = parseCommand(s);
		}
		return commands;
	}

	/**
	 * @see #parseCommands(Scanner)
	 * 
	 * @param s
	 *            un obiect de tip Scanner
	 * @return un obiect de tipul comenzii respective(add, count, remove, list)
	 *         , unde AddCommand, CountCommand, RemoveCommand, ListCommand
	 *         mostenesc clasa command.
	 */
	private Command parseCommand(Scanner s) {
		// type este tipul comenzii
		int type = s.nextInt();
		switch (type) {
		case Command.ADD:
			return new AddCommand(s.next());
		case Command.COUNT:
			return new CountCommand(s.next());
		case Command.REMOVE:
			return new RemoveCommand(s.next());
		case Command.LIST:
			return new ListCommand(s.next());
		}
		return null;
	}

	/**
	 * @see #words
	 * @return the words
	 */
	public String[] getWords() {
		return words;
	}

	/**
	 * @see #firstCommands
	 * @return the firstCommands
	 */
	public Command[] getFirstCommands() {
		return firstCommands;
	}

	/**
	 * @see #secondCommands
	 * @return the secondCommands
	 */
	public Command[] getSecondCommands() {
		return secondCommands;
	}

}
