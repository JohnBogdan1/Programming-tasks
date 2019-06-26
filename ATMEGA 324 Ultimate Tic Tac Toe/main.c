#include "gfx.h"

#define __PROG_TYPES_COMPAT__
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//#include <limits.h>
#include <util/delay.h>

typedef struct {
	int16_t mX, mY, score;
} Move;

#define ROWS 9
#define COLS 9
#define square_length 10

int16_t mMacroboard[ROWS / 3][COLS / 3];
int16_t mBoard[ROWS][COLS];

bool firstMove = false;

int16_t lx, hx, ly, hy;
int16_t maxScore = 1234;
int16_t minScore = -1234;
int16_t playerId = 1, enemyId = 2;

int16_t weights[] = { 3, 2, 3, 2, 4, 2, 3, 2, 3 };
int16_t matrixWeights[][3] = { {3, 2, 3}, {2, 4, 2}, {3, 2, 3 } };

int16_t miniBoardsBounds[][2] = { { 0, 0 }, { 3, 0 }, { 6, 0 }, { 0, 3 }, { 3, 3 }, { 6, 3 },
			{ 0, 6 }, { 3, 6 }, { 6, 6 } };

int16_t posibleWinSeq[][3][2] = { { { 0, 0 }, { 0, 1 }, { 0, 2 } },
			{ { 1, 0 }, { 1, 1 }, { 1, 2 } }, { { 2, 0 }, { 2, 1 }, { 2, 2 } }, { { 0, 0 }, { 1, 0 }, { 2, 0 } },
			{ { 0, 1 }, { 1, 1 }, { 2, 1 } }, { { 0, 2 }, { 1, 2 }, { 2, 2 } }, { { 0, 0 }, { 1, 1 }, { 2, 2 } },
			{ { 0, 2 }, { 1, 1 }, { 2, 0 } } };

int16_t winMicroScore = 7;
int16_t MacroBoardWeight = 23;			

int16_t init_x = 10;
int16_t init_y = 10;
int16_t turn = 1;

void setup_buttons() {

	// setup the buttons
	DDRA &= ~(1 << PA0);
    PORTA |= (1 << PA0);

	DDRA &= ~(1 << PA1);
    PORTA |= (1 << PA1);

	DDRA &= ~(1 << PA2);
    PORTA |= (1 << PA2);

	DDRA &= ~(1 << PA3);
    PORTA |= (1 << PA3);

	DDRA &= ~(1 << PA4);
    PORTA |= (1 << PA4);
}

void draw_grid() {
	int16_t i;

	for (i = 0; i < 10; i++) {
		if (i % 3 == 0 && i != 0 && i != 9) {
			GFX_draw_line(10 * (i + 1), 10, 10 * (i + 1), 100, ST7735_RED);
			GFX_draw_line(10, 10 * (i + 1), 100, 10 * (i + 1), ST7735_RED);
		} else {
			GFX_draw_line(10 * (i + 1), 10, 10 * (i + 1), 100, ST7735_BLACK);
			GFX_draw_line(10, 10 * (i + 1), 100, 10 * (i + 1), ST7735_BLACK);
		}
	}
}

void draw_X(int16_t x, int16_t y, uint16_t color) {
	GFX_draw_line(x, y, x + square_length, y + square_length, color);
	GFX_draw_line(x + square_length, y, x, y + square_length, color);
}

void draw_O(int16_t x, int16_t y, int16_t radius, uint16_t color) {
	GFX_draw_circle(x, y, radius, color, 1);
}

void draw_curr_position(int16_t x, int16_t y, uint16_t color) {

	if ((x < 10) | (y < 10))
		return;

	if ((x >= 100) | (y >= 100))
		return;

	int16_t linked_to_x = x + square_length;
	int16_t linked_to_y = y + square_length;

	GFX_draw_line(x, y, x, linked_to_y, color);
	GFX_draw_line(x, y, linked_to_x, y, color);
	GFX_draw_line(linked_to_x, linked_to_y, x, linked_to_y, color);
	GFX_draw_line(linked_to_x, y, linked_to_x, linked_to_y, color);
}

