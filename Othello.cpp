// Othello.cpp : Defines the entry point for the console application.
// hi

#define UP 1
#define DOWN 2
#define SELECT 3

#include <iostream>
#include <sstream>
#include <ctime>
#include <random>
#include <string>
#include <windows.h>
#include <conio.h>
#include<algorithm>

void gotoxy(int x, int y) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(consoleHandle, pos);
}

using namespace std;

class Board {
protected:
	int squares[8][8];

public:
	Board();
	void toString();
	bool play_square(int, int, int);
	bool move_is_valid(int, int, int);
	bool check_or_flip_path(int, int, int, int, int, bool);
	int get_square(int, int);
	int score();
	bool full_board();
	bool has_valid_move(int);
	void set_squares(Board* b); //copy over another board's squares
	int eval(int, int); //heuristic evaluation of a current board for use in mimimax
	int free_neighbors(int, int);
};


pair<int, int> minimax_decision(Board* b, int cpuval);
int max_value(Board* b, int cpuval, int alpha, int beta, int depth, int maxdepth, time_t start);
int min_value(Board* b, int cpuval, int alpha, int beta, int depth, int maxdepth, time_t start);

Board::Board() {
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			squares[i][j] = 0;
	squares[3][3] = -1;
	squares[4][4] = -1;
	squares[3][4] = 1;
	squares[4][3] = 1;
}

void Board::toString() {
	gotoxy(58, 20); cout << "  1  2  3  4  5  6  7  8" << endl;
	for (int i = 0; i < 8; i++) {
		gotoxy(58, 21 + i);
		cout << i + 1 << '|';
		for (int j = 0; j < 8; j++)
		{
			if (squares[i][j] == -1)
				cout << "●" << '|'; //white
			if (squares[i][j] == 0)
				cout << "__" << '|';
			if (squares[i][j] == 1)
				cout << "○" << '|'; //black
		}
		cout << endl;
	}
}

//returns if player with val has some valid move in this configuration
bool Board::has_valid_move(int val) {
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			if (move_is_valid(i + 1, j + 1, val))
				return true;
	return false;
}

//r and c zero indexed here
//checks whether path in direction rinc, cinc results in flips for val
//will actually flip the pieces along path when doFlips is true
bool Board::check_or_flip_path(int r, int c, int rinc, int cinc, int val, bool doFlips) {
	int pathr = r + rinc;
	int pathc = c + cinc;
	if (pathr < 0 || pathr > 7 || pathc < 0 || pathc > 7 || squares[pathr][pathc] != -1 * val)
		return false;
	//check for some chip of val's along the path:
	while (true) {
		pathr += rinc;
		pathc += cinc;
		if (pathr < 0 || pathr > 7 || pathc < 0 || pathc > 7 || squares[pathr][pathc] == 0)
			return false;
		if (squares[pathr][pathc] == val) {
			if (doFlips) {
				pathr = r + rinc;
				pathc = c + cinc;
				while (squares[pathr][pathc] != val) {
					squares[pathr][pathc] = val;
					pathr += rinc;
					pathc += cinc;
				}
			}
			return true;
		}
	}
	return false;
}


//returns whether given move is valid in this configuration
bool Board::move_is_valid(int row, int col, int val) {
	int r = row - 1;
	int c = col - 1;
	if (r < 0 || r > 7 || c < 0 || c > 7)
		return false;
	//check whether space is occupied:
	if (squares[r][c] != 0)
		return false;
	//check that there is at least one path resulting in flips:
	for (int rinc = -1; rinc <= 1; rinc++)
		for (int cinc = -1; cinc <= 1; cinc++) {
			if (check_or_flip_path(r, c, rinc, cinc, val, false))
				return true;
		}
	return false;
}

//executes move if it is valid.  Returns false and does not update board otherwise
bool Board::play_square(int row, int col, int val) {
	if (!move_is_valid(row, col, val))
		return false;
	squares[row - 1][col - 1] = val;
	for (int rinc = -1; rinc <= 1; rinc++)
		for (int cinc = -1; cinc <= 1; cinc++) {
			check_or_flip_path(row - 1, col - 1, rinc, cinc, val, true);
		}
	return true;
}

bool Board::full_board() {
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			if (squares[i][j] == 0)
				return false;
	return true;
}

//returns score, positive for X player's advantage
int Board::score() {
	int sum = 0;
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			sum += squares[i][j];
	return sum;
}

int Board::get_square(int row, int col) {
	return squares[row - 1][col - 1];
}

void Board::set_squares(Board* b) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			squares[i][j] = b->get_square(i + 1, j + 1);
		}
	}
}

