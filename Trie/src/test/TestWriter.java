package test;

import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.logging.Level;
import java.util.logging.Logger;

import trie.TrieElement;

/**
 * Scrie date in fisier.
 * 
 * @author Stefan
 */
public class TestWriter {

	private PrintWriter out = null;

	/**
	 * 
	 * @param filename
	 *            numele fisierului de iesire(un string)
	 * @exception FileNotFoundException
	 *                daca nu este gasit fisierul
	 */
	public TestWriter(String filename) {
		try {
			out = new PrintWriter(filename);

		} catch (FileNotFoundException ex) {
			Logger.getLogger(TestWriter.class.getName()).log(Level.SEVERE, null, ex);
		}
	}

	/**
	 * @param count
	 *            numarul de aparitii ale unui cuvant
	 */
	public void printCount(int count) {
		out.println(count);
	}

	/**
	 * @see #out
	 * @param words
	 *            un vector care contine cuvintele care incep cu prefixul dat
	 */
	public void printSortedWords(TrieElement[] words) {
		for (TrieElement word : words) {
			out.print(word + " ");
		}
		out.println();
	}

	/**
	 * Inchide fisier
	 */
	public void close() {
		out.close();
	}

}