void convert_mat_xy_to_lcd_xy(int16_t x, int16_t y, int16_t offset) {
	init_x = square_length * (y + 1) + offset;
	init_y = square_length * (x + 1) + offset;
}

int16_t convert_lcd_x_to_mat_y(int16_t offset) {
	return (init_x - square_length - offset) / square_length;
}

int16_t convert_lcd_y_to_mat_x(int16_t offset) {
	return (init_y - square_length - offset) / square_length;
}

bool isInActiveMicroboard(int16_t x, int16_t y) {
	return mMacroboard[(int16_t) x / 3][(int16_t) y / 3] == -1;
}

void getAvailableMoves(Move *moves, int16_t *size) {
	int16_t x, y, index = 0;
	memset(moves, -1, sizeof(Move) * ROWS * COLS);

	for (x = 0; x < ROWS; x++) {
		for (y = 0; y < COLS; y++) {
			if (isInActiveMicroboard(x, y) && mBoard[x][y] == 0) {
				Move move;
				move.mX = x;
				move.mY = y;
				moves[index++] = move;
			}
		}
	}

	*size = index;
}

bool checkMacro() {
	int16_t count = 0, x, y;
	for (x = 0; x < ROWS / 3; x++) {
		for (y = 0; y < COLS / 3; y++) {
			if (mMacroboard[x][y] == -1)
				count++;
		}
	}
	if (count > 1)
		return true;
	return false;
}

void matrix(int16_t aux_matrix[ROWS / 3][COLS / 3], int16_t lx, int16_t hx, int16_t ly, int16_t hy) {
	int16_t i = 0, j = 0, x, y;

	for (x = lx; x < hx; x++) {
		for (y = ly; y < hy; y++) {
			aux_matrix[i][j] = mBoard[x][y];
			j++;
		}
		i++;
		j = 0;
	}
}

// macroboard->flag=true, microboard->flag=false
int16_t checkForVictory(int16_t lx, int16_t hx, int16_t ly, int16_t hy, bool flag) {
	bool victory = false;
	int16_t id, x, y, i, j;
	int16_t aux_matrix[ROWS / 3][COLS / 3];

	if (!flag)
		matrix(aux_matrix, lx, hx, ly, hy);
	else {
		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				aux_matrix[i][j] = mMacroboard[i][j];
			}
		}
	}

	// Verific liniile din matrice
	for (x = 0; x < 3; x++) {
		id = aux_matrix[x][0];

		if (id != 0 && id != -1 && id != -2) {
			victory = true;

			for (y = 0; y < 3; y++) {
				if (aux_matrix[x][y] != id) {
					victory = false;
					break;
				}
			}

			if (victory == true)
				return id;
		}
	}

	// Verific coloanele din matrice
	for (y = 0; y < 3; y++) {

		id = aux_matrix[0][y];

		if (id != 0 && id != -1 && id != -2) {
			victory = true;
			for (x = 0; x < 3; x++) {

				if (aux_matrix[x][y] != id) {
					victory = false;
					break;
				}
			}
			if (victory == true)
				return id;
		}
	}
	
	// Verific diagonala principala din matrice
	id = aux_matrix[0][0];
	if (aux_matrix[1][1] == id && aux_matrix[2][2] == id) {
		return id;
	}

	// Verific diagonala secundara din matrice
	id = aux_matrix[0][2];
	if (aux_matrix[1][1] == id && aux_matrix[2][0] == id) {
		return id;
	}

	// Daca mai exista celule libere, atunci nu este un patrat terminat
	// returnez 0 in acest caz
	if (!flag) {
		for (x = 0; x < 3; x++) {
			for (y = 0; y < 3; y++) {
				if (aux_matrix[x][y] == 0)
					return 0;
			}
		}
	} else {
		int16_t size;
		Move got_moves[ROWS * COLS];
		getAvailableMoves(got_moves, &size);
		if (size > 0)
			return 0;
	}

	// egalitate
	if (!flag)
		return -1;
	else
		return -3;
}

