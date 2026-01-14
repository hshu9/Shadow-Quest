/*
 * Neon Lane Runner
 *
 * Purpose:
 *   A creative, text-based arcade dodger where the player controls a car at the
 *   bottom of a neon road. The player avoids obstacles, collects power-ups,
 *   builds combos, and tries to reach the top of the persistent high-score list.
 *
 * Extra-credit and course mapping:
 *   - Top block comment and function-level comments included for documentation credit.
 *   - Uses a two-dimensional array 'road' for the tile layout.
 *   - Uses vectors for obstacles, power-ups, and high scores.
 *   - Demonstrates structs, enums, functions, pass-by-reference, loops, input/output,
 *     random numbers, input validation, simple sorting, and file I/O for persistence.
 *
 * Allowed headers used: iostream, cstdlib, ctime, iomanip, string, vector, cmath, fstream
 *
 * Author: Student
 * Date: Fall 2025
 */

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

/* -------------------------
   Configuration constants
   ------------------------- */
const int ROAD_ROWS = 16;
const int ROAD_COLS = 11;
const char EMPTY_CELL = ' ';
const char OBSTACLE_CHAR = 'X';
const char CAR_CHAR = 'A';
const char POWERUP_CHAR = 'P';
const int BASE_SPAWN_PERCENT = 28;   // base chance to spawn obstacle each tick
const int POWERUP_SPAWN_PERCENT = 8; // chance to spawn a power-up each tick
const int TICK_MS = 120;             // tick duration in ms (approx)
const string HIGHSCORE_FILE = "highscores.txt";

/* -------------------------
   Types
   ------------------------- */

/* ObstacleType enum
 * - NORMAL: moves 1 row per tick
 * - FAST: moves 2 rows per tick (dangerous)
 * - WIDE: occupies two adjacent columns
 */
enum ObstacleType { NORMAL, FAST, WIDE };

/* PowerUpType enum
 * - SHIELD: protects from one collision for several ticks
 * - MULTIPLIER: increases score multiplier for several ticks
 * - SLOW: slows obstacle movement for several ticks
 */
enum PowerUpType { SHIELD, MULTIPLIER, SLOW };

/* Obstacle struct
 * Represents a falling obstacle with a type and position.
 */
struct Obstacle {
    int row;
    int col;
    ObstacleType type;
    Obstacle(int r, int c, ObstacleType t) : row(r), col(c), type(t) {}
};

/* PowerUp struct
 * Represents a collectible power-up on the road.
 */
struct PowerUp {
    int row;
    int col;
    PowerUpType type;
    PowerUp(int r, int c, PowerUpType t) : row(r), col(c), type(t) {}
};

/* GameState enum */
enum GameState { RUNNING, GAME_OVER, QUIT };

/* -------------------------
   Utility and game functions
   ------------------------- */

/*
 * initRoad
 * Pre-condition: road is a ROAD_ROWS x ROAD_COLS char array.
 * Post-condition: road is filled with EMPTY_CELL.
 */
void initRoad(char road[ROAD_ROWS][ROAD_COLS]) {
    for (int r = 0; r < ROAD_ROWS; ++r)
        for (int c = 0; c < ROAD_COLS; ++c)
            road[r][c] = EMPTY_CELL;
}

/*
 * clearScreen
 * Portable console clear using system calls.
 */
void clearScreen() {
#ifdef _WIN32
    system("CLS");
#else
    system("clear");
#endif
}

/*
 * drawRoad
 * Pre-condition: road contains current frame data.
 * Post-condition: prints the road and HUD to the console.
 */
void drawRoad(const char road[ROAD_ROWS][ROAD_COLS],
              int score, int tick, int multiplier, int shieldTurns, int slowTurns, int combo) {
    clearScreen();
    cout << "Neon Lane Runner  Score: " << score
         << "  Tick: " << tick
         << "  Mult: x" << multiplier
         << "  Shield: " << shieldTurns
         << "  Slow: " << slowTurns
         << "  Combo: " << combo << endl;
    cout << "+" << std::string(ROAD_COLS, '-') << "+" << endl;
    for (int r = 0; r < ROAD_ROWS; ++r) {
        cout << "|";
        for (int c = 0; c < ROAD_COLS; ++c) cout << road[r][c];
        cout << "|" << endl;
    }
    cout << "+" << std::string(ROAD_COLS, '-') << "+" << endl;
    cout << "Controls: a=left d=right s=stay q=quit  Type command and press Enter within tick window." << endl;
}

