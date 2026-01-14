// Shadow Quest - A Terminal RPG Adventure Game
// NAME; Hitesh KUMAR
// Course: CSCI 14 - Fall 2025
//
// PROGRAM DESCRIPTION:
// Shadow Quest is a turn-based RPG where the player explores a grid-based world,
// battles enemies, collects items, and manages inventory. The player starts in
// a village and must defeat the Shadow Lord in the final dungeon.
//
// FEATURES:
// - 10x10 grid-based world map (2D array)
// - Turn-based combat system
// - Inventory management with arrays and vectors
// - Character stats and leveling
// - Random encounters and item drops
// - Save/load functionality


#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <cmath>

using namespace std;


// CONSTANTS

const int MAP_SIZE = 10;
const int MAX_ENEMIES = 7;
const int MAX_ITEMS = 10;
const int MAX_INVENTORY = 20;


// ENUMERATIONS

enum EnemyType { SLIME, GOBLIN, WOLF, SKELETON, TROLL, DRAGON, SHADOW_LORD };
enum ItemType { HEALTH_POTION, MANA_POTION, SWORD, SHIELD, ARMOR };
enum Terrain { GRASS, FOREST, MOUNTAIN, WATER, VILLAGE, DUNGEON, BOSS_ROOM };

// STRUCTURES


// Player character structure
struct Player {
    string name;
    int hp;
    int maxHp;
    int mp;
    int maxMp;
    int attack;
    int defense;
    int level;
    int exp;
    int gold;
    int x;
    int y;
};

// Enemy structure
struct Enemy {
    string name;
    int hp;
    int maxHp;
    int attack;
    int defense;
    int expReward;
    int goldReward;
    EnemyType type;
};

// Item structure
struct Item {
    string name;
    ItemType type;
    int value;
    int quantity;
};



// 2D Array for world map (meets 2D array requirement)
Terrain worldMap[MAP_SIZE][MAP_SIZE];

// C-style array for enemy names (meets array requirement)
string enemyNames[MAX_ENEMIES] = {
    "Slime", "Goblin", "Wolf", "Skeleton", "Troll", "Dragon", "Shadow Lord"
};

// C-style array for enemy stats [enemyType][stat]
// stats: [hp, attack, defense, exp, gold]
int enemyStats[MAX_ENEMIES][5] = {
    {30, 5, 2, 10, 5},      // Slime
    {50, 8, 4, 20, 10},     // Goblin
    {70, 12, 5, 30, 15},    // Wolf
    {100, 15, 8, 50, 25},   // Skeleton
    {150, 20, 12, 80, 40},  // Troll
    {300, 35, 20, 200, 100},// Dragon
    {500, 50, 30, 500, 500} // Shadow Lord
};

// C-style array for item names (meets array requirement)
string itemNames[MAX_ITEMS] = {
    "Health Potion", "Mana Potion", "Iron Sword", "Wooden Shield",
    "Leather Armor", "Steel Sword", "Iron Shield", "Chain Mail",
    "Magic Staff", "Holy Armor"
};

// Vector for player inventory (meets vector requirement)
vector<Item> inventory;

// Player global instance
Player player;



// Initialization functions
void initializeGame();
void createCharacter();
void initializeWorldMap();

// Display functions
void displayTitle();
void displayMap();
void displayPlayerStats();
void displayInventory();
void displayMainMenu();
void displayCombatMenu();

// Game loop functions
void gameLoop();
void exploreWorld();
void movePlayer(char direction);

// Combat functions
bool startCombat(Enemy& enemy);
void playerAttack(Enemy& enemy);
void enemyAttack(Enemy& enemy);
Enemy createEnemy(EnemyType type);

// Item and inventory functions (pass by reference)
void addItemToInventory(Item& item);
bool useItem(int index);
void sortInventoryByName();  // Search/sort algorithm
int findItemInInventory(string itemName);  // Search algorithm

// Utility functions
int randomInt(int min, int max);
bool percentChance(int percent);
void gainExperience(int exp);
void levelUp();
bool checkVictory();

// Save/Load functions
void saveGame(string filename);
void loadGame(string filename);