int Board::eval(int cpuval, int depth) { // originally used score, but it led to bad ai
					// instead we evaluate based maximizing the
					// difference between computer's available move count
					// and the player's. Additionally, corners will be
					// considered as specially beneficial since they cannot ever be
					// flipped.

	int score = 0; // evaluation score

	// count available moves for computer and player
	int mc = 0; int mp = 0;
	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (move_is_valid(i, j, cpuval))
				mc++;
			if (move_is_valid(i, j, -1 * cpuval))
				mp++;
		}
	}

	// add the difference to score (scaled)
	score += 20 * (mc - mp); // the number is just some scale determined through playing
	//score += 7*mc;

	/*
	// additionally, if mp is 0 this is real good so add some more points,
	// and if mc is 0 this is real bad so subtract more points
	// because of skipped turns

	if (mp == 0)
		score += 50;
	if (mc == 0)
		score -= 50;
	*/

	// count corners for computer and player
	int cc = 0; int cp = 0;
	if (get_square(1, 1) == cpuval)
		cc++;
	else if (get_square(1, 1) == -1 * cpuval)
		cp++;

	if (get_square(1, 8) == cpuval)
		cc++;
	else if (get_square(1, 8) == -1 * cpuval)
		cp++;

	if (get_square(8, 1) == cpuval)
		cc++;
	else if (get_square(8, 1) == -1 * cpuval)
		cp++;

	if (get_square(8, 8) == cpuval)
		cc++;
	else if (get_square(8, 8) == -1 * cpuval)
		cp++;

	// add the difference to score (scaled)
	score += 200 * (cc - cp);

	/*
	// squares adjacent to corners on edges also useful, but not as much since it could lead to a corner
	int ac = 0; int ap = 0;
	if (get_square(1, 2) == cpuval)
		ac++;
	else if (get_square(1, 2) == -1*cpuval)
		ap++;
	if (get_square(2, 1) == cpuval)
		ac++;
	else if (get_square(2, 1) == -1*cpuval)
		ap++;

	if (get_square(1, 7) == cpuval)
		ac++;
	else if (get_square(1, 7) == -1*cpuval)
		ap++;
	if (get_square(2, 8) == cpuval)
		ac++;
	else if (get_square(2, 8) == -1*cpuval)
		ap++;

	if (get_square(7, 1) == cpuval)
		ac++;
	else if (get_square(7, 1) == -1*cpuval)
		ap++;
	if (get_square(8, 2) == cpuval)
		ac++;
	else if (get_square(8, 2) == -1*cpuval)
		ap++;

	if (get_square(7, 8) == cpuval)
		ac++;
	else if (get_square(7, 8) == -1*cpuval)
		ap++;
	if (get_square(8, 7) == cpuval)
		ac++;
	else if (get_square(8, 7) == -1*cpuval)
		ap++;

	score += 30*(ac - ap);

	// scale so bigger depths are worth more
	score += 10*depth;

	*/

	// limit the amount of space around our pieces so we don't surround as much (which leads to big gains endgame for opponent)
	int sc = 0; int sp = 0; // counts for open spaces neighboring a player/comp's pieces
	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (get_square(i, j) == cpuval) {
				//add count to sc
				sc += free_neighbors(i, j);
			}
			if (get_square(i, j) == -1 * cpuval) {
				//add count to sp
				sp += free_neighbors(i, j);
			}
		}
	}

	score -= 10 * (sc - sp); // subtract because we are trying to minimize it
	return score;
}

int Board::free_neighbors(int i, int j) {
	int count = 0;

	// examine the 8 possible neighborings unless not possible positions
	if ((i + 1) > 0 && j > 0 && (i + 1) < 9 && j < 9 && get_square(i + 1, j) == 0)
		count++;
	if ((i + 1) > 0 && (j - 1) > 0 && (i + 1) < 9 && (j - 1) < 9 && get_square(i + 1, j - 1) == 0)
		count++;
	if (i > 0 && (j - 1) > 0 && i < 9 && (j - 1) < 9 && get_square(i, j - 1) == 0)
		count++;
	if ((i - 1) > 0 && (j - 1) > 0 && (i - 1) < 9 && (j - 1) < 9 && get_square(i - 1, j - 1) == 0)
		count++;
	if ((i - 1) > 0 && j > 0 && (i - 1) < 9 && j < 9 && get_square(i - 1, j) == 0)
		count++;
	if ((i - 1) > 0 && (j + 1) > 0 && (i - 1) < 9 && (j + 1) < 9 && get_square(i - 1, j + 1) == 0)
		count++;
	if (i > 0 && (j + 1) > 0 && i < 9 && (j + 1) < 9 && get_square(i, j + 1) == 0)
		count++;
	if ((i + 1) > 0 && (j + 1) > 0 && (i + 1) < 9 && (j + 1) < 9 && get_square(i + 1, j + 1) == 0)
		count++;

	return count;

}

bool make_simple_cpu_move(Board* b, int cpuval) {
	for (int i = 1; i < 9; i++)
		for (int j = 1; j < 9; j++)
			if (b->get_square(i, j) == 0)
				if (b->play_square(i, j, cpuval))
					return true;
	cout << "Computer passes." << endl;
	return false;
}

bool make_smarter_cpu_move(Board* b, int cpuval) {
	pair<int, int> temp = minimax_decision(b, cpuval);
	if (b->get_square(temp.first, temp.second) == 0) {
		if (b->play_square(temp.first, temp.second, cpuval))
			return true;
	}
	cout << "Computer passes." << endl;
	return false;
}

