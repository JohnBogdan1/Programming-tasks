package trie;

/**
 *
 * @author Stefan
 */
public interface TrieElement {

	/**
	 * @return string-ul in format lowercase si convertit la char[] pentru prima
	 *         implementare(Trie1), iar pentru a doua implementare(Trie2)
	 *         string-ul fara caracterele -_() si convertit la char[].
	 */
	public char[] toCharArray();

}