// Input validation
int getValidatedInt(int min, int max);
string getValidatedString();


int main() {
    // Seed random number generator
    srand(static_cast<unsigned int>(time(0)));

    displayTitle();

    cout << "\n1. New Game\n";
    cout << "2. Load Game\n";
    cout << "3. Exit\n";
    cout << "\nChoice: ";

    int choice = getValidatedInt(1, 3);

    if (choice == 1) {
        initializeGame();
        gameLoop();
    } else if (choice == 2) {
        cout << "Enter save file name: ";
        string filename = getValidatedString();
        loadGame(filename);
        gameLoop();
    } else {
        cout << "\nThanks for playing!\n";
        return 0;
    }

    return 0;
}



/**
 * Initialize the game
 * Pre-conditions: None
 * Post-conditions: All game systems are initialized
 */
void initializeGame() {
    createCharacter();
    initializeWorldMap();

    // Clear inventory
    inventory.clear();

    // Give starting items
    Item healthPotion = {"Health Potion", HEALTH_POTION, 50, 3};
    addItemToInventory(healthPotion);

    cout << "\n===========================================\n";
    cout << "Your adventure begins!\n";
    cout << "===========================================\n\n";
}

/**
 * Create player character
 * Pre-conditions: None
 * Post-conditions: Player structure is initialized with user input
 */
void createCharacter() {
    cout << "\n=== CHARACTER CREATION ===\n";
    cout << "Enter your name: ";
    player.name = getValidatedString();

    // Initialize player stats
    player.maxHp = 100;
    player.hp = player.maxHp;
    player.maxMp = 50;
    player.mp = player.maxMp;
    player.attack = 10;
    player.defense = 5;
    player.level = 1;
    player.exp = 0;
    player.gold = 50;
    player.x = 5;  // Start in center
    player.y = 5;

    cout << "\nWelcome, " << player.name << "!\n";
}

/**
 * Initialize the world map (2D array)
 * Pre-conditions: worldMap array exists
 * Post-conditions: Map is filled with terrain types
 */
void initializeWorldMap() {
    // Fill map with default terrain
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            int rand = randomInt(1, 100);

            if (rand <= 50) {
                worldMap[i][j] = GRASS;
            } else if (rand <= 75) {
                worldMap[i][j] = FOREST;
            } else if (rand <= 85) {
                worldMap[i][j] = MOUNTAIN;
            } else {
                worldMap[i][j] = WATER;
            }
        }
    }

    // Place special locations
    worldMap[5][5] = VILLAGE;  // Starting village
    worldMap[0][0] = DUNGEON;  // Top-left dungeon
    worldMap[9][9] = BOSS_ROOM; // Bottom-right boss room
}

// DISPLAY FUNCTIONS


void displayTitle() {
    cout << "\n";
    cout << "========================================\n";
    cout << "          SHADOW QUEST\n";
    cout << "     A Terminal RPG Adventure\n";
    cout << "========================================\n";
}

/**
 * Display the game map
 * Shows player position and terrain
 */
void displayMap() {
    cout << "\n=== WORLD MAP ===\n\n";
    cout << "  ";
    for (int j = 0; j < MAP_SIZE; j++) {
        cout << j << " ";
    }
    cout << "\n";

    for (int i = 0; i < MAP_SIZE; i++) {
        cout << i << " ";
        for (int j = 0; j < MAP_SIZE; j++) {
            // Show player position
            if (i == player.x && j == player.y) {
                cout << "@ ";
                continue;
            }

            // Show terrain
            switch (worldMap[i][j]) {
                case GRASS:     cout << ". "; break;
                case FOREST:    cout << "T "; break;
                case MOUNTAIN:  cout << "^ "; break;
                case WATER:     cout << "~ "; break;
                case VILLAGE:   cout << "V "; break;
                case DUNGEON:   cout << "D "; break;
                case BOSS_ROOM: cout << "B "; break;
            }
        }
        cout << "\n";
    }

    cout << "\nLegend: @ = You, . = Grass, T = Forest, ^ = Mountain\n";
    cout << "        ~ = Water, V = Village, D = Dungeon, B = Boss\n";
}

