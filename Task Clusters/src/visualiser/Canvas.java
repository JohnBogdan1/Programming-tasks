package visualiser;

import java.util.List;

/**
 * Clasa care defineste o zona din editor.
 * 
 * @author Johnny
 *
 */
public final class Canvas extends EditorArea {

	public Canvas(List<EditorElement> pathInPage) {
		super(pathInPage);
	}

	/**
	 * Returneaza culoarea zonei.
	 */
	@Override
	public Color getVisualisationColor() {
		return Color.RED;
	}
}
