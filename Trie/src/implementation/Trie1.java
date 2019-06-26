package implementation;

import trie.TrieElement;

/**
 * Clasa Trie1 implementeaza TrieElement si transforma un string in lowercase.
 * 
 * @author Johnny
 *
 */
public class Trie1 implements TrieElement {

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
		return this.s.toLowerCase().toCharArray();
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
