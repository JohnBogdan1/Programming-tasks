package test;

/**
 *
 * @author Stefan
 */
public class AddCommand extends Command {

	/**
	 * Seteaza word la parametrul dat, iar type la tipul comenzii.
	 * 
	 * @param word
	 *            este un string dat care initializeaza atributul word.
	 */
	public AddCommand(String word) {
		this.word = word;
		this.type = Command.ADD;
	}
}
