package test;

/**
 *
 * @author Johnny
 */
public class ListCommand extends Command {

	/**
	 * Seteaza word la parametrul dat, iar type la tipul comenzii.
	 * 
	 * @param word
	 *            este un string dat care initializeaza atributul word.
	 */
	public ListCommand(String word) {
		this.word = word;
		this.type = Command.LIST;
	}
}
