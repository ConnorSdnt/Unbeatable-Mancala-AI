#include <iostream>
#include <vector>
#include <iomanip>

#define PLAYER1 0
#define PLAYER2 1
#define NUM_PITS 6
#define NUM_STONES 4
#define DEPTH 14
#define DEBUG

class Board {
    public:
        Board() : pits(2, std::vector<int>(NUM_PITS, NUM_STONES)), store(2, 0), lastStoneInStore(false){}
        //Displays the board in its current state
        void displayBoard() {

            std::cout << "\n";

            //Display Player 2's pits
            int counter = NUM_PITS - 1;
            for(int i = NUM_PITS - 1; i >= 0; i--) {
                std::cout << std::setw(3) << counter;
                counter--;
            }
            std::cout << std::endl;

            std::cout << "--------------------\n";

            for(int i = NUM_PITS - 1; i >= 0; i--) {
                std::cout << std::setw(3) << pits[1][i];
            }
            std::cout << std::endl;

            //Display the stores
            std::cout << store[1] << std::setw((NUM_PITS * 3) + 1) << store[0];
            std::cout << std::endl;

            //Display Player 1's pits
            for(int i = 0; i < NUM_PITS; i++) {
                std::cout << std::setw(3) << pits[0][i];
            }
            std::cout << std::endl;

            std::cout << "--------------------\n";

            counter = 0;
            for(int i = NUM_PITS - 1; i >= 0; i--) {
                std::cout << std::setw(3) << counter;
                counter++;
            }
            std::cout << "\n" << std::endl;

        }

        //distributes the stones from the pit chosen by the player in Player.makeMove()
        void distributeStones(int pitIndex, int currentPlayer, bool sim) {
            bool falseExtraTurn = false; 
            player = currentPlayer;
            //Sets the value 'stones' to the value of the chosen pit
            int stones = pits[currentPlayer][pitIndex];
            //Sets the side of the board to be that of the current player
            int playerSide = currentPlayer;
            //Sets the chosen pit to 0 stones and moves the pitIndex forward
            pits[currentPlayer][pitIndex] = 0;
            pitIndex++;
            
            while (stones > 0) {

                if (pitIndex < NUM_PITS) {
                    pits[playerSide][pitIndex]++;
                    pitIndex++;
                    stones--;
                    if (pitIndex == NUM_PITS && stones == 0) {
                        // std::cout << "fasle turn" << std::endl;
                        falseExtraTurn = true;
                    }
                } else {
                    //Handles the event that pitIndex becomes equal to NUM_PIT
                    if (playerSide == currentPlayer) {
                        store[currentPlayer]++;
                        stones--;
                    }
                    playerSide = switchSides(playerSide);
                    if (stones != 0) {
                        pitIndex = 0;
                    }
                }

            }

            //Checks if the last stone deposited was in an empty pit on the players side
            if (playerSide == currentPlayer && pits[playerSide][pitIndex - 1] == 1 && pits[switchSides(currentPlayer)][NUM_PITS - pitIndex] > 0) {
                stealStones(playerSide, currentPlayer, pitIndex);
                if (sim == false){
                    std::cout << "You have stolen your opponents stones!!" << std::endl;
                }
            }

            //Checks whether the last stone was deposited in the players store
            if (pitIndex == NUM_PITS && playerSide != currentPlayer && falseExtraTurn == false) {
                lastStoneInStore = true;
                // std::cout << "last stone activated" << std::endl;
            } else {
                lastStoneInStore = false;
                falseExtraTurn = false;
            }
        }

        //Switches the current playing side. i.e player 1's side to player 2's side
        int switchSides(int playerSide) {
            return (playerSide == PLAYER1) ? PLAYER2 : PLAYER1;
        }

        //Returns a flag
        bool isLastStoneInStore() {
            return lastStoneInStore;
        }

        //Checks whether the game has ended
        bool checkForWinner() {
            bool player1SideEmpty = std::all_of(pits[PLAYER1].begin(), pits[PLAYER1].end(), [](int stones) {return stones == 0;});
            bool player2SideEmpty = std::all_of(pits[PLAYER2].begin(), pits[PLAYER2].end(), [](int stones) {return stones == 0;});

            if (player1SideEmpty) {
                remainingStones(PLAYER2);
                return true;
            } else if (player2SideEmpty) {
                remainingStones(PLAYER1);
                return true;
            } else {
                return false;
            }

        }
    
