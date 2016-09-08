#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string.h>
#include <cstring>
#include <assert.h>
#include <sstream>

#define INT_MAX 2147483647
#define INT_MIN -2147483647

using namespace std;

const int magicSquare[3][3] = { {8,1,6}, {3,5,7}, {4,9,2} };
const int scoreMultiplyer[3][3] = { { 3,2,3 },{ 2,4,2 },{ 3,2,3 } };
const int scorePerGrid = 20;

int getOpponent(int player)
{
	/*Takes an input player-id and returns the opposing player-id*/
	assert(player == 1 || player == 2);
	return !(player - 1) + 1;
}

class board {
public:
	int field[3][3][3][3];
	int macroBoard[3][3];

	board* choices[3][3][3][3];
	board* copy();
	
	void play_move(int player, int fieldX, int fieldY);
	void evaluateScore();
	
	int getWinner() { return winner; }
	int evaluateMoveCount();
	int getMoveCount() { return moveCount; }
	int getScore(int player);

	board();
	~board();

private:
	int winner = 0;
	int moveCount = 0;
	int score = 0;
	int P1gridScores[3][3];
	int P2gridScores[3][3];
	int P1Multiplyer[3][3] = { { 3,2,3 }, { 2,4,2 }, { 3,2,3 } };
	int P2Multiplyer[3][3] = { { 3,2,3 }, { 2,4,2 }, { 3,2,3 } };

	void evaluateMacroboard(int gridX, int gridY, int x, int y);
	void evaluateWinner(int gridX, int gridY);
	void evaluateMultiplyers();
};

board::board()
{
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			for (int y = 0; y < 3; ++y)
			{
				for (int x = 0; x < 3; ++x)
				{
					choices[gridX][gridY][x][y] = NULL;
				}
			}
		}
	}
}

board::~board()
{
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			for (int y = 0; y < 3; ++y)
			{
				for (int x = 0; x < 3; ++x)
				{
					if (choices[gridX][gridY][x][y] != NULL)
					 delete choices[gridX][gridY][x][y];
				}
			}
		}
	}
}

board* board::copy()
{
	board* b = new board();
	memcpy(b, this, sizeof(board));
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			for (int y = 0; y < 3; ++y)
			{
				for (int x = 0; x < 3; ++x)
				{
					b->choices[gridX][gridY][x][y] = NULL;
				}
			}
		}
	}
	return b;
}