void setWinnerMicroSquare(int16_t x, int16_t y, int16_t winner) {
	mMacroboard[x][y] = winner;
}

void clearMacroBoardWhenSentToAWonSquare() {
	int16_t x, y;
	for (x = 0; x < 3; x++) {
		for (y = 0; y < 3; y++) {
			if (mMacroboard[x][y] == 0)
				mMacroboard[x][y] = -1;
		}
	}
}

void unClearMacroBoardWhenSentToAWonSquare() {
	int16_t x, y;
	for (x = 0; x < 3; x++) {
		for (y = 0; y < 3; y++) {
			if (mMacroboard[x][y] == -1)
				mMacroboard[x][y] = 0;
		}
	}
}

void setMove(int16_t x, int16_t y, int16_t player) {
	mBoard[x][y] = player;
	mMacroboard[x / 3][y / 3] = 0;

	if (mMacroboard[x % 3][y % 3] == 0)
		mMacroboard[x % 3][y % 3] = -1;
	else if (mMacroboard[x % 3][y % 3] == 1 || mMacroboard[x % 3][y % 3] == 2 || mMacroboard[x % 3][y % 3] == -2)
		clearMacroBoardWhenSentToAWonSquare();
}

void unMove(int16_t x, int16_t y, int16_t player) {
	mBoard[x][y] = player;

	if (mMacroboard[x % 3][y % 3] == -1)
		mMacroboard[x % 3][y % 3] = 0;
	else if (mMacroboard[x % 3][y % 3] == 1 || mMacroboard[x % 3][y % 3] == 2 || mMacroboard[x % 3][y % 3] == -2)
		unClearMacroBoardWhenSentToAWonSquare();

	mMacroboard[x / 3][y / 3] = -1;
}

int16_t getPlayerIdFromMacro(int16_t row, int16_t column) {
	return mMacroboard[row][column];
}

// -------code for AI--------

void checkX(Move move) {
	if (move.mX < 3) {
		lx = 0;
		hx = 3;
	} else if (move.mX >= 3 && move.mX < 6) {
		lx = 3;
		hx = 6;
	} else if (move.mX >= 6 && move.mX < 9) {
		lx = 6;
		hx = 9;
	}
}

void boundaries(Move move) {
	if (move.mY < 3) {
		ly = 0;
		hy = 3;
		checkX(move);
	} else if (move.mY >= 3 && move.mY < 6) {
		ly = 3;
		hy = 6;
		checkX(move);
	} else if (move.mY >= 6 && move.mY < 9) {
		ly = 6;
		hy = 9;
		checkX(move);
	}
}

void setMicroWins() {

	// verific daca e castigator patratul(sau terminat in urma unui egal)
	int16_t score_id = checkForVictory(lx, hx, ly, hy, false);

	// daca eu castig
	if (score_id == playerId) {

		// cand mutarea trimite catre acelasi patrat care devine castigat
		// atunci toate valorile cu 0 din MacroBoard devin -1
		if (getPlayerIdFromMacro(lx / 3, ly / 3) == -1) {
			clearMacroBoardWhenSentToAWonSquare();
		}

		setWinnerMicroSquare(lx / 3, ly / 3, score_id);
	// daca oponentul castiga
	} else if (score_id == enemyId) {

		if (getPlayerIdFromMacro(lx / 3, ly / 3) == -1) {
			clearMacroBoardWhenSentToAWonSquare();
		}

		setWinnerMicroSquare(lx / 3, ly / 3, score_id);
	// daca este egal
	} else if (score_id == -1) {

		if (getPlayerIdFromMacro(lx / 3, ly / 3) == -1) {
			clearMacroBoardWhenSentToAWonSquare();
		}

		// -2 sta pentru egal
		setWinnerMicroSquare(lx / 3, ly / 3, -2);
	}

}

