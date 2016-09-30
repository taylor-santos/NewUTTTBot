#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <vector>
#include "scores.h"

#define SHRT_MAX 32767 
#define SHRT_MIN -32768

using namespace std;

const int magicSquare[3][3] = { {8,1,6}, {3,5,7}, {4,9,2} };
const int scoreMultiplyer[3][3] = { { 3,2,3 },{ 2,4,2 },{ 3,2,3 } };
const char boxCoord[9]  = { 0,1,2,0,1,2,0,1,2 };
const char gridCoord[9] = { 0,0,0,1,1,1,2,2,2 };
const char adjacentCoord[3] =  { 1,2,0 };
const char adjacentCoord2[3] = { 2,0,1 };
const unsigned short int pow3[9] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561 };
const unsigned int pow4[9] = { 1, 4, 16, 64, 256, 1024, 4096, 16384, 65536 };
const int scorePerGrid = 20;

//signed char scores[19683];
//char gridWinners[19683];
//short int gridScores[262144];

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
	board_small* PVNode = NULL;
	signed char PVCoord = -1;

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
	delete PVNode;
}

short int alpha_beta_fast(board_small* b, char player, short int alpha, short int beta, char depth, unsigned int* count)
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
								newBoard->PVNode = NULL;
								newBoard->PVCoord = -1;
								newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
								(*count)++;
								short int score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 2, count); //Subtract 2 from depth to improve performance when encountering full-board moves.
								if (score > bestScore)
								{
									bestScore = score;
									delete b->PVNode;
									b->PVNode = newBoard;
									b->PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
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
					newBoard->PVNode = NULL;
					newBoard->PVCoord = -1;
					newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
					(*count)++;
					short int score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1, count);
					if (score > bestScore)
					{
						bestScore = score;
						delete b->PVNode;
						b->PVNode = newBoard;
						b->PVCoord = 9 * (3 * gridY + y) + 3 * gridX + x;
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
						return beta;
					}
				}
				field /= 3;
			}
		}
	}
	return bestScore;
}

