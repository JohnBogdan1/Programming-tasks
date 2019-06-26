package implementation;

import java.util.ArrayList;

import trie.AbstractTrie;
import trie.TrieElement;

/**
 * Clasa care implementeaza interfata AbstractTrie
 * 
 * @author Johnny
 *
 */
public class TrieFunctions implements AbstractTrie {

	// root este radacina trie-ului.
	private TrieNode root;

	// nrElements este folosit pentru a numara numarul de elemente adaugate.
	private int nrElements = 0;

	/**
	 * Constructor care initializeaza root.
	 */
	public TrieFunctions() {
		root = new TrieNode("");
	}

	/**
	 * Intoarce stringul mai mic din punct de vedere lexicografic.
	 * 
	 * @param oldS
	 *            este vechiul string.
	 * @param newS
	 *            este stringul cu care este comparat oldS.
	 */
	public String lexicographicalOrder(String oldS, String newS) {
		if (oldS.compareTo(newS) < 0)
			return oldS;
		else if (oldS.compareTo(newS) > 0)
			return newS;
		return oldS;

	}

	@Override
	public void add(TrieElement element) {

		/*
		 * Convertesc elementul in functie de implementare(element poate fi de
		 * tipul Trie1, respectiv Trie2).
		 */
		char[] arrayLetters = null;
		if (element instanceof Trie1)
			arrayLetters = element.toCharArray();
		else if (element instanceof Trie2)
			arrayLetters = element.toCharArray();

		// Setez nodul curent la root.
		TrieNode currentNode = root;

		/*
		 * Codifications este clasa care implementeaza codificarile
		 * caracterelor.
		 */
		Codifications c;

		/*
		 * Pentru fiecare caracter din cuvant, fac verificarea daca exista in
		 * vectorul din nodul curent. Daca exista, trec pe urmatorul nivel,
		 * altfel creez un nou nod.
		 */
		for (char character : arrayLetters) {

			// Preiau pozitia data de codificare pentru a adauga in vector.
			c = new Codifications(character);
			int index = c.encode() - 1;

			if (currentNode.children[index] == null) {
				currentNode.children[index] = new TrieNode(character);
			}
			currentNode = currentNode.children[index];
		}

		/*
		 * Daca nu exista un string, adaug. Altfel, compar string-urile din
		 * punct de vedere lexicografic si il adaug pe cel mai mic.
		 */
		if (currentNode.string == null) {
			if (element instanceof Trie1)
				currentNode.string = ((Trie1) element).getS();
			else if (element instanceof Trie2)
				currentNode.string = ((Trie2) element).getS();
		} else {
			if (element instanceof Trie1)
				currentNode.string = lexicographicalOrder(currentNode.string, ((Trie1) element).getS());
			else if (element instanceof Trie2)
				currentNode.string = lexicographicalOrder(currentNode.string, ((Trie2) element).getS());
		}

		/*
		 * Incrementez numarul de intrari echivalente in Trie si atasez nodului
		 * curent atributul de cuvant intreg(am adaugat intreg cuvantul).
		 */
		currentNode.count++;
		currentNode.fullWord = true;
	}

	/**
	 * Parcurge in pre-ordine a arborele.
	 * 
	 * @param root
	 *            nodul de la care incepe cautarea cuvintelor care au prefixul
	 *            respectiv.
	 * @param trieElement
	 *            un ArrayList de string-uri in care pun cuvintele gasite.
	 * @return trieElement
	 */
	public ArrayList<String> wordsWithSamePrefix(TrieNode root, ArrayList<String> trieElement) {

		if (root == null)
			return null;

		/*
		 * Daca am gasit un nod care e leaf(ultimul nod in care se afla cuvantul
		 * stocat), adaug cuvantul.
		 */
		if (root.string != null && root.fullWord) {
			// Adauga cuvant in lista, daca numarul de aparitii e mai mare ca 0.
			if (root.count > 0) {
				trieElement.add(root.string);

				// Incrementez numarul de elemente adaugate.
				this.nrElements++;
			}
		}

		/*
		 * Verific toate nodurile recursiv din vectorii(copii) asociati
		 * nodurilor parinte respective.
		 */
		for (TrieNode node : root.children) {
			wordsWithSamePrefix(node, trieElement);
		}

		return trieElement;
	}

