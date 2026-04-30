#include <chrono>
#include <iostream>
#include <string>
#include <random>
#include <vector>

using namespace std;

//RPG Environment classes
class Monster {
public:
    //Monster Stats
    long health;                     //Starting health
    long maxHealth;                  //Max health
    long attack;                     //Attack damage
    long gold;                       //Gold recieved upon kill

    Monster(int numKills) {
        health = (long) (pow(numKills, 2) / 4) + 10;
        maxHealth = health;
        attack = (long) (pow(numKills, 2) / 6) + 1;
        gold = (long) (pow(numKills, 2) / 2) + 5;
    }
};

class Hero {
public:
    //Hero Stats
        //Health
    long maxHealth;					//Maximum health
    long curHealth;			        //Current health remaining
        //Energy (used to attack)
    int maxEnergy;					//Maximum energy storage
    int curEnergy;			        //Current stored energy (-x energy per attack, cannot attack if below x)
    int attackCost;                 //Energy used per attack (see above)
        //Battle
    long attack;                    //Damage dealt to enemy
    int isDefending;                //Whether hero is defending; -1 for false, 1 for true; Defending halves incoming damage for x turns
    int defenseDuration;            //Remaining defense turns
    int maxDefenseDuration;         //Starting number for defenseDuration
        //Misc
    long gold;						//Used to buy upgrades, potions, and supplies; Used for final scoring
    long goldSpent;                 //Used for scoring
    long potions;					//Used to heal mid-battle
    long supplies;					//Used to restore health & energy while resting
    long kills;						//Used for final scoring
    long upgrades;                  //# of upgrades purchased, used for final scoring
    int invalidMoves;               //# of moves that do nothing, used for final scoring
    bool lastMoveWasInvalid;        //Whether the last move did anything
    long turnsSurvived;             //How long the hero has survived
    bool visitedShop;               //Used for scoring

    //Set the variables
    Hero(long maxHealth, int maxEnergy, int attackCost, long attack, long gold, long potions, long supplies) {
        //set health stats
        this->maxHealth = maxHealth;
        this->curHealth = maxHealth;
        //set energy stats
        this->maxEnergy = maxEnergy;
        this->curEnergy = maxEnergy;
        this->attackCost = attackCost;
        //set battle stats
        this->attack = attack;
        this->isDefending = -1;
        this->defenseDuration = 0;
        this->maxDefenseDuration = 3;
        //set misc stats
        this->gold = gold;
        this->goldSpent = 0;
        this->potions = potions;
        this->supplies = supplies;
        this->kills = 0;
        this->upgrades = 0;
        this->invalidMoves = 0;
        this->lastMoveWasInvalid = false;
        this->turnsSurvived = 0;
        this->visitedShop = false;
    }

    //Reset the variables for repeated runs
    void resetHero(long maxHealth, int maxEnergy, int attackCost, long attack, long gold, long potions, long supplies) {
        //set health stats
        this->maxHealth = maxHealth;
        this->curHealth = maxHealth;
        //set energy stats
        this->maxEnergy = maxEnergy;
        this->curEnergy = maxEnergy;
        this->attackCost = attackCost;
        //set battle stats
        this->attack = attack;
        this->isDefending = -1;
        this->defenseDuration = 0;
        this->maxDefenseDuration = 3;
        //set misc stats
        this->gold = gold;
        this->goldSpent = 0;
        this->potions = potions;
        this->supplies = supplies;
        this->kills = 0;
        this->upgrades = 0;
        this->invalidMoves = 0;
        this->lastMoveWasInvalid = false;
        this->turnsSurvived = 0;
        this->visitedShop = false;
    }

    //Print all player stats
    void printStats() {
        cout << "\033[31mHealth: " << curHealth << "/" << maxHealth << "\t";
        cout << "\033[34mEnergy: " << curEnergy << "/" << maxEnergy << "\t";
        cout << "\033[93mAttack: " << attack << " \033[34m(" << attackCost << ")" << endl;
        cout << "\033[33mGold: " << gold << "\t";
        cout << "\033[91mPotions: " << potions << "\t";
        cout << "\033[32mSupplies: " << supplies << "\t";
        cout << "\033[0mKills: " << kills << endl;
    }

