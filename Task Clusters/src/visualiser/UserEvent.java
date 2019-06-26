package visualiser;

/**
 * Clasa care defineste un eveniment.
 * 
 * @author Johnny
 *
 */
public final class UserEvent {
	// Zona + timestamp-ul evenimentului.
	private EditorArea area;
	private int timestamp;

	/**
	 * Constructor fara argumente.
	 */
	public UserEvent() {
	}

	/**
	 * Constructor cu argumente.
	 * 
	 * @param area
	 *            o zona(EditorArea).
	 * @param timestamp
	 *            un intreg.
	 */
	public UserEvent(EditorArea area, int timestamp) {
		this.area = area;
		this.timestamp = timestamp;
	}

	/**
	 * Getter.
	 * 
	 * @return o zona.
	 */
	public EditorArea getPageArea() {
		return area;
	}

	/**
	 * Setter.
	 * 
	 * @param area
	 *            o zona(EditorArea).
	 */
	public void setPageArea(EditorArea area) {
		this.area = area;
	}

	/**
	 * Getter.
	 * 
	 * @return timestamp-ul evenimentului.
	 */
	public int getTimestamp() {
		return timestamp;
	}

	/**
	 * Setter.
	 * 
	 * @param timestamp
	 *            un intreg.
	 */
	public void setTimestamp(int timestamp) {
		this.timestamp = timestamp;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((area == null) ? 0 : area.hashCode());
		result = prime * result + timestamp;
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
		UserEvent other = (UserEvent) obj;
		if (area == null) {
			if (other.area != null)
				return false;
		} else if (!area.equals(other.area))
			return false;
		if (timestamp != other.timestamp)
			return false;
		return true;
	}

}