void board::play_move(int player, int fieldX, int fieldY)
{
	/*Updates the board according to a move by the input player at the input coordinates*/
	int opponent = getOpponent(player);
	int x = fieldX % 3;
	int y = fieldY % 3;
	int gridX = fieldX / 3;
	int gridY = fieldY / 3;
	assert(macroBoard[gridX][gridY] == -1); //Grid must be playable
	assert(field[gridX][gridY][x][y] == 0); //board must be open at spot
	field[gridX][gridY][x][y] = player; //Place the actual move onto the board
	evaluateMacroboard(gridX, gridY, x, y); //Check if grid has been won

//	score -= P1gridScores[gridX][gridY] * P1Multiplyer[gridX][gridY];
//	score += P2gridScores[gridX][gridY] * P2Multiplyer[gridX][gridY];

	if (macroBoard[gridX][gridY] > 0) //If the move either won or tied the grid...
	{
		evaluateWinner(gridX, gridY); //Check if the board now has a winner.

		if (winner != 0)
			return;

	

		//Set the individual scores based on whether or not each player is the winner of the grid
		P1gridScores[gridX][gridY] = macroBoard[gridX][gridY] == 1 ? scorePerGrid : 0;
		P2gridScores[gridX][gridY] = macroBoard[gridX][gridY] == 2 ? scorePerGrid : 0;

		int gridWinner = macroBoard[gridX][gridY]; //Could be 3 (TIE)

		//	Each multiplyer is set up to count the number of possible wins that could result from each grid.
		//	If the opponent wins a grid, certain multiplyers for the player need to be subtracted, and vice-versa
		if (gridWinner != 1)
		{
			P1Multiplyer[gridX][gridY] = 0;
			if (macroBoard[(gridX + 1) % 3][gridY] != 2 && macroBoard[(gridX + 1) % 3][gridY] != 3 &&
				macroBoard[(gridX + 2) % 3][gridY] != 2 && macroBoard[(gridX + 2) % 3][gridY] != 3)
			{
				P1Multiplyer[(gridX + 1) % 3][gridY]--;
				P1Multiplyer[(gridX + 2) % 3][gridY]--;
//				score -= P1gridScores[(gridX + 1) % 3][gridY];
//				score -= P1gridScores[(gridX + 2) % 3][gridY];
			}
			if (macroBoard[gridX][(gridY + 1) % 3] != 2 && macroBoard[gridX][(gridY + 1) % 3] != 3 &&
				macroBoard[gridX][(gridY + 2) % 3] != 2 && macroBoard[gridX][(gridY + 2) % 3] != 3)
			{
				P1Multiplyer[gridX][(gridY + 1) % 3]--;
				P1Multiplyer[gridX][(gridY + 2) % 3]--;
//				score -= P1gridScores[gridX][(gridY + 1) % 3];
//				score -= P1gridScores[gridX][(gridY + 2) % 3];
			}
			if (gridX == gridY &&
				macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] != 2 && macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] != 3 &&
				macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] != 2 && macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] != 3)
			{
				P1Multiplyer[(gridX + 1) % 3][(gridY + 1) % 3]--;
				P1Multiplyer[(gridX + 2) % 3][(gridY + 2) % 3]--;
//				score -= P1gridScores[(gridX + 1) % 3][(gridY + 1) % 3];
//				score -= P1gridScores[(gridX + 2) % 3][(gridY + 2) % 3];
			}
			if (gridX == 2 - gridY &&
				macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] != 2 && macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] != 3 &&
				macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] != 2 && macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] != 3)
			{
				P1Multiplyer[(gridX + 2) % 3][(gridY + 1) % 3]--;
				P1Multiplyer[(gridX + 1) % 3][(gridY + 2) % 3]--;
//				score -= P1gridScores[(gridX + 2) % 3][(gridY + 1) % 3];
//				score -= P1gridScores[(gridX + 1) % 3][(gridY + 2) % 3];
			}
		}

		if (gridWinner != 2)
		{
			P2Multiplyer[gridX][gridY] = 0;
			if (macroBoard[(gridX + 1) % 3][gridY] != 1 && macroBoard[(gridX + 1) % 3][gridY] != 3 &&
				macroBoard[(gridX + 2) % 3][gridY] != 1 && macroBoard[(gridX + 2) % 3][gridY] != 3)
			{
				P2Multiplyer[(gridX + 1) % 3][gridY]--;
				P2Multiplyer[(gridX + 2) % 3][gridY]--;
//				score += P2gridScores[(gridX + 1) % 3][gridY];
//				score += P2gridScores[(gridX + 2) % 3][gridY];
			}
			if (macroBoard[gridX][(gridY + 1) % 3] != 1 && macroBoard[gridX][(gridY + 1) % 3] != 3 &&
				macroBoard[gridX][(gridY + 2) % 3] != 1 && macroBoard[gridX][(gridY + 2) % 3] != 3)
			{
				P2Multiplyer[gridX][(gridY + 1) % 3]--;
				P2Multiplyer[gridX][(gridY + 2) % 3]--;
//				score += P2gridScores[gridX][(gridY + 1) % 3];
//				score += P2gridScores[gridX][(gridY + 2) % 3];
			}
			if (gridX == gridY &&
				macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] != 1 && macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] != 3 &&
				macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] != 1 && macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] != 3)
			{
				P2Multiplyer[(gridX + 1) % 3][(gridY + 1) % 3]--;
				P2Multiplyer[(gridX + 2) % 3][(gridY + 2) % 3]--;
//				score += P2gridScores[(gridX + 1) % 3][(gridY + 1) % 3];
//				score += P2gridScores[(gridX + 2) % 3][(gridY + 2) % 3];
			}
			if (gridX == 2 - gridY &&
				macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] != 1 && macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] != 3 &&
				macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] != 1 && macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] != 3)
			{
				P2Multiplyer[(gridX + 2) % 3][(gridY + 1) % 3]--;
				P2Multiplyer[(gridX + 1) % 3][(gridY + 2) % 3]--;
//				score += P2gridScores[(gridX + 2) % 3][(gridY + 1) % 3];
//				score += P2gridScores[(gridX + 1) % 3][(gridY + 2) % 3];
			}
		}

	}
	else {
		P1gridScores[gridX][gridY] = 0;
		P2gridScores[gridX][gridY] = 0;
		int DiagP1Count[2] = { 0,0 };
		int DiagP2Count[2] = { 0,0 };
		for (int i = 0; i < 3; ++i)
		{
			int HorizP1Count = 0;
			int HorizP2Count = 0;
			int VertP1Count = 0;
			int VertP2Count = 0;

			for (int j = 0; j < 3; ++j)
			{
				if (field[gridX][gridY][j][i] == 1)
					HorizP1Count++;
				else if (field[gridX][gridY][j][i] == 2)
					HorizP2Count++;

				if (field[gridX][gridY][i][j] == 1)
					VertP1Count++;
				else if (field[gridX][gridY][i][j] == 2)
					VertP2Count++;
			}

			if (field[gridX][gridY][i][i] == 1)
				DiagP1Count[0]++;
			else if (field[gridX][gridY][i][i] == 2)
				DiagP2Count[0]++;

			if (field[gridX][gridY][i][2 - i] == 1)
				DiagP1Count[1]++;
			else if (field[gridX][gridY][i][2 - i] == 2)
				DiagP2Count[1]++;

			if (HorizP2Count == 0)
				P1gridScores[gridX][gridY] += HorizP1Count*HorizP1Count;
			else if (HorizP1Count == 0)
				P2gridScores[gridX][gridY] += HorizP2Count*HorizP2Count;

			if (VertP2Count == 0)
				P1gridScores[gridX][gridY] += VertP1Count*VertP1Count;
			else if (VertP1Count == 0)
				P2gridScores[gridX][gridY] += VertP2Count*VertP2Count;
		}
		if (DiagP2Count[0] == 0)
			P1gridScores[gridX][gridY] += DiagP1Count[0] * DiagP1Count[0];
		else if (DiagP1Count[0] == 0)
			P2gridScores[gridX][gridY] += DiagP2Count[0] * DiagP2Count[0];

		if (DiagP2Count[1] == 0)
			P1gridScores[gridX][gridY] += DiagP1Count[1] * DiagP1Count[1];
		else if (DiagP1Count[1] == 0)
			P2gridScores[gridX][gridY] += DiagP2Count[1] * DiagP2Count[1];
	}

