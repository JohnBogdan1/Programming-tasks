package visualiser;

import java.util.List;

/**
 * Factory Pattern.
 * 
 * @author Johnny
 *
 */
public class ZoneFactory {
	/**
	 * Returneaza o instanta a zonei in care se afla primul element din lista.
	 * 
	 * @param elements
	 *            o lista de elemente(EditorElement)
	 * @return o instanta a zonei(EditorArea)
	 */
	public static EditorArea createZone(List<EditorElement> elements) {

		for (EditorElement ee : elements) {
			if (ee.getType().contains("card") || ee.getType().contains("input") || ee.getType().contains("page")) {
				return new Canvas(elements);
			} else if ((ee.getType().contains("menu") && !ee.getType().contains("menu-button"))
					|| ee.getType().contains("menu-button") || ee.getType().contains("icon")) {
				return new Menu(elements);
			} else if (ee.getType().contains("dialog")) {
				return new DialogBox(elements);
			} else
				return new UnknownArea(elements);
		}
		return null;
	}
}
