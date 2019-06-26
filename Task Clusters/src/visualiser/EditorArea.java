package visualiser;

import java.util.List;

/**
 * Clasa care defineste o zona.
 * 
 * @author Johnny
 *
 */
abstract class EditorArea {

	// Lista de elemente dintr-un eveniment.
	private final List<EditorElement> pathInEditor;

	/**
	 * Constructor cu parametrii.
	 * 
	 * @param pathInEditor
	 *            o lista de elemente.
	 */
	public EditorArea(List<EditorElement> pathInEditor) {
		this.pathInEditor = pathInEditor;
	}

	/**
	 * Getter.
	 * 
	 * @return o lista de elemente.
	 */
	public List<EditorElement> getPathInEditor() {
		return pathInEditor;
	}

	/**
	 * Metoda abstracta implementata de metodele care extind EditorArea.
	 * 
	 * @return o culoare din Color.
	 */
	public abstract Color getVisualisationColor();

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((pathInEditor == null) ? 0 : pathInEditor.hashCode());
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
		EditorArea other = (EditorArea) obj;
		if (pathInEditor == null) {
			if (other.pathInEditor != null)
				return false;
		} else if (!pathInEditor.equals(other.pathInEditor))
			return false;
		return true;
	}

}