//	score += P1gridScores[gridX][gridY] * P1Multiplyer[gridX][gridY];
//	score -= P2gridScores[gridX][gridY] * P2Multiplyer[gridX][gridY];

	bool openBoard = false;
	if (macroBoard[x][y] > 0)
		openBoard = true;
	moveCount = 0;
	for (int newGridY = 0; newGridY < 3; ++newGridY)
	{
		for (int newGridX = 0; newGridX < 3; ++newGridX)
		{
			if (macroBoard[newGridX][newGridY] <= 0)
			{
				macroBoard[newGridX][newGridY] = openBoard ? -1 : 0;
				if (newGridX == x && newGridY == y)
					macroBoard[newGridX][newGridY] = -1;
			}
			if (macroBoard[newGridX][newGridY] == -1)
			{
				for (int newY = 0; newY < 3; ++newY)
				{
					for (int newX = 0; newX < 3; ++newX)
					{
						if (field[newGridX][newGridY][newX][newY] == 0)
						{
							moveCount++;
						}
					}
				}
			}
		}
	}
	return;
}

void board::evaluateMacroboard(int gridX, int gridY, int x, int y)
{
	//Search adjacent spaces to see if grid has been won
	int player = field[gridX][gridY][x][y];
	if (player == 0)
		return;
	if (field[gridX][gridY][(x + 1) % 3][y] == player &&
	    field[gridX][gridY][(x + 2) % 3][y] == player)
		macroBoard[gridX][gridY] = player;
	else if (field[gridX][gridY][x][(y + 1) % 3] == player &&
	         field[gridX][gridY][x][(y + 2) % 3] == player)
		macroBoard[gridX][gridY] = player;
	else if (x == y &&
	         field[gridX][gridY][(x + 1) % 3][(y + 1) % 3] == player &&
	         field[gridX][gridY][(x + 2) % 3][(y + 2) % 3] == player)
		macroBoard[gridX][gridY] = player;
	else if (x == 2 - y &&
	         field[gridX][gridY][(x + 2) % 3][(y + 1) % 3] == player &&
	         field[gridX][gridY][(x + 1) % 3][(y + 2) % 3] == player)
		macroBoard[gridX][gridY] = player;
	else
	{
		macroBoard[gridX][gridY] = 3; //Assume grid is tied until an open space is found
		for (int fieldY = 0; fieldY < 3; ++fieldY)
		{
			for (int fieldX = 0; fieldX < 3; ++fieldX)
			{
				if (field[gridX][gridY][fieldX][fieldY] == 0)
				{
					macroBoard[gridX][gridY] = 0;
					return;
				}
			}
		}
	}
	return;
}

