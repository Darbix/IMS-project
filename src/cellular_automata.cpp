/**
 * @file cellular_automata.cpp
 * @author David Kedra, xkedra00
 * @author Petr Kolařík, xkolar79
 * @brief Cullular automata and rules 
 * 
 * IMS Project - Cellular automata
 * VUT FIT Brno, 2022/2023 
 */

#include <simlib.h>
#include <iostream>
#include <cmath>
#include <tuple>

#include "cellular_automata.hpp"

Rules::Rules(){
    // Static rules definitions
    vector<rule_t> temp{
        // Wrap fluoride to (hydrofluoric acid) water
        {{
            {CType::stomach, CType::fluoride, CType::water},
            {CType::stomach, CType::stomach, CType::stomach},
            {CType::stomach, CType::stomach, CType::stomach}
        }, CType::water},
        {{
            {CType::stomach, CType::stomach, CType::stomach},
            {CType::stomach, CType::stomach, CType::stomach},
            {CType::stomach, CType::fluoride, CType::water}
        }, CType::water},
        {{
            {CType::any, CType::stomach, CType::water},
            {CType::any, CType::any ^ CType::fluoride, CType::stomach},
            {CType::any, CType::stomach, CType::water}
        }, CType::water},
        // Create a 3 layer hydrofluoric water column
        {{
            {CType::stomach, CType::stomach, CType::stomach},
            {CType::stomach, CType::stomach, CType::water},
            {CType::stomach, CType::water, CType::stomach}
        }, CType::water},
        {{
            {CType::stomach, CType::water, CType::stomach},
            {CType::stomach, CType::stomach, CType::water},
            {CType::stomach, CType::stomach, CType::stomach}
        }, CType::water},
        // Repetitive movement of hydrofluoric
        {{
            {CType::stomach, CType::stomach, CType::water},
            {CType::stomach, CType::stomach, CType::water},
            {CType::stomach, CType::stomach, CType::water}
        }, CType::water},
        {{
            {CType::stomach, CType::water, CType::any},
            {CType::any, CType::water, CType::any},
            {CType::stomach, CType::water, CType::any}
        }, CType::stomach},
        {{
            {CType::stomach, CType::stomach, CType::stomach},
            {CType::water, CType::water, CType::water},
            {CType::stomach, CType::stomach, CType::stomach}
        }, CType::stomach},
        {{
            {CType::water, CType::stomach, CType::water},
            {CType::stomach, CType::stomach, CType::stomach},
            {CType::water, CType::stomach, CType::water}
        }, CType::water},
        {{
            {CType::water, CType::stomach, CType::stomach},
            {CType::stomach, CType::water, CType::stomach},
            {CType::water, CType::stomach, CType::stomach}
        }, CType::stomach},
        {{
            {CType::water, CType::stomach, CType::any},
            {CType::stomach, CType::water, CType::stomach},
            {CType::water, CType::stomach, CType::any}
        }, CType::stomach},
        // Hydrofluoric reduction from the right
        {{
            {CType::stomach, CType::stomach, CType::stomach},
            {CType::stomach, CType::water, CType::stomach},
            {CType::water, CType::stomach, CType::stomach}
        }, CType::stomach},
        {{
            {CType::any, CType::any, CType::stomach},
            {CType::water, CType::water, CType::stomach},
            {CType::any, CType::any, CType::stomach}
        }, CType::stomach},
        // Blood weakness generators
        {{
            {CType::any, CType::any, CType::any},
            {CType::any, CType::any ^ CType::toxic, CType::any},
            {CType::toxic, CType::oxygen, CType::any}
        }, CType::weak},
        {{
            {CType::any, CType::any, CType::toxic},
            {CType::any, CType::oxygen, CType::toxic},
            {CType::any, CType::any, CType::any}
        }, CType::weak},
        {{
            {CType::any, CType::weak, CType::any},
            {CType::blood, CType::oxygen, CType::weak},
            {CType::any, CType::blood, CType::any}
        }, CType::weak},
        {{
            {CType::any, CType::blood, CType::any},
            {CType::any, CType::any ^ CType::toxic, CType::any},
            {CType::any, CType::weak, CType::any}
        }, CType::weak},
        // Blood regenerators
        {{
            {CType::weak, CType::any, CType::any},
            {CType::weak, CType::any ^ CType::toxic, CType::any},
            {CType::weak, CType::any, CType::any}
        }, CType::blood},
        {{
            {CType::any, CType::weak, CType::any},
            {CType::any, CType::weak, CType::any},
            {CType::any, CType::weak, CType::any}
        }, CType::blood},
        {{
            {CType::any, CType::any, CType::any},
            {CType::any, CType::any ^ CType::toxic, CType::any},
            {CType::weak, CType::fluoride, CType::any}
        }, CType::blood},
        // Oxygen regenerators
        {{
            {CType::any, CType::weak, CType::any},
            {CType::weak, CType::weak, CType::any},
            {CType::any, CType::any, CType::any}
        }, CType::oxygen},
        {{
            {CType::any, CType::weak, CType::weak},
            {CType::toxic, CType::any ^ CType::toxic, CType::weak},
            {CType::any, CType::any, CType::any}
        }, CType::oxygen},
    };
    rules = temp;
}

