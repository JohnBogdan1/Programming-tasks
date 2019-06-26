package visualiser;

/**
 * Clasa care defineste un element.
 * 
 * @author Johnny
 *
 */
public final class EditorElement {

	// Tipul elementului.
	private final String type;

	/**
	 * Constructor cu parametrii.
	 * 
	 * @param type
	 *            un string.
	 */
	public EditorElement(String type) {
		this.type = type;
	}

	/**
	 * Getter.
	 * 
	 * @return tipul elementului(numele).
	 */
	public String getType() {
		return type;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((type == null) ? 0 : type.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		EditorElement other = (EditorElement) obj;
		if (type == null) {
			if (other.type != null)
				return false;
		} else if (!type.equals(other.type))
			return false;
		return true;
	}

}
