package test;

/**
 * Returneaza tipul comenzii(add, count, remove, list) si un cuvant asupra
 * caruia este executata comanda. Aceste date sunt preluate din fisierul de
 * input.
 * 
 * @author Stefan
 */
public abstract class Command {

	public static final int ADD = 0;
	public static final int COUNT = 2;
	public static final int REMOVE = 1;
	public static final int LIST = 3;

	protected String word;
	protected int type;

	/**
	 * 
	 * @return un cuvant.
	 */
	public String getWord() {
		return word;
	}

	/**
	 * @return tipul comenzii.
	 */
	public int getType() {
		return type;
	}
}