tuple <uint8_t, uint8_t, uint8_t>getStateColor(CType cType){
    // Return a color for a state in a tuple <Blue, Green, Red>
    switch(cType){
        case tissue:
            return make_tuple(130,170,190);
        case toxic:
                return make_tuple(250,0,50);
        case fluoride:
                return make_tuple(255,0,0);
        case blood:
                return make_tuple(0,0,230);
        case stomach:
                return make_tuple(25,0,60);
        case oxygen:
                return make_tuple(210,210,210);
        case water:
                return make_tuple(200,145,13);
        case weak:
                return make_tuple(70,80,70);
        default:
            return make_tuple(50,50,50);
    }
}


unsigned getCellCoord(int val){
    return val < 0? 0: (val >= N_WIDTH? N_WIDTH - 1: val);
}

CA::CA():
        curr(N_WIDTH, vector<CType>(N_WIDTH)),
        temp(N_WIDTH, vector<CType>(N_WIDTH)){
    this->r = new Rules();
}

void CA::applyRulesToTemp(int x, int y){
    int x00 = x - 1; // Cellular matrix top left X coord for the 3x3 rule 
    int y00 = y - 1; // Cellular matrix top left Y coord for the 3x3 rule

    // Compare the rule cells with the matrix cells
    for(auto & rule : this->r->rules) {
        if(        (rule.exp[0][0] & this->curr[getCellCoord(0 + y00)][getCellCoord(0 + x00)])
                && (rule.exp[0][1] & this->curr[getCellCoord(0 + y00)][getCellCoord(1 + x00)])
                && (rule.exp[0][2] & this->curr[getCellCoord(0 + y00)][getCellCoord(2 + x00)])
                && (rule.exp[1][0] & this->curr[getCellCoord(1 + y00)][getCellCoord(0 + x00)])
                && (rule.exp[1][1] & this->curr[getCellCoord(1 + y00)][getCellCoord(1 + x00)])
                && (rule.exp[1][2] & this->curr[getCellCoord(1 + y00)][getCellCoord(2 + x00)])
                && (rule.exp[2][0] & this->curr[getCellCoord(2 + y00)][getCellCoord(0 + x00)])
                && (rule.exp[2][1] & this->curr[getCellCoord(2 + y00)][getCellCoord(1 + x00)])
                && (rule.exp[2][2] & this->curr[getCellCoord(2 + y00)][getCellCoord(2 + x00)])){
            if(this->temp[y][x] == CType::none)
                this->temp[y][x] = rule.output;
            break;
        }
    }

    // Check the neighborhood of fluoride and toxic fluoride cells
    if(this->curr[y][x] == CType::fluoride || this->curr[y][x] == CType::toxic){
        int cntWater = 0;
        int cntTissue = 0;
        int cntBlood = 0;
        int cntWeak = 0;
        int cntOxygen = 0;

        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                CType point = this->curr[getCellCoord(i + y00)][getCellCoord(j + x00)];
                
                if(point == CType::water)
                    cntWater++;
                else if(point == CType::tissue)
                    cntTissue++;
                else if(point == CType::blood)
                    cntBlood++;
                else if(point == CType::weak)
                    cntWeak++;
                else if(point == CType::oxygen)
                    cntOxygen++;
            }
        }

        // Rule: Transform fluoride to a toxic particle on the border between a tissue and water (hydrofluoric acid)
        if(cntWater > 1 && cntTissue > 0 && this->curr[y][x] == CType::fluoride && this->temp[y][x] == CType::none){
            this->temp[y][x] = CType::toxic;
        }
        // Rule: Move a fluoride left (if there is not already a fluoride and 2+ hydrofluoric is around)
        else if(this->curr[y][x] == CType::fluoride && cntWater > 1
                && this->temp[y][x] != CType::fluoride && this->temp[y][getCellCoord(x - 1)] != CType::fluoride){
            
            double moveLeftProb = 0.5; // Probability to move a fluoride left along water (hydrofluoric acid)
            if(simlib3::Random() < moveLeftProb){
                this->temp[y][x] = this->temp[y][getCellCoord(x - 1)];
                this->temp[y][getCellCoord(x - 1)] = CType::fluoride;
            }
        }
        // Rules for a toxic fluoride in tissues 
        else if(this->curr[y][x] == CType::toxic){
            // Rule: If there is a blood around or a toxic still is in a vein, randomly move
            if(cntBlood > 0 || (cntBlood == 0 && cntWeak > 0)){
                // Single cell size step left, right, up or down
                int nx = getCellCoord(x + lround(simlib3::Random() * 2 - 1));
                int ny = getCellCoord(y + lround(simlib3::Random() * 2 - 1));

                int tries = 0; // Number of tries to find a blood
                // Move to a blood cell
                while(this->curr[ny][nx] != CType::blood){
                    nx = getCellCoord(x + lround(simlib3::Random() * 2 - 1));
                    ny = getCellCoord(y + lround(simlib3::Random() * 2 - 1));

                    // Every (1/0.1)th try the toxic cells chooses random x and y +- 3 up or down
                    if(simlib3::Random() < 0.05){
                        nx = getCellCoord(lround(simlib3::Random() * N_WIDTH / 2 - 1));
                        ny = getCellCoord(y + lround(simlib3::Random() * 6 - 3));
                    }
                    if(tries >= 9)
                        break;

                    tries++;
                }
                // Randomly move a toxic cell
                static const float probToMove = 0.4;
                if(simlib3::Random() < probToMove && this->temp[y][x] != CType::toxic && this->temp[ny][nx] != CType::toxic){
                    // Last location replace with blood if there are not lots of tissues around
                    this->temp[y][x] = cntBlood + cntOxygen + cntWeak >= cntTissue - 2? CType::blood: CType::tissue;
                    this->temp[ny][nx] = CType::toxic;
                }
            }
            // Rule: Move toxic cells left (do not overwrite another toxic)
            else if(this->temp[y][x] != CType::toxic && this->temp[y][getCellCoord(x-1)] != CType::toxic){
                CType last = this->temp[y][getCellCoord(x-1)];
                this->temp[y][getCellCoord(x-1)] = CType::toxic;
                this->temp[y][x] = last;
            }
        }
    }

    // Not changed cells yet are copied
    if(this->temp[y][x] == CType::none){
        this->temp[y][x] = this->curr[y][x];
    }
}