    //Calculate final score
    long calcFitness() {
        long score = kills * 3;
        score += goldSpent;
        score += upgrades * 5;
        score += (upgrades * kills) / (gold + 1);
        score += turnsSurvived / 20;
        score -= invalidMoves * 3;
        score += visitedShop ? 15 : 0;
        return score;
    }
};

class GameManager {
public:
    bool inBattle = false;
    bool inShop = false;
    bool resting = true;
    string battleOptions = "What will you do?\n1. Attack\n2. Defend\n3. Use Potion (-1 potion, +10 health)\n4. Run Away\n5. Examine Enemy Stats";
    string restOptions = "What will you do?\n1. Find Enemy\n2. Visit the Shop\n3. Rest (-10 supplies, +25 health, full energy)";
    string shopOptions = "What will you do?\n1. Buy Potions (-10 gold, +3 potions)\n2. Buy Supplies (-10 gold, +15 supplies)\n3. Upgrade Equipment (-20 gold, +5 max health, +2 damage)\n4. Leave";
    char lastAction = '0';

    Hero* hero;

    GameManager(Hero* hero) {
        this->hero = hero;
    }

    void printOptions() {
        if (inBattle) {
            cout << battleOptions << endl;
        }
        if (inShop) {
            cout << shopOptions << endl;
        }
        if (resting) {
            cout << restOptions << endl;
        }
    }

