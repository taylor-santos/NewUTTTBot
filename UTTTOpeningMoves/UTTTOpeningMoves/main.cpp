#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <vector>
#include <fstream>
#include "scores.h"

#define SHRT_MAX 32767 
#define SHRT_MIN -32768

using namespace std;

const char boxCoord[9] = { 0,1,2,0,1,2,0,1,2 };
const char gridCoord[9] = { 0,0,0,1,1,1,2,2,2 };
const char adjacentCoord[3] = { 1,2,0 };
const char adjacentCoord2[3] = { 2,0,1 };
const unsigned short int pow3[9] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };
const unsigned int pow4[9] = { 1, 4, 16, 64, 256, 1024, 4096, 16384, 65536 };
const unsigned int pow10[7] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };

signed char optMoves[3*81000000];

char getOpponent(char player)
{
	/*Takes an input player-id and returns the opposing player-id*/
	return player ^ 3; //Player XOR 3 will toggle the first two bits. 1 -> 2 and 2 -> 1.
}

struct board_small
{
	signed char playable = 9;
	char winner = 0;
	short int score = 0;
	int macroBoard = 0;
	short int field[3][3] = { { 0,0,0 },{ 0,0,0 },{ 0,0,0 } };
	//board_small* PVNode = NULL;
	signed char PVCoord = -1;
	char playedCoord = 81;

	void draw_board();
	void play_move(char player, char fieldX, char fieldY);
	void evaluate_score();

	bool operator==(const board_small& b)
	{
		for (int y = 0; y < 3; ++y)
		{
			for (int x = 0; x < 3; ++x)
			{
				if (field[x][y] != b.field[x][y])
					return false;
			}
		}
		if (macroBoard != b.macroBoard ||
			playable != b.playable ||
			winner != b.winner ||
			score != b.score)
			return false;
		return true;
	}

	~board_small();
};

struct less_than_key
{
	inline bool operator() (const board_small* struct1, const board_small* struct2)
	{
		return (struct1->playedCoord < struct2->playedCoord);
	}
};

void board_small::draw_board()
{
	short int val[3][3];
	memcpy(val, field, sizeof(short int) * 3 * 3);
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int y = 0; y < 3; ++y)
		{
			for (int gridX = 0; gridX < 3; ++gridX)
			{
				for (int x = 0; x < 3; ++x)
				{
					int box = val[gridX][gridY] % 3;
					if (box != 0)
						cerr << box;
					else
						cerr << "_";
					val[gridX][gridY] /= 3;
				}
				cerr << " ";
			}
			cerr << endl;
		}
		cerr << endl;
	}

	int mboard = macroBoard;
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			int b = mboard % 4;
			if (b == 0 && (3 * gridY + gridX == playable || playable == -1))
			{
				cerr << "-1 ";
			}
			else {
				cerr << " " << b << " ";
			}
			mboard /= 4;
		}
		cerr << endl;
	}
	cerr << endl;
}

void board_small::play_move(char player, char fieldX, char fieldY)
{
	char x = boxCoord[fieldX];
	char y = boxCoord[fieldY];
	char gridX = gridCoord[fieldX];
	char gridY = gridCoord[fieldY];

	playedCoord = 9 * fieldY + fieldX;

	score -= scores[field[gridX][gridY]];

	field[gridX][gridY] += pow3[3 * y + x] * player;
	char gridWinner = gridWinners[field[gridX][gridY]];
	if (gridWinner != 0)
	{
		score -= gridScores[macroBoard];
		macroBoard += pow4[3 * gridY + gridX] * gridWinner;
		int gridScore = gridScores[macroBoard];
		if (gridScore == SHRT_MAX)
		{
			winner = 1;
			score = SHRT_MAX;
			return;
		}
		else if (gridScore == SHRT_MIN + 1)
		{
			winner = 2;
			score = SHRT_MIN + 1;
			return;
		}
		else if (gridScore == 0)
		{
			winner = 3;
			int mBoard = macroBoard;
			for (int i = 0; i < 9; ++i)
			{
				if (mBoard % 4 == 0)
				{
					winner = 0;
					break;
				}
				mBoard /= 4;
			}
			if (mBoard == 3)
			{
				while (1);
			}
		}
		else {
			score += gridScore;
		}
	}
	else
		score += scores[field[gridX][gridY]];
	playable = 3 * y + x;
	if ((macroBoard / pow4[playable]) % 4 != 0)
	{
		playable = -1;
	}
}

