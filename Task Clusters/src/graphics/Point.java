package graphics;

import visualiser.Color;

/**
 * Clasa care defineste un punct. Ea este implementata cu un Fluent Builder
 * Pattern.
 * 
 * @author Johnny
 *
 */
public final class Point {

	// Coordonatele si culoarea punctului.
	private int x, y;
	private Color color;

	/**
	 * 
	 * @return coordonata x.
	 */
	public int getX() {
		return x;
	}

	/**
	 * Seteaza coordonata x.
	 * 
	 * @param x
	 *            un intreg.
	 */
	public void setX(int x) {
		this.x = x;
	}

	/**
	 * 
	 * @return coordonata y.
	 */
	public int getY() {
		return y;
	}

	/**
	 * Seteaza coordonata y.
	 * 
	 * @param y
	 *            un intreg.
	 */
	public void setY(int y) {
		this.y = y;
	}

	/**
	 * 
	 * @return culoarea punctului.
	 */
	public Color getColor() {
		return color;
	}

	/**
	 * Seteaza culoarea.
	 * 
	 * @param color
	 *            o culoare din enumeratia Color.
	 */
	public void setColor(Color color) {
		this.color = color;
	}

	/**
	 * Fluent Builder.
	 * 
	 * @author Johnny
	 *
	 */
	public static class Builder {

		private Point point = new Point();

		/**
		 * Seteaza valoarea x.
		 * 
		 * @param x
		 *            un intreg.
		 * @return this
		 */
		public Builder withX(int x) {
			point.setX(x);
			return this;
		}

		/**
		 * Seteaza valoarea y.
		 * 
		 * @param y
		 *            un intreg.
		 * @return this
		 */
		public Builder withY(int y) {
			point.setY(y);
			return this;
		}

		/**
		 * Seteaza culoarea.
		 * 
		 * @param color
		 *            o culoare.
		 * @return this
		 */
		public Builder withColor(Color color) {
			point.setColor(color);
			return this;
		}

		/**
		 * 
		 * @return un punct cu toate atributele specificate.
		 */
		public Point build() {
			return point;
		}
	}
}