void board::evaluateWinner(int gridX, int gridY)
{
	int player = macroBoard[gridX][gridY];
	if (winner != 0 || (player != 1 && player != 2))
	{
		return;
	}
	if (macroBoard[(gridX + 1) % 3][gridY] == player &&
	    macroBoard[(gridX + 2) % 3][gridY] == player)
		winner = player;
	else if (macroBoard[gridX][(gridY + 1) % 3] == player &&
		     macroBoard[gridX][(gridY + 2) % 3] == player)
		winner = player;
	else if (gridX == gridY &&
	         macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] == player &&
	         macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] == player)
		winner = player;
	else if (gridX == 2 - gridY &&
	         macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] == player &&
	         macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] == player)
		winner = player;
	else
	{
		winner = 3; //Assume game is tied until an open grid is found
		for (gridY = 0; gridY < 3; ++gridY)
		{
			for (gridX = 0; gridX < 3; ++gridX)
			{
				if (macroBoard[gridX][gridY] <= 0)
				{
					winner = 0;
					return;
				}
			}
		}
	}
	return;
}

void board::evaluateScore()
{
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			P1gridScores[gridX][gridY] = 0;
			P2gridScores[gridX][gridY] = 0;
			if (macroBoard[gridX][gridY] <= 0)
			{
				int DiagP1Count[2] = { 0,0 };
				int DiagP2Count[2] = { 0,0 };
				for (int i = 0; i < 3; ++i)
				{
					int HorizP1Count = 0;
					int HorizP2Count = 0;
					int VertP1Count = 0;
					int VertP2Count = 0;

					for (int j = 0; j < 3; ++j)
					{
						if (field[gridX][gridY][j][i] == 1)
							HorizP1Count++;
						else if (field[gridX][gridY][j][i] == 2)
							HorizP2Count++;

						if (field[gridX][gridY][i][j] == 1)
							VertP1Count++;
						else if (field[gridX][gridY][i][j] == 2)
							VertP2Count++;
					}

					if (field[gridX][gridY][i][i] == 1)
						DiagP1Count[0]++;
					else if (field[gridX][gridY][i][i] == 2)
						DiagP2Count[0]++;

					if (field[gridX][gridY][i][2 - i] == 1)
						DiagP1Count[1]++;
					else if (field[gridX][gridY][i][2 - i] == 2)
						DiagP2Count[1]++;

					if (HorizP2Count == 0)
						P1gridScores[gridX][gridY] += HorizP1Count*HorizP1Count;
					else if (HorizP1Count == 0)
						P2gridScores[gridX][gridY] += HorizP2Count*HorizP2Count;

					if (VertP2Count == 0)
						P1gridScores[gridX][gridY] += VertP1Count*VertP1Count;
					else if (VertP1Count == 0)
						P2gridScores[gridX][gridY] += VertP2Count*VertP2Count;
				}
				if (DiagP2Count[0] == 0)
					P1gridScores[gridX][gridY] += DiagP1Count[0] * DiagP1Count[0];
				else if (DiagP1Count[0] == 0)
					P2gridScores[gridX][gridY] += DiagP2Count[0] * DiagP2Count[0];

				if (DiagP2Count[1] == 0)
					P1gridScores[gridX][gridY] += DiagP1Count[1] * DiagP1Count[1];
				else if (DiagP1Count[1] == 0)
					P2gridScores[gridX][gridY] += DiagP2Count[1] * DiagP2Count[1];
			}
			else if (macroBoard[gridX][gridY] == 1)
				P1gridScores[gridX][gridY] = scorePerGrid;
			else if (macroBoard[gridX][gridY] == 2)
				P2gridScores[gridX][gridY] = scorePerGrid;
		}
	}

	evaluateMultiplyers();

	score = 0;
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			score += P1gridScores[gridX][gridY] * P1Multiplyer[gridX][gridY];
			score -= P2gridScores[gridX][gridY] * P2Multiplyer[gridX][gridY];
		}
	}

	/*
	score = 0;
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			if (macroBoard[gridX][gridY] == 1)
			{
				if (macroBoard[(gridX + 1) % 3][gridY] != 2 && macroBoard[(gridX + 2) % 3][gridY] != 2 &&
					macroBoard[(gridX + 1) % 3][gridY] != 3 && macroBoard[(gridX + 2) % 3][gridY] != 3)
				{
					score += scorePerGrid;
				}

				if (macroBoard[gridX][(gridY + 1) % 3] != 2 && macroBoard[gridX][(gridY + 2) % 3] != 2 &&
					macroBoard[gridX][(gridY + 1) % 3] != 3 && macroBoard[gridX][(gridY + 2) % 3] != 3)
				{
					score += scorePerGrid;
				}

				if (gridX == gridY && macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] != 2 && macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] != 2 &&
					gridX == gridY && macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] != 3 && macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] != 3)
				{
					score += scorePerGrid;
				}

				if (gridX == 2 - gridY && macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] != 2 && macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] != 2 &&
					gridX == 2 - gridY && macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] != 3 && macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] != 3)
				{
					score += scorePerGrid;
				}


			}
			else if (macroBoard[gridX][gridY] == 2)
			{
				if (macroBoard[(gridX + 1) % 3][gridY] != 1 && macroBoard[(gridX + 2) % 3][gridY] != 1 &&
					macroBoard[(gridX + 1) % 3][gridY] != 3 && macroBoard[(gridX + 2) % 3][gridY] != 3)
				{
					score -= scorePerGrid;
				}

				if (macroBoard[gridX][(gridY + 1) % 3] != 1 && macroBoard[gridX][(gridY + 2) % 3] != 1 &&
					macroBoard[gridX][(gridY + 1) % 3] != 3 && macroBoard[gridX][(gridY + 2) % 3] != 3)
				{
					score -= scorePerGrid;
				}

				if (gridX == gridY && macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] != 1 && macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] != 1 &&
					gridX == gridY && macroBoard[(gridX + 1) % 3][(gridY + 1) % 3] != 3 && macroBoard[(gridX + 2) % 3][(gridY + 2) % 3] != 3)
				{
					score -= scorePerGrid;
				}

				if (gridX == 2 - gridY && macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] != 1 && macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] != 1 &&
					gridX == 2 - gridY && macroBoard[(gridX + 2) % 3][(gridY + 1) % 3] != 3 && macroBoard[(gridX + 1) % 3][(gridY + 2) % 3] != 3)
				{
					score -= scorePerGrid;
				}
			}
			else if (macroBoard[gridX][gridY] <= 0)
			{
				for (int y = 0; y < 3; ++y)
				{
					for (int x = 0; x < 3; ++x)
					{
						if (field[gridX][gridY][x][y] == 1)
						{
							if (field[gridX][gridY][(x + 1) % 3][y] != 2 && field[gridX][gridY][(x + 2) % 3][y] != 2)
							{
								score++;
							}

							if (field[gridX][gridY][x][(y + 1) % 3] != 2 && field[gridX][gridY][x][(y + 2) % 3] != 2)
							{
								score++;
							}

							if (x == y && field[gridX][gridY][(x + 1) % 3][(y + 1) % 3] != 2 && field[gridX][gridY][(x + 2) % 3][(y + 2) % 3] != 2)
							{
								score++;
							}

							if (x == 2 - y && field[gridX][gridY][(x + 2) % 3][(y + 1) % 3] != 2 && field[gridX][gridY][(x + 1) % 3][(y + 2) % 3] != 2)
							{
								score++;
							}
						}
						else if (field[gridX][gridY][x][y] == 2)
						{
							if (field[gridX][gridY][(x + 1) % 3][y] != 1 && field[gridX][gridY][(x + 2) % 3][y] != 1)
							{
								score--;
							}

							if (field[gridX][gridY][x][(y + 1) % 3] != 1 && field[gridX][gridY][x][(y + 2) % 3] != 1)
							{
								score--;
							}

							if (x == y && field[gridX][gridY][(x + 1) % 3][(y + 1) % 3] != 1 && field[gridX][gridY][(x + 2) % 3][(y + 2) % 3] != 1)
							{
								score--;
							}

							if (x == 2 - y && field[gridX][gridY][(x + 2) % 3][(y + 1) % 3] != 1 && field[gridX][gridY][(x + 1) % 3][(y + 2) % 3] != 1)
							{
								score--;
							}
						}
					}
				}
			}
		}
	}
	*/
}