void board_small::evaluate_score()
{
	score = gridScores[macroBoard];
	if (score == SHRT_MIN + 1 || score == SHRT_MAX)
		return;
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			if (gridWinners[field[gridX][gridY]] == 0)
			{
				score += scores[field[gridX][gridY]];
			}
		}
	}
}

board_small::~board_small()
{
	//delete PVNode;
}

short int alpha_beta_fast(board_small* b, char player, short int alpha, short int beta, char depth)
{
	if (b->winner != 0 || depth <= 0)
	{
		return b->score * (player == 1 ? 1 : -1);
	}
	short int bestScore = SHRT_MIN + 1;
	char opponent = getOpponent(player);
	if (b->playable == -1)
	{
		int macroBoard = b->macroBoard;
		for (char gridY = 0; gridY < 3; ++gridY)
		{
			for (char gridX = 0; gridX < 3; ++gridX)
			{
				int board = macroBoard % 4;
				macroBoard /= 4;
				if (board == 0)
				{
					short int field = b->field[gridX][gridY];
					for (int y = 0; y < 3; ++y)
					{
						for (int x = 0; x < 3; ++x)
						{
							if (field % 3 == 0)
							{
								board_small* newBoard = new board_small();
								memcpy(newBoard, b, sizeof(board_small));
								//newBoard->PVNode = NULL;
								newBoard->PVCoord = -1;
								newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
								short int score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1); //Subtract 2 from depth to improve performance when encountering full-board moves.
								if (score > bestScore)
								{
									bestScore = score;
									//delete b->PVNode;
									//b->PVNode = newBoard;
									b->PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
								}
								delete newBoard;

								if (score > alpha)
								{
									alpha = score;
								}
								if (beta <= alpha)
								{
									return beta;
								}
							}
							field /= 3;
						}
					}
				}
			}
		}
	}
	else {
		char gridX = boxCoord[b->playable];
		char gridY = gridCoord[b->playable];
		short int field = b->field[gridX][gridY];
		for (char y = 0; y < 3; ++y)
		{
			for (char x = 0; x < 3; ++x)
			{
				if (field % 3 == 0) //If grid space is empty
				{
					board_small* newBoard = new board_small();
					memcpy(newBoard, b, sizeof(board_small));
					//newBoard->PVNode = NULL;
					newBoard->PVCoord = -1;
					newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
					short int score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1);
					if (score > bestScore)
					{
						bestScore = score;
						//delete b->PVNode;
						//b->PVNode = newBoard;
						b->PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
					}
					delete newBoard;

					if (score > alpha)
					{
						alpha = score;
					}
					if (beta <= alpha)
					{
						return beta;
					}
				}
				field /= 3;
			}
		}
	}
	return bestScore;
}
/*
short int PVSearch(board_small* b, char player, short int alpha, short int beta, char depth)
{
	if (b->winner != 0 || depth <= 0)
	{
		return b->score * (player == 1 ? 1 : -1);
	}
	short int bestScore = SHRT_MIN + 1;
	char opponent = getOpponent(player);
	board_small* PVNode = NULL;
	signed char PVCoord = -1;
	if (b->PVNode == NULL || b->PVCoord == -1)
	{
		return alpha_beta_fast(b, player, alpha, beta, depth);
	}
	if (b->PVCoord != -1)
	{
		short int score = -PVSearch(b->PVNode, opponent, -beta, -alpha, depth - 1);
		if (score > bestScore)
		{
			bestScore = score;
		}
		PVNode = b->PVNode;
		b->PVNode = NULL;
		PVCoord = b->PVCoord;
		if (score > alpha)
		{
			alpha = score;
		}
		if (beta <= alpha)
		{
			b->PVNode = PVNode;
			b->PVCoord = PVCoord;
			return beta;
		}
	}
	if (b->playable == -1)
	{
		int macroBoard = b->macroBoard;
		for (char gridY = 0; gridY < 3; ++gridY)
		{
			for (char gridX = 0; gridX < 3; ++gridX)
			{
				int board = macroBoard % 4;
				macroBoard /= 4;
				if (board == 0)
				{
					short int field = b->field[gridX][gridY];
					for (int y = 0; y < 3; ++y)
					{
						for (int x = 0; x < 3; ++x)
						{
							if (field % 3 == 0)
							{
								if (b->PVCoord != 9 * (3 * gridY + y) + 3 * gridX + x)
								{
									board_small* newBoard = new board_small();
									memcpy(newBoard, b, sizeof(board_small));
									newBoard->PVNode = NULL;
									newBoard->PVCoord = -1;
									newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
									short int score = -alpha_beta_fast(newBoard, opponent, -(alpha + 1), -alpha, depth - 1); //Subtract 2 from depth to improve performance when encountering full-board moves.
									if (alpha < score && score < beta)
									{
										score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1);
									}

									if (score > bestScore)
									{
										bestScore = score;
										delete PVNode;
										PVNode = newBoard;
										PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
									}
									else
									{
										delete newBoard;
									}
									if (score > alpha)
									{
										alpha = score;
									}
									if (beta <= alpha)
									{
										b->PVNode = PVNode;
										b->PVCoord = PVCoord;
										return beta;
									}
								}
							}
							field /= 3;
						}
					}
				}
			}
		}
	}
	else {
		char gridX = boxCoord[b->playable];
		char gridY = gridCoord[b->playable];
		short int field = b->field[gridX][gridY];
		for (char y = 0; y < 3; ++y)
		{
			for (char x = 0; x < 3; ++x)
			{
				if (field % 3 == 0) //If grid space is empty
				{
					if (b->PVCoord != 9 * (3 * gridY + y) + 3 * gridX + x)
					{
						board_small* newBoard = new board_small();
						memcpy(newBoard, b, sizeof(board_small));
						newBoard->PVNode = NULL;
						newBoard->PVCoord = -1;
						newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
						short int score = -alpha_beta_fast(newBoard, opponent, -(alpha + 1), -alpha, depth - 1);
						if (alpha < score && score < beta)
						{
							score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1);
						}

						if (score > bestScore)
						{
							bestScore = score;
							delete PVNode;
							PVNode = newBoard;
							PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
						}
						else
						{
							delete newBoard;
						}
						if (score > alpha)
						{
							alpha = score;
						}
						if (beta <= alpha)
						{
							b->PVNode = PVNode;
							b->PVCoord = PVCoord;
							return beta;
						}
					}
				}
				field /= 3;
			}
		}
	}
	delete b->PVNode;
	b->PVCoord = PVCoord;
	b->PVNode = PVNode;
	return bestScore;
}

short int PVSearchWithAllMoves(board_small* b, char player, short int alpha, short int beta, char depth, vector<board_small*>* allMoves)
{
	if (b->winner != 0 || depth <= 0)
	{
		return b->score * (player == 1 ? 1 : -1);
	}
	short int bestScore = SHRT_MIN + 1;
	char opponent = getOpponent(player);
	board_small* PVNode = NULL;
	signed char PVCoord = -1;
	if (b->PVCoord != -1)
	{
		short int score = -PVSearch(b->PVNode, opponent, -beta, -alpha, depth - 1);
		allMoves->push_back(b->PVNode);
		if (score > bestScore)
		{
			bestScore = score;
		}

		PVNode = b->PVNode;
		b->PVNode = NULL;
		PVCoord = b->PVCoord;
		if (score > alpha)
		{
			alpha = score;
		}
		if (beta <= alpha)
		{
			b->PVNode = PVNode;
			b->PVCoord = PVCoord;
			return beta;
		}
	}
	if (b->playable == -1)
	{
		int macroBoard = b->macroBoard;
		for (char gridY = 0; gridY < 3; ++gridY)
		{
			for (char gridX = 0; gridX < 3; ++gridX)
			{
				int board = macroBoard % 4;
				macroBoard /= 4;
				if (board == 0)
				{
					short int field = b->field[gridX][gridY];
					for (int y = 0; y < 3; ++y)
					{
						for (int x = 0; x < 3; ++x)
						{
							if (field % 3 == 0)
							{
								if (b->PVCoord != 9 * (3 * gridY + y) + 3 * gridX + x)
								{
									board_small* newBoard = new board_small();
									memcpy(newBoard, b, sizeof(board_small));
									newBoard->PVNode = NULL;
									newBoard->PVCoord = -1;
									newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
									short int score = -alpha_beta_fast(newBoard, opponent, -(alpha + 1), -alpha, depth - 1); //Subtract 2 from depth to improve performance when encountering full-board moves.
									allMoves->push_back(newBoard);

									if (alpha < score && score < beta)
									{
										score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1);
									}

									if (score > bestScore)
									{
										bestScore = score;
										//	delete PVNode;
										PVNode = newBoard;
										PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
									}
									else
									{
										//	delete newBoard;
									}
									if (score > alpha)
									{
										alpha = score;
									}
									if (beta <= alpha)
									{
										b->PVNode = PVNode;
										b->PVCoord = PVCoord;
										return beta;
									}
								}
							}
							field /= 3;
						}
					}
				}
			}
		}
	}
	else {
		char gridX = boxCoord[b->playable];
		char gridY = gridCoord[b->playable];
		short int field = b->field[gridX][gridY];
		for (char y = 0; y < 3; ++y)
		{
			for (char x = 0; x < 3; ++x)
			{
				if (field % 3 == 0) //If grid space is empty
				{
					if (b->PVCoord != 9 * (3 * gridY + y) + 3 * gridX + x)
					{
						board_small* newBoard = new board_small();
						memcpy(newBoard, b, sizeof(board_small));
						newBoard->PVNode = NULL;
						newBoard->PVCoord = -1;
						newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
						short int score = -alpha_beta_fast(newBoard, opponent, -(alpha + 1), -alpha, depth - 1);
						allMoves->push_back(newBoard);
						if (alpha < score && score < beta)
						{
							score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1);
						}

						if (score > bestScore)
						{
							bestScore = score;
							//	delete PVNode;
							PVNode = newBoard;
							PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
						}
						else
						{
							//	delete newBoard;
						}
						if (score > alpha)
						{
							alpha = score;
						}
						if (beta <= alpha)
						{
							b->PVNode = PVNode;
							b->PVCoord = PVCoord;
							return beta;
						}
					}
				}
				field /= 3;
			}
		}
	}
	delete b->PVNode;
	b->PVCoord = PVCoord;
	b->PVNode = PVNode;
	return bestScore;
}
*/
short int alpha_beta_moves(board_small* b, char player, short int alpha, short int beta, char depth, vector<board_small*>* allMoves)
{
	if (b->winner != 0 || depth <= 0)
	{
		return b->score * (player == 1 ? 1 : -1);
	}
	short int bestScore = SHRT_MIN + 1;
	char opponent = getOpponent(player);
	if (b->playable == -1)
	{
		int macroBoard = b->macroBoard;
		for (char gridY = 0; gridY < 3; ++gridY)
		{
			for (char gridX = 0; gridX < 3; ++gridX)
			{
				int board = macroBoard % 4;
				macroBoard /= 4;
				if (board == 0)
				{
					short int field = b->field[gridX][gridY];
					for (int y = 0; y < 3; ++y)
					{
						for (int x = 0; x < 3; ++x)
						{
							if (field % 3 == 0)
							{
								board_small* newBoard = new board_small();
								memcpy(newBoard, b, sizeof(board_small));
								//newBoard->PVNode = NULL;
								newBoard->PVCoord = -1;
								newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
								short int score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1); //Subtract 2 from depth to improve performance when encountering full-board moves.
								allMoves->push_back(newBoard);
								if (score > bestScore)
								{
									bestScore = score;
									//delete b->PVNode;
									//b->PVNode = newBoard;
									b->PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
								}
								else
								{
									//delete newBoard;
								}

								if (score > alpha)
								{
									alpha = score;
								}
								//Remove Alpha-Beta pruning, because every possible opponent move should be saved.
							}
							field /= 3;
						}
					}
				}
			}
		}
	}
	else {
		char gridX = boxCoord[b->playable];
		char gridY = gridCoord[b->playable];
		short int field = b->field[gridX][gridY];
		for (char y = 0; y < 3; ++y)
		{
			for (char x = 0; x < 3; ++x)
			{
				if (field % 3 == 0) //If grid space is empty
				{
					board_small* newBoard = new board_small();
					memcpy(newBoard, b, sizeof(board_small));
					//newBoard->PVNode = NULL;
					newBoard->PVCoord = -1;
					newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
					short int score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1);
					allMoves->push_back(newBoard);
					if (score > bestScore)
					{
						bestScore = score;
						//delete b->PVNode;
						//b->PVNode = newBoard;
						b->PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
					}
					else
					{
						//delete newBoard;
					}

					if (score > alpha)
					{
						alpha = score;
					}
					//Remove Alpha-Beta pruning, because every possible opponent move should be saved.
				}
				field /= 3;
			}
		}
	}
	return bestScore;
}