/**
 * Display player statistics
 */
void displayPlayerStats() {
    cout << "\n=== " << player.name << " ===\n";
    cout << "Level: " << player.level << " | EXP: " << player.exp << "\n";
    cout << "HP: " << player.hp << "/" << player.maxHp << " | ";
    cout << "MP: " << player.mp << "/" << player.maxMp << "\n";
    cout << "Attack: " << player.attack << " | Defense: " << player.defense << "\n";
    cout << "Gold: " << player.gold << " | Position: (" << player.x << "," << player.y << ")\n";
}

/**
 * Display player inventory
 */
void displayInventory() {
    cout << "\n=== INVENTORY ===\n";

    if (inventory.empty()) {
        cout << "Your inventory is empty.\n";
        return;
    }

    // Display all items using for loop
    for (int i = 0; i < static_cast<int>(inventory.size()); i++) {
        cout << (i + 1) << ". " << inventory[i].name;
        cout << " (x" << inventory[i].quantity << ")";
        cout << " - Value: " << inventory[i].value << "\n";
    }
}

void displayMainMenu() {
    cout << "\n--- ACTIONS ---\n";
    cout << "1. Move (W/A/S/D)\n";
    cout << "2. View Stats\n";
    cout << "3. Inventory\n";
    cout << "4. Rest\n";
    cout << "5. Save Game\n";
    cout << "6. Quit\n";
    cout << "\nChoice: ";
}

void displayCombatMenu() {
    cout << "\n--- COMBAT ---\n";
    cout << "1. Attack\n";
    cout << "2. Use Item\n";
    cout << "3. Flee\n";
    cout << "\nChoice: ";
}


// GAME LOOP FUNCTIONS


/**
 * Main game loop
 * Continues until player wins, loses, or quits
 */
void gameLoop() {
    bool playing = true;

    while (playing) {
        displayMap();
        displayPlayerStats();
        displayMainMenu();

        int choice = getValidatedInt(1, 6);

        switch (choice) {
            case 1: {  // Move
                cout << "Direction (W/A/S/D): ";
                char dir;
                cin >> dir;
                movePlayer(dir);
                break;
            }
            case 2:  // Stats
                displayPlayerStats();
                break;
            case 3: {  // Inventory
                displayInventory();
                if (!inventory.empty()) {
                    cout << "\nUse item? (0 for no, or item number): ";
                    int itemChoice = getValidatedInt(0, static_cast<int>(inventory.size()));
                    if (itemChoice > 0) {
                        useItem(itemChoice - 1);
                    }
                }
                break;
            }
            case 4:  // Rest
                player.hp = player.maxHp;
                player.mp = player.maxMp;
                cout << "\nYou rest and recover your HP and MP!\n";
                break;
            case 5: {  // Save
                cout << "Enter save file name: ";
                string filename = getValidatedString();
                saveGame(filename);
                break;
            }
            case 6:  // Quit
                cout << "\nThanks for playing!\n";
                playing = false;
                break;
        }

        // Check victory condition
        if (checkVictory()) {
            cout << "\n\n========================================\n";
            cout << "     CONGRATULATIONS!\n";
            cout << "  You defeated the Shadow Lord!\n";
            cout << "========================================\n\n";
            playing = false;
        }
    }
}

/**
 * Handle player movement
 * @param direction - W/A/S/D for movement
 * Pre-conditions: direction is valid character
 * Post-conditions: Player position updated, random encounter may occur
 */