void board::evaluateMultiplyers()
{
	memcpy(P1Multiplyer, scoreMultiplyer, sizeof(int) * 3 * 3);
	memcpy(P2Multiplyer, scoreMultiplyer, sizeof(int) * 3 * 3);

	int DiagP1Count[2] = { 0,0 };
	int DiagP2Count[2] = { 0,0 };
	int DiagTieCount[2] = { 0,0 };
	for (int i = 0; i < 3; ++i)
	{
		int HorizP1Count = 0;
		int HorizP2Count = 0;
		int HorizTieCount = 0;
		int VertP1Count = 0;
		int VertP2Count = 0;
		int VertTieCount = 0;

		for (int j = 0; j < 3; ++j)
		{
			if (macroBoard[j][i] == 1)
				HorizP1Count++;
			else if (macroBoard[j][i] == 2)
				HorizP2Count++;
			else if (macroBoard[j][i] == 3)
				HorizTieCount++;

			if (macroBoard[i][j] == 1)
				VertP1Count++;
			else if (macroBoard[i][j] == 2)
				VertP2Count++;
			else if (macroBoard[i][j] == 3)
				VertTieCount++;
		}

		if (macroBoard[i][i] == 1)
			DiagP1Count[0]++;
		else if (macroBoard[i][i] == 2)
			DiagP2Count[0]++;
		else if (macroBoard[i][i] == 3)
			DiagTieCount[0]++;

		if (macroBoard[i][2 - i] == 1)
			DiagP1Count[1]++;
		else if (macroBoard[i][2 - i] == 2)
			DiagP2Count[1]++;
		else if (macroBoard[i][2 - i] == 3)
			DiagTieCount[1]++;

		if (HorizP1Count > 0 || HorizTieCount > 0)
		{
			P2Multiplyer[0][i]--;
			P2Multiplyer[1][i]--;
			P2Multiplyer[2][i]--;
		}
		if (HorizP2Count > 0 || HorizTieCount > 0)
		{
			P1Multiplyer[0][i]--;
			P1Multiplyer[1][i]--;
			P1Multiplyer[2][i]--;
		}

		if (VertP1Count > 0 || VertTieCount > 0)
		{
			P2Multiplyer[i][0]--;
			P2Multiplyer[i][1]--;
			P2Multiplyer[i][2]--;
		}
		if (VertP2Count > 0 || VertTieCount > 0)
		{
			P1Multiplyer[i][0]--;
			P1Multiplyer[i][1]--;
			P1Multiplyer[i][2]--;
		}
	}

	if (DiagP1Count[0] > 0 || DiagTieCount[0] > 0)
	{
		P2Multiplyer[0][0]--;
		P2Multiplyer[1][1]--;
		P2Multiplyer[2][2]--;
	}
	if (DiagP2Count[0] > 0 || DiagTieCount[0] > 0)
	{
		P1Multiplyer[0][0]--;
		P1Multiplyer[1][1]--;
		P1Multiplyer[2][2]--;
	}

	if (DiagP1Count[1] > 0 || DiagTieCount[1] > 0)
	{
		P2Multiplyer[2][0]--;
		P2Multiplyer[1][1]--;
		P2Multiplyer[0][2]--;
	}
	if (DiagP2Count[1] > 0 || DiagTieCount[1] > 0)
	{
		P1Multiplyer[2][0]--;
		P1Multiplyer[1][1]--;
		P1Multiplyer[0][2]--;
	}
}

