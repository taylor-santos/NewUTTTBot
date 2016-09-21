#include <iostream>
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
										score = -alpha_beta_moves(newBoard, opponent, -beta, -alpha, depth - 1, count, &newMoves);
									}

									if (score > bestScore)
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
							score = -alpha_beta_fast(newBoard, opponent, -beta, -alpha, depth - 1, count);
						}

						if (score > bestScore)
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
	char player_names[16];
	char your_bot[8];
	int your_botid = 0;

	board_small* smallB = new board_small();
	
	scanf("settings timebank %i\n", &max_timebank);
	scanf("settings time_per_move %i\n", &time_per_move);
	scanf("settings player_names %s\n", player_names);
	scanf("settings your_bot %s\n", your_bot);
	scanf("settings your_botid %i\n", &your_botid);

	while (1)
	{
		
		char depth = 10;
		unsigned int count = 0;

		scanf("update game round %i\n", &round);
		+scanf("update game move %i\n", &move);
		+scanf("update game field ");

		board_small* newBoard = new board_small();
		for (int gridY = 0; gridY < 3; ++gridY)
		{
			for (int y = 0; y < 3; ++y)
			{
				for (int gridX = 0; gridX < 3; ++gridX)
				{
					for (int x = 0; x < 3; ++x)
					{
						int boxVal = 0;
						scanf("%i,", &boxVal);
						newBoard->field[gridX][gridY] += pow3[3*y+x] * boxVal;
					}
				}
			}
		}
		bool openBoard = false;
		newBoard->playable = -1;
		scanf("\nupdate game macroboard ");
		for (int i = 0; i < 9; ++i)
		{
			int gridVal = 0;
			scanf("%i,", &gridVal);
			if (gridVal == -1)
			{
				if (openBoard == false)
				{
					if (newBoard->playable != -1)
					{
						openBoard = true;
						newBoard->playable = -1;
					}else{
						newBoard->playable = i;
					}
				}
				gridVal = 0;
			}
			newBoard->macroBoard += pow4[i] * gridVal;

		}
		cerr << "ORIGINAL:" << endl;
		newBoard->draw_board();

		vector<board_small*> nextMoves;
		int score = PVSearchWithOpponentMoves(newBoard, your_botid, SHRT_MIN + 1, SHRT_MAX, depth, &count, &nextMoves);

		cerr << "PLAYER " << your_botid << " MOVE:" << endl;
		newBoard->PVNode->draw_board();
		cerr << "PLAYER " << getOpponent(your_botid) << " MOVES:" << endl;
		for (vector<board_small*>::iterator it = nextMoves.begin(); it != nextMoves.end(); ++it)
		{
			(*it)->draw_board();
			delete (*it);
		}
		nextMoves.clear();
		while (1);

		int moveCount = 1;
		int timeBank = 10000;
		while (smallB->winner == 0)
		{
			smallB->draw_board();
			int clock1 = clock();
			int score = PVSearch(smallB, 1, SHRT_MIN + 1, SHRT_MAX, depth, &count);
			int clock2 = clock();
			float actualTime = ((clock2 - clock1) / (CLOCKS_PER_SEC / 1000.0f));
			timeBank = min(timeBank - (int)actualTime + 500, 10000);
			float nodesPerMs = count / actualTime;
			cerr << moveCount++ << ": Depth " << (int)depth << ", Score " << score << " with " << count << " nodes found in " << actualTime << " ms. (" << (int)(nodesPerMs * 1000) << " nodes per second), Time remaining: " << timeBank << endl;
			smallB = smallB->PVNode;
			smallB->draw_board();
			if (smallB->winner == 0)
				smallB = smallB->PVNode;
			if (actualTime > 1250 || timeBank < 2000)
				depth--;
			else if (actualTime < 750)
				depth++;
		}

		while (1);
	}
	while (1);
}