void movePlayer(char direction) {
    int newX = player.x;
    int newY = player.y;

    // Calculate new position
    if (direction == 'w' || direction == 'W') newX--;
    else if (direction == 's' || direction == 'S') newX++;
    else if (direction == 'a' || direction == 'A') newY--;
    else if (direction == 'd' || direction == 'D') newY++;
    else {
        cout << "Invalid direction!\n";
        return;
    }

    // Validate movement
    if (newX < 0 || newX >= MAP_SIZE || newY < 0 || newY >= MAP_SIZE) {
        cout << "You can't go that way!\n";
        return;
    }

    // Check terrain
    if (worldMap[newX][newY] == WATER) {
        cout << "You can't walk on water!\n";
        return;
    }

    // Update position
    player.x = newX;
    player.y = newY;

    cout << "\nYou moved to (" << player.x << "," << player.y << ")\n";

    // Random encounter check (except in village)
    if (worldMap[player.x][player.y] != VILLAGE) {
        if (percentChance(30)) {  // 30% chance
            cout << "\n!!! ENEMY ENCOUNTER !!!\n";

            // Determine enemy type based on location
            EnemyType enemyType;
            if (worldMap[player.x][player.y] == BOSS_ROOM) {
                enemyType = SHADOW_LORD;
            } else if (worldMap[player.x][player.y] == DUNGEON) {
                enemyType = static_cast<EnemyType>(randomInt(3, 5));
            } else {
                enemyType = static_cast<EnemyType>(randomInt(0, 2));
            }

            Enemy enemy = createEnemy(enemyType);
            startCombat(enemy);
        }
    }
}


// COMBAT FUNCTIONS


/**
 * Start combat encounter
 * @param enemy - Enemy to fight (pass by reference)
 * @return true if player wins, false if player flees
 */
bool startCombat(Enemy& enemy) {
    cout << "\nA " << enemy.name << " appears!\n";
    cout << "HP: " << enemy.hp << " | ATK: " << enemy.attack << " | DEF: " << enemy.defense << "\n";

    bool fighting = true;

    while (fighting) {
        displayCombatMenu();
        int choice = getValidatedInt(1, 3);

        if (choice == 1) {  // Attack
            playerAttack(enemy);

            if (enemy.hp <= 0) {
                cout << "\nYou defeated the " << enemy.name << "!\n";
                cout << "Gained " << enemy.expReward << " EXP and " << enemy.goldReward << " gold!\n";
                gainExperience(enemy.expReward);
                player.gold += enemy.goldReward;

                // Random item drop
                if (percentChance(40)) {
                    Item drop = {"Health Potion", HEALTH_POTION, 50, 1};
                    addItemToInventory(drop);
                    cout << "The enemy dropped a Health Potion!\n";
                }

                return true;
            }

            enemyAttack(enemy);

            if (player.hp <= 0) {
                cout << "\n\n========================================\n";
                cout << "       GAME OVER\n";
                cout << "  You have been defeated...\n";
                cout << "========================================\n\n";
                exit(0);
            }
        } else if (choice == 2) {  // Use Item
            displayInventory();
            if (!inventory.empty()) {
                cout << "Use which item? (0 to cancel): ";
                int itemIndex = getValidatedInt(0, static_cast<int>(inventory.size()));
                if (itemIndex > 0) {
                    useItem(itemIndex - 1);
                }
            }
        } else if (choice == 3) {  // Flee
            if (enemy.type == SHADOW_LORD) {
                cout << "You cannot flee from the Shadow Lord!\n";
            } else if (percentChance(50)) {
                cout << "You successfully fled!\n";
                return false;
            } else {
                cout << "You couldn't escape!\n";
                enemyAttack(enemy);

                if (player.hp <= 0) {
                    cout << "\n\n========================================\n";
                    cout << "       GAME OVER\n";
                    cout << "  You have been defeated...\n";
                    cout << "========================================\n\n";
                    exit(0);
                }
            }
        }
    }

    return false;
}

/**
 * Player attacks enemy
 * @param enemy - Enemy being attacked (pass by reference)
 */
void playerAttack(Enemy& enemy) {
    int damage = player.attack - enemy.defense / 2;
    if (damage < 1) damage = 1;

    // Add variance
    damage += randomInt(-2, 5);

    enemy.hp -= damage;
    if (enemy.hp < 0) enemy.hp = 0;

    cout << "\nYou attack the " << enemy.name << " for " << damage << " damage!\n";
    cout << enemy.name << " HP: " << enemy.hp << "/" << enemy.maxHp << "\n";
}

/**
 * Enemy attacks player
 * @param enemy - Enemy attacking
 */
