package visualiser;

import graphics.Point;
import graphics.Renderer;
import io.Parser;
import visualiser.ClusterManager.Cluster;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.swing.JFrame;

/**
 * 
 * @author Johnny
 *
 */
final class Main {
	private static Renderer renderer;

	public static void main(String[] args) {
		// Preiau task-urile.
		List<Task> tasks = readTasks("logs/alfred");

		/*
		 * Definesc o mapare intre culorile disponibile in enumeratia Color si
		 * culorile echivalente disponibile in pachetul awt.
		 */
		Map<Color, java.awt.Color> map = new HashMap<>();

		map.put(Color.RED, new java.awt.Color(255, 0, 0));
		map.put(Color.BLUE, new java.awt.Color(0, 0, 255));
		map.put(Color.GREEN, new java.awt.Color(0, 128, 0));
		map.put(Color.YELLOW, new java.awt.Color(255, 255, 0));
		map.put(Color.GREY, new java.awt.Color(128, 128, 128));

		/*
		 * Completez cu dimensiunile potrivite Apelez drawAll pentru a desena
		 * toate punctele si drawClusters pentru a desena clusterele Am folosit
		 * un Fluent Builder
		 */
		renderer = new Renderer.Builder()
				.withCircleSize(10)
				.withMap(map)
				.withWindowSize(1500, 300)
				.withDefaultCloseOperation(JFrame.EXIT_ON_CLOSE)
				.withTitle("All data")
				.build();

		drawAll(tasks);
		renderer.draw();

		renderer = new Renderer.Builder()
				.withCircleSize(10)
				.withMap(map)
				.withWindowSize(1500, 300)
				.withDefaultCloseOperation(JFrame.EXIT_ON_CLOSE)
				.withTitle("Clusters")
				.build();

		drawClusters(tasks);
		renderer.draw();
	}

	/**
	 * Functia care creeaza o lista de task-uri.
	 * 
	 * @param path
	 *            un string, care reprezinta calea catre fisiere.
	 * @return o lista de task-uri.
	 */
	private static List<Task> readTasks(String path) {
		File[] filesList = new File(path).listFiles();
		List<Task> tasks = new ArrayList<>();
		Task task = null;

		if (filesList == null) {
			return tasks;
		}

		try {
			// Apelez consecutiv functia readFromFile, pentru fiecare fisier in
			// parte.
			for (File file : filesList) {

				// In clasa Task se va apela metoda de parsare a logurilor pe
				// string-ul respectiv.
				task = new Task(Parser.readFromFile(file.getName()));

				// Adaug un task in lista de task-uri.
				tasks.add(task);
			}
		} catch (IOException e) {
			e.printStackTrace();
		}

		return tasks;
	}

	/**
	 * Functia trebuie sa adauge, pe rand, cate un punct din log-urile parsate
	 * intr-o instanta a clasei Renderer, utilizand valori alese conventional
	 * pentru x si y. Culoarea utilizata va fi cea intoarsa de suprafata din
	 * editor pe care s-a dat click.
	 * 
	 * @param tasks
	 *            o lista de task-uri.
	 */
	private static void drawAll(List<Task> tasks) {

		// Initializez o lista de puncte petnru acest renderer.
		renderer.setPoints(new ArrayList<Point>());

		// Pentru fiecare UserEvent din lista de userEvents din fiecare task,
		// adaug cate un punct in lista.
		for (Task task : tasks) {
			for (UserEvent ue : task.getUserEvents()) {
				renderer.addPoint(getXValueFor(ue), getYValueFor(ue), ue.getPageArea().getVisualisationColor());
			}

		}
	}

	/**
	 * Functia care deseneaza mai intai toate punctele si apoi clusterele.
	 * 
	 * @param tasks
	 *            o lista de task-uri.
	 */
	private static void drawClusters(List<Task> tasks) {

		renderer.setPoints(new ArrayList<Point>());

		ArrayList<Color> colors = new ArrayList<>();

		// Creez o lista de culori pe care o voi parcurge circular mai jos..
		for (Color color : Color.values()) {
			if (color != Color.GREY)
				colors.add(color);
		}

		// Intai desenez toate punctele cu gri.
		for (Task task : tasks) {
			for (UserEvent ue : task.getUserEvents()) {
				renderer.addPoint(getXValueFor(ue), getYValueFor(ue), Color.GREY);
			}

		}

		int i = 0;

		ClusterManager cm;

		// Desenez clusterele.
		for (Task task : tasks) {
			cm = new ClusterManager(10);

			for (Cluster cluster : cm.cluster(task.getUserEvents())) {
				for (UserEvent ue : cluster.getUserEvents()) {
					// Daca am ajuns cu i la final, o iau de la capat.
					if (i == colors.size() - 1)
						i = 0;

					renderer.addPoint(getXValueFor(ue), getYValueFor(ue), colors.get(i));
				}
				i++;
			}
		}
	}

	/**
	 * Intoarce o valoare potrivita pentru x, in functie de evenimentul primit.
	 * 
	 * @param e
	 *            un eveniment.
	 * @return o valoare intreaga.
	 */
	private static int getXValueFor(UserEvent e) {
		return e.getTimestamp() * 10 + 10;
	}

	/**
	 * Intoarce o valoare potrivita pentru y, in functie de evenimentul primit.
	 * Valoarea returnata depinde de primul element.
	 * 
	 * @param e
	 *            un eveniment.
	 * @return o valoare intreaga.
	 */
	private static int getYValueFor(UserEvent e) {

		String element = e.getPageArea().getPathInEditor().get(0).getType();
		int offset = Math.abs(e.getPageArea().getPathInEditor().get(0).hashCode() % renderer.getHeight());

		if (element.contains("card"))
			return 1 + offset;
		else if (element.contains("input"))
			return 2 + offset;
		else if (element.contains("page"))
			return 3 + offset;
		else if (element.contains("menu") && !element.contains("menu-button"))
			return 4 + offset;
		else if (element.contains("menu-button"))
			return 5 + offset;
		else if (element.contains("icon"))
			return 6 + offset;
		else if (element.contains("dialog"))
			return 7 + offset;
		else
			return offset;

	}
}