void CA::randomMove(CType moveType, float fullness){
    // Random movement of moveType cells in the stomach
    // Possible range to move cell at
    int stepRange = 2 * MAX_STEP;

    for(unsigned i = 0; i < N_WIDTH; i++){
        for(unsigned j = 0; j < N_WIDTH; j++){

            // The previous iteration could have already placed a point at the current cell, do not overdraw
            // Copy this cell to temp
            if(this->temp[i][j] != moveType)
                this->temp[i][j] = this->curr[i][j];

            static const float initFullFactor = 0.8; // Fluoride absorbs in a speed adjusted by this coeficient
            static float probToMove = 1.0 - fullness * initFullFactor; // Probability to move: (1.0 for empty, 1-initFullFactor for full)
            
            // Conditionally move the current cell
            if(this->curr[i][j] == moveType && simlib3::Random() <= probToMove){
                // Random move at any of 3x3 positions (1/9 probability) for MAX_STEP == 1
                unsigned y = getCellCoord(i + lround(simlib3::Random() * stepRange - MAX_STEP));
                unsigned x = getCellCoord(j + lround(simlib3::Random() * stepRange - MAX_STEP));

                // If there was not (or still is not) a free space, do not move at the position
                if(!(this->curr[y][x] & (CType::stomach)) || !(this->temp[y][x] & (CType::stomach)))
                    continue;
                else{
                    // Move and replace last position with stomach 
                    this->temp[i][j] = CType::stomach;
                    this->temp[y][x] = moveType;
                }
            }
        }
    }
    // Reassign temp to a current cellular matrix
    this->curr = this->temp;
}