int16_t calculateMacroBoardScore(int16_t player) {
	int16_t playerScore = 0, opponentScore = 0;
	int16_t playerBlocks = 0, opponentBlocks = 0;
	int16_t i, j;
	bool fullSquare;
	
	for (i = 0; i < 8; i++) {
		
		fullSquare = false;
		playerBlocks = 0;
		opponentBlocks = 0;
		
		for (j = 0; j < 3; j++) {
			int16_t index_0 = posibleWinSeq[i][j][0];
			int16_t index_1 = posibleWinSeq[i][j][1];

			if (mMacroboard[index_0][index_1] == player)
				playerBlocks++;
			else if (mMacroboard[index_0][index_1] == player % 2 + 1)
				opponentBlocks++;
			else if (mMacroboard[index_0][index_1] == -2)
				fullSquare = true;
		}

		if (!fullSquare) {
			if (playerBlocks > 0) {
				if (opponentBlocks > 0)
					continue;
				if (playerBlocks == 2)
					playerScore += winMicroScore;
				playerScore += 1;
			} else if (opponentBlocks > 0) {
				if (opponentBlocks == 2)
					opponentScore += winMicroScore;
				opponentScore += 1;
			}
		}
	}

	int16_t value = 0, x = 0, y = 0;;

	for (i = 0; i < 3; i++) {
		y = 0;
		for (j = 0; j < 3; j++) {
			if (mBoard[i][j] == player) {
				value += matrixWeights[x][y];
			} else if (mBoard[i][j] == player % 2 + 1) {
				value -= matrixWeights[x][y];
			}
			y++;
		}
		x++;
	}

	return playerScore - opponentScore;
}

int16_t calculateBoardScore(int16_t lowX, int16_t lowY, int16_t player) {
	int16_t playerScore = 0, opponentScore = 0;
	int16_t playerBlocks = 0, opponentBlocks = 0;
	int16_t i, j;
	bool fullSquare;
	
	for (i = 0; i < 8; i++) {
		
		fullSquare = false;
		playerBlocks = 0;
		opponentBlocks = 0;
		
		for (j = 0; j < 3; j++) {
			int16_t index_0 = posibleWinSeq[i][j][0];
			int16_t index_1 = posibleWinSeq[i][j][1];

			if (mBoard[lowX + index_0][lowY + index_1] == player)
				playerBlocks++;
			else if (mBoard[lowX + index_0][lowY + index_1] == player % 2 + 1)
				opponentBlocks++;
			else if (mBoard[lowX + index_0][lowY + index_1] == -2)
				fullSquare = true;
		}

		if (!fullSquare) {
			if (playerBlocks > 0) {
				if (opponentBlocks > 0)
					continue;
				if (playerBlocks == 2)
					playerScore += winMicroScore;
				playerScore += 1;
			} else if (opponentBlocks > 0) {
				if (opponentBlocks == 2)
					opponentScore += winMicroScore;
				opponentScore += 1;
			}
		}
	}

	int16_t value = 0, x = 0, y = 0;;

	for (i = lowX; i < lowX + 3; i++) {
		y = 0;
		for (j = lowY; j < lowY + 3; j++) {
			if (mBoard[i][j] == player) {
				value += matrixWeights[x][y];
			} else if (mBoard[i][j] == player % 2 + 1) {
				value -= matrixWeights[x][y];
			}
			y++;
		}
		x++;
	}

	return playerScore - opponentScore;
}