pair<int, int> minimax_decision(Board* b, int cpuval) {
	// returns a pair<int, int> <i, j> for row, column of best move
	Board* bt = new Board();
	bt->set_squares(b);

	int tempval;

	// computer always trying to maximize eval function
	// need a number higher than eval can ever be to treat as initial max val
	int maxval = 9000;
	int maxi;
	int maxj;

	bool nomove = true;

	int depth = 0; // iterative deepening
	// start clock
	time_t start; time_t now;
	time(&start);

	//int temp;

	while (true) {
		depth++; // increase our depth limit for ID

		for (int i = 1; i < 9; i++) {
			for (int j = 1; j < 9; j++) {
				if (bt->get_square(i, j) == 0) {
					if (bt->play_square(i, j, cpuval)) {

						tempval = max_value(bt, cpuval, 9000, -9000, 1, depth, start); // start alpha at 9000, beta at -9000
						if (tempval <= maxval) { // found a new minimum max value
							nomove = false;
							maxi = i;
							maxj = j;
							//temp = depth;
							maxval = tempval;
						}
						bt->set_squares(b); // either way, erase the play and try next one
					}
				}
			}
		}
		time(&now);
		if (difftime(now, start) >= 20)
			break;

		// add a little randomness to throw off other computer who thinks we are playing optimally
		// by cutting off at a ID depth sometimes
		//if ( (rand() % 100) <= 3)
		//	break;
	}

	pair<int, int> ret;


	if (nomove) {
		nomove = false;
		maxi = 1; // just return something so comp can pass
		maxj = 1;
	}

	ret.first = maxi;
	ret.second = maxj;

	//printf("%d\t%d\n", depth, temp);

	return ret;
}

int max_value(Board* b, int cpuval, int alpha, int beta, int depth, int maxdepth, time_t start) {
	// scoring and heuristics of current board if terminal
	// 2 ways of being terminal: game is over (usually not the case until endgame)
	//    or reached depth limit

	// additionally, if time limit of 20 sec is reached just return the heuristic

	// if the game is over and computer wins, return maximum number possible (9000)
	// if the game is over and player wins, -9000
	// if tie, 0
	if (b->full_board() || (!(b->has_valid_move(cpuval)) && !(b->has_valid_move(-1 * cpuval)))) {
		int score = b->score();
		if (score == 0)
			return 0;
		else if ((score > 0 && (cpuval == 1)) || (score < 0 && (cpuval == -1)))
			return 9000;
		else
			return -9000;
	}

	// reached depth limit or time limit, score the board according to heuristic function
	time_t now;
	time(&now);
	if (depth == maxdepth || difftime(now, start) >= 20)
		return b->eval(cpuval, depth);

	// maximize the min value of successors
	int minval = beta;
	int tempval;

	Board* bt = new Board();
	bt->set_squares(b);

	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (b->get_square(i, j) == 0) {
				if (b->play_square(i, j, -1 * cpuval)) { // since this is the player's turn, change the val

					tempval = min_value(b, cpuval, alpha, minval, depth + 1, maxdepth, start); // new alpha/beta corresponding, our minval will always be >= beta
					if (tempval >= minval) { // found a new maximum min value
						minval = tempval;
					}

					b->set_squares(bt); // either way, erase the play and try next one

					// alpha-beta pruning
					if (minval > alpha) {
						return alpha;
					}
				}
			}
		}
	}
	return minval;
}

int min_value(Board* b, int cpuval, int alpha, int beta, int depth, int maxdepth, time_t start) {
	// scoring and heuristics of current board if terminal

	// 2 ways of being terminal: game is over  or no more valid moves (usually not the case until endgame)
	//    or reached depth limit

	// additionally if time limit reached (20 sec) just ret heuristic scoring

	// if the game is over and computer wins, return maximum number possible (9000)
	// if the game is over and player wins, -9000
	// if the game is over and nobody wins, score 0
	if (b->full_board() || (!(b->has_valid_move(cpuval)) && !(b->has_valid_move(-1 * cpuval)))) {
		int score = b->score();
		if (score == 0)
			return 0;
		else if ((score > 0 && (cpuval == 1)) || (score < 0 && (cpuval == -1)))
			return 9000;
		else
			return -9000;
	}

	time_t now;
	time(&now);
	// reached depth limit, score the board according to heuristic function
	if (depth == maxdepth || difftime(now, start) >= 20)
		return b->eval(cpuval, depth);

	// minimize the max value of successors
	int maxval = alpha;
	int tempval;

	Board* bt = new Board();
	bt->set_squares(b);

	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (b->get_square(i, j) == 0) {
				if (b->play_square(i, j, cpuval)) {

					tempval = max_value(b, cpuval, maxval, beta, depth + 1, maxdepth, start); // our maxval always <= alpha
					if (tempval <= maxval) { // found a new maximum min value
						maxval = tempval;
					}

					b->set_squares(bt); // either way, erase the play and try next one

					if (maxval < beta) {
						return beta;
					}
				}
			}
		}
	}
	return maxval;
}

class Multi_Board : public Board {
private:
	int mode; // 0: normal mode, 1: chance mode
	int goods;
	int good_coor[6]; //(row, col, color) * 2
	int bads;
	int bad_coor[6]; //(row, col, color) * 2
	int chances;
	int chance1_row;
	int chance1_col;
	int chance2_row;
	int chance2_col;
	int check_chance[4];

public:
	Multi_Board();
	void Mode_select();
	void toString();
	void Chance_Placing();
	void Good_chance(int, int, int);
	void Good_chance_second(int, int, int);
	void Check_good();
	void Bad_chance(int, int, int);
	void Check_bad();
	bool play_square(int, int, int);
	bool move_is_valid(int, int, int);
	bool check_or_flip_path(int, int, int, int, int, bool);
	int get_square(int, int);
	int score();
	bool full_board();
	bool has_valid_move(int);
	void set_squares(Board* b);
	int eval(int, int);
	int free_neighbors(int, int);
};