short int PVSearch(board_small* b, char player, short int alpha, short int beta, char depth, unsigned int* count)
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
		return alpha_beta_fast(b, player, alpha, beta, depth, count);
	}
	if (b->PVCoord != -1)
	{
		short int score = -PVSearch(b->PVNode, opponent, -beta, -alpha, depth - 1, count);
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
									(*count)++;
									short int score = -alpha_beta_fast(newBoard, opponent, -(alpha + 1), -alpha, depth - 2, count); //Subtract 2 from depth to improve performance when encountering full-board moves.
									if (alpha < score && score < beta)
									{
										score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 2, count);
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
						(*count)++;
						short int score = -alpha_beta_fast(newBoard, opponent, -(alpha + 1), -alpha, depth - 1, count);
						if (alpha < score && score < beta)
						{
							score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1, count);
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

short int PVSearchWithAllMoves(board_small* b, char player, short int alpha, short int beta, char depth, unsigned int* count, vector<board_small*>* allMoves)
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
		short int score = -PVSearch(b->PVNode, opponent, -beta, -alpha, depth - 1, count);
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
									(*count)++;
									short int score = -alpha_beta_fast(newBoard, opponent, -(alpha + 1), -alpha, depth - 2, count); //Subtract 2 from depth to improve performance when encountering full-board moves.
									allMoves->push_back(newBoard);
									
									if (alpha < score && score < beta)
									{
										score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 2, count);
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
						(*count)++;
						short int score = -alpha_beta_fast(newBoard, opponent, -(alpha + 1), -alpha, depth - 1, count);
						allMoves->push_back(newBoard);
						if (alpha < score && score < beta)
						{
							score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1, count);
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

short int alpha_beta_moves(board_small* b, char player, short int alpha, short int beta, char depth, unsigned int* count, vector<board_small*>* allMoves)
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
								newBoard->PVNode = NULL;
								newBoard->PVCoord = -1;
								newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
								(*count)++;
								short int score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 2, count); //Subtract 2 from depth to improve performance when encountering full-board moves.
								allMoves->push_back(newBoard);
								if (score > bestScore)
								{
									bestScore = score;
									//delete b->PVNode;
									b->PVNode = newBoard;
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
					newBoard->PVNode = NULL;
					newBoard->PVCoord = -1;
					newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
					(*count)++;
					short int score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1, count);
					allMoves->push_back(newBoard);
					if (score > bestScore)
					{
						bestScore = score;
						//delete b->PVNode;
						b->PVNode = newBoard;
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

short int PVSearchWithOpponentMoves(board_small* b, char player, short int alpha, short int beta, char depth, unsigned int* count, vector<board_small*>* allMoves)
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
		short int score = -PVSearchWithAllMoves(b->PVNode, opponent, -beta, -alpha, depth - 1, count, &newMoves);
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
									newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
									(*count)++;
									vector<board_small*> newMoves;
									short int score = -alpha_beta_moves(newBoard, opponent, -(alpha + 1), -alpha, depth - 2, count, &newMoves); //Subtract 2 from depth to improve performance when encountering full-board moves.

									if (alpha < score && score < beta)
									{
										for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
										{
											delete (*it);
										}
										newMoves.clear();
										newBoard->PVNode = NULL;
										newBoard->PVCoord = -1;
										score = -alpha_beta_moves(newBoard, opponent, -beta, -alpha, depth - 2, count, &newMoves);
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
						(*count)++;
						vector<board_small*> newMoves;
						short int score = -alpha_beta_moves(newBoard, opponent, -(alpha + 1), -alpha, depth - 1, count, &newMoves); 

						if (alpha < score && score < beta)
						{
							for (vector<board_small*>::iterator it = newMoves.begin(); it != newMoves.end(); ++it)
							{
								delete (*it);
							}
							newMoves.clear();
							newBoard->PVNode = NULL;
							newBoard->PVCoord = -1;
							score = -alpha_beta_moves(newBoard, opponent, -beta, -alpha, depth - 1, count, &newMoves);
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

int main()
{
	int max_timebank = 0;
	int time_per_move = 0;
	int round = 0;
	int move = 0;
	string player_names;
	string your_bot;
	int your_botid = 0;

	board_small* smallB = new board_small();
	
	string input = "";
	string compare = "";

	getline(cin, input);
	compare = "settings timebank ";
	if (input.compare(0, compare.length(), compare) == 0)
	{
		input.erase(0, compare.length());
		max_timebank = std::stoi(input);
	}

	getline(cin, input);
	compare = "settings time_per_move ";
	if (input.compare(0, compare.length(), compare) == 0)
	{
		input.erase(0, compare.length());
		time_per_move = std::stoi(input);
	}

	getline(cin, input);
	compare = "settings player_names ";
	if (input.compare(0, compare.length(), compare) == 0)
	{
		input.erase(0, compare.length());
		player_names = input;
	}

	getline(cin, input);
	compare = "settings your_bot ";
	if (input.compare(0, compare.length(), compare) == 0)
	{
		input.erase(0, compare.length());
		your_bot = input;
	}

	getline(cin, input);
	compare = "settings your_botid ";
	if (input.compare(0, compare.length(), compare) == 0)
	{
		input.erase(0, compare.length());
		your_botid = std::stoi(input);
	}
	
	vector<board_small*> nextMoves;
	char depth = 11;
	while (1)
	{
		
		
		unsigned int count = 0;
		board_small* newBoard = new board_small();

		getline(cin, input);
		compare = "update game round ";
		if (input.compare(0, compare.length(), compare) == 0)
		{
			input.erase(0, compare.length());
			round = std::stoi(input);
		}
		
		getline(cin, input);
		compare = "update game move ";
		if (input.compare(0, compare.length(), compare) == 0)
		{
			input.erase(0, compare.length());
			move = std::stoi(input);
		}

		getline(cin, input);
		compare = "update game field ";
		if (input.compare(0, compare.length(), compare) == 0)
		{
			input.erase(0, compare.length());
			std::istringstream ss(input);
			for (int gridY = 0; gridY < 3; ++gridY)
			{
				for (int y = 0; y < 3; ++y)
				{
					for (int gridX = 0; gridX < 3; ++gridX)
					{
						for (int x = 0; x < 3; ++x)
						{
							string boxVal;
							getline(ss, boxVal, ',');
							newBoard->field[gridX][gridY] += pow3[3 * y + x] * stoi(boxVal);
						}
					}
				}
			}
		}

		bool openBoard = false;
		newBoard->playable = -1;
		getline(cin, input);
		compare = "update game macroboard ";
		if (input.compare(0, compare.length(), compare) == 0)
		{
			input.erase(0, compare.length());
			std::istringstream ss(input);
			for (int gridY = 0; gridY < 3; ++gridY)
			{
				for (int gridX = 0; gridX < 3; ++gridX)
				{
					string gridVal;
					getline(ss, gridVal, ',');
					int val = stoi(gridVal);
					if (val == -1)
					{
						if (openBoard == false)
						{
							if (newBoard->playable != -1)
							{
								openBoard = true;
								newBoard->playable = -1;
							}
							else {
								newBoard->playable = 3*gridY + gridX;
							}
						}
						val = 0;
					}
					newBoard->macroBoard += pow4[3 * gridY + gridX] * gridWinners[newBoard->field[gridX][gridY]];
				}
			}
		}
		newBoard->evaluate_score();
		int timeBank;
		getline(cin, input);
		compare = "action move ";
		if (input.compare(0, compare.length(), compare) == 0)
		{
			input.erase(0, compare.length());
			timeBank = std::stoi(input);
		}

		for (vector<board_small*>::iterator it = nextMoves.begin(); it != nextMoves.end(); ++it)
		{
			if (*(*it) == *newBoard)
			{
				newBoard->PVNode = (*it)->PVNode;
				(*it)->PVNode = NULL;
				newBoard->PVCoord = (*it)->PVCoord;
				cerr << "Found previous Pricipal Variation with coordinate (" << newBoard->PVCoord % 9 << "," << newBoard->PVCoord / 9 << ")" << endl;
			}
			delete (*it);
		}
		nextMoves.clear();

		int time1 = clock();
		int score = PVSearchWithOpponentMoves(newBoard, your_botid, SHRT_MIN + 1, SHRT_MAX, depth, &count, &nextMoves);
		cout << "place_move " << newBoard->PVCoord % 9 << " " << newBoard->PVCoord / 9 << endl;
		int time2 = clock();

		int timeInMs = (time2 - time1) * 1000.0 / (CLOCKS_PER_SEC);
		int countPerSec = count * 1000.0 / timeInMs;

		cerr << "Depth " << (int)depth << " : Score " << score << " : (" << newBoard->PVCoord % 9 << "," << newBoard->PVCoord / 9 << ") : " << timeInMs << " ms : " << count << " Nodes (" << countPerSec << " nps)" << endl;

		if (timeInMs > timeBank / 7)
			depth--;
		else if (timeInMs < timeBank / 12)
			depth++;

		newBoard->PVNode->draw_board();

		
	}
	while (1);
}