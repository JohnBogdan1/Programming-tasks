package visualiser;

import java.util.List;

/**
 * Clasa care defineste o zona din editor.
 * 
 * @author Johnny
 *
 */
final class UnknownArea extends EditorArea {

	public UnknownArea(List<EditorElement> pathInPage) {
		super(pathInPage);
	}

	/**
	 * Returneaza culoarea zonei.
	 */
	@Override
	public Color getVisualisationColor() {
		return Color.GREY;
	}

}
