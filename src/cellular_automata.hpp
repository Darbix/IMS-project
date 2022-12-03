/**
 * @file cellular_automata.hpp
 * @author David Kedra, xkedra00
 * @author Petr Kolařík, xkolar79
 * @brief Declarations of Cullular automata and rules 
 * 
 * IMS Project - Cellular automata
 * VUT FIT Brno, 2022/2023 
 */

#pragma once

#include <vector>
#include <cstdint>
#include <tuple>

#define SIZE 700    ///< Size of the window in pixels
#define N_WIDTH 100 ///< Size of the cellurar automata (Number of cells in each row)
#define MAX_STEP 2  ///< Maximal cell step for a random movement

using namespace std;

/**
 * Possible cell states each defined as a specific bit in uint8_t to be able to execute binary operations 
 */
enum CType: uint8_t {none=0, tissue=1, toxic=2, fluoride=4, blood=8, stomach=16, oxygen=32, water=64, weak=128, any=255};

/**
 * Rule with an expected input 3x3 matrix and an output cell
 */
typedef struct{
    uint8_t exp[3][3];
    CType output;
}rule_t;

/**
 * Get a bounding save coordination with respect to a automata size.
 * Outer points are converted to a bounding (0 or N_WIDTH - 1)
 * @param val Matrix coordinate
 * @return unsigned Save matrix coordinate
 */
unsigned getCellCoord(int val);

/**
 * Get a tuple of RGB bytes for a cell state
 * @param cType 
 */
tuple <uint8_t, uint8_t, uint8_t>getStateColor(CType cType);

/**
 * Class with all the cellular rules
 */
class Rules{
    public:
        vector<rule_t> rules;
        Rules();
};

/**
 * Class with 2 cellular matrices and rules
 */
class CA{
    public:
        vector<vector<CType>> curr; ///< Current displayed matrix with cells
        vector<vector<CType>> temp; ///< Next displayed matrix for applying rules
        Rules *r; ///< Rules

        CA();
        
        /**
         * Go through all the rulles for a specific center cell.
         * Compares the 'curr' matrix with rules and outputs to 'temp' matrix 
         * @param x X matrix coordinate as a center
         * @param y Y matrix coordinate as a center
         */
        void applyRulesToTemp(int x, int y);

        /**
         * Randomly move all the 'moveType' cells around their locations
         * @param moveType Cell state to be moved
         * @param fullness Food stomach fullness to affecting the tendency to move 
         */
        void randomMove(CType moveType, float fullness);
};