        //Returns the players score
        int getPlayerScore(int playerID) {
            return store[playerID];
        }

        //Returns the value of the stones within a specified pit
        int getPitValue(int playerID, int selectedPit) {
            return pits[playerID][selectedPit];
        }

        //Returns the current player
        int getCurrentPlayer() {
            return player;
        }

    private:
        std::vector<std::vector<int> > pits;
        std::vector<int> store;
        bool lastStoneInStore;
        int player;

        //Steals the opponents stones 
        void stealStones(int playerSide, int currentPlayer, int pitIndex) {
            //Adds the stones stored in the players pit to their score
            store[playerSide] += pits[playerSide][pitIndex - 1];
            //Sets the players pit to 0
            pits[playerSide][pitIndex - 1] = 0;
            //Adds the stones stored in the opposite pit to the players store
            store[playerSide] += pits[switchSides(currentPlayer)][NUM_PITS - pitIndex];
            //Sets the opposite pits stones to 0
            pits[switchSides(currentPlayer)][NUM_PITS - pitIndex] = 0; 
        }

        //Adds all remaining stones at the end of the game to that players store
        void remainingStones(int playerID) {
            for (int i = 0; i < NUM_PITS; i++) {
                store[playerID] += pits[playerID][i];
                pits[playerID][i] = 0;
            }
        }   

};

class AI {
    public:
        AI(int id) : playerID(id), playerNickname(id + 1) {}

        void makeMove(Board& board) {
            std::cout << "\nPlayer " << playerNickname << " (AI) is making a move...\n";

            int depth = DEPTH;
            int alpha = std::numeric_limits<int>::min();
            int beta = std::numeric_limits<int>::max();
            int bestMove = -1;

            for (int i = 0; i < NUM_PITS; i++) {
                if (board.getPitValue(playerID, i) > 0) {
                    Board tempBoard = board;
                    tempBoard.distributeStones(i, playerID, true);

            //Detects and executes extra turns that are earned during the simulation
                    int extraTurnAlpha = std::numeric_limits<int>::min();
                    int extraTurnBeta = std::numeric_limits<int>::max();
                    if (tempBoard.isLastStoneInStore() == true && tempBoard.getCurrentPlayer() == playerID) {
                        int score = minimax(tempBoard, depth - 1, extraTurnAlpha, extraTurnBeta, true);
                        if (score > extraTurnAlpha) {
                            extraTurnBeta = score;
                            bestMove = i;
                        }
                        break;
                    }

                    int score = minimax(tempBoard, depth - 1, alpha, beta, false);

                    if (score > alpha) {
                        alpha = score;
                        bestMove = i;
                    }
                }
            }

            board.distributeStones(bestMove, playerID, false);
        }
        
    private:
        int minimax(Board& board, int depth, int alpha, int beta, bool maximisingPlayer) {
            if (depth == 0 || board.checkForWinner()) {
                return evaluate(board);
            }

            if (maximisingPlayer) {
                int maxEval = std::numeric_limits<int>::min();
                for (int i = 0; i < NUM_PITS; ++i) {
                    if (board.getPitValue(playerID, i) > 0) {
                        Board tempBoard = board;
                        tempBoard.distributeStones(i, playerID, true);
                        int eval = minimax(tempBoard, depth - 1, alpha, beta, false);
                        maxEval = std::max(maxEval, eval);
                        alpha = std::max(alpha, eval);
                        if (beta <= alpha) {
                            break;
                        }
                    }
                }   
                return maxEval;     
            } else {
                int minEval = std::numeric_limits<int>::max();
                for (int i = 0; i < NUM_PITS; ++i) {
                    if (board.getPitValue(board.switchSides(playerID), i) > 0) {
                        Board tempBoard = board;
                        tempBoard.distributeStones(i, board.switchSides(playerID), true);
                        int eval = minimax(tempBoard, depth - 1, alpha, beta, true);
                        minEval = std::min(minEval, eval);
                        beta = std::min(beta, eval);
                        if (beta <= alpha) {
                            break;
                        }
                    }
                }
                return minEval;
            }
        }

        int evaluate(Board& board) {
            int scoreDifference = (board.getPlayerScore(playerID) - board.getPlayerScore(board.switchSides(playerID))) * 5;
            int lastStoneInStoreBonus = (board.isLastStoneInStore() && board.getCurrentPlayer() == playerID) ? 5:0;

            return scoreDifference + setUpPenalty(board) + lastStoneInStoreBonus + setUpBonus(board);
        }