Multi_Board::Multi_Board() {
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			squares[i][j] = 0;
	squares[3][3] = -1;
	squares[4][4] = -1;
	squares[3][4] = 1;
	squares[4][3] = 1;

	mode = 0; //default
	goods = 0;
	bads = 0;
	chances = 0;
	chance1_row = 0;
	chance1_col = 0;
	chance2_row = 0;
	chance2_col = 0;
	check_chance[0] = 0; // chance card 1 type (1= good ,2= bad)
	check_chance[1] = 0; // color
	check_chance[2] = 0; // chance card 2 type (1= good ,2= bad)
	check_chance[3] = 0; // color 
}

void Multi_Board::Mode_select() {
	string a;
	gotoxy(62, 21); cout << "Need chances? (Y/N)" << endl;
	gotoxy(62, 22); cin >> a;

	if (a == "Y" || a == "y") {
		mode = 1;
		system("cls");
		gotoxy(62, 20); cout << "Chance mode selected." << endl;
		Chance_Placing();
	}
	else {
		mode = 0;
		system("cls");
		gotoxy(62, 20); cout << "Normal mode selected." << endl;
	}
}

void Multi_Board::toString() {
	gotoxy(58, 20); cout << "  1  2  3  4  5  6  7  8" << endl;
	for (int i = 0; i < 8; i++) {
		gotoxy(58, 21 + i); cout << i + 1 << '|';

		for (int j = 0; j < 8; j++)
		{
			if (squares[i][j] == -1)
				cout << "●" << '|'; //white
			if (squares[i][j] == 0)
				cout << "__" << '|';
			if (squares[i][j] == 1)
				cout << "○" << '|'; //black
			if (squares[i][j] == 2)
				cout << "??" << '|';
		}
		cout << endl;
		string chance1_color;
		string chance2_color;

		if (check_chance[1] == -1)
		{
			chance1_color = "White";
		}
		if (check_chance[1] == 1)
		{
			chance1_color = "Black";
		}
		if (check_chance[3] == -1)
		{
			chance2_color = "White";
		}
		if (check_chance[3] == 1)
		{
			chance2_color = "Black";
		}
		if (mode == 1 && chances ==0)
		{
			gotoxy(90, 21); cout << "chance cards are unknown yet" << endl;
		}
		if (mode == 1 && chances == 1)
		{
			gotoxy(90, 21); cout << "one chance card opened" << endl;
			gotoxy(90, 22); cout << "it was nothing" << endl;
			gotoxy(90, 23); cout << "another chance card is unknown yet" << endl;
		}
		if (mode == 1 && chances == 3)
		{
			gotoxy(90, 21); cout << "one chance card opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "another chance card is unknown yet" << endl;
		}
		if (mode == 1 && chances == 9)
		{
			gotoxy(90, 21); cout << "one chance card opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "another chance card is unknown" << endl;
		}
		if (mode == 1 && chances == 27)
		{
			gotoxy(90, 21); cout << "one chance card opened" << endl;
			gotoxy(90, 22); cout << "you changed enemies ball" << endl;
			gotoxy(90, 23); cout << "another chance card is unknown yet" << endl;
		}
		if (mode == 1 && chances == 2)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "every chance cards were nothing" << endl;
		}
		if (mode == 1 && chances == 4)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "another chance card was nothing" << endl;
		}
		if (mode == 1 && chances == 10)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "another chance card was nothing" << endl;
		}
		if (mode == 1 && chances == 28)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "you changed enemies ball" << endl;
			gotoxy(90, 23); cout << "another chance card was nothing" << endl;
		}
		if (mode == 1 && chances == 6)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "chance card[" << chance2_row << "][" << chance2_col << "]is always " << chance2_color << " from now!!" << endl;
		}
		if (mode == 1 && chances == 12)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "chance card[" << chance2_row << "][" << chance2_col << "]is always " << chance2_color << " from now!!" << endl;
		}
		if (mode == 1 && chances == 30)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "another chance card, you changed enemies ball" << endl;
		}
		if (mode == 1 && chances == 18)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "chance card[" << chance2_row << "][" << chance2_col << "]is always " << chance2_color << " from now!!" << endl;
		}
		if (mode == 1 && chances == 36)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "chance card[" << chance1_row << "][" << chance1_col << "]is always " << chance1_color << " from now!!" << endl;
			gotoxy(90, 23); cout << "another chance card, you changed enemies ball" << endl;
		}
		if (mode == 1 && chances == 54)
		{
			gotoxy(90, 21); cout << "chance cards are all opened" << endl;
			gotoxy(90, 22); cout << "all chance card was changing enemies ball" << endl;
		}
	}
}
void Multi_Board::Chance_Placing() {
	int chance_row, chance_col;

	gotoxy(58, 21); cout << "Where do you want to set first chance card row: ";
	cin >> chance_row;
	gotoxy(58, 22); cout << "Where do you want to set first chance card col: ";
	cin >> chance_col;
	squares[chance_row - 1][chance_col - 1] = 2;
	chance1_row = chance_row; chance1_col = chance_col;
	gotoxy(58, 23); cout << "Where do you want to set second chance card row: ";
	cin >> chance_row;
	gotoxy(58, 24); cout << "Where do you want to set second chance card col: ";
	cin >> chance_col;
	squares[chance_row - 1][chance_col - 1] = 2;
	chance2_row = chance_row; chance2_col = chance_col;
}