void FillArray(board_small* b, int player, int maxDepth, int currDepth, int index)
{
	if (currDepth > maxDepth)
		return;
	vector<board_small*> nextMoves;
	int score = alpha_beta_moves(b, player, SHRT_MIN + 1, SHRT_MAX, 6, &nextMoves);
	
	int bestMove = b->PVCoord;
	assert(bestMove != -1);
	optMoves[3 * index + 0] = '0' + bestMove / 10;
	optMoves[3 * index + 1] = '0' + bestMove % 10;

	for (vector<board_small*>::iterator move = nextMoves.begin(); move != nextMoves.end(); ++move)
	{
		if ((*move)->playable != -1)
		{
			/*
			int coord = (*move)->playedCoord;
			int newIndex = index;
			char gridX = boxCoord[coord % 9];
			char gridY = boxCoord[coord / 9];
			int newCoord = 3 * gridY + gridX;
			newIndex += (newCoord + 1) * 81 * pow10[currDepth];
			*/
			int coord = (*move)->playedCoord;
			int newRealIndex = index;
			char gridX = boxCoord[coord % 9];
			char gridY = boxCoord[coord / 9];
			int newCoord = 3 * gridY + gridX;
			newRealIndex += (newCoord + 1) * 81 * pow10[currDepth];

			FillArray((*move), getOpponent(player), maxDepth, currDepth + 1, newRealIndex);
		}
		delete(*move);
	}
}

