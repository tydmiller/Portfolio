/*
file: Board.cpp
author: Tyler Miller
Digipen email: miller.t@digipen.edu
Assignment #2
date: 9-13-18
Description: this program checks the functions list bellow that
creates, displays, and manages a tic tac toe game
*/

#include <iostream>
#include "Board.h"
#include "TicTacToe.h"

namespace CS170
{
	struct Board
	{
		int *data; // The 2D game board represented as an array.
	};
	/*This creates the board*/
	Board * BoardCreate()
	{
		Board *theBoard = new Board;
		theBoard->data = new int[boardLength * boardLength];
		for (int j = 0; boardLength * boardLength > j; j++)
		{
			theBoard->data[j] = tsEMPTY;
		}
		return theBoard;
	}
	/*This frees the board from the memory*/
	void BoardFree(Board **theBoard)
	{
		delete[] (*theBoard)->data;
		delete *theBoard;
		*theBoard = nullptr;
	}
	/*This displays the board*/
	void BoardDisplay(const Board &board)
	{
		std::cout << " -------------" << std::endl;
		for(int i = 0; boardLength > i; i++)
		{
			std::cout << " ";
			for (int j = 0; boardLength > j; j++)
			{
				int state = board.data[i * 3 + j];
				std::cout << '|';
				if (state == tsEMPTY)
				{
					std::cout << "   ";
				}
				if (state == tsPLAYER_ONE)
				{
					std::cout << " X ";
				}
				if (state == tsPLAYER_TWO)
				{
					std::cout << " O ";
				}
			}
			std::cout << '|';
			std::cout << '\n';
			std::cout << " -------------" << std::endl;
		}
	}
	/*This gets the board and returns the output*/
	PlaceResult BoardPlaceToken(Board &board, unsigned row, unsigned column, TileState value)
	{
		unsigned int index = row * 3 + column;
		if (row > 2 || column > 2)
		{
			return prREJECTED_OUTOFBOUNDS;
		}

		int place = board.data[index];

		if (place == tsPLAYER_ONE || place == tsPLAYER_TWO)
		{
			return prREJECTED_OCCUPIED;
		}
		if (place == tsEMPTY)
		{
			board.data[index] = value;
			return prACCEPTED;
		}
		return PlaceResult();
	}
	/*This clears the board*/
	void BoardClear(Board &board)
	{
		for (int j = 0; boardLength * boardLength > j; j++)
		{
			board.data[j] = tsEMPTY;
		}
	}
	/*This checks for win statements*/
	BoardState BoardGetState(const Board & board)
	{
		bool EmptySpace = false;
		//horizontal check
			for (int j = 0; boardLength > j; j++)
			{
				//check for empty tiles horizontally
				if(board.data[3 * j] == tsEMPTY || board.data[3 * j + 1] == tsEMPTY || board.data[3 * j + 2] == tsEMPTY)
				{
					EmptySpace = true; 
				}
				if(board.data[3 * j] == board.data[3 * j + 1] && board.data[3 * j] == board.data[3 * j + 2])
				{
					if (board.data[3 * j] == tsPLAYER_ONE)
					{
						return bsWIN_ONE;
					}
					if (board.data[3 * j] == tsPLAYER_TWO)
					{
						return bsWIN_TWO;
					}
				}
			}
			//vertical check
			for (int i = 0; boardLength > i; i++)
			{
				if (board.data[i] == board.data[i + 3] && board.data[6 + i] == board.data[i])
				{
					if (board.data[i] == tsPLAYER_ONE)
					{
						return bsWIN_ONE;
					}
					if (board.data[i] == tsPLAYER_TWO)
					{
						return bsWIN_TWO;
					}
				}
			}
			//diagonal test upper left to lower right
			
			if (board.data[0] == board.data[4] && board.data[8] == board.data[4])
			{
				if (board.data[0] == tsPLAYER_ONE)
				{
					return bsWIN_ONE;
				}
				if (board.data[0] == tsPLAYER_TWO)
				{
					return bsWIN_TWO;
				}
			}
			if (board.data[2] == board.data[4] && board.data[6] == board.data[4])
			{
				if (board.data[2] == tsPLAYER_ONE)
				{
					return bsWIN_ONE;
				}
				if (board.data[2] == tsPLAYER_TWO)
				{
					return bsWIN_TWO;
				}
			}
		if (EmptySpace == true)
		{
			return bsOPEN;
		}
		if (EmptySpace == false)
		{
			return bsTIE;
		}
		return BoardState();
	}
}