    void processChoice(char choice, bool print) {
        lastAction = choice;
        hero->turnsSurvived++;
        if (resting) {
            switch (choice) {
                case '1':
                    resting = false;
                    inBattle = true;
                    enemy = new Monster(hero->kills);
                    if (print) cout << "Encountered an enemy with " << enemy->health << " health and " << enemy->attack << " attack power!" << endl;
                    hero->lastMoveWasInvalid = false;
                    break;
                case '2':
                    resting = false;
                    inShop = true;
                    hero->lastMoveWasInvalid = false;
                    if (!hero->visitedShop) hero->visitedShop = true;
                    if (print) cout << "Moved from Resting to Shop." << endl;
                    break;
                case '3':
                    if (hero->supplies >= suppliesToRest) {
                        hero->supplies -= suppliesToRest;
                        hero->curHealth = min(hero->maxHealth, hero->curHealth + healthFromRest);
                        hero->curEnergy = hero->maxEnergy;
                        if (print) cout << "You rested, using " << suppliesToRest << " supplies and healing " << healthFromRest << " health." << endl;
                        hero->lastMoveWasInvalid = false;
                    }
                    else {
                        if (print) cout << "You don't have sufficient supplies to rest." << endl;
                        hero->invalidMoves++;
                        hero->lastMoveWasInvalid = true;
                    }
                    break;
                default:
                    if (print) cout << "Please enter a valid option." << endl;
                    hero->invalidMoves++;
                    hero->lastMoveWasInvalid = true;
            }
        }
        else if (inShop) {
            switch (choice) {
                case '1':
                    if (hero->gold >= potionCost) {
                        hero->gold -= potionCost;
                        hero->goldSpent += potionCost;
                        hero->potions += potionsPerPurchase;
                        if (print) cout << "You bought " << potionsPerPurchase << " potions for " << potionCost << " gold." << endl;
                        hero->lastMoveWasInvalid = false;
                    }
                    else {
                        if (print) cout << "You can't afford any potions." << endl;
                        hero->invalidMoves++;
                        hero->lastMoveWasInvalid = true;
                    }
                    break;
                case '2':
                    if (hero->gold >= suppliesCost) {
                        hero->gold -= suppliesCost;
                        hero->gold += suppliesCost;
                        hero->supplies += suppliesPerPurchase;
                        if (print) cout << "You bought " << suppliesPerPurchase << " supplies for " << suppliesCost << " gold." << endl;
                        hero->lastMoveWasInvalid = false;
                    }
                    else {
                        if (print) cout << "You can't afford any supplies." << endl;
                        hero->invalidMoves++;
                        hero->lastMoveWasInvalid = true;
                    }
                    break;
                case '3':
                    if (hero->gold >= upgradeCost) {
                        hero->gold -= upgradeCost;
                        hero->goldSpent += upgradeCost;
                        hero->maxHealth += healthUpgrade;
                        hero->attack += attackUpgrade;
                        hero->upgrades++;
                        if (print) cout << "You upgraded your equipment for " << upgradeCost << " gold." << endl;
                        hero->lastMoveWasInvalid = false;
                    }
                    else {
                        if (print) cout << "You can't afford to upgrade your equipment." << endl;
                        hero->invalidMoves++;
                        hero->lastMoveWasInvalid = true;
                    }
                    break;
                case '4':
                    inShop = false;
                    resting = true;
                    if (print) cout << "Moved from Shop to Resting." << endl;
                    hero->lastMoveWasInvalid = false;
                    break;
                default:
                    if (print) cout << "Please enter a valid option." << endl;
                    hero->invalidMoves++;
                    hero->lastMoveWasInvalid = true;
            }
        }
        else {
            switch (choice) {
                case '1':
                    enemy->health -= hero->attack;
                    if (print) cout << "You hit the enemy for " << hero->attack << " damage!" << endl;
                    enemyTurn(print);
                    hero->lastMoveWasInvalid = false;
                    break;
                case '2':
                    hero->isDefending = 1;
                    hero->defenseDuration = hero->maxDefenseDuration;
                    if (print) cout << "You began defending! Damage taken is halved for " << hero->maxDefenseDuration << " turns!" << endl;
                    enemyTurn(print);
                    hero->lastMoveWasInvalid = false;
                    break;
                case '3':
                    if (hero->potions >= 1) {
                        hero->potions--;
                        hero->curHealth = min(hero->curHealth + healthFromPotions, hero->maxHealth);
                        if (print) cout << "You drank a potion and restored " << healthFromPotions << " health!" << endl;
                        enemyTurn(print);
                        hero->lastMoveWasInvalid = false;
                    }
                    else {
                        if (print) cout << "You don't have any potions to use!";
                        hero->invalidMoves++;
                        hero->lastMoveWasInvalid = true;
                    }
                    break;
                case '4':
                    inBattle = false;
                    resting = true;
                    delete enemy;
                    hero->lastMoveWasInvalid = false;
                    if (print) cout << "Fled from enemy." << endl;
                    break;
                case '5':
                    if (print) cout << "Enemy Health: " << enemy->health << "\nEnemy Attack: " << enemy->attack << endl;
                    break;
                default:
                    if (print) cout << "please enter a valid option." << endl;
                    hero->invalidMoves++;
                    hero->lastMoveWasInvalid = true;
            }
        }
    }

    vector<double> calcInputs() {
        vector<double> inputs;
        inputs.resize(16, 0);
        inputs[0] = (double) hero->curHealth / hero->maxHealth;                          //Health %
        inputs[1] = (double) hero->curEnergy / hero->maxEnergy;                          //Energy %
        inputs[2] = (double) hero->gold / (hero->gold + 50);                             //Gold (normalized)
        inputs[3] = (double) hero->supplies / (hero->supplies + 50);                     //Supplies (normalized)
        inputs[4] = (double) hero->potions / (hero->potions + 10);                       //Potions (normalized)
        inputs[5] = (double) hero->defenseDuration / hero->maxDefenseDuration;           //How long defending for
        if (inBattle) {
            inputs[6] = (double) enemy->health / (enemy->maxHealth);                     //Enemy health (normalized)
            inputs[7] = (double) enemy->attack / (enemy->attack + 20);                   //Enemy attack (normalized)
            inputs[8] = 1.0;                                                             //Is in battle
        }
        else {
            if (inShop) {
                inputs[9] = 1.0;                                                        //Is shopping
            }
            else {
                inputs[10] = 1.0;                                                       //Is resting
            }
        }
        inputs[11] = hero->lastMoveWasInvalid ? -1.0 : 1.0;                             //Whether the last move was valid
        inputs[12] = lastAction == '1' ? 1.0 : 0;                                       //The previos action
        inputs[13] = lastAction == '2' ? 1.0 : 0;
        inputs[14] = lastAction == '3' ? 1.0 : 0;
        inputs[15] = lastAction == '4' ? 1.0 : 0;
        return inputs;
    }

private:
    //RESTING
    int suppliesToRest = 5;
    int healthFromRest = 25;
    