/*
short int PVSearchWithOpponentMoves(board_small* b, char player, short int alpha, short int beta, char depth, vector<board_small*>* allMoves)
{
	if (b->winner != 0 || depth <= 0)
	{
		return b->score * (player == 1 ? 1 : -1);
	}
	short int bestScore = SHRT_MIN + 1;
	char opponent = getOpponent(player);
	board_small* PVNode = NULL;
	signed char PVCoord = -1;
	if (b->PVCoord != -1)
	{
		vector<board_small*> newMoves;
		short int score = -PVSearchWithAllMoves(b->PVNode, opponent, -beta, -alpha, depth - 1, &newMoves);
		if (score > bestScore)
		{
			allMoves->swap(newMoves);
			for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
			{
				delete (*it);
			}
			newMoves.clear();
			bestScore = score;
		}

		PVNode = b->PVNode;
		b->PVNode = NULL;
		PVCoord = b->PVCoord;
		if (score > alpha)
		{
			alpha = score;
		}
		if (beta <= alpha)
		{
			b->PVNode = PVNode;
			b->PVCoord = PVCoord;
			return beta;
		}
	}
	if (b->playable == -1)
	{
		int macroBoard = b->macroBoard;
		for (char gridY = 0; gridY < 3; ++gridY)
		{
			for (char gridX = 0; gridX < 3; ++gridX)
			{
				int board = macroBoard % 4;
				macroBoard /= 4;
				if (board == 0)
				{
					short int field = b->field[gridX][gridY];
					for (int y = 0; y < 3; ++y)
					{
						for (int x = 0; x < 3; ++x)
						{
							if (field % 3 == 0)
							{
								if (b->PVCoord != 9 * (3 * gridY + y) + 3 * gridX + x)
								{
									board_small* newBoard = new board_small();
									memcpy(newBoard, b, sizeof(board_small));
									newBoard->PVNode = NULL;
									newBoard->PVCoord = -1;
									vector<board_small*> newMoves;
									short int score = -alpha_beta_moves(newBoard, opponent, -(alpha + 1), -alpha, depth - 1, &newMoves); //Subtract 2 from depth to improve performance when encountering full-board moves.

									if (alpha < score && score < beta)
									{
										for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
										{
											delete (*it);
										}
										newMoves.clear();
										newBoard->PVNode = NULL;
										newBoard->PVCoord = -1;
										score = -alpha_beta_moves(newBoard, opponent, -beta, -alpha, depth - 1, &newMoves);
									}

									if (score > bestScore || PVCoord == -1)
									{
										bestScore = score;
										allMoves->swap(newMoves);
										for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
										{
											delete (*it);
										}
										newMoves.clear();
										//	delete PVNode;
										PVNode = newBoard;
										PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
									}
									else
									{
										for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
										{
											delete (*it);
										}
										newMoves.clear();
										//	delete newBoard;
									}
									if (score > alpha)
									{
										alpha = score;
									}
									if (beta <= alpha)
									{
										b->PVNode = PVNode;
										b->PVCoord = PVCoord;
										return beta;
									}
								}
							}
							field /= 3;
						}
					}
				}
			}
		}
	}
	else {
		char gridX = boxCoord[b->playable];
		char gridY = gridCoord[b->playable];
		short int field = b->field[gridX][gridY];
		for (char y = 0; y < 3; ++y)
		{
			for (char x = 0; x < 3; ++x)
			{
				if (field % 3 == 0) //If grid space is empty
				{
					if (b->PVCoord != 9 * (3 * gridY + y) + 3 * gridX + x)
					{
						board_small* newBoard = new board_small();
						memcpy(newBoard, b, sizeof(board_small));
						newBoard->PVNode = NULL;
						newBoard->PVCoord = -1;
						newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
						vector<board_small*> newMoves;
						short int score = -alpha_beta_moves(newBoard, opponent, -(alpha + 1), -alpha, depth - 1, &newMoves);

						if (alpha < score && score < beta)
						{
							for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
							{
								delete (*it);
							}
							newMoves.clear();
							newBoard->PVNode = NULL;
							newBoard->PVCoord = -1;
							score = -alpha_beta_moves(newBoard, opponent, -beta, -alpha, depth - 1, &newMoves);
						}

						if (score > bestScore || PVCoord == -1)
						{
							bestScore = score;
							allMoves->swap(newMoves);
							for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
							{
								delete (*it);
							}
							newMoves.clear();
							//	delete PVNode;
							PVNode = newBoard;
							PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
						}
						else
						{
							for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
							{
								delete (*it);
							}
							newMoves.clear();
							//	delete newBoard;
						}
						if (score > alpha)
						{

							alpha = score;
						}
						if (beta <= alpha)
						{
							b->PVNode = PVNode;
							b->PVCoord = PVCoord;
							return beta;
						}
					}
				}
				field /= 3;
			}
		}
	}
	delete b->PVNode;
	b->PVCoord = PVCoord;
	b->PVNode = PVNode;
	return bestScore;
}
*/
int main()
{
	//fill_n(optMoves, 81000000, -1);
	for (int i = 0; i <81000000; ++i)
	{
		optMoves[3 * i + 0] = '-';
		optMoves[3 * i + 1] = '1';
		optMoves[3 * i + 2] = '\n';
	}
	//ofstream file;
	//file.open("moves.txt");
	FILE* pFile;
	pFile = fopen("moves.txt", "w");

	board_small* b = new board_small();
	b->playable = -1;
	int player = 1;
	
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int y = 0; y < 3; ++y)
		{
			for (int gridX = 0; gridX < 3; ++gridX)
			{
				for (int x = 0; x < 3; ++x)
				{
					board_small* newBoard = new board_small();
					memcpy(newBoard, b, sizeof(board_small));
					newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
					FillArray(newBoard, getOpponent(player), 6, 0, 9*(3*gridY + y) + 3*gridX + x);
					delete newBoard;
					cout << ".";
				}
				cout << " ";
			}
			cout << endl;
		}
		cout << endl;
	}
	fwrite(optMoves, sizeof(char), sizeof(optMoves), pFile);
	fclose(pFile);
	/*
	for (int i = 0; i < 81000000; ++i)
	{
		int move = optMoves[i];
		if (move == -1)
		{
			file << "-1";
		}
		else if (move < 10)
		{
			file << "0" << to_string(move);
		}
		else {
			file << to_string(move);
		}
		file << endl;
	}

	file.close();
	*/

	cout << "Done." << endl;

	while (1);

