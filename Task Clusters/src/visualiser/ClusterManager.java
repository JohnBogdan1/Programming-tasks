package visualiser;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ClusterManager este implementata cu Singletorn Pattern.
 * 
 * @author Johnny
 *
 */
final class ClusterManager {

	// Intervalul in care apartine un cluster.
	private static int window = 10;

	private static final ClusterManager INSTANCE = new ClusterManager();

	// Previne instantierea din alte clase.
	private ClusterManager() {

	}

	/**
	 * 
	 * @return instanta INSTANCE.
	 */
	public static ClusterManager getInstance() {
		return INSTANCE;
	}

	/**
	 * Constructor care seteaza window.
	 * 
	 * @param window
	 *            un intreg.
	 */
	public ClusterManager(int window) {
		ClusterManager.window = window;
	}

	/**
	 * Metoda care gaseste secvente repetate ale acelorasi elemente. Folosesc o
	 * fereastra (window) pentru a gasi aceste secvente.
	 * 
	 * @param userEvents
	 *            o lista de evenimente.
	 * @return o lista de clustere. Fiecare cluster are are lista de evenimente,
	 *         un startTimestamp si un endTimestamp.
	 */
	public List<Cluster> cluster(List<UserEvent> userEvents) {
		List<Cluster> clustersList = new ArrayList<>();

		/*
		 * Folosesc o mapare intre numele primului element din lista de elemente
		 * a unui eveniment si un Cluster.
		 */
		Map<String, Cluster> map = new HashMap<>();

		// Parcurg lista de evenimente.
		for (UserEvent ue : userEvents) {
			// Daca map-ul contine elementul
			if (map.containsKey(ue.getPageArea().getPathInEditor().get(0).getType())) {
				// Preiau clusterul asociat elementului din eveniment.
				Cluster cluster = map.get(ue.getPageArea().getPathInEditor().get(0).getType());

				/*
				 * Daca timestamp-ul curent depaseste limita impusa de inceputul
				 * clusterului+window, inseamna ca evenimentul se afla in alt
				 * cluster, chiar daca e acelasi element
				 */
				if (ue.getTimestamp() > cluster.startTimestamp + window) {

					// Adaug clusterul in lista.
					clustersList.add(cluster);

					/*
					 * Creez o noua pereche in map de tipul <nume_element,
					 * Cluster>. nume_element va suprascrie un alt nume_element,
					 * daca exista in map.
					 */
					map.put(ue.getPageArea().getPathInEditor().get(0).getType(),
							new Cluster(new ArrayList<UserEvent>(), ue.getTimestamp(), ue.getTimestamp() + 9));

					// Adaug in noul cluster evenimentul curent.
					map.get(ue.getPageArea().getPathInEditor().get(0).getType()).addUserEvent(ue);
				} else {
					// Setez timestamp-ul final.
					cluster.setEndTimestamp(ue.getTimestamp() + 9);

					// Adaug evenimentul la lista clusterului.
					cluster.addUserEvent(ue);
				}
			} else {
				/*
				 * Daca map-ul nu contine elemenul, creez o noua pereche cu
				 * elementul curent si un nou cluster la care adaug evenimentul
				 * curent.
				 */
				map.put(ue.getPageArea().getPathInEditor().get(0).getType(),
						new Cluster(new ArrayList<UserEvent>(), ue.getTimestamp(), ue.getTimestamp() + 9));
				map.get(ue.getPageArea().getPathInEditor().get(0).getType()).addUserEvent(ue);
			}
		}

		/*
		 * Adaug restul clusterelor in lista, cu conditia sa aiba cel putin un
		 * eveniment in lista de evenimente.
		 */
		for (Cluster c : map.values()) {
			if (c.userEvents.size() != 1)
				clustersList.add(c);
		}

		return clustersList;
	}

	/**
	 * Clasa interna care implementeaza un cluster.
	 * 
	 * @author Johnny
	 *
	 */
	static class Cluster {
		private List<UserEvent> userEvents;
		private int startTimestamp;
		private int endTimestamp;

		/**
		 * Constructor cu parametrii.
		 * 
		 * @param userEvents
		 *            o lista de evenimente.
		 * @param start
		 *            un intreg.
		 * @param end
		 *            un intreg.
		 */
		public Cluster(List<UserEvent> userEvents, int start, int end) {
			this.userEvents = userEvents;
			this.startTimestamp = start;
			this.endTimestamp = end;
		}

		/**
		 * Adauga un eveniment in lista.
		 * 
		 * @param e
		 *            un eveniment.
		 */
		public void addUserEvent(UserEvent e) {
			userEvents.add(e);
		}

		/**
		 * Setter.
		 * 
		 * @param userEvents
		 *            o lista de evenimente.
		 */
		public void setUserEvents(List<UserEvent> userEvents) {
			this.userEvents = userEvents;
		}

		/**
		 * 
		 * @return o lista de evenimente.
		 */
		public List<UserEvent> getUserEvents() {
			return userEvents;
		}

		/**
		 * Setter.
		 * 
		 * @param startTimestamp
		 *            un intreg.
		 */
		public void setStartTimestamp(int startTimestamp) {
			this.startTimestamp = startTimestamp;
		}

		/**
		 * Setter.
		 * 
		 * @param endTimestamp
		 *            un intreg.
		 */
		public void setEndTimestamp(int endTimestamp) {
			this.endTimestamp = endTimestamp;
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + endTimestamp;
			result = prime * result + startTimestamp;
			result = prime * result + ((userEvents == null) ? 0 : userEvents.hashCode());
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
			Cluster other = (Cluster) obj;
			if (endTimestamp != other.endTimestamp)
				return false;
			if (startTimestamp != other.startTimestamp)
				return false;
			if (userEvents == null) {
				if (other.userEvents != null)
					return false;
			} else if (!userEvents.equals(other.userEvents))
				return false;
			return true;
		}

	}
}