int16_t evaluate(int16_t player) {
	int16_t lowX, lowY, i;

	int16_t value = calculateMacroBoardScore(player) * MacroBoardWeight;

	for (i = 0; i < 9; i++) {
		lowX = miniBoardsBounds[i][0];
		lowY = miniBoardsBounds[i][1];

		// daca nu este un patrat terminat
		if (mMacroboard[lowX / 3][lowY / 3] == 0 || mMacroboard[lowX / 3][lowY / 3] == -1) {

			// la acel scor adun scorul pentru fiecare miniBoard
			// pe care il inmultesc cu valoarea respectiva asociata patratului
			value += calculateBoardScore(lowX, lowY, player) * weights[i];
		}
	}

	return value;
}

Move minimax(int16_t player, int16_t depth, int16_t alpha, int16_t beta) {

	setMicroWins();

	int16_t macroScore = checkForVictory(0, 0, 0, 0, true);

	// daca eu castig
	if (macroScore == playerId) {

		Move move;
		move.score = maxScore + depth;
		return move;

	// daca oponentul castiga
	} else if (macroScore == enemyId) {

		Move move;
		move.score = minScore - depth;
		return move;

	// daca este egal
	} else if (macroScore == -3) {

		Move move;
		move.score = 0;
		return move;

	// daca nu este niciuna de mai sus si am ajuns la final
	} else if (depth == 0) {

		if (player == playerId) {
			Move move;
			move.score = evaluate(player);
			return move;
		} else if (player == enemyId) {
			Move move;
			move.score = -evaluate(player);
			return move;
		}
	}

	Move bestMove;
	int16_t size, i;

	Move got_moves[ROWS * COLS];
	getAvailableMoves(got_moves, &size);

	for (i = 0; i < size; i++) {
		Move move = got_moves[i];

		boundaries(move);

		setMove(move.mX, move.mY, player);

		// daca este randul meu
		if (player == playerId) {

			// calculez scorul mutarii, apeland recursiv pe min
			move.score = minimax((player % 2) + 1, depth - 1, alpha, beta).score;

			// daca scorul este mai mare ca alpha, actualizez alpha
			// setez cea mai buna mutare pana acum, bestMove, in cazul lui
			// max
			if (move.score > alpha) {

				alpha = move.score;
				bestMove = move;
			}
		// daca este randul lui
		} else if (player == enemyId) {

			// // calculez scorul mutarii, apeland recursiv pe max
			move.score = minimax((player % 2) + 1, depth - 1, alpha, beta).score;

			// daca scorul este mai mic ca beta, actualizez beta
			// setez cea mai buna mutare pana acum, bestMove, in cazul lui
			// min
			if (move.score < beta) {
				beta = move.score;
				bestMove = move;
			}

		}

		unMove(move.mX, move.mY, 0);

		// intrerup cautarea, deoarece nu mai are cum sa influenteze
		// rezultatul
		if (alpha >= beta) {
			break;
		}
	}

	// returnez alpha si mutarea cea mai buna, daca sunt eu
	// respectiv beta si mutarea cea mai buna, daca este oponentul
	if (player == playerId) {
		bestMove.score = alpha;
	} else {
		bestMove.score = beta;
	}

	return bestMove;
}

bool checkMacroVictory () {
	
	int16_t macroScore = checkForVictory(0, 0, 0, 0, true);

	if (macroScore == playerId) {

		return playerId;

	// daca oponentul castiga
	} else if (macroScore == enemyId) {

		return enemyId;

	// daca este egal
	} else if (macroScore == -3) {

		return macroScore;
	}

	return 0;
}