void Multi_Board::Good_chance(int row, int col, int color) {
	gotoxy(62, 30); cout << "Lucky!" << endl;
	Sleep(1500);
	if (goods == 0) {
		good_coor[0] = row - 1;
		good_coor[1] = col - 1;
		good_coor[2] = color;
		goods = 1;
		if (chances == 0 || chances == 1 || chances == 9 || chances == 27)
		{
			chance1_row = row;
			chance1_col = col;
			check_chance[0] = 1;
			check_chance[1] = color;
			chances += 3;
		}
	}
	else { //goods == 1
		good_coor[3] = row - 1;
		good_coor[4] = col - 1;
		good_coor[5] = color;
		goods = 2;

		chance2_row = row;
		chance2_col = col;
		check_chance[2] = 1;
		check_chance[3] = color;
		chances += 3;
	}
}

void Multi_Board::Good_chance_second(int row, int col, int val) {
	Multi_Board* b = new Multi_Board();
	int change_row, change_col;
	gotoxy(90, 21); cout << "Lucky!" << endl;
	Sleep(1500);
	gotoxy(90, 22); cout << "You can change onf of the other player's color!" << endl;
	gotoxy(90, 23); cout << "Where do you want to set chance card2 row: ";
	cin >> change_row;
	gotoxy(90, 24); cout << "Where do you want to set chance card1 col: ";
	cin >> change_col;
	if (val == -1) {	// white
		squares[change_row - 1][change_col - 1] = -1;
	}
	else if (val == 1) {	//black
		squares[change_row - 1][change_col - 1] = 1;
	}
	chances += 27;
	system("cls");
	b->toString();
}

void Multi_Board::Check_good() {
	if (goods == 1) {
		squares[good_coor[0]][good_coor[1]] = good_coor[2];
	}
	else if (goods == 2) {
		squares[good_coor[0]][good_coor[1]] = good_coor[2];
		squares[good_coor[3]][good_coor[4]] = good_coor[5];
	}
}

void Multi_Board::Bad_chance(int row, int col, int color) {
	gotoxy(62, 30); cout << "Too bad!" << endl;
	Sleep(1500);
	if (bads == 0) {
		bad_coor[0] = row - 1;
		bad_coor[1] = col - 1;
		bad_coor[2] = -1 * color;
		bads = 1;
		if (chances == 0 || chances ==1 || chances == 3 || chances == 27)
		{
			chance1_row = row;
			chance1_col = col;
			check_chance[0] = 1;
			check_chance[1] = -1 * color;
			chances += 9;
		}
	}
	else { //bads == 1
		bad_coor[3] = row - 1;
		bad_coor[4] = col - 1;
		bad_coor[5] = -1 * color;
		bads = 2;

		chance2_row = row;
		chance2_col = col;
		check_chance[2] = 1;
		check_chance[3] = -1 * color;
		chances += 9;
	}
}

void Multi_Board::Check_bad() {
	if (bads == 1) {
		squares[bad_coor[0]][bad_coor[1]] = bad_coor[2];
	}
	else if (bads == 2) {
		squares[bad_coor[0]][bad_coor[1]] = bad_coor[2];
		squares[bad_coor[3]][bad_coor[4]] = bad_coor[5];
	}
}

bool Multi_Board::has_valid_move(int val) {
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			if (move_is_valid(i + 1, j + 1, val))
				return true;
	return false;
}

bool Multi_Board::check_or_flip_path(int r, int c, int rinc, int cinc, int val, bool doFlips) {
	int pathr = r + rinc;
	int pathc = c + cinc;
	if (pathr < 0 || pathr > 7 || pathc < 0 || pathc > 7 || squares[pathr][pathc] != -1 * val) //돌을 놓는 곳 바로 옆은 돌 색깔이 달라야 함
		return false;
	//check for some chip of val's along the path:
	while (true) {
		pathr += rinc;
		pathc += cinc;
		if (pathr < 0 || pathr > 7 || pathc < 0 || pathc > 7 || squares[pathr][pathc] == 0 || squares[pathr][pathc] == 2)
			return false;
		if (squares[pathr][pathc] == val) {
			if (doFlips) {
				pathr = r + rinc;
				pathc = c + cinc;
				while (squares[pathr][pathc] != val) {
					squares[pathr][pathc] = val;
					pathr += rinc;
					pathc += cinc;
				}
			}
			return true;
		}
	}
	return false;
}

bool Multi_Board::move_is_valid(int row, int col, int val) {
	int r = row - 1;
	int c = col - 1;
	if (r < 0 || r > 7 || c < 0 || c > 7)
		return false;
	if (squares[r][c] != 0 && squares[r][c] != 2)
		return false;
	for (int rinc = -1; rinc <= 1; rinc++) //팔방체크
		for (int cinc = -1; cinc <= 1; cinc++) {
			if (check_or_flip_path(r, c, rinc, cinc, val, false)) //flase: check
				return true;
		}
	return false;
}

bool Multi_Board::play_square(int row, int col, int val) {
	if (!move_is_valid(row, col, val))
		return false;
	//찬스 칸에 놓는 것이 확정된 후에 좋은 찬스인지 나쁜 찬스인지 정한다.
	if (squares[row - 1][col - 1] == 2) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(1, 99);
		int random_number;

		random_number = dis(gen) % 4;
		if (random_number == 0)
			Good_chance(row, col, val);
		else if (random_number == 1)
			Bad_chance(row, col, val);
		else if (random_number == 2)
			Good_chance_second(row, col, val);
		else
		{
			chances += 1;
			gotoxy(62, 30);  cout << "Nothing happend!" << endl;
			Sleep(1500);
		}

	}

	squares[row - 1][col - 1] = val;
	for (int rinc = -1; rinc <= 1; rinc++)
		for (int cinc = -1; cinc <= 1; cinc++) {
			check_or_flip_path(row - 1, col - 1, rinc, cinc, val, true); //true: flip
		}
	return true;
}

