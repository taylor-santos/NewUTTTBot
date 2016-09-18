#include <iostream>
#include <fstream>

using namespace std;

int main()
{
	ofstream file;
	file.open("scores.txt");
	for (int i = 0; i < pow(3, 9); ++i)
	{
		int val = i;
		int horizP1[3] = { 0,0,0 };
		int horizP2[3] = { 0,0,0 };
		int vertP1[3] = { 0,0,0 };
		int vertP2[3] = { 0,0,0 };
		int diagP1[2] = { 0,0 };
		int diagP2[2] = { 0,0 };
		for (int y = 0; y < 3; ++y)
		{
			for (int x = 0; x < 3; ++x)
			{
				int box = val % 3;
				val /= 3;
				if (box == 1)
				{
					horizP1[y]++;
					vertP1[x]++;
					if (x == y)
						diagP1[0]++;
					if (x == 2 - y)
						diagP1[1]++;
				}
				else if (box == 2)
				{
					horizP2[y]++;
					vertP2[x]++;
					if (x == y)
						diagP2[0]++;
					if (x == 2 - y)
						diagP2[1]++;
				}
				//cout << box;
			}
			//cout << endl;
		}
		int score = 0;

		int completionsP1 = 0;
		int completionsP2 = 0;

		if (diagP1[0] == 3)
			completionsP1++;
		if (diagP2[0] == 3)
			completionsP2++;
		if (diagP1[1] == 3)
			completionsP1++;
		if (diagP2[1] == 3)
			completionsP2++;

		for (int j = 0; j < 3; ++j)
		{
			if (horizP1[j] == 3)
				completionsP1++;
			if (horizP2[j] == 3)
				completionsP2++;
			if (vertP1[j] == 3)
				completionsP1++;
			if (vertP2[j] == 3)
				completionsP2++;
		}

		if (completionsP1 + completionsP2 > 1)
		{
			score = 0;
		}
		else if (completionsP1 == 1)
		{
			score = 30;
		}
		else if (completionsP2 == 1)
		{
			score = -30;
		}
		else {
			for (int j = 0; j < 3; ++j)
			{
				if (horizP2[j] == 0)
					score += pow(horizP1[j], 2);
				else if (horizP1[j] == 0)
					score -= pow(horizP2[j], 2);
				if (vertP2[j] == 0)
					score += pow(vertP1[j], 2);
				else if (vertP1[j] == 0)
					score -= pow(vertP2[j], 2);
			}

			if (diagP2[0] == 0)
				score += pow(diagP1[0], 2);
			else if (diagP1[0] == 0)
				score -= pow(diagP2[0], 2);
			if (diagP2[1] == 0)
				score += pow(diagP1[1], 2);
			else if (diagP1[1] == 0)
				score -= pow(diagP2[1], 2);
		}
		file << score << endl;
		//cout << score << endl << endl;
	}
	file.close();
	file.open("gridWinners.txt");
	for (int i = 0; i < pow(3, 9); ++i)
	{
		int val = i;
		int box[3][3];
		bool tie = true;
		for (int y = 0; y < 3; ++y)
		{
			for (int x = 0; x < 3; ++x)
			{
				box[x][y] = val % 3;
				val /= 3;
				if (box[x][y] == 0)
					tie = false;
			}
		}
		for (int i = 0; i < 3; ++i)
		{
			if (box[i][0] == 1 && box[i][1] == 1 && box[i][2] == 1)
			{
				file << "1" << endl;
				goto foundWinner;
			}
			if (box[i][0] == 2 && box[i][1] == 2 && box[i][2] == 2)
			{
				file << "2" << endl;
				goto foundWinner;
			}
			if (box[0][i] == 1 && box[1][i] == 1 && box[2][i] == 1)
			{
				file << "1" << endl;
				goto foundWinner;
			}
			if (box[0][i] == 2 && box[1][i] == 2 && box[2][i] == 2)
			{
				file << "2" << endl;
				goto foundWinner;
			}
		}
		if (box[0][0] == 1 && box[1][1] == 1 && box[2][2] == 1)
		{
			file << "1" << endl;
			goto foundWinner;
		}
		if (box[0][0] == 2 && box[1][1] == 2 && box[2][2] == 2)
		{
			file << "2" << endl;
			goto foundWinner;
		}
		if (box[2][0] == 1 && box[1][1] == 1 && box[0][2] == 1)
		{
			file << "1" << endl;
			goto foundWinner;
		}
		if (box[2][0] == 2 && box[1][1] == 2 && box[0][2] == 2)
		{
			file << "2" << endl;
			goto foundWinner;
		}
		if (tie)
		{
			file << "3" << endl;
		}
		else {
			file << "0" << endl;
		}
	foundWinner:
		continue;
	}
	file.close();
	file.open("boardScores.txt");
	for (int i = 0; i < pow(4, 9); ++i)
	{
		int box[3][3];
		bool tie = true;
		int val = i;
		int horizP1[3] = { 0,0,0 };
		int horizP2[3] = { 0,0,0 };
		int horizTie[3] = { 0,0,0 };
		int vertP1[3] = { 0,0,0 };
		int vertP2[3] = { 0,0,0 };
		int vertTie[3] = { 0,0,0 };
		int diagP1[2] = { 0,0 };
		int diagP2[2] = { 0,0 };
		int diagTie[2] = { 0,0 };
		for (int y = 0; y < 3; ++y)
		{
			for (int x = 0; x < 3; ++x)
			{
				box[x][y] = val % 4;
				val /= 4;
				if (box[x][y] == 0)
					tie = false;
				if (box[x][y] == 1)
				{
					horizP1[y]++;
					vertP1[x]++;
					if (x == y)
						diagP1[0]++;
					if (x == 2 - y)
						diagP1[1]++;
				}
				else if (box[x][y] == 2)
				{
					horizP2[y]++;
					vertP2[x]++;
					if (x == y)
						diagP2[0]++;
					if (x == 2 - y)
						diagP2[1]++;
				}
				else if (box[x][y] == 3)
				{
					horizTie[y]++;
					vertTie[x]++;
					if (x == y)
						diagTie[0]++;
					if (x == 2 - y)
						diagTie[1]++;
				}
			}
		}
		for (int i = 0; i < 3; ++i)
		{
			if (box[i][0] == 1 && box[i][1] == 1 && box[i][2] == 1)
			{
				file << SHRT_MAX << endl;
				goto foundWinner2;
			}
			if (box[i][0] == 2 && box[i][1] == 2 && box[i][2] == 2)
			{
				file << SHRT_MIN+1 << endl;
				goto foundWinner2;
			}
			if (box[0][i] == 1 && box[1][i] == 1 && box[2][i] == 1)
			{
				file << SHRT_MAX << endl;
				goto foundWinner2;
			}
			if (box[0][i] == 2 && box[1][i] == 2 && box[2][i] == 2)
			{
				file << SHRT_MIN+1 << endl;
				goto foundWinner2;
			}
		}
		if (box[0][0] == 1 && box[1][1] == 1 && box[2][2] == 1)
		{
			file << SHRT_MAX << endl;
			goto foundWinner2;
		}
		if (box[0][0] == 2 && box[1][1] == 2 && box[2][2] == 2)
		{
			file << SHRT_MIN+1 << endl;
			goto foundWinner2;
		}
		if (box[2][0] == 1 && box[1][1] == 1 && box[0][2] == 1)
		{
			file << SHRT_MAX << endl;
			goto foundWinner2;
		}
		if (box[2][0] == 2 && box[1][1] == 2 && box[0][2] == 2)
		{
			file << SHRT_MIN+1 << endl;
			goto foundWinner2;
		}
		if (tie)
		{
			file << "0" << endl;
		}
		else {
			int score = 0;
			for (int j = 0; j < 3; ++j)
			{
				if (horizTie[j] == 0)
				{
					if (horizP2[j] == 0)
						score += pow(horizP1[j], 2);
					else if (horizP1[j] == 0)
						score -= pow(horizP2[j], 2);
				}
				if (vertTie[j] == 0)
				{
					if (vertP2[j] == 0)
						score += pow(vertP1[j], 2);
					else if (vertP1[j] == 0)
						score -= pow(vertP2[j], 2);
				}
			}

			if (diagP2[0] == 0)
				score += pow(diagP1[0], 2);
			else if (diagP1[0] == 0)
				score -= pow(diagP2[0], 2);
			if (diagP2[1] == 0)
				score += pow(diagP1[1], 2);
			else if (diagP1[1] == 0)
				score -= pow(diagP2[1], 2);

			file << score * 30 << endl;
		}
	foundWinner2:
		continue;
	}
	return 0;
}