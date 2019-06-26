package implementation;

import java.util.regex.Pattern;

import trie.TrieElement;

/**
 * Clasa Trie2 implementeaza TrieElement si elimina caracterele -_() dintr-un
 * string.
 * 
 * @author Johnny
 *
 */
public class Trie2 implements TrieElement {

	private String s;

	/**
	 * Setter care initializeaza s.
	 * 
	 * @param s
	 *            string dat ca parametru functiei.
	 */
	public void setS(String s) {
		this.s = s;
	}

	/**
	 * Getter care returneaza s.
	 * 
	 * @return string-ul s.
	 */
	public String getS() {
		return this.s;
	}

	@Override
	public char[] toCharArray() {
		// Am folosit o expresie regulata pentru eficienta
		return Pattern.compile("[-_()]+").matcher(this.s).replaceAll("").toCharArray();
	}

	/**
	 * Suprascrie metoda toString si afiseaza cuvantul s in loc de adresa
	 * obiectului.
	 * 
	 * @return string-ul s.
	 */
	public String toString() {
		return this.s;
	}
}
