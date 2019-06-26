package test;

/**
 *
 * @author Stefan
 */
public class RemoveCommand extends Command {

	/**
	 * Seteaza word la parametrul dat, iar type la tipul comenzii.
	 * 
	 * @param word
	 *            este un string dat care initializeaza atributul word.
	 */
	public RemoveCommand(String word) {
		this.word = word;
		this.type = Command.REMOVE;
	}
}