/*	
	vector<board_small*> moveList;
	vector<int> realIndexList;
	int indexList_index = 0;
	//moveList.push_back(b);
	int player = 1;

	vector<board_small*> nextMoves;
	int score = alpha_beta_moves(b, player, SHRT_MIN + 1, SHRT_MAX, 5, &nextMoves);
	//sort(nextMoves.begin(), nextMoves.end(), less_than_key());
	moveList.insert(moveList.end(), nextMoves.begin(), nextMoves.end());

	delete b;

	for (int j = 0; j < nextMoves.size(); ++j)
	{
		int newIndex = nextMoves[j]->playedCoord;
		realIndexList.push_back(newIndex);
	}

	for (int depth = 0; depth < 7; ++depth)
	{
		vector<board_small*> newMoveList;
		vector<int> newRealIndexList;
		for (int i = 0; i < moveList.size(); ++i)
		{
			vector<board_small*> nextMoves;
			//int score = PVSearchWithAllMoves(moveList[i], player, SHRT_MIN + 1, SHRT_MAX, 2, &nextMoves);
			int score = alpha_beta_moves(moveList[i], player, SHRT_MIN + 1, SHRT_MAX, 2, &nextMoves);
			
			for (int j = nextMoves.size() - 1; j >= 0; j--)
			{
				if (nextMoves[j]->playable == -1)
				{
					/*
					string newIndex = indexList[indexList_index];
					newIndex += "->";
					int coord = nextMoves[j]->playedCoord;
					if (coord < 10)
						newIndex += "0";
					newIndex += to_string(coord);
					cout << newIndex << " Deleted." << endl;
					*/