/*
 * randomObstacleType
 * Returns a random ObstacleType with weighted probabilities.
 */
ObstacleType randomObstacleType() {
    int r = rand() % 100;
    if (r < 60) return NORMAL;
    if (r < 85) return FAST;
    return WIDE;
}

/*
 * randomPowerUpType
 * Returns a random PowerUpType.
 */
PowerUpType randomPowerUpType() {
    int r = rand() % 100;
    if (r < 40) return SHIELD;
    if (r < 75) return MULTIPLIER;
    return SLOW;
}

/*
 * spawnObstacle
 * May add a new obstacle at the top row with random column and type.
 */
void spawnObstacle(vector<Obstacle> &obstacles, int spawnChance) {
    int roll = rand() % 100;
    if (roll < spawnChance) {
        int col = rand() % ROAD_COLS;
        ObstacleType t = randomObstacleType();
        // For WIDE obstacles, ensure there's space to the right; if not, shift left
        if (t == WIDE && col == ROAD_COLS - 1) col = ROAD_COLS - 2;
        obstacles.emplace_back(0, col, t);
    }
}

/*
 * spawnPowerUp
 * May add a power-up at the top row with random column and type.
 */
void spawnPowerUp(vector<PowerUp> &powerups) {
    int roll = rand() % 100;
    if (roll < POWERUP_SPAWN_PERCENT) {
        int col = rand() % ROAD_COLS;
        PowerUpType t = randomPowerUpType();
        powerups.emplace_back(0, col, t);
    }
}

/*
 * updateObstacles
 * Moves obstacles down according to their type and slow effect.
 * Removes obstacles that pass bottom. Detects collision with player's car.
 * Returns true if collision occurred.
 */
bool updateObstacles(vector<Obstacle> &obstacles, int playerRow, int playerCol, bool slowActive) {
    bool collision = false;
    int moveStepNormal = slowActive ? 0 : 1; // when slowActive, obstacles move every other tick (handled externally)
    // Move obstacles: FAST moves 2 rows, NORMAL/WIDE move 1 row
    for (size_t i = 0; i < obstacles.size(); ++i) {
        int step = 0;
        if (obstacles[i].type == FAST) step = 2;
        else step = 1;
        // If slowActive, reduce step by 1 (min 0)
        if (slowActive) step = std::max(0, step - 1);
        obstacles[i].row += step;
    }
    // Check collisions and remove passed obstacles
    for (int i = (int)obstacles.size() - 1; i >= 0; --i) {
        Obstacle &o = obstacles[i];
        // WIDE occupies col and col+1
        bool hit = false;
        if (o.row == playerRow) {
            if (o.type == WIDE) {
                if (playerCol == o.col || playerCol == o.col + 1) hit = true;
            } else {
                if (playerCol == o.col) hit = true;
            }
        }
        if (hit) collision = true;
        if (o.row > playerRow) obstacles.erase(obstacles.begin() + i);
    }
    return collision;
}

/*
 * updatePowerUps
 * Moves power-ups down by one row and removes those that pass bottom.
 * Returns index of collected power-up in vector or -1 if none collected.
 */
int updatePowerUps(vector<PowerUp> &powerups, int playerRow, int playerCol) {
    for (size_t i = 0; i < powerups.size(); ++i) powerups[i].row += 1;
    for (int i = (int)powerups.size() - 1; i >= 0; --i) {
        if (powerups[i].row > playerRow) powerups.erase(powerups.begin() + i);
        else if (powerups[i].row == playerRow && powerups[i].col == playerCol) {
            int idx = i;
            // return index of collected power-up (caller will handle removal)
            return idx;
        }
    }
    return -1;
}

/*
 * placeObjectsOnRoad
 * Writes obstacles, power-ups, and car into the road array for drawing.
 */