int board::evaluateMoveCount()
{
	moveCount = 0;
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			if (macroBoard[gridX][gridY] == -1)
			{
				for (int y = 0; y < 3; ++y)
				{
					for (int x = 0; x < 3; ++x)
					{
						if (field[gridX][gridY][x][y] == 0)
						{
							moveCount++;
						}
					}
				}
			}
		}
	}
	return moveCount;
}

int board::getScore(int player)
{
	score = 0;
	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			score += P1gridScores[gridX][gridY] * P1Multiplyer[gridX][gridY];
			score -= P2gridScores[gridX][gridY] * P2Multiplyer[gridX][gridY];
		}
	}
	return player == 1 ? score : -score;
}

int alphaBetaWithCount(board* b, int player, bool maximizing, int alpha, int beta, int* count)
{
	int opponent = getOpponent(player);
	int scorePlayer = maximizing ? player : opponent;
	if (b->getWinner() != 0)
	{
		(*count)--;
		return (b->getWinner() == scorePlayer ? INT_MAX : INT_MIN);
	}
	int moveCount = b->getMoveCount();
	if (moveCount > (*count)) 
	{
		(*count)--;
		return b->getScore(scorePlayer);
	}
	int remainingMoves = moveCount;
	int bestScore = maximizing ? INT_MIN : INT_MAX;

	for (int gridY = 0; gridY < 3; ++gridY)
	{
		for (int gridX = 0; gridX < 3; ++gridX)
		{
			if (b->macroBoard[gridX][gridY] == -1)
			{
				for (int y = 0; y < 3; ++y)
				{
					for (int x = 0; x < 3; ++x)
					{
						if (b->field[gridX][gridY][x][y] == 0)
						{
							board* newBoard = b->choices[gridX][gridY][x][y];
							if (newBoard == NULL)
							{
								newBoard = b->copy();
								newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
								b->choices[gridX][gridY][x][y] = newBoard;
								
							}
							//board* newBoard = b->copy();
							//newBoard->play_move(player, 3 * gridX + x, 3 * gridY + y);
							/*
							int prevScore = newBoard->getScore(scorePlayer);
							newBoard->evaluateScore();
							if (newBoard->getWinner() == 0 && prevScore != newBoard->getScore(scorePlayer))
							{
								board* copy = b->copy();
								copy->play_move(player, 3 * gridX + x, 3 * gridY + y);
								while (1);
							}
							*/

							int newCount = (*count) / remainingMoves;
							remainingMoves--;
							(*count) -= newCount;
							int score = alphaBetaWithCount(newBoard, opponent, !maximizing, alpha, beta, &newCount);
							(*count) += newCount;

							if (maximizing)
							{
								if (score > bestScore)
								{
									bestScore = score;
								}
								else if (newBoard != NULL)
								{
								//	delete newBoard;
								//	b->choices[gridX][gridY][x][y] = NULL;
								}
								if (score > alpha)
									alpha = score;
								//bestScore = max(bestScore, score);
								//alpha = max(alpha, bestScore);
								if (bestScore == INT_MAX)
									return bestScore;
							}
							else {
								if (score < bestScore)
									bestScore = score;
								if (score < beta)
									beta = score;
								//bestScore = min(bestScore, score);
								//beta = min(beta, bestScore);
								if (bestScore == INT_MIN)
									return bestScore;
							}
							if (beta <= alpha)
								return beta;
						}
					}
				}
			}
		}
	}

	return bestScore;
}

