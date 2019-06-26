package trie;

/**
 *
 * @author Stefan
 */
public interface AbstractTrie {

	/**
	 * Adauga un TrieElement in trie.
	 * 
	 * @param element
	 *            este elementul(cuvantul) folosit pentru a adauga o cale in
	 *            trie.
	 */
	public abstract void add(TrieElement element);

	/**
	 * @param element
	 *            este cuvantul care trebuie cautat.
	 * @return numarul de aparitii ale cuvantului.
	 */
	public abstract int count(TrieElement element);

	/**
	 * Sterge o aparitie a cuvantului din arbore.
	 * 
	 * @param element
	 *            este cuvantul care trebuie cautat.
	 */
	public abstract void remove(TrieElement element);

	/**
	 * @param prefix
	 *            este prefixul dupa care trebuie cautate celelalte cuvinte din
	 *            arbore.
	 * @return TrieElement[], un vector de obiecte TrieElement.
	 */
	public abstract TrieElement[] getSortedElements(TrieElement prefix);

}