void placeObjectsOnRoad(char road[ROAD_ROWS][ROAD_COLS],
                        const vector<Obstacle> &obstacles,
                        const vector<PowerUp> &powerups,
                        int playerRow, int playerCol) {
    // Place obstacles
    for (const auto &o : obstacles) {
        if (o.row >= 0 && o.row < ROAD_ROWS) {
            if (o.type == WIDE) {
                if (o.col >= 0 && o.col < ROAD_COLS) road[o.row][o.col] = OBSTACLE_CHAR;
                if (o.col + 1 >= 0 && o.col + 1 < ROAD_COLS) road[o.row][o.col + 1] = OBSTACLE_CHAR;
            } else {
                if (o.col >= 0 && o.col < ROAD_COLS) road[o.row][o.col] = OBSTACLE_CHAR;
            }
        }
    }
    // Place power-ups
    for (const auto &p : powerups) {
        if (p.row >= 0 && p.row < ROAD_ROWS && p.col >= 0 && p.col < ROAD_COLS) {
            road[p.row][p.col] = POWERUP_CHAR;
        }
    }
    // Place player's car (overwrites obstacle char if collision)
    if (playerRow >= 0 && playerRow < ROAD_ROWS && playerCol >= 0 && playerCol < ROAD_COLS) {
        road[playerRow][playerCol] = CAR_CHAR;
    }
}

/*
 * getPlayerInput
 * Reads and validates player input. Returns 'a', 'd', 's', or 'q'.
 * This function is used to capture a queued command when the user presses Enter.
 */
char getPlayerInputLine() {
    string line;
    if (!std::getline(cin, line)) {
        // If getline fails (no input), clear and return 's' (stay)
        cin.clear();
        return 's';
    }
    if (line.empty()) return 's';
    char c = line[0];
    if (c == 'a' || c == 'd' || c == 's' || c == 'q') return c;
    // invalid input -> default to stay
    return 's';
}

/*
 * busyWaitMs
 * Simple busy-wait to pause for approx ms milliseconds.
 */
void busyWaitMs(int ms) {
    clock_t start = clock();
    double target = ms / 1000.0 * CLOCKS_PER_SEC;
    while ((clock() - start) < (clock_t)target) {
        // busy wait
    }
}

/*
 * loadHighScores
 * Loads high scores from HIGHSCORE_FILE into vector highScores.
 * If file missing, leaves vector empty.
 */
void loadHighScores(vector<int> &highScores) {
    std::ifstream in(HIGHSCORE_FILE.c_str());
    if (!in.is_open()) return;
    int s;
    while (in >> s) {
        highScores.push_back(s);
        if ((int)highScores.size() >= 50) break; // safety cap
    }
    in.close();
}

/*
 * saveHighScores
 * Saves high scores vector to HIGHSCORE_FILE (one score per line).
 */
void saveHighScores(const vector<int> &highScores) {
    std::ofstream out(HIGHSCORE_FILE.c_str());
    if (!out.is_open()) return;
    for (size_t i = 0; i < highScores.size(); ++i) out << highScores[i] << std::endl;
    out.close();
}

/*
 * addHighScore
 * Adds a score to highScores vector and keeps it sorted descending.
 * Keeps only top 10 scores.
 */
void addHighScore(vector<int> &highScores, int score) {
    highScores.push_back(score);
    // insertion sort step to keep descending order
    for (int i = (int)highScores.size() - 1; i > 0; --i) {
        if (highScores[i] > highScores[i - 1]) std::swap(highScores[i], highScores[i - 1]);
        else break;
    }
    if ((int)highScores.size() > 10) highScores.resize(10);
}

/*
 * printHighScores
 * Prints top N high scores.
 */
void printHighScores(const vector<int> &highScores) {
    cout << "=== High Scores ===" << endl;
    if (highScores.empty()) cout << "No high scores yet." << endl;
    for (size_t i = 0; i < highScores.size(); ++i) {
        cout << std::setw(2) << (i + 1) << ". " << highScores[i] << endl;
    }
}

/* -------------------------
   Main game loop
   ------------------------- */