void enemyAttack(Enemy& enemy) {
    int damage = enemy.attack - player.defense / 2;
    if (damage < 1) damage = 1;

    // Add variance
    damage += randomInt(-2, 3);

    player.hp -= damage;
    if (player.hp < 0) player.hp = 0;

    cout << "\nThe " << enemy.name << " attacks you for " << damage << " damage!\n";
    cout << "Your HP: " << player.hp << "/" << player.maxHp << "\n";
}

/**
 * Create an enemy of given type
 * @param type - Enemy type to create
 * @return Enemy structure with initialized stats
 */
Enemy createEnemy(EnemyType type) {
    Enemy enemy;
    enemy.type = type;
    enemy.name = enemyNames[type];
    enemy.maxHp = enemyStats[type][0];
    enemy.hp = enemy.maxHp;
    enemy.attack = enemyStats[type][1];
    enemy.defense = enemyStats[type][2];
    enemy.expReward = enemyStats[type][3];
    enemy.goldReward = enemyStats[type][4];

    return enemy;
}


// ITEM AND INVENTORY FUNCTIONS


/**
 * Add item to inventory
 * @param item - Item to add (pass by reference)
 * Pre-conditions: Item is valid
 * Post-conditions: Item added to inventory or stacked
 */
void addItemToInventory(Item& item) {
    // Search for existing item to stack
    for (int i = 0; i < static_cast<int>(inventory.size()); i++) {
        if (inventory[i].name == item.name) {
            inventory[i].quantity += item.quantity;
            return;
        }
    }

    // Add new item if not found
    if (static_cast<int>(inventory.size()) < MAX_INVENTORY) {
        inventory.push_back(item);
    } else {
        cout << "Inventory full!\n";
    }
}

/**
 * Use item from inventory
 * @param index - Index of item to use
 * @return true if item was used successfully
 */
bool useItem(int index) {
    if (index < 0 || index >= static_cast<int>(inventory.size())) {
        return false;
    }

    Item& item = inventory[index];

    switch (item.type) {
        case HEALTH_POTION:
            player.hp += item.value;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
            cout << "\nUsed " << item.name << "! Restored " << item.value << " HP!\n";
            break;
        case MANA_POTION:
            player.mp += item.value;
            if (player.mp > player.maxMp) player.mp = player.maxMp;
            cout << "\nUsed " << item.name << "! Restored " << item.value << " MP!\n";
            break;
        default:
            cout << "\nYou can't use that right now!\n";
            return false;
    }

    // Decrease quantity
    item.quantity--;

    // Remove item if quantity is 0
    if (item.quantity <= 0) {
        inventory.erase(inventory.begin() + index);
    }

    return true;
}

/**
 * Sort inventory by name (bubble sort algorithm)
 * Pre-conditions: inventory exists
 * Post-conditions: inventory is sorted alphabetically
 */
void sortInventoryByName() {
    int n = static_cast<int>(inventory.size());

    // Bubble sort
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (inventory[j].name > inventory[j + 1].name) {
                // Swap
                Item temp = inventory[j];
                inventory[j] = inventory[j + 1];
                inventory[j + 1] = temp;
            }
        }
    }
}

/**
 * Find item in inventory (linear search algorithm)
 * @param itemName - Name of item to find
 * @return Index of item, or -1 if not found
 */
int findItemInInventory(string itemName) {
    for (int i = 0; i < static_cast<int>(inventory.size()); i++) {
        if (inventory[i].name == itemName) {
            return i;
        }
    }
    return -1;
}


// UTILITY FUNCTIONS

/**
 * Generate random integer in range [min, max]
 * Uses rand() from cstdlib
 */
int randomInt(int min, int max) {
    return min + (rand() % (max - min + 1));
}

/**
 * Check if random event occurs based on percentage
 * @param percent - Percentage chance (0-100)
 * @return true if event occurs
 */
bool percentChance(int percent) {
    return (rand() % 100) < percent;
}

/**
 * Give player experience points
 * @param exp - Experience to add
 */
void gainExperience(int exp) {
    player.exp += exp;

    // Check for level up (100 * level exp needed)
    int expNeeded = 100 * player.level;

    while (player.exp >= expNeeded) {
        levelUp();
        expNeeded = 100 * player.level;
    }
}