    //SHOPPING
    int potionCost = 10;
    int potionsPerPurchase = 3;
    int suppliesCost = 10;
    int suppliesPerPurchase = 15;
    int upgradeCost = 20;
    int healthUpgrade = 5;
    int attackUpgrade = 2;

    //BATTLES
    Monster* enemy;
    int healthFromPotions = 10;

    void enemyTurn(bool print) {
        if (enemy->health > 0) {
            int enemyDamage = (hero->isDefending > 0) ? (enemy->attack / 2) : enemy->attack;
            hero->curHealth -= enemyDamage;
            if (print) cout << "Enemy hit you for " << enemyDamage << " damage!" << endl;
            if (hero->curHealth <= 0 && print) {
                endGame();
            }
            if (hero->defenseDuration > 0) {
                hero->defenseDuration--;
                if (hero->defenseDuration == 0) {
                    if (print) cout << "You are no longer defending!" << endl;
                    hero->isDefending = -1;
                }
            }
        }
        else {
            if (print) cout << "Enemy defeated!" << endl;
            inBattle = false;
            resting = true;
            hero->kills++;
            hero->gold += enemy->gold;
            delete enemy;
            hero->isDefending = -1;
            hero->defenseDuration = 0;
        }
    }

    void endGame() {
        cout << "You died!" << endl;
        cout << "Final stats:\n\033[31mMax Health: " << hero->maxHealth << "\n\033[93mAttack Damage: " << hero->attack << endl;
        cout << "\033[91mPotions: " << hero->potions << "\n\033[33mGold: " << hero->gold << "\n\033[0mKills: " << hero->kills << endl;
    }
};


//AI class
class Brain {
public:
    int inputs;                                 //Number of inputs - still deciding.
    vector<int> layers;                         //# of neurons (still deciding) for each layer besides input (still deciding)
    vector<vector<vector<double>>> weights;     //All weights for the network - read as weights[layer][weight index][neuron] (to allow matrix calculations)
    vector<vector<double>> biases;              //All biases for the network - read as biases[layer][neuron]

    Brain(int inputs, vector<int> layers, uniform_real_distribution<double> random, default_random_engine engine) {
        this->inputs = inputs;
        this->layers = layers;
        createRandomWeights(random, engine);
    }

    Brain(int inputs, vector<int> layers, vector<vector<vector<double>>> weights, vector<vector<double>> biases) {
        this->inputs = inputs;
        this->layers = layers;
        this->weights = weights;
        this->biases = biases;
    }

    Brain* clone() {
        return new Brain(inputs, layers, weights, biases);
    }