/*
int MTDF(board* b, int f, int player, bool maximizing, int depth)
{
	int g = f;
	int upperBound = INT_MAX;
	int lowerBound = INT_MIN;
	while (lowerBound < upperBound)
	{
		int beta = max(g, lowerBound + 1);
		g = alphaBetaWithDepth(b, depth, player, maximizing, beta - 1, beta);
		if (g < beta)
			upperBound = g;
		else
			lowerBound = g;
	}
	return g;
}
*/

int MTDF_With_Count(board* b, int f, int player, bool maximizing, int* count)
{
	int g = f;
	int upperBound = INT_MAX;
	int lowerBound = INT_MIN;
	while (lowerBound < upperBound)
	{
		int beta = max(g, lowerBound + 1);
		g = alphaBetaWithCount(b, player, maximizing, beta-1, beta, count);
		if (g < beta)
			upperBound = g;
		else
			lowerBound = g;
	}
	return g;
}

int main()
{
	int max_timebank = 0;
	int time_per_move = 0;
	char player_names[16];
	char your_bot[8];
	int your_botid = 0;
	
	scanf("settings timebank %i\n", &max_timebank);
	scanf("settings time_per_move %i\n", &time_per_move);
	scanf("settings player_names %s\n", player_names);
	scanf("settings your_bot %s\n", your_bot);
	scanf("settings your_botid %i\n", &your_botid);
	/*
	max_timebank = 10000;
	time_per_move = 500;
	strncpy(player_names, string("player1,player2").c_str(), sizeof(player_names));
	strncpy(your_bot, string("player1").c_str(), sizeof(your_bot));
	your_botid = 1;
	*/
	board* b = new board();
	int round = 0;
	int move = 0;
	int timebank = 0;
	int nodesPerMs = -1;
	float usedRatio = 1.0f;
	bool couldlose = false;

	int prevScore = 0;

	while (1)
	{
		
		scanf("update game round %i\n", &round);
		scanf("update game move %i\n", &move);
		scanf("update game field ");
		/*
		round = 19;
		move = 37;
		string field = "1,0,2,0,2,0,0,0,2,1,2,0,0,0,0,1,0,0,0,0,0,1,1,0,1,0,0,0,1,0,2,0,1,0,0,0,0,0,2,0,1,0,2,0,1,0,1,2,1,0,0,2,0,1,2,0,0,0,2,0,1,0,2,2,2,2,0,2,0,0,0,1,0,1,0,0,2,1,1,0,2";
		stringstream ss(field);
		int index = 0;
		*/

		for (int gridY = 0; gridY < 3; ++gridY)
		{
			for (int y = 0; y < 3; ++y)
			{
				for (int gridX = 0; gridX < 3; ++gridX)
				{
					for (int x = 0; x < 3; ++x)
					{
						scanf("%i,", &(b->field[gridX][gridY][x][y]));
						/*
						ss >> index;
						b->field[gridX][gridY][x][y] = index;
						if (ss.peek() == ',')
							ss.ignore();
						*/
					}
				}
			}
		}
		scanf("\nupdate game macroboard ");
		/*
		string macroBoard = "0,0,-1,0,1,0,2,2,0";
		stringstream newss(macroBoard);
		*/
		for (int gridY = 0; gridY < 3; ++gridY)
		{
			for (int gridX = 0; gridX < 3; ++gridX)
			{
				scanf("%i,", &(b->macroBoard[gridX][gridY]));
				/*
				newss >> index;
				b->macroBoard[gridX][gridY] = index;
				if (newss.peek() == ',')
					newss.ignore();
				*/
			}
		}
		scanf("\naction move %i", &timebank);
		//timebank = 10000;
		if (move == 1)
		{
			cout << "place_move 4 4" << endl;
		} else {
			/*
			if (prevBoard != NULL)
			{
				for (int gridY = 0; gridY < 3; ++gridY)
				{
					for (int gridX = 0; gridX < 3; ++gridX)
					{
						if (prevBoard->macroBoard[gridX][gridY] == -1)
						{
							for (int y = 0; y < 3; ++y)
							{
								for (int x = 0; x < 3; ++x)
								{
									if (prevBoard->field[gridX][gridY][x][y] == 0)
									{
										if (b->field[gridX][gridY][x][y] != 0)
										{
											memcpy(b->choices, prevBoard->choices[gridX][gridY][x][y]->choices, sizeof(board*) * 9 * 9);
											goto foundOpponentChoice;
										}
									}
								}
							}
						}
					}
				}
			}
			foundOpponentChoice:
			*/

			b->evaluateMoveCount();
			b->evaluateScore();

			if (nodesPerMs <= 0)
			{
				int count = 0;
				int start_time = clock();
				do {
					int testCount = 10000;
					//alphaBetaWithCount(b, your_botid, true, INT_MIN, INT_MAX, &testCount);
					MTDF_With_Count(b, prevScore, your_botid, true, &testCount);
					count += (10000 - testCount);
				} while ((double)(clock() - start_time) / (CLOCKS_PER_SEC / 1000.0) < 200);
				nodesPerMs = count / 200;
			}

			int allocatedTime = 500 - min(10000 - timebank,400);
			if (round > 15)
			{
				allocatedTime = timebank / 4;
			}
			if (couldlose)
				allocatedTime = timebank / 2;

			int count = (int)((nodesPerMs * allocatedTime / max(usedRatio,0.1f)));
			if (count < 0)
				count = INT_MAX;
			int start_count = count;
			cerr << "Round " << round << ": Allocating " << count << " nodes to be searched in " << allocatedTime << " ms." << endl;
			int start_time = clock();

			int best_score = INT_MIN;
			int best_index_x = -1;
			int best_index_y = -1;
			for (int gridY = 0; gridY < 3; ++gridY)
			{
				for (int y = 0; y < 3; ++y)
				{
					for (int gridX = 0; gridX < 3; ++gridX)
					{
						for (int x = 0; x < 3; ++x)
						{
							if (b->macroBoard[gridX][gridY] == -1 && b->field[gridX][gridY][x][y] == 0)
							{
								if (best_index_x == -1)
								{
									best_index_x = 3 * gridX + x;
									best_index_y = 3 * gridY + y;
								}
								board* newBoard = b->copy();
								newBoard->play_move(your_botid, 3 * gridX + x, 3 * gridY + y);
								int newCount = count / b->getMoveCount();
								count -= newCount;
								//int newScore = alphaBetaWithCount(newBoard, getOpponent(your_botid), false, best_score, INT_MAX, &newCount);
								int newScore = MTDF_With_Count(newBoard, prevScore, getOpponent(your_botid), false, &newCount);
								count += newCount;
								delete newBoard;
								if (newScore > best_score)
								{
									best_score = newScore;
									best_index_x = 3 * gridX + x;
									best_index_y = 3 * gridY + y;
									//delete prevBoard;
									//prevBoard = newBoard;
								}
								else {
									
								}
								cerr << newScore << " (" << (clock() - start_time) / (CLOCKS_PER_SEC / 1000) << "ms)";

								if (!couldlose && newScore == INT_MIN)
								{
									couldlose = true;
								}
								if (newScore == INT_MAX)
								{
									cout << endl;
									goto foundMax;
								}
							}
							else {
								cerr << "[ " << b->field[gridX][gridY][x][y] << " ]";
							}
							cerr << " ";
						}
						cerr << " ";
					}
					cerr << endl;
				}
				cerr << endl;
			}
			foundMax:
			int stop_time = clock();
			int actualTime = max((int)((stop_time - start_time) / (CLOCKS_PER_SEC / 1000)), 1);
			int usedCount = start_count - count;
			usedRatio = (float)usedCount / start_count;
			nodesPerMs = usedCount / actualTime;

			cout << "place_move " << best_index_x << " " << best_index_y << endl;
			cerr << "Found score " << best_score << " in " << actualTime << " ms, with " << usedCount << " nodes (" << usedRatio * 100 << " %) = " << (int)((float)usedCount/actualTime*1000) << " nodes per second." << endl;

			prevScore = best_score;
		}
		scanf("\n");
	}

	while (1);
}