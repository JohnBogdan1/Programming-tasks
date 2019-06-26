package visualiser;

import java.util.List;

/**
 * Clasa care defineste o zona din editor.
 * 
 * @author Johnny
 *
 */
public final class Menu extends EditorArea {

	public Menu(List<EditorElement> pathInPage) {
		super(pathInPage);
	}

	/**
	 * Returneaza culoarea zonei.
	 */
	@Override
	public Color getVisualisationColor() {
		return Color.GREEN;
	}
}