        int setUpPenalty(Board& board) {
            int penalty = 0;
            for (int i = 0; i < NUM_PITS; i++) {
                if (board.getPitValue(board.switchSides(playerID), i) == (NUM_PITS - 1)) {
                    penalty = penalty + 10;
                }
            }
            return penalty;
        }

        int setUpBonus(Board & board) {
            int bonus = 0;
            for (int i = 0; i < NUM_PITS; i++) {
                if (board.getPitValue(board.switchSides(playerID), i) == (NUM_PITS - 1)) {
                    bonus = bonus + 5;
                }
            }
            return bonus;
        }

        int playerID;
        int playerNickname;
};

class Player {
    public:
        Player(int id) : playerID(id), playerNickname(id + 1) {}
        void makeMove(Board& board) {
            int pitIndex = playerInput(board);
            board.distributeStones(pitIndex, playerID, false);
        }

    private:
        int playerInput(Board& board) {
            int selectedPit;
            bool validInput = false;

            while (!validInput) {
                std::cout << "\nPlayer " << playerNickname << " please select a pit: \n";
                std::cin >> selectedPit;

                if (std::cin.fail()) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input. Please enter a valid integer.\n";
                } else if (selectedPit < 0 || selectedPit >= NUM_PITS) {
                    std::cout << "Invalid input. Please enter an integer between 0 and " << NUM_PITS - 1 << ".\n";
                } else if (board.getPitValue(playerID, selectedPit) == 0) {
                    std::cout << "Selected pit is empty. Please select a pit with stones.\n";
                } else {
                    validInput = true;
                }
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return selectedPit;
        }
        int playerID;
        int playerNickname;
};

class Game {
    public:
        Game() : currentPlayer(PLAYER1), playerP1(PLAYER1), playerP2(PLAYER2), aiP1(PLAYER1), aiP2(PLAYER2) {}

        void start() {
            std::cout << "\n\n-----MANCALA-----\n" << std::setw(14) << "\n--New Game--\n" << std::endl;

            char userChoice;
            bool validResponse = false;

            while (!validResponse) {
                std::cout << "Do you want to make the first move (Y/N): ";
                std::cin >> userChoice;
                std::cout << std::endl;

                if (userChoice == 'Y' || userChoice == 'y' || userChoice == 'N' || userChoice == 'n') {
                    validResponse = true;
                }
            }

            while (!board.checkForWinner()) {
                board.displayBoard();

                if (currentPlayer == PLAYER1) {
                    if (userChoice == 'Y' || userChoice == 'y') {
                        playerP1.makeMove(board);
                    } else {
                        aiP1.makeMove(board);
                    }
                } else {
                    if (userChoice == 'Y' || userChoice == 'y') {
                        aiP2.makeMove(board);
                    } else {
                        playerP2.makeMove(board);
                    }
                }

                if (board.isLastStoneInStore() && currentPlayer == board.getCurrentPlayer()) {
                    std::cout << "\nYou have earned an extra turn!\n" << std::endl;
                } else {
                    switchTurn();
                }
            }

            end();
        }

        void end() {
            int player1Score = board.getPlayerScore(PLAYER1);
            int player2Score = board.getPlayerScore(PLAYER2);

            board.displayBoard();

            std::cout << "SCORES: \n" << "  Player 1: " << player1Score << "\nPlayer 2: " << player2Score;
            std::cout << std::endl;

            if (player1Score == player2Score) {
                std::cout << "DRAW";
            } else if (player1Score > player2Score) {
                std::cout << "WINNER: PLAYER 1!";
            } else {
                std::cout << "WINNER: PLAYER 2!";
            }
            std::cout << std::endl;
        }

        void switchTurn() {
            currentPlayer = (currentPlayer == PLAYER1) ? PLAYER2 : PLAYER1;
        }
    
    private:
        Player playerP1;
        Player playerP2;
        AI aiP1;
        AI aiP2;
        Board board;
        int currentPlayer;
        bool gameStarted;
};

int main() {
    while (true) {
        Game game;
        char response;

        game.start();

        while (true) {
            std::cout << "Would you like to play again? (Y/N)" << std::endl;
            std::cin >> response;

            if (response == 'Y' || response == 'y') {
                break;
            } else if (response == 'N' || response == 'n') {
                return 0;
            } else {
                std::cout << "Invalid response. Choose either (Y/N): " << std::endl;
            }
        }
    }

    return 0;
}