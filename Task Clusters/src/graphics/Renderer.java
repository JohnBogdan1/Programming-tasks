package graphics;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.geom.Ellipse2D;
import java.util.List;
import java.util.Map;

import javax.swing.JFrame;

/**
 * Clasa care deseneaza punctele intr-o fereastra(frame). Ea este implementata
 * cu ajutorul unui Fluent Builder Pattern.
 * 
 * @author Johnny
 *
 */
public final class Renderer extends JFrame {

	private static final long serialVersionUID = 1L;

	// Lista de puncte.
	private List<Point> points;

	// Dimensiunea unui punct.
	private int circleSize;

	// O mapare intre o culoare de din Color si o culoare din awt.Color.
	Map<visualiser.Color, Color> map;

	/**
	 * 
	 * @return lista de puncte.
	 */
	public List<Point> getPoints() {
		return points;
	}

	/**
	 * Seteaza lista de puncte.
	 * 
	 * @param points
	 *            o lista de puncte.
	 */
	public void setPoints(List<Point> points) {
		this.points = points;
	}

	/**
	 * 
	 * @return dimensiunea punctului.
	 */
	public int getCircleSize() {
		return circleSize;
	}

	/**
	 * Seteaza dimensiunea punctului.
	 * 
	 * @param circleSize
	 *            un intreg.
	 */
	public void setCircleSize(int circleSize) {
		this.circleSize = circleSize;
	}

	/**
	 * 
	 * @return O mapare intre o culoare de din Color si o culoare din awt.Color.
	 */
	public Map<visualiser.Color, Color> getMap() {
		return map;
	}

	/**
	 * Seteaza o mapare map.
	 * 
	 * @param map
	 *            O mapare intre o culoare de din Color si o culoare din
	 *            awt.Color.
	 */
	public void setMap(Map<visualiser.Color, Color> map) {
		this.map = map;
	}

	/**
	 * Fluent Builder.
	 * 
	 * @author Johnny
	 *
	 */
	public static class Builder {

		private Renderer renderer = new Renderer();

		/**
		 * Seteaza lista de puncte.
		 * 
		 * @param points
		 *            o lista de puncte.
		 * @return this
		 */
		public Builder withPoints(List<Point> points) {
			renderer.setPoints(points);
			return this;
		}

		/**
		 * Seteaza dimensiunea unui punct.
		 * 
		 * @param circleSize
		 *            un intreg.
		 * @return this
		 */
		public Builder withCircleSize(int circleSize) {
			renderer.setCircleSize(circleSize);
			return this;
		}

		/**
		 * Seteaza maparea intre o culoare de din Color si o culoare din
		 * awt.Color.
		 * 
		 * @param map
		 *            O mapare intre o culoare de din Color si o culoare din
		 *            awt.Color.
		 * @return this
		 */
		public Builder withMap(Map<visualiser.Color, Color> map) {
			renderer.setMap(map);
			return this;
		}

		/**
		 * Seteaza dimensiunea ferestrii(frame-ului).
		 * 
		 * @param width
		 *            un intreg.
		 * @param height
		 *            un intreg.
		 * @return this
		 */
		public Builder withWindowSize(int width, int height) {
			renderer.setSize(width, height);
			return this;
		}

		/**
		 * Seteaza titlul ferestrii.
		 * 
		 * @param string
		 *            numele.
		 * @return this
		 */
		public Builder withTitle(String string) {
			renderer.setTitle(string);
			return this;
		}

		/**
		 * Inchide fereastra.
		 * 
		 * @param exitOnClose
		 *            un intreg.
		 * @return this
		 */
		public Builder withDefaultCloseOperation(int exitOnClose) {
			renderer.setDefaultCloseOperation(exitOnClose);
			return this;
		}

		/**
		 * 
		 * @return un renderer cu atributurile specificate.
		 */
		public Renderer build() {
			return renderer;
		}

	}

	/**
	 * Afiseaza fereastra.
	 */
	public void draw() {
		setVisible(true);
	}

	/**
	 * Adauga listei curente de puncte ce trebuiesc desenate, un nou punct cu
	 * dimensiunea si culoarea primite ca parametrii.
	 * 
	 * @param x
	 *            un intreg.
	 * @param y
	 *            un intreg.
	 * @param color
	 *            o culoare.
	 */
	public void addPoint(int x, int y, visualiser.Color color) {
		Point p = new Point();

		p.setX(x);
		p.setY(y);
		p.setColor(color);

		this.points.add(p);
	}

	/**
	 * Deseneaza un punct cu dimensiunea, coordonatele si culoarea respective.
	 */
	@Override
	public void paint(Graphics g) {
		Shape circle;
		Graphics2D ga = (Graphics2D) g;

		for (Point p : points) {
			circle = new Ellipse2D.Float(p.getX(), p.getY(), circleSize, circleSize);
			ga.draw(circle);
			ga.setPaint(getColor(p));
			ga.fill(circle);
		}
	}

	/**
	 * Returneaza culoarea (Color) din pachetul awt, aferenta punctului curent.
	 * 
	 * @param p
	 *            un punct.
	 * @return o culoare(awt.Color).
	 */
	private Color getColor(Point p) {
		return map.get(p.getColor());
	}
}
