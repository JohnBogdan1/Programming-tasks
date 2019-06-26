package visualiser;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;

@RunWith(JUnit4.class)
public class TestClusterManager {
	private static final List<UserEvent> oneClusterSimple = Lists.newArrayList(
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 0),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 2),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 4));
	private static final List<UserEvent> twoClustersSimple = Lists.newArrayList(
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 0),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("card"))), 1),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 2),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 4),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("card"))), 5),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 10));
	private static final List<UserEvent> threeClusters = Lists.newArrayList(
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 0),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("card"))), 1),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 2),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 4),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("card"))), 5),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 10),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 21),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 26));
	private static final List<UserEvent> threeClustersNoSingles = Lists.newArrayList(
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 0),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("card"))), 1),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 2),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 4),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("card"))), 5),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 10),
			new UserEvent(new Menu(Lists.newArrayList(new EditorElement("menu-button"))), 11),
			new UserEvent(new Menu(Lists.newArrayList(new EditorElement("icon"))), 13),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 21),
			new UserEvent(new Canvas(Lists.newArrayList(new EditorElement("input"))), 26));
	
	@Test
	public void oneClusterSimple() {
		List<ClusterManager.Cluster> expectedClusters = Lists.newArrayList(
				new ClusterManager.Cluster(oneClusterSimple, 0, 13));
		List<ClusterManager.Cluster> computedClusters =
				ClusterManager.getInstance().cluster(oneClusterSimple);
		
		assertTrue(Iterables.elementsEqual(
				expectedClusters,
				computedClusters));
	}
	
	@Test
	public void twoClusterSimple() {
		List<ClusterManager.Cluster> expectedClusters = Lists.newArrayList(
				new ClusterManager.Cluster(Lists.newArrayList(
						twoClustersSimple.get(0),
						twoClustersSimple.get(2),
						twoClustersSimple.get(3),
						twoClustersSimple.get(5)), 0, 19),
				new ClusterManager.Cluster(Lists.newArrayList(
						twoClustersSimple.get(1),
						twoClustersSimple.get(4)), 1, 14));
		
		List<ClusterManager.Cluster> computedClusters =
				ClusterManager.getInstance().cluster(twoClustersSimple);
		
	assertEquals(computedClusters.size(), 2);
		assertEquals(
				new HashSet<ClusterManager.Cluster>(expectedClusters),
				new HashSet<ClusterManager.Cluster>(computedClusters));
	}
	
	@Test
	public void threeClusters() {
		List<ClusterManager.Cluster> expectedClusters = Lists.newArrayList(
				new ClusterManager.Cluster(Lists.newArrayList(
						threeClusters.get(0),
						threeClusters.get(2),
						threeClusters.get(3),
						threeClusters.get(5)), 0, 19),
				new ClusterManager.Cluster(Lists.newArrayList(
						threeClusters.get(1),
						threeClusters.get(4)), 1, 14),
				new ClusterManager.Cluster(Lists.newArrayList(
						threeClusters.get(6),
						threeClusters.get(7)), 21, 35));
		
		List<ClusterManager.Cluster> computedClusters =
				ClusterManager.getInstance().cluster(threeClusters);
		
		assertEquals(computedClusters.size(), 3);
		assertEquals(
				new HashSet<ClusterManager.Cluster>(expectedClusters),
				new HashSet<ClusterManager.Cluster>(computedClusters));
	}
	
	@Test
	public void threeClustersNoSingles() {
		List<ClusterManager.Cluster> expectedClusters = Lists.newArrayList(
				new ClusterManager.Cluster(Lists.newArrayList(
						threeClustersNoSingles.get(0),
						threeClustersNoSingles.get(2),
						threeClustersNoSingles.get(3),
						threeClustersNoSingles.get(5)), 0, 19),
				new ClusterManager.Cluster(Lists.newArrayList(
						threeClustersNoSingles.get(1),
						threeClustersNoSingles.get(4)), 1, 14),
				new ClusterManager.Cluster(Lists.newArrayList(
						threeClustersNoSingles.get(8),
						threeClustersNoSingles.get(9)), 21, 35));
		
		List<ClusterManager.Cluster> computedClusters =
				ClusterManager.getInstance().cluster(threeClustersNoSingles);
		
		assertEquals(computedClusters.size(), 3);
		assertEquals(
				new HashSet<ClusterManager.Cluster>(expectedClusters),
				new HashSet<ClusterManager.Cluster>(computedClusters));
	}
}
