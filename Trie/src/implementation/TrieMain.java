package implementation;

import test.Command;
import test.TestReader;
import test.TestWriter;

/**
 * Clasa principala care executa programul.
 * 
 * @author Johnny
 *
 */
public class TrieMain {

	public static void main(String[] args) {

		// Clasa TestReader prea informatiile din fisierul de intrare.
		TestReader read = new TestReader("trie.in");

		// Clasa TestWriter va scrie output-ul in fisierul de iesire.
		TestWriter write = new TestWriter("trie.out");

		// Clasa TrieFunctions implementeaza interfata AbstractTrie.
		// Prima implementare a trie-ului.
		TrieFunctions tf1 = new TrieFunctions();

		// Clasa Trie1 implementeaza interfata TrieElement.
		// Instantierea este doar initiala.
		Trie1 p = new Trie1();

		// Adaug fiecare cuvant din lista in trie.
		for (String word : read.getWords()) {
			// Setez atributul String s al obiectului p.
			p.setS(word);
			tf1.add(p);
		}

		// Identific comanda respectiva si o execut.
		for (Command command : read.getFirstCommands()) {
			if (command.getType() == Command.ADD) {
				// Adaug un cuvant in trie.
				p.setS(command.getWord());
				tf1.add(p);
			} else if (command.getType() == Command.COUNT) {
				// Afisez numarul de aparitii ale cuvantului.
				p.setS(command.getWord());
				write.printCount(tf1.count(p));
			} else if (command.getType() == Command.REMOVE) {
				// Sterg o aparitie a cuvantului din lista.
				p.setS(command.getWord());
				tf1.remove(p);
			} else if (command.getType() == Command.LIST) {
				// Afisez cuvintele din trie care incep cu prefixul dat.
				p.setS(command.getWord());
				write.printSortedWords(tf1.getSortedElements(p));

			}
		}

		// A doua implementare a trie-ului.
		TrieFunctions tf2 = new TrieFunctions();

		// Clasa Trie1 implementeaza interfata TrieElement.
		// Instantierea este doar initiala.
		Trie2 q = new Trie2();

		// Adaug fiecare cuvant din lista in trie.
		for (String word : read.getWords()) {
			// Setez atributul String s al obiectului q.
			q.setS(word);
			tf2.add(q);
		}

		// Identific comanda respectiva si o execut.
		for (Command command : read.getSecondCommands()) {
			if (command.getType() == Command.ADD) {
				// Adaug un cuvant in trie
				q.setS(command.getWord());
				tf2.add(q);
			} else if (command.getType() == Command.COUNT) {
				// Afisez numarul de aparitii ale cuvantului.
				q.setS(command.getWord());
				write.printCount(tf2.count(q));

			} else if (command.getType() == Command.REMOVE) {
				// Sterg o aparitie a cuvantului din lista.
				q.setS(command.getWord());
				tf2.remove(q);
			} else if (command.getType() == Command.LIST) {
				// Afisez cuvintele din trie care incep cu prefixul dat.
				q.setS(command.getWord());
				write.printSortedWords(tf2.getSortedElements(q));

			}
		}

		// Inchid fisierul de iesire dupa scrierea datelor.
		write.close();

	}
}