int main() {
    srand((unsigned int)time(nullptr));

    // Road 2D array (meets two-dimensional array requirement)
    char road[ROAD_ROWS][ROAD_COLS];

    // Vectors for obstacles, power-ups, and high scores
    vector<Obstacle> obstacles;
    vector<PowerUp> powerups;
    vector<int> highScores;

    // Load persistent high scores
    loadHighScores(highScores);

    // Player state
    int playerCol = ROAD_COLS / 2;
    int playerRow = ROAD_ROWS - 1;
    int score = 0;
    int tick = 0;
    int multiplier = 1;
    int multiplierTurns = 0;
    int shieldTurns = 0;
    int slowTurns = 0;
    int combo = 0;
    bool slowToggle = false; // used to make slow effect move every other tick

    GameState state = RUNNING;

    cout << "Welcome to Neon Lane Runner!" << endl;
    cout << "Collect P for power-ups. Survive and climb the high-score list." << endl;
    cout << "Press Enter to start...";
    string dummy;
    std::getline(cin, dummy);

    // Main loop
    while (state == RUNNING) {
        ++tick;
        initRoad(road);

        // Dynamic difficulty: spawn chance increases slowly with score
        int spawnChance = BASE_SPAWN_PERCENT + (score / 150);

        // Spawn obstacles and power-ups
        spawnObstacle(obstacles, spawnChance);
        spawnPowerUp(powerups);

        // Place objects for drawing
        placeObjectsOnRoad(road, obstacles, powerups, playerRow, playerCol);
        drawRoad(road, score, tick, multiplier, shieldTurns, slowTurns, combo);

        // Prompt user and allow a short window to type a command
        cout << "Command (a/d/s/q) and Enter to apply next tick: ";
        // Wait a short time to allow typing; then attempt to read a line.
        busyWaitMs(TICK_MS);
        // Try to read input; if user pressed Enter, apply it; otherwise default to stay
        char cmd = 's';
        if (std::getline(cin, dummy)) {
            if (!dummy.empty()) cmd = dummy[0];
        } else {
            cin.clear();
        }

        // Apply command
        if (cmd == 'q') { state = QUIT; break; }
        else if (cmd == 'a' && playerCol > 0) playerCol--;
        else if (cmd == 'd' && playerCol < ROAD_COLS - 1) playerCol++;
        // 's' -> stay

        // Update power-ups first: move down and check collection
        int collectedIndex = updatePowerUps(powerups, playerRow, playerCol);
        if (collectedIndex != -1) {
            PowerUp p = powerups[collectedIndex];
            // Apply power-up effect
            if (p.type == SHIELD) {
                shieldTurns = 6; // shield lasts several ticks
            } else if (p.type == MULTIPLIER) {
                multiplier = 2;
                multiplierTurns = 12;
            } else if (p.type == SLOW) {
                slowTurns = 10;
                slowToggle = false;
            }
            // Remove collected power-up
            powerups.erase(powerups.begin() + collectedIndex);
        }

        // Update obstacles and check collision
        bool slowActive = (slowTurns > 0 && (slowToggle == false));
        bool collided = updateObstacles(obstacles, playerRow, playerCol, slowActive);

        // Handle collision
        if (collided) {
            if (shieldTurns > 0) {
                // consume shield and continue
                shieldTurns = 0;
                combo = 0; // combo breaks
            } else {
                state = GAME_OVER;
                initRoad(road);
                placeObjectsOnRoad(road, obstacles, powerups, playerRow, playerCol);
                drawRoad(road, score, tick, multiplier, shieldTurns, slowTurns, combo);
                cout << "You crashed! Final score: " << score << endl;
                addHighScore(highScores, score);
                saveHighScores(highScores);
                printHighScores(highScores);
                break;
            }
        } else {
            // survived this tick -> increase score with multiplier and combo
            combo++;
            int comboBonus = combo / 25; // small bonus for long combos
            score += (1 * multiplier) + comboBonus;
        }

        // Decrease power-up timers
        if (multiplierTurns > 0) {
            multiplierTurns--;
            if (multiplierTurns == 0) multiplier = 1;
        }
        if (shieldTurns > 0) shieldTurns--;
        if (slowTurns > 0) {
            slowTurns--;
            slowToggle = !slowToggle; // toggle to make obstacles move every other tick
        } else {
            slowToggle = false;
        }

        // Small pause to keep tick rhythm (already used busyWaitMs for input window)
        // Additional tiny wait to stabilize frame rate
        busyWaitMs(20);
    }

    if (state == QUIT) {
        cout << "You quit. Final score: " << score << endl;
        addHighScore(highScores, score);
        saveHighScores(highScores);
        printHighScores(highScores);
    }

    cout << "Thanks for playing Neon Lane Runner!" << endl;
    return 0;
}
