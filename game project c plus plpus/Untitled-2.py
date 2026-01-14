/*
Program Name: Simple Grid Treasure Game
Author: Student
Course: Gaming / Programming

Purpose:
This program is a simple grid-based game created for extra credit.
The player moves around a 2D board to collect treasures while avoiding obstacles.
This project demonstrates the use of:
- A two-dimensional array (game board)
- A vector (to store collected treasures)
- Arrays and vectors working together
*/

#include <iostream>
#include <vector>
using namespace std;

// Board size constants
const int ROWS = 5;
const int COLS = 5;

// Function Prototypes
void displayBoard(char board[ROWS][COLS]);
bool isValidMove(int r, int c);

int main() {
    /*
    Main Function
    Pre-condition: None
    Post-condition: Runs the game loop until the player quits or collects all treasures
    */

    // 2D array representing the game board
    char board[ROWS][COLS] = {
        {'P', '.', 'T', '.', '.'},
        {'.', '#', '.', '#', '.'},
        {'.', '.', '.', '.', 'T'},
        {'#', '.', '#', '.', '.'},
        {'.', '.', '.', 'T', '.'}
    };

    // Vector to store collected treasures
    vector<string> collectedTreasures;

    int playerRow = 0;
    int playerCol = 0;
    char move;

    cout << "Welcome to the Treasure Game!" << endl;

    // Game loop
    while (true) {
        displayBoard(board);
        cout << "Move (W/A/S/D) or Q to quit: ";
        cin >> move;

        int newRow = playerRow;
        int newCol = playerCol;

        // Determine player movement
        if (move == 'W' || move == 'w') newRow--;
        else if (move == 'S' || move == 's') newRow++;
        else if (move == 'A' || move == 'a') newCol--;
        else if (move == 'D' || move == 'd') newCol++;
        else if (move == 'Q' || move == 'q') break;
        else continue;

        // Check if move is valid
        if (isValidMove(newRow, newCol) && board[newRow][newCol] != '#') {
            // Update player position
            if (board[newRow][newCol] == 'T') {
                collectedTreasures.push_back("Treasure");
                cout << "You collected a treasure!" << endl;
            }

            board[playerRow][playerCol] = '.';
            playerRow = newRow;
            playerCol = newCol;
            board[playerRow][playerCol] = 'P';
        }

        // End condition
        if (collectedTreasures.size() == 3) {
            cout << "You collected all treasures. You win!" << endl;
            break;
        }
    }

    cout << "Game Over. Treasures collected: " << collectedTreasures.size() << endl;
    return 0;
}

// Displays the game board
void displayBoard(char board[ROWS][COLS]) {
    /*
    Function: displayBoard
    Purpose: Prints the current game board to the screen
    Pre-condition: Board must be initialized
    Post-condition: Board is displayed
    */
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            cout << board[i][j] << " ";
        }
        cout << endl;
    }
}

// Checks if a move is inside the board
bool isValidMove(int r, int c) {
    /*
    Function: isValidMove
    Purpose: Checks if player stays within board boundaries
    Pre-condition: Row and column provided
    Post-condition: Returns true if valid, false otherwise
    */
    return (r >= 0 && r < ROWS && c >= 0 && c < COLS);
}