void make_move() {
	if((PINA & (1 << PA0)) == 0) {
		if (init_x < 90) {
			draw_grid();
			init_x += square_length;
		}
	}

	if((PINA & (1 << PA1)) == 0) {
		if (init_y < 90) {
			draw_grid();
			init_y += square_length;
		}
	}

	if((PINA & (1 << PA2)) == 0) {
		if (init_x > 10) {
			draw_grid();
			init_x -= square_length;
		}
	}

	if((PINA & (1 << PA3)) == 0) {
		if (init_y > 10) {
			draw_grid();
			init_y -= square_length;
		}
	}

	if (turn == 1) {
		if((PINA & (1 << PA4)) == 0) {
			if (mBoard[convert_lcd_y_to_mat_x(0)][convert_lcd_x_to_mat_y(0)] == 0 && 
			mMacroboard[convert_lcd_y_to_mat_x(0) / 3][convert_lcd_x_to_mat_y(0) / 3] == -1) {

				draw_X(init_x, init_y, ST7735_BLACK);
				
				if (!firstMove)
					unClearMacroBoardWhenSentToAWonSquare();

				setMove(convert_lcd_y_to_mat_x(0), convert_lcd_x_to_mat_y(0), enemyId);

				// verific daca e castigat un patrat 3x3 din cel de 9x9
				Move move;
				move.mX = convert_lcd_y_to_mat_x(0);
				move.mY = convert_lcd_x_to_mat_y(0);
				boundaries(move);
				setMicroWins();

				turn = 0;
			}
		} 
	} else {
		if((PINA & (1 << PA4)) == 0) {
			if (mBoard[convert_lcd_y_to_mat_x(0)][convert_lcd_x_to_mat_y(0)] == 0  && 
			mMacroboard[convert_lcd_y_to_mat_x(0) / 3][convert_lcd_x_to_mat_y(0) / 3] == -1) {

				draw_O(init_x + square_length / 2, init_y + square_length / 2, square_length / 2, ST7735_BLACK);
				setMove(convert_lcd_y_to_mat_x(0), convert_lcd_x_to_mat_y(0), playerId);

				// verific daca e castigat un patrat 3x3 din cel de 9x9
				Move move;
				move.mX = convert_lcd_y_to_mat_x(0);
				move.mY = convert_lcd_x_to_mat_y(0);
				boundaries(move);
				setMicroWins();

				turn = 1;
			}
		}

		/*
		//--AI stuff--
		int16_t size;
		Move got_moves[ROWS * COLS];
		getAvailableMoves(got_moves, &size);
		boundaries(got_moves[0]);

		turn = 1;
		int16_t my_int_min = -9999, my_max_int = 9999;
		Move move = minimax(playerId, 0, my_int_min, my_max_int);
		setMove(move.mX, move.mY, playerId);
		convert_mat_xy_to_lcd_xy(move.mX, move.mY, square_length / 2);
		draw_O(init_x, init_y, square_length / 2, ST7735_BLACK);*/
	}
}

int main(void){
    // init the lcd display
    LCD_init();

	LCD_fill_screen(ST7735_WHITE);

	setup_buttons();

	draw_grid();

	memset(mBoard, 0, sizeof(int16_t) * ROWS * COLS);
	memset(mMacroboard, -1, sizeof(int16_t) * ROWS / 3 * COLS / 3);

    while(1) {

		// al cui rand este
		if (turn == 1) {
			draw_O(115, 55, square_length / 2, ST7735_WHITE);
			draw_X(110, 50, ST7735_BLACK);
		} else {
			draw_X(110, 50, ST7735_WHITE);
			draw_O(115, 55, square_length / 2, ST7735_BLACK);
		}

		draw_curr_position(init_x, init_y, ST7735_CYAN);

		// verific castigatorul
		if (checkMacroVictory() == playerId) {
			// desenez un O
			draw_O(55, 115, square_length / 2, ST7735_BLACK);
		} else if (checkMacroVictory() == enemyId) {
			// desenez un x deasupra
			draw_X(50, 110, ST7735_BLACK);
		} else if (checkMacroVictory() == -3) {
			// egalitate, desenez XO
			draw_X(50, 110, ST7735_BLACK);
			draw_O(45, 115, square_length / 2, ST7735_BLACK);
		}

		// playerii fac o miscare si pun X/O
		make_move();

		_delay_ms(150);
    }

    return 0;
}