bool Multi_Board::full_board() {
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			if (squares[i][j] == 0 || squares[i][j] == 2)
				return false;
	return true;
}
int Multi_Board::score() {
	int sum = 0;
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			sum += squares[i][j];
	return sum;
}

int Multi_Board::get_square(int row, int col) {
	return squares[row - 1][col - 1];
}

void Multi_Board::set_squares(Board* b) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			squares[i][j] = b->get_square(i + 1, j + 1);
		}
	}
}

int Multi_Board::eval(int cpuval, int depth) { //multi에선 필요 없을 듯

	int score = 0; // evaluation score

	// count available moves for computer and player
	int mc = 0; int mp = 0;
	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (move_is_valid(i, j, cpuval))
				mc++;
			if (move_is_valid(i, j, -1 * cpuval))
				mp++;
		}
	}

	score += 20 * (mc - mp); // the number is just some scale determined through playing


	int cc = 0; int cp = 0;
	if (get_square(1, 1) == cpuval)
		cc++;
	else if (get_square(1, 1) == -1 * cpuval)
		cp++;

	if (get_square(1, 8) == cpuval)
		cc++;
	else if (get_square(1, 8) == -1 * cpuval)
		cp++;

	if (get_square(8, 1) == cpuval)
		cc++;
	else if (get_square(8, 1) == -1 * cpuval)
		cp++;

	if (get_square(8, 8) == cpuval)
		cc++;
	else if (get_square(8, 8) == -1 * cpuval)
		cp++;

	score += 200 * (cc - cp);



	// limit the amount of space around our pieces so we don't surround as much (which leads to big gains endgame for opponent)
	int sc = 0; int sp = 0; // counts for open spaces neighboring a player/comp's pieces
	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (get_square(i, j) == cpuval) {
				//add count to sc
				sc += free_neighbors(i, j);
			}
			if (get_square(i, j) == -1 * cpuval) {
				//add count to sp
				sp += free_neighbors(i, j);
			}
		}
	}

	score -= 10 * (sc - sp); // subtract because we are trying to minimize it
	return score;
}

int Multi_Board::free_neighbors(int i, int j) {
	int count = 0;

	// examine the 8 possible neighborings unless not possible positions
	if ((i + 1) > 0 && j > 0 && (i + 1) < 9 && j < 9 && get_square(i + 1, j) == 0)
		count++;
	if ((i + 1) > 0 && (j - 1) > 0 && (i + 1) < 9 && (j - 1) < 9 && get_square(i + 1, j - 1) == 0)
		count++;
	if (i > 0 && (j - 1) > 0 && i < 9 && (j - 1) < 9 && get_square(i, j - 1) == 0)
		count++;
	if ((i - 1) > 0 && (j - 1) > 0 && (i - 1) < 9 && (j - 1) < 9 && get_square(i - 1, j - 1) == 0)
		count++;
	if ((i - 1) > 0 && j > 0 && (i - 1) < 9 && j < 9 && get_square(i - 1, j) == 0)
		count++;
	if ((i - 1) > 0 && (j + 1) > 0 && (i - 1) < 9 && (j + 1) < 9 && get_square(i - 1, j + 1) == 0)
		count++;
	if (i > 0 && (j + 1) > 0 && i < 9 && (j + 1) < 9 && get_square(i, j + 1) == 0)
		count++;
	if ((i + 1) > 0 && (j + 1) > 0 && (i + 1) < 9 && (j + 1) < 9 && get_square(i + 1, j + 1) == 0)
		count++;

	return count;

}

