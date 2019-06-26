package visualiser;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.collect.Maps;

/**
 * 
 * @author Johnny
 *
 */
public final class Task {

	// O lista de evenimente.
	private final List<UserEvent> userEvents;

	/**
	 * 
	 * @return lista de evenimente.
	 */
	public List<UserEvent> getUserEvents() {
		return userEvents;
	}

	/**
	 * Constructor fara parametrii. Acesta initializeaza lista de evenimente.
	 */
	public Task() {
		this.userEvents = new ArrayList<UserEvent>();

	}

	/**
	 * Constructor cu parametru.
	 * 
	 * @param logs
	 *            logurile din fisier sub forma de string.
	 */
	public Task(String logs) {
		this.userEvents = parseLogs(logs);
	}

	/**
	 * Metoda care va calcula frecventa medie pe intervale de 10 secunde.
	 * 
	 * @return
	 */
	public Double meanFrequencyPerTenSeconds() {
		List<UserEvent> events = this.userEvents;

		// Suma de frecvente pe fiecare interval.
		double frequencies = 0;

		// Numarul de Timestamps pe interval + numarul de intervale
		int nrTimeStampsPerInterval = 0, nrIntervals = 0;

		// O folosesc pentru a verifica daca am parcurs un interval.
		boolean uncheckedInterval = false;

		// Valoarea initiala.
		int initialValue = events.get(0).getTimestamp();

		// Valoarea finala.
		int finalValue = initialValue + 9;

		/*
		 * Pentru fiecare UserEvent calculez frecventa si o adun la frequencies
		 * si numarul de intervale.
		 */
		for (UserEvent ue : events) {

			// Daca Timestamp-ul curent e mai mare ca valoarea finala
			// Inseamna ca trec in alt interval.
			if (ue.getTimestamp() > finalValue) {
				uncheckedInterval = true;
			}

			// Cand trec in alt interval, calculez ce am parcurs pana acum.
			if (uncheckedInterval == true) {

				// Adun frecventa
				frequencies += (double) nrTimeStampsPerInterval / 10;

				// Incrementez numarul de intervale.
				nrIntervals++;

				// Resetez numarul de Timestamps.
				nrTimeStampsPerInterval = 0;

				// Setez valoarea finala.
				finalValue += +10;
			}

			/*
			 * Incrementez numarul de Timestamps(un Timestamp corespunde la un
			 * eveniment)
			 */
			nrTimeStampsPerInterval++;

			// Setez uncheckedInterval ca fiind false pentru intervalul curent.
			uncheckedInterval = false;
		}

		// Adun ce mai ramane.
		frequencies += (double) nrTimeStampsPerInterval / 10;
		nrIntervals++;

		return frequencies / nrIntervals;
	}

	/**
	 * Calculeaza numarul de click-uri date pe fiecare zona a editorului.
	 * 
	 * @return o mapare intre o zona a editorului si numarul de click-uri.
	 */
	public Map<String, Double> computeClicksPerArea() {

		// Lista de evenimente.
		List<UserEvent> events = this.userEvents;

		// Maparea intre o zona a editorului si numarul de click-uri.
		Map<String, Double> map = Maps.newHashMap();

		// Initializez cate o variabila pentru fiecare zona
		// Pe care o incrementez, cand gasesc zona respectiva.
		double canvas = 1, menu = 1, dialog = 1, unknown = 1;

		/*
		 * Parcurg lista de evenimente, verific daca este o zona anume si adaug
		 * in map.
		 */
		for (UserEvent ue : events) {
			if (printName(ue.getPageArea()) == Canvas.class.getCanonicalName()) {
				map.put(Canvas.class.getCanonicalName(), canvas++);
			} else if (printName(ue.getPageArea()) == Menu.class.getCanonicalName()) {
				map.put(Menu.class.getCanonicalName(), menu++);
			} else if (printName(ue.getPageArea()) == DialogBox.class.getCanonicalName()) {
				map.put(DialogBox.class.getCanonicalName(), dialog++);
			} else if (printName(ue.getPageArea()) == UnknownArea.class.getCanonicalName()) {
				map.put(UnknownArea.class.getCanonicalName(), unknown++);
			}
		}

		return map;
	}

	/**
	 * 
	 * @param area
	 *            un obiect.
	 * @return numele canonic al clasei..
	 */
	public String printName(Object area) {
		return area.getClass().getCanonicalName();
	}

	/**
	 * Metoda care va parsa log-urile primite in constructor
	 * 
	 * @param logs
	 *            un string de log-uri.
	 * @return o lista de evenimente.
	 */
	@VisibleForTesting
	List<UserEvent> parseLogs(String logs) {

		UserEvent ue = null;
		List<UserEvent> userEvents = new ArrayList<UserEvent>();
		List<EditorElement> pathInPage = null;

		// Impart string-ul intr-o lista de string-uri
		// Elementul de separare este "\n"
		for (String s : logs.split("\n")) {
			// Daca string-ul contine "user_event {", creez un nou eveniment
			// Si o noua lista de elemente.
			if (s.contains("user_event {")) {
				ue = new UserEvent();
				pathInPage = new ArrayList<EditorElement>();
			} else if (s.contains("element")) {
				// Daca string-ul contine "element", adaug un element in lista.
				int index = s.indexOf(':');
				pathInPage.add(new EditorElement(s.substring(index + 2, s.length())));
			} else if (s.contains("timestamp")) {
				/*
				 * Daca string-ul contine "timestamp", setez timestamp-ul pentru
				 * acest eveniment.
				 */
				int index = s.indexOf(':');
				ue.setTimestamp(Integer.parseInt(s.substring(index + 2)));
			} else if (s.contains("} user_event")) {
				// Setez zona evenimentului
				// Ma folosesc de un Factory Pattern.
				ue.setPageArea(ZoneFactory.createZone(pathInPage));

				// Adaug evenimentul in lista.
				userEvents.add(ue);
			}
		}

		return userEvents;
	}

	/**
	 * Metoda care determina zona din editor unde s-a dat click, in functie de
	 * lista de elemente ce identifica in mod unic zona respectiva.
	 * 
	 * @param elements
	 *            o lista de elemente.
	 * @return o zona.
	 */
	@VisibleForTesting
	EditorArea determineAreaForElements(List<EditorElement> elements) {

		/*
		 * Verific doar primul element si returnez o instanta a zonei
		 * respective.
		 */
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