    Brain* reproduce(uniform_real_distribution<double> random, default_random_engine engine) {

        Brain* child = new Brain(inputs, layers, weights, biases);

        //Slightly modify weights - 20% chance to change at all, 5% chance to be completely re-randomized
        for (int i = 0; i < weights.size(); i++) {
            for (int j = 0; j < weights[i].size(); j++) {
                for (int k = 0; k < weights[i][j].size(); k++) {
                    double roll = random(engine);
                    if (roll >= 0.95) {
                        child->weights[i][j][k] = (2 * random(engine)) - 1.0;      //Completely re-randomize between [-1, 1]
                    }
                    else if (roll >= 0.8) {
                        child->weights[i][j][k] += (random(engine) * 0.15) - 0.075;     //Modify by between [-0.075, 0.075]
                    }
                    child->weights[i][j][k] = max(-2.0, min(2.0, child->weights[i][j][k]));
                }
            }
            for (int j = 0; j < biases[i].size(); j++) {
                double roll = random(engine);
                if (roll >= 0.95) {
                    child->biases[i][j] = (2 * random(engine)) - 1.0;      //Completely re-randomize between [-1, 1]
                }
                else if (roll >= 0.8) {
                    child->biases[i][j] += (random(engine) * 0.15) - 0.075;     //Modify by between [-0.075, 0.075]
                }
                child->biases[i][j] = max(-2.0, min(2.0, child->biases[i][j]));
            }
        }
        return child;
    }

    void printWeights() {
        for (int i = 0; i < weights.size(); i++) {
            cout << "Layer " << i + 1 << " / " << weights.size() << endl;
            for (int j = 0; j < weights[i][0].size(); j++) {
                cout << "  Neuron " << j + 1 << " / " << weights[i][0].size() << "  Bias: " << biases[i][j] << endl;
                for (int k = 0; k < weights[i].size(); k++) {
                    cout << "    Weight Index " << k + 1 << " / " << weights[i].size() << " : " << weights[i][k][j] << endl;
                }
            }
        }
    }

    vector<double> think(vector<double> inputs) {
        if (inputs.size() != this->inputs) {
            cout << "ERROR! INVALID NUMBER OF INPUTS!" << endl;
            return vector<double> {-1};
        }
        vector<double> output = inputs;
        for (int i = 0; i < weights.size(); i++) {
            output = calcOutput(output, i);
            output = sigmoid(output);
        }
        for (int i = 0; i < 4; i++) {
            output[i] *= output[i] * output[i] * output[i];
        }
        return output;
    }

    vector<double> calcOutput(vector<double> inputs, int layer) {
        vector<double> result;
        result.resize(weights[layer][0].size(), 0);                     //return the same number of outputs as neurons in current layer
        
        for (int i = 0; i < inputs.size(); i++) {                       //for # of inputs
            for (int j = 0; j < weights[layer][0].size(); j++) {        //for # of outputs
                result[j] += inputs[i] * weights[layer][i][j];          //Dot of inputs & weights
            }
        }

        //Add biases to results
        for (int i = 0; i < result.size(); i++) {
            result[i] += biases[layer][i];
        }

        return result;
    }

    vector<double> sigmoid(vector<double> output) {
        vector<double> result;
        result.resize(output.size(), 0);
        for (int i = 0; i < output.size(); i++) {
            result[i] = 0.5 * (output[i] / (1 + abs(output[i])) + 1);
        }
        return result;
    }

private:
    void createRandomWeights(uniform_real_distribution<double> random, default_random_engine engine) {

        /*Set up sizes for weight matrix*/

        weights.resize(layers.size());                  //# of layers besides input

        weights[0].resize(inputs);                      //1st hidden layer has weight index for each of [inputs] inputs
        for (int i = 0; i < inputs; i++) {
            weights[0][i].resize(layers[0], 0);         //For each weight index, can be for [layer size] neurons
        }

        //For each layer, it has [previous layer size] weight indexes. For each weight index, it can be for any of [layer size] neurons.
        for (int i = 1; i < layers.size(); i++) {
            weights[i].resize(layers[i - 1]);
            for (int j = 0; j < layers[i - 1]; j++) {
                weights[i][j].resize(layers[i], 0);
            }
        }

        /*Initialize all weight to random betwen -0.5 to +0.5*/

        for (int i = 0; i < weights.size(); i++) {
            for (int j = 0; j < weights[i].size(); j++) {
                for (int k = 0; k < weights[i][j].size(); k++) {
                    weights[i][j][k] = random(engine) - 0.5;
                }
            }
        }

        /*Create random biases for each neuron*/

        biases.resize(layers.size());

        for (int i = 0; i < layers.size(); i++) {
            biases[i].resize(layers[i]);
            for (int j = 0; j < biases[i].size(); j++) {
                biases[i][j] = (2 * random(engine)) - 1;
            }
        }
    }
};