/*					delete nextMoves[j];
					nextMoves.erase(nextMoves.begin() + j);
				}
				else {
					//delete nextMoves[j]->PVNode;
					//nextMoves[j]->PVNode = NULL;
					nextMoves[j]->PVCoord = -1;
				}
			}
			
			newMoveList.insert(newMoveList.end(), nextMoves.begin(), nextMoves.end());
			
			for (int j = 0; j < nextMoves.size(); ++j)
			{
				int coord = nextMoves[j]->playedCoord;
				int newRealIndex = realIndexList[indexList_index];
				char gridX = boxCoord[coord % 9];
				char gridY = boxCoord[coord / 9];
				int newCoord = 3 * gridY + gridX;
				newRealIndex += (newCoord + 1) * 81 * pow10[depth];
				newRealIndexList.push_back(newRealIndex);
			}
			
			//if (indexList_index >= 0)
			//	file << indexList[indexList_index] << " " << realIndexList[indexList_index] << endl;
			optMoves[realIndexList[indexList_index]] = moveList[i]->PVCoord;

			indexList_index++;

			//moveList[i]->PVNode = NULL;
			//delete moveList[i];
			if (i % (moveList.size() / 10) == 0)
				cout << ((float)i / moveList.size())*100 << "% Done." << endl;
		}
		moveList.swap(newMoveList);
		realIndexList.swap(newRealIndexList);

		for (vector<board_small*>::iterator it = newMoveList.begin(); it != newMoveList.end(); ++it)
		{
			delete (*it);
		}

		indexList_index = 0;
		cout << "Done with depth " << depth << endl;
		player = getOpponent(player);
	}

	for (vector<board_small*>::iterator it = moveList.begin(); it != moveList.end(); ++it)
	{
		delete (*it);
	}

	for (int i = 0; i < 81000000; ++i)
	{
		int move = optMoves[i];
		if (move == -1)
		{
			file << "-1";
		}
		else if (move < 10)
		{
			file << "0" << to_string(move);
		}
		else {
			file << to_string(move);
		}
		file << endl;
	}
	
	file.close();
*/
	/*
	vector<board_small*> moves;
	vector<vector<int>> indices;
	moves.push_back(b);
	for (int i = 0; i < 5; ++i)
	{
		vector<board_small*> newMoves;
		vector<vector<int>> newIndices;
		vector<vector<int>>::iterator indexIt = indices.begin();
		for (vector<board_small*>::iterator it = moves.begin(); it != moves.end(); it++)
		{
			for (int gridY = 0; gridY < 3; ++gridY)
			{
				for (int gridX = 0; gridX < 3; ++gridX)
				{
					if ((*it)->playable == 3 * gridY + gridX || ((*it)->playable == -1 && ((*it)->macroBoard / pow4[3 * gridY + gridX]) % 4 == 0))
					{
						for (int y = 0; y < 3; ++y)
						{
							for (int x = 0; x < 3; ++x)
							{
								if (((*it)->field[gridX][gridY] / pow3[3 * y + x]) % 3 == 0)
								{
									board_small* newBoard = new board_small();
									memcpy(newBoard, b, sizeof(board_small));
									newBoard->play_move(1, 3 * gridX + x, 3 * gridY + y);
									int score = alpha_beta_fast(newBoard, 1, SHRT_MIN + 1, SHRT_MAX, 6);
									newMoves.push_back(newBoard);
									vector<int> nextIndices;
									if (indices.size() != 0)
									{
										nextIndices = (*indexIt);
									}
									if (i == 0)
										nextIndices.push_back(9 * (3 * gridY + y) + 3 * gridX + x);
									else
										nextIndices.push_back(3 * y + x);
									newIndices.push_back(nextIndices);
									if (i == 4)
									{
										cout << (*nextIndices.begin());
										for (vector<int>::iterator j = nextIndices.begin() + 1; j != nextIndices.end(); ++j)
										{
											cout << "->" << (*j);
										}
										cout << " = " << score << endl;
									}
								}
							}
						}
					}
				}
			}
			if (indices.size() != 0)
				indexIt++;
		}
		newMoves.swap(moves);
		newIndices.swap(indices);
	}
	*/
}