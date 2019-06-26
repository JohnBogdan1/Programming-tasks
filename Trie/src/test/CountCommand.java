package test;

/**
 *
 * @author Stefan
 */
public class CountCommand extends Command {

	/**
	 * Seteaza word la parametrul dat, iar type la tipul comenzii.
	 * 
	 * @param word
	 *            este un string dat care initializeaza atributul word.
	 */
	public CountCommand(String word) {
		this.word = word;
		this.type = Command.COUNT;
	}
}