/**
 * Level up the player
 * Pre-conditions: Player has enough experience
 * Post-conditions: Player stats increase
 */
void levelUp() {
    player.level++;
    player.exp = 0;

    // Increase stats
    player.maxHp += 20;
    player.hp = player.maxHp;
    player.maxMp += 10;
    player.mp = player.maxMp;
    player.attack += 3;
    player.defense += 2;

    cout << "\n*** LEVEL UP! ***\n";
    cout << "You are now level " << player.level << "!\n";
    cout << "HP +20, MP +10, ATK +3, DEF +2\n";
}

/**
 * Check if player has won the game
 * @return true if Shadow Lord is defeated
 */
bool checkVictory() {
    // Check if boss room is cleared (simplified - assumes cleared if reached)
    return (player.x == 9 && player.y == 9 && player.level >= 5);
}

// SAVE/LOAD FUNCTIONS


/**
 * Save game to file
 * @param filename - Name of save file
 */
void saveGame(string filename) {
    ofstream outFile(filename);

    if (!outFile) {
        cout << "Error: Could not create save file!\n";
        return;
    }

    // Save player data
    outFile << player.name << endl;
    outFile << player.hp << " " << player.maxHp << " ";
    outFile << player.mp << " " << player.maxMp << endl;
    outFile << player.attack << " " << player.defense << endl;
    outFile << player.level << " " << player.exp << " " << player.gold << endl;
    outFile << player.x << " " << player.y << endl;

    // Save inventory size
    outFile << inventory.size() << endl;

    // Save each item
    for (int i = 0; i < static_cast<int>(inventory.size()); i++) {
        outFile << inventory[i].name << endl;
        outFile << inventory[i].type << " ";
        outFile << inventory[i].value << " ";
        outFile << inventory[i].quantity << endl;
    }

    outFile.close();
    cout << "\nGame saved to " << filename << "!\n";
}

/**
 * Load game from file
 * @param filename - Name of save file
 */
void loadGame(string filename) {
    ifstream inFile(filename);

    if (!inFile) {
        cout << "Error: Save file not found!\n";
        cout << "Starting new game...\n";
        initializeGame();
        return;
    }

    // Load player data
    getline(inFile, player.name);
    inFile >> player.hp >> player.maxHp;
    inFile >> player.mp >> player.maxMp;
    inFile >> player.attack >> player.defense;
    inFile >> player.level >> player.exp >> player.gold;
    inFile >> player.x >> player.y;

    // Load inventory
    int invSize;
    inFile >> invSize;
    inFile.ignore();  // Clear newline

    inventory.clear();
    for (int i = 0; i < invSize; i++) {
        Item item;
        getline(inFile, item.name);
        int type;
        inFile >> type >> item.value >> item.quantity;
        inFile.ignore();
        item.type = static_cast<ItemType>(type);
        inventory.push_back(item);
    }

    inFile.close();

    // Initialize world
    initializeWorldMap();

    cout << "\nGame loaded successfully!\n";
    cout << "Welcome back, " << player.name << "!\n";
}

// INPUT VALIDATION FUNCTIONS


/**
 * Get validated integer input in range
 * @param min - Minimum valid value
 * @param max - Maximum valid value
 * @return Valid integer in range [min, max]
 */
int getValidatedInt(int min, int max) {
    int value;

    while (true) {
        cin >> value;

        // Check if input failed
        if (cin.fail()) {
            cin.clear();  // Clear error state
            cin.ignore(10000, '\n');  // Discard invalid input
            cout << "Invalid input! Please enter a number: ";
            continue;
        }

        // Check range
        if (value < min || value > max) {
            cout << "Please enter a number between " << min << " and " << max << ": ";
            continue;
        }

        cin.ignore(10000, '\n');  // Clear remaining input
        return value;
    }
}

/**
 * Get validated string input
 * @return Non-empty string
 */
string getValidatedString() {
    string input;

    while (true) {
        getline(cin, input);

        if (input.empty()) {
            cout << "Input cannot be empty! Try again: ";
            continue;
        }

        return input;
    }
}

// END OF PROGRAM