//Main Program
int main() {

    Hero* hero = new Hero(15, 25, 1, 1, 20, 5, 20);

    GameManager* manager = new GameManager(hero);

    vector<vector<Brain*>> brains;

    vector<vector<long>> fitness;
    vector<vector<long>> bestFitnesses;
    vector<double> avgFitnesses;
    vector<vector<vector<vector<char>>>> runs;

    char runAI;

    do {
        cout << "Run AI? [y/n]: ";
        cin >> runAI;
    } while ((runAI != 'y') && (runAI != 'n'));

    int numGenerations;             //In outermost scope to be used later

    //Set up randomness
    default_random_engine engine;
    uniform_real_distribution<double> rand(0.0, 1.0);

    if (runAI == 'n') {
        cout << "Press 's' for stats, or 'q' to quit." << endl;
    }
    else {
        //Get number of generations to run
        cout << "Number of generations: ";
        cin >> numGenerations;

        //Resize to fit numGenerations
        fitness.resize(numGenerations);
        bestFitnesses.resize(numGenerations);
        avgFitnesses.resize(numGenerations, 0.0);
        runs.resize(numGenerations);
        brains.resize(numGenerations);

        //Resize inner vectors to appropriate sizes
        for (int i = 0; i < numGenerations; i++) {
            fitness[i].resize(50, 0);
            bestFitnesses[i].resize(5, -60);
            runs[i].resize(50);
            brains[i].resize(50);
            for (int j = 0; j < 50; j++) {
                runs[i][j].resize(10);
            }
        }

        for (int i = 0; i < 50; i++) {
            brains[0][i] = new Brain(16, vector<int> {16, 8, 4}, rand, engine);
        }
    }

    int runIndex = 0;               //The current run (of 10) for the current brain
    int brainIndex = 0;             //The current brain (of 50) for the current generation
    int currentGeneration = 0;      //The current generation (of numGenerations)

    long brainFitness = 0;          //Track total fitness for each brain's runs to average

	//Keep dungeon loop active
	bool runDungeon(true);
	while (runDungeon) {
        //Allow the user to play the RPG
        if (runAI == 'n') {
            char choice;
            manager->printOptions();
            cin >> choice;
            if (choice >= '0' && choice <= '9') {
                manager->processChoice(choice, true);
                if (hero->curHealth <= 0) {
                    runDungeon = false;
                }
            }
            if (choice == 's') {
                hero->printStats();
            }
            if (choice == 'd') {
                cout << "Welcome to the debug menu! Which stat would you like to modify?" << endl;
                cout << "c. Cancel\n0. Max Health\n1. Current Health\n2. Max Energy\n3. Current Energy\n4. Attack Cost\n5. Attack Damage\n6. Gold\n7. Potions\n8. Supplies\n9. Kills" << endl;
                cin >> choice;
                if (choice >= '0' && choice <= '9') {
                    cout << "Amount to modify: ";
                    int modify;
                    cin >> modify;
                    cout << "Modifying " << choice << " by " << modify << endl;
                    switch (choice) {
                    case '0':
                        hero->maxHealth += modify;
                        break;
                    case '1':
                        hero->curHealth += modify;
                        break;
                    case '2':
                        hero->maxEnergy += modify;
                        break;
                    case '3':
                        hero->curEnergy += modify;
                        break;
                    case '4':
                        hero->attackCost += modify;
                        break;
                    case '5':
                        hero->attack += modify;
                        break;
                    case '6':
                        hero->gold += modify;
                        break;
                    case '7':
                        hero->potions += modify;
                        break;
                    case '8':
                        hero->supplies += modify;
                        break;
                    case '9':
                        hero->kills += modify;
                        break;
                    default:
                        cout << "You shouldn't see this." << endl;
                    }
                }
            }
            if (choice == 'q') {
                runDungeon = false;
            }
        }
        //Run AI through the RPG
        else {

            //Get the AI results
            vector<double> choice = brains[currentGeneration][brainIndex]->think(manager->calcInputs());

            //Get the AI's choice
            //If in first 5 generations, has a 15% chance to do completely random action
            char AIchoice = '1';
            if (currentGeneration <= 5 && rand(engine) <= 0.15) {
                AIchoice = floor(rand(engine) * 4.0) + '1';
            }
            else {
                double outputSum = choice[0] + choice[1] + choice[2] + choice[3];
                double roll = rand(engine) * outputSum;
                for (int i = 0; i < 4; i++) {
                    roll -= choice[i];
                    if (roll <= 0) {
                        AIchoice = i + '1';
                        break;
                    }
                }
            }

            runs[currentGeneration][brainIndex][runIndex].push_back(AIchoice);
            manager->processChoice(AIchoice, false);

            //If the current run ended
            int maxInvalidMoves = 20;
            int maxTurns = 200;
            if (hero->curHealth <= 0 || hero->invalidMoves >= maxInvalidMoves || hero->turnsSurvived >= maxTurns) {

                brainFitness += hero->calcFitness();
                //fitness[currentGeneration][brainIndex] = hero->calcFitness();                  //Calculate Brain's score
                
                hero->resetHero(15, 25, 1, 1, 20, 5, 20);                   //Reset Hero stats
                manager->inBattle = false;                                  //Reset game state
                manager->inShop = false;
                manager->resting = true;
                
                runIndex++;                                                 //Move to next run

                //Check for ending run
                bool endEarly = false;
                long currentAverage;
                if (runIndex == 3 || runIndex == 7 || runIndex == 10) {
                    if (currentGeneration > 0 && runIndex == 3) {
                        currentAverage = brainFitness / 3;
                        bool inBest = false;
                        if (currentAverage >= bestFitnesses[currentGeneration - 1][4] - 20) {
                            inBest = true;
                        }
                        if (!inBest) {
                            endEarly = true;
                        }
                    }
                    else if (currentGeneration > 0 && runIndex == 7) {
                        currentAverage = brainFitness / 7;
                        bool inBest = false;
                        if (currentAverage >= bestFitnesses[currentGeneration - 1][4] - 5) {
                            inBest = true;
                        }
                        if (!inBest) {
                            endEarly = true;
                        }
                    }
                    if (endEarly || runIndex == 10) {
                        currentAverage = (runIndex == 10) ? brainFitness / 10 : currentAverage;
                        runIndex = 0;
                        fitness[currentGeneration][brainIndex] = currentAverage;        //Get average fitness of brain
                        brainFitness = 0;                                               //Reset for next brain
                        brainIndex++;                                                   //Move to next Brain
                    }
                }

                //Check for end of current generation
                if (brainIndex == 50) {
                    brainIndex = 0;
                    if (currentGeneration + 1 == numGenerations) {
                        cout << "-----Final generation processed!-----" << endl;
                        runDungeon = false;                                 //If it was the final generation, end the RPG
                    }
                    else {
                        cout << "-----Finished Generation " << currentGeneration + 1 << "-----" << endl;
                    }

                    //Sort copy of list of scores
                    pair<long, int> sortedFitness[50];
                    long totalFitness = 0;
                    for (int i = 0; i < 50; i++) {
                        sortedFitness[i] = {fitness[currentGeneration][i], i};
                        totalFitness += fitness[currentGeneration][i];
                    }
                    sort(begin(sortedFitness), end(sortedFitness));

                    //Getting indices for top 5 scores
                    int best[5] = {0};
                    for (int i = 0; i < 5; i++) {
                        best[i] = sortedFitness[49 - i].second;
                        bestFitnesses[currentGeneration][i] = sortedFitness[49 - i].first;
                    }

                    avgFitnesses[currentGeneration] = (double) totalFitness / 50;

                    //Print top 5 scores
                    cout << "Top 5 Scores: ";
                    for (int i = 0; i < 5; i++) {
                        cout << sortedFitness[49 - i].first << " (" << best[i] + 1 << ")";
                        if (i < 4) {
                            cout << ", ";
                        }
                    }
                    cout << endl;

                    currentGeneration++;        //Increment to next generation
                    if (currentGeneration != numGenerations) {

                        //Clear brains and repopulate from best
                        for (int i = 0; i < 50; i++) {
                            if (i < 5) {
                                brains[currentGeneration][i] = brains[currentGeneration - 1][best[i]]->clone();
                            }
                            else if (i < 20) {
                                double parent = rand(engine) * 15;
                                if (parent < 5) {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[0]]->clone();
                                }
                                else if (parent < 9) {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[1]]->clone();
                                }
                                else if (parent < 12) {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[2]]->clone();
                                }
                                else if (parent < 14) {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[3]]->clone();
                                }
                                else {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[4]]->clone();
                                }
                            }
                            else if (i < 47) {
                                double parent = rand(engine) * 15;
                                if (parent < 5) {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[0]]->reproduce(rand, engine);
                                }
                                else if (parent < 9) {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[1]]->reproduce(rand, engine);
                                }
                                else if (parent < 12) {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[2]]->reproduce(rand, engine);
                                }
                                else if (parent < 14) {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[3]]->reproduce(rand, engine);
                                }
                                else {
                                    brains[currentGeneration][i] = brains[currentGeneration - 1][best[4]]->reproduce(rand, engine);
                                }
                            }
                            else {
                                brains[currentGeneration][i] = new Brain(16, vector<int> {16, 8, 4}, rand, engine);
                            }
                        }   //END reproduction logic
                    }
                }   //END end-of-generation logic
            }   //END end-of-run logic
        }   //END AI-sim logic
	}   //END main loop

    if (runAI == 'y') {
        int checkGeneration;
        while (true) {
            cout << "Which generation would you like to examine? [1 to " << numGenerations << "; 0 to exit]" << endl;
            cin >> checkGeneration;
            if (checkGeneration >= 1 && checkGeneration <= numGenerations) {
                cout << "Generation number: " << checkGeneration-- << endl;
                cout << "Best fitness: " << bestFitnesses[checkGeneration][0] << endl;
                cout << "Average fitness: " << avgFitnesses[checkGeneration] << endl;
                cout << "Which brain would you like to examine? [1 to 50; 0 to skip]" << endl;
                int checkBrain;
                cin >> checkBrain;
                if (checkBrain >= 1 && checkBrain <= 50) {
                    checkBrain--;
                    cout << "To examine a run, enter a number [0-9] || To view weights, enter 'w'" << endl;
                    char checkRun;
                    cin >> checkRun;
                    if (checkRun >= '0' && checkRun <= '9') {
                        checkRun -= '0';
                        for (int i = 0; i < runs[checkGeneration][checkBrain][checkRun].size(); i++) {
                            manager->processChoice(runs[checkGeneration][checkBrain][checkRun][i], true);
                        }
                        cout << "Final fitness: " << hero->calcFitness() << endl;
                        hero->resetHero(15, 25, 1, 1, 20, 5, 20);                   //Reset Hero stats
                        manager->inBattle = false;                                  //Reset game state
                        manager->inShop = false;
                        manager->resting = true;
                    }
                    else if (checkRun == 'w') {
                        brains[checkGeneration][checkBrain]->printWeights();
                    }
                }
            }
            else {
                break;
            }
        }
    }
    cout << "Wait a moment for memory management" << endl;
    delete hero;
    delete manager;
    for (int i = 0; i < numGenerations; i++) {
        for (int j = 0; j < 50; j++) {
            delete brains[i][j];
        }
    }
}