void play_single(int cpuval) {
	Board* b = new Board();
	int humanPlayer = -1 * cpuval;
	int cpuPlayer = cpuval;
	system("cls"); // 보드판 출력을 위해 화면 초기화
	b->toString();
	int consecutivePasses = 0;

	int row, col;

	if (cpuPlayer == -1) { // cpu plays second: player = white, cpu = black
		while (!b->full_board() && consecutivePasses < 2) {
			//check if player must pass:
			if (!b->has_valid_move(humanPlayer)) {
				gotoxy(58, 30); cout << "You must pass." << endl;
				consecutivePasses++;
			}
			else {
				consecutivePasses = 0;
				gotoxy(58, 30); cout << "Your move row (1-8): ";
				cin >> row;
				gotoxy(58, 31); cout << "Your move col (1-8): ";
				cin >> col;
				if (!b->play_square(row, col, humanPlayer)) {
					system("cls");
					b->toString();
					gotoxy(64, 29); cout << "Illegal move." << endl;
					continue;
				}
			}
			//move for computer:
			if (b->full_board())
				break;
			else {
				system("cls");
				b->toString();
				gotoxy(58, 30); cout << "AI is thinking now, please wait" << endl;
				//if(make_simple_cpu_move(b, cpuPlayer))
				//	consecutivePasses=0;
				if (make_smarter_cpu_move(b, cpuPlayer))
					consecutivePasses = 0;
				else
					consecutivePasses++;
				system("cls");
				b->toString();
			}
		}
	}
	else { // cpu plays first: player = white, cpu = black
		while (!b->full_board() && consecutivePasses < 2) {
			//move for computer:
			if (b->full_board())
				break;
			else {
				gotoxy(58, 30); cout << "..." << endl;
				//if(make_simple_cpu_move(b, cpuPlayer))
				//	consecutivePasses=0;
				if (make_smarter_cpu_move(b, cpuPlayer))
					consecutivePasses = 0;
				else
					consecutivePasses++;
				system("cls");
				b->toString();
			}

			//check if player must pass:
			if (!b->has_valid_move(humanPlayer)) {
				gotoxy(58, 30); cout << "You must pass." << endl;
				consecutivePasses++;
			}
			else {
				consecutivePasses = 0;
				while (true) {
					gotoxy(58, 30); cout << "Your move row (1-8): ";
					cin >> row;
					gotoxy(58, 31); cout << "Your move col (1-8): ";
					cin >> col;
					if (!b->play_square(row, col, humanPlayer)) {
						system("cls");
						b->toString();
						gotoxy(62, 29); cout << "Illegal move." << endl;
					}
					else
						break;
				}
				system("cls");
				b->toString();
			}
		}
	}

	int score = b->score();
	if (score == 0) {
		gotoxy(58, 30); cout << "Tie game." << endl;
	}
	else if ((score > 0 && (cpuval == 1)) || (score < 0 && (cpuval == -1))) {
		gotoxy(58, 31); cout << "Computer wins by " << abs(score) << endl;
	}
	else {
		gotoxy(58, 32); cout << "Player wins by " << abs(score) << endl;
	}

	Sleep(3000);

	return;
}

void play_multi(void) {
	Multi_Board* b = new Multi_Board();
	b->Mode_select();
	system("cls");
	b->toString();
	gotoxy(62, 18); cout << "Black goes first." << endl;

	int consecutivePasses = 0;

	int row, col;

	while (!b->full_board() && consecutivePasses < 2) {
		//check if player must pass:
		gotoxy(62, 29); cout << "Black's turn" << endl;
		if (!b->has_valid_move(1)) {
			gotoxy(58, 30); cout << "You must pass." << endl;
			consecutivePasses++;
		}
		else {
			consecutivePasses = 0;
			gotoxy(58, 31); cout << "Your move row (1-8): ";
			cin >> row;
			gotoxy(58, 32); cout << "Your move col (1-8): ";
			cin >> col;
			if (!b->play_square(row, col, 1)) {
				system("cls");
				b->toString();
				gotoxy(62, 30); cout << "Illegal move." << endl;
				continue;
			}

			b->Check_good();
			b->Check_bad();
			system("cls");
			b->toString();
		}

		//move for white:
		gotoxy(62, 29); cout << "White's turn" << endl;
		if (!b->has_valid_move(-1)) {
			gotoxy(58, 30); cout << "You must pass." << endl;
			consecutivePasses++;
		}
		else {
			consecutivePasses = 0;
			while (true) {
				gotoxy(58, 31); cout << "Your move row (1-8): ";
				cin >> row;
				gotoxy(58, 32); cout << "Your move col (1-8): ";
				cin >> col;
				if (!b->play_square(row, col, -1)) {
					system("cls");
					b->toString();
					gotoxy(62, 29); cout << "White's turn" << endl;
					gotoxy(62, 30); cout << "Illegal move." << endl;
				}
				else
					break;
			}
			b->Check_good();
			b->Check_bad();
			system("cls");
			b->toString();
		}
	}
	int score = b->score();
	if (score == 0) {
		gotoxy(58, 30); cout << "Tie game." << endl;
	}
	else if (score > 0) {
		gotoxy(58, 31); cout << "Black wins by " << abs(score) << endl;
	}
	else {
		gotoxy(58, 32); cout << "White wins by " << abs(score) << endl;
	}

	Sleep(3000);

	return;
}

void MainMenu() {
	//cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	cout << "\t\t"; cout << "                                     @@      @@                     @@  @@                 \n";
	cout << "\t\t"; cout << "                      @@@@@@@@       @@      @@                     @@  @@                 \n";
	cout << "\t\t"; cout << "                     @@      @@      @@      @@                     @@  @@                 \n";
	cout << "\t\t"; cout << "                    @@        @@     @@      @@                     @@  @@                 \n";
	cout << "\t\t"; cout << "                   @@@        @@@    @@      @@                     @@  @@                 \n";
	cout << "\t\t"; cout << "                   @@          @@  @@@@@@@   @@@@@@@@      @@@@     @@  @@      @@@@@      \n";
	cout << "\t\t"; cout << "                   @@          @@    @@      @@@    @@   @@    @@   @@  @@    @@     @@    \n";
	cout << "\t\t"; cout << "                   @@          @@    @@      @@     @@  @@      @@  @@  @@   @@       @@   \n";
	cout << "\t\t"; cout << "                   @@          @@    @@      @@     @@ @@        @@ @@  @@  @@@        @@  \n";
	cout << "\t\t"; cout << "                   @@          @@    @@      @@     @@ @@@@@@@@@@@@ @@  @@  @@         @@  \n";
	cout << "\t\t"; cout << "                   @@          @@    @@      @@     @@ @@           @@  @@  @@         @@  \n";
	cout << "\t\t"; cout << "                    @@        @@     @@      @@     @@ @@           @@  @@   @@       @@   \n";
	cout << "\t\t"; cout << "                     @@      @@      @@      @@     @@  @@          @@  @@    @@     @@    \n";
	cout << "\t\t"; cout << "                      @@@@@@@@       @@@@@@@ @@     @@    @@@@@@@@@ @@  @@      @@@@@      \n";
	cout << "\t\t"; cout << "\n\n\n\n\n\n";
	cout << "\t\t"; cout << "                                                                       OthelloGame By SW3 5-TIM";
}