	/**
	 * 
	 * @param element
	 *            este elementul(cuvantul) care trebuie cautat in arbore.
	 * @return nodul la care se gaseste elementul(cuvantul) stocat.
	 */
	public TrieNode search(TrieElement element) {

		// Convertesc elementul in functie de implementare(tipul obiectului
		// element).
		char[] arrayLetters = null;

		if (element instanceof Trie1)
			arrayLetters = element.toCharArray();
		else if (element instanceof Trie2)
			arrayLetters = element.toCharArray();

		// Setez nodul curent la root.
		TrieNode currentNode = root;

		/*
		 * Codifications este clasa care implementeaza codificarile
		 * caracterelor.
		 */
		Codifications c;

		/*
		 * Pentru fiecare caracter din cuvant, trec la urmatorul nivel pe nodul
		 * respectiv din vector dat de codificare. Daca nu exista, ma opresc si
		 * returnez null(nu s-a gasit).
		 */
		for (char charC : arrayLetters) {
			if (currentNode == null)
				return null;
			c = new Codifications(charC);
			int index = c.encode() - 1;
			currentNode = currentNode.children[index];
		}

		if (currentNode == null)
			return null;

		/*
		 * Daca am gasit un prefix pe care il cautam, returnez nodul.
		 * 
		 * Altfel, inseamna ca am gasit nodul in care se afla cuvantul pe care
		 * il cautam si il returnez.
		 */
		if (currentNode != null && !currentNode.fullWord)
			return currentNode;
		else
			return currentNode;

	}

	@Override
	public int count(TrieElement element) {
		TrieNode node = search(element);
		if (node != null) {
			return node.count;
		}
		return 0;

	}

	@Override
	public void remove(TrieElement element) {
		TrieNode node = search(element);
		if (node != null) {
			if (node.count > 0)
				node.count--;
			/*
			 * Pun un caracter in string cu un cod ASCII superior alfabetului
			 * meu pentru a putea fi inlocuit de metoda lexicographicalOrder in
			 * cazul aparitiei unui nou cuvant echivalent a carui cale sa duca
			 * la acest nod.
			 *
			 */
			if (node.count == 0) {
				node.string = "~";
				node.fullWord = false;
			}
		}

	}

	@Override
	public TrieElement[] getSortedElements(TrieElement prefix) {

		// Returnez nodul in care se afla ultimul caracter din prefix.
		TrieNode currentNode = search(prefix);

		ArrayList<String> trieElement = new ArrayList<>();

		// Pornesc cautarea de la acel nod.
		trieElement = wordsWithSamePrefix(currentNode, trieElement);

		/*
		 * nrElements reprezinta numarul de elemente care trebuie adaugate in
		 * vector. Daca nrElements e 0, nu se va afisa nimic in fisier deoarece
		 * este un vector gol.
		 */
		TrieElement[] trie = new TrieElement[this.nrElements];
		int i = 0;

		/*
		 * Iau fiecare string din ArrayList, creez un obiect al unei clase care
		 * implementeaza TrieElement, setez atributul s(string) la stringul
		 * respectiv si adaug obiectul in vectorul TrieElement[].
		 */

		if (this.nrElements > 0 && trieElement != null) {
			for (String s : trieElement) {
				/*
				 * Clasa poate fi atat Trie1, cat si Trie2. Aceasta instanta o
				 * folosesc doar pentru a prelua cuvintele din ArrayList.
				 */
				Trie1 p = new Trie1();

				// Setez atributul String s al obiectului p.
				p.setS(s);

				// Adaug obiectul in vector.
				trie[i++] = p;
			}
		}

		// Setez numarul de elemente la 0.
		this.nrElements = 0;

		return trie;
	}

}
