package implementation;

/**
 * Clasa TrieNode implementeaza un nod dintr-un trie. Fiecare nod va avea un
 * char, un string, un int count care sa retina numarul de aparitii, o variabila
 * de tip boolean care sa verifice daca am ajuns la capatul cuvantului si
 * vectorul de noduri. In cazul nostru, doar ultimul nod dintr-o cale va avea un
 * string si un count specific, la restul nodurilor fiind nule.
 * 
 * @author Johnny
 *
 */
public class TrieNode {
	char letter;
	int count = 0;
	String string = null;
	TrieNode[] children;
	boolean fullWord = false;

	/**
	 * Atribuie nodului un caracter si un vector.
	 * 
	 * @param letter
	 *            este caracterul dat cu care este initializat atributul letter.
	 */
	TrieNode(char letter) {
		this.letter = letter;
		this.children = new TrieNode[68];
	}

	/**
	 * Atribuie nodului un string si un vector. Este folosit pentru nodul root.
	 * 
	 * @param s
	 *            este string-ul dat cu care este initializat atributul string.
	 */
	TrieNode(String s) {
		this.string = s;
		this.children = new TrieNode[68];
	}
}