int Keycontrol() {
	int key = _getch();

	if (key == 224 || key == 0) {
		key = _getch();
		switch (key) {
		case 72:
			return UP;
		case 80:
			return DOWN;
		}
	}
	else if (key == 13 || key == 32) { //엔터 또는 스페이스
		return SELECT;
	}
}

int menuDraw() {
	int x = 65;
	int y = 40;
	gotoxy(x - 2, y);
	cout << "> 게 임 시 작";
	gotoxy(x, y + 1);
	cout << "게 임 정 보";
	gotoxy(x, y + 2);
	cout << "   종 료  ";
	while (1) {
		int n = Keycontrol();
		switch (n) {
		case UP: {
			gotoxy(x - 2, y);
			cout << " ";
			y = y - 1;
			if (y < 40) {
				y = 40;
			}
			gotoxy(x - 2, y);
			cout << ">";
			break;
		}
		case DOWN: {
			gotoxy(x - 2, y);
			cout << " ";
			y = y + 1;
			if (y > 42) {
				y = 42;
			}
			gotoxy(x - 2, y);
			cout << ">";
			break;
		}
		case SELECT: {
			return y - 40;
		}
		}
	}
}

void infoDraw() {
	system("cls");
	cout << "\t\t"; cout << "                                             오셀로 게임                                   \n";
	cout << "\t\t"; cout << "                        검은 색 또는 하얀 색 작은 원판을 8x8의 판 위에 늘어 놓는 게임      \n\n\n\n";
	cout << "\t\t"; cout << "                                                [규칙]                                     \n";
	cout << "\t\t"; cout << "                        1. 컴퓨터와 플레이할지 다른 플레이와 플레이할지 결정합니다         \n";
	cout << "\t\t"; cout << "                        2. 찬스카드가 포함된 게임을 할 것인지 결정합니다                   \n";
	cout << "\t\t"; cout << "                        3. 보드판 가운데에 각자 2개의 게임말을 대각선으로 꽂고 시작합니다  \n";
	cout << "\t\t"; cout << "                        4. 본인의 순서가 되면 자신의 게임말 1개를 보드판에 꽂습니다        \n";
	cout << "\t\t"; cout << "                        5. 상대방의 게임말을 자신의 게임말이 양쪽(가로, 세로, 대각선 방향) \n";
	cout << "\t\t"; cout << "                           으로 에워싸게 되면 가운데(상대편)게임말을 뒤집을 수 있습니다    \n";
	cout << "\t\t"; cout << "                        6. 이렇게 게임판에 더 이상 올릴 게이말이 없으면 게임은 종료됩니다  \n";
	cout << "\t\t"; cout << "                        7. 게임판의 게임말을 세어서 더 많은 플레이어가 승리합니다.         \n";

	while (1) {
		if (Keycontrol() == SELECT)
			break;
	}
}

int main(int argc, char* argv[])
{

	system("mode con cols=150 lines=50 | title 오셀로 게임"); // 콘솔창 크기 및 제목 설정
	while (1) {
		MainMenu(); // 메인 메뉴 그리기 생성자 호출
		int menuCode = menuDraw();
		if (menuCode == 0) {
			system("cls");
			char a;
			char re = 'Y';

			while (re == 'Y' || re == 'y') {
				gotoxy(62, 20); cout << "Single play? (Y/N)" << endl;
				gotoxy(62, 21); cin >> a;

				while (a != 'Y' && a != 'y' && a != 'N' && a != 'n') {
					system("cls");
					gotoxy(62, 20); cout << "Type Y or N." << endl;
					gotoxy(62, 21); cout << "Single play? (Y/N)" << endl;
					gotoxy(62, 22);  cin >> a;
				}

				if (a == 'Y' || a == 'y') {
					system("cls");
					gotoxy(62, 20); cout << "Single play mode selected." << endl;
					gotoxy(62, 21); cout << "Do you want to go first?" << endl;
					gotoxy(62, 22); cin >> a;

					while (a != 'Y' && a != 'y' && a != 'N' && a != 'n') {
						system("cls");
						gotoxy(62, 20); cout << "Type Y or N." << endl;
						gotoxy(62, 21); cout << "Do you want to go first? (Y/N)" << endl;
						gotoxy(62, 22); cin >> a;
					}

					if (a == 'Y' || a == 'y') {
						gotoxy(62, 23); cout << "You are black." << endl;
						play_single(-1); // our cpu's val is -1
					}
					else {
						gotoxy(62, 23); cout << "You are white." << endl;
						play_single(1);
					}
				}
				else {
					system("cls");
					gotoxy(62, 20); cout << "Multi play mode selected." << endl;
					play_multi();
				}
				system("cls");
				gotoxy(62, 20); cout << "Re? (Y/N)" << endl;
				gotoxy(62, 21); cin >> re;
				system("cls");
			}

			return 0;
		}
		else if (menuCode == 1) {
			infoDraw();
		}
		if (menuCode == 2) {
			return 0;
		}
		system("cls");
	}
	return 0;
}
