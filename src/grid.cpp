/**
 * @file grid.cpp
 * @author David Kedra, xkedra00
 * @author Petr Kolařík, xkolar79
 * @brief Grid initialization functions 
 * 
 * IMS Project - Cellular automata
 * VUT FIT Brno, 2022/2023 
 */

#include "grid.hpp"

void drawCell(Mat &plane, unsigned x, unsigned y, CType cType){
    static unsigned width = SIZE / N_WIDTH;
    tuple <uint8_t, uint8_t, uint8_t>color = getStateColor(cType);

    rectangle(plane,
        Point(x * width, y * width),
        Point(x * width + width, y * width + width),
        Scalar(get<0>(color), get<1>(color), get<2>(color)), FILLED);
}

void initCellularMatrix(CA *ca){
    // Background: Left side tissue and veins, Right side stomach
    for(unsigned y = 0; y < N_WIDTH; y++){
        for(unsigned x = 0; x < N_WIDTH; x++){

            if(x >= (N_WIDTH / 2)){
                // Border between sides is not fluent, but jagged
                if(ca->curr[y][x-1] == CType::tissue)
                    // Create a discontinuity on the border with a defined probability
                    ca->curr[y][x] = simlib3::Random() > 0.65? CType::tissue: CType::stomach;
                else
                    ca->curr[y][x] = CType::stomach;
            }
            else
                ca->curr[y][x] = CType::tissue;

            // Place water cells
            if(x > N_WIDTH / 2 && simlib3::Random() < WATER_PERC)
                ca->curr[y][x] = CType::water;
        }
    }
}

void placeBloodCells(CA *ca){
    const int bloodPerRow = 6;          // Approximate number of veins in a row (blood cells)
    const double expPlaceProb = 0.975;  // Probability to look more like blood veins (less random)
    
    unsigned lastX[bloodPerRow];        // Vector that remembers all veins on a current row to place blood around
    // Init the veins with random coordinates
    for(int k = 0; k < bloodPerRow; k++)
        lastX[k] = getCellCoord((int)(simlib3::Random() * N_WIDTH / 2));

    int lastXCntr = 0;
    int probX = 0;

    // Iterate over all rows and create a vein looking blood distribution
    for(unsigned y = 0; y < N_WIDTH; y++){
        for(int k = 0; k < bloodPerRow; k++){
            
            // Exponentially determined random position around a vein for the current row vein
            if(simlib3::Random() < expPlaceProb)
                probX = (int)(simlib3::Exponential(0.8)) + lastX[lastXCntr] - (int)(simlib3::Exponential(0.8));
            else
                probX = (int)(simlib3::Random() * N_WIDTH / 2);

            int x = getCellCoord(probX) % (N_WIDTH / 2); // Picked vein looking or random X coordinate

            // Update the X coordinate as a previous for the next row
            lastX[lastXCntr] = x;

            // Place the cells around a selected x to create a wider vein
            ca->curr[y][x] = CType::blood;
            ca->curr[y][getCellCoord(x-1)] = CType::blood;
            const float probToWiden = 0.3;
            if(simlib3::Random() > probToWiden)
                ca->curr[y][getCellCoord(x+1)] = CType::blood;
            if(simlib3::Random() > probToWiden)
                ca->curr[getCellCoord(y+1)][x] = CType::blood;

            lastXCntr = (lastXCntr + 1) % bloodPerRow;
        }
    }
}

void placeOxygenCells(CA *ca, unsigned *amountBlood, unsigned *amountOxygen){
    for(unsigned y = 0; y < N_WIDTH; y++){
        for(unsigned x = 0; x < N_WIDTH / 2; x++){

            if(ca->curr[y][x] == CType::blood){
                (*amountBlood)++;

                // Place an oxygen cell if there are not enough cells to meet the percentage 
                if(1.0 * *amountOxygen / *amountBlood < BOUNDED_OXYGEN){
                    ca->curr[y][x] = CType::oxygen;
                    (*amountOxygen)++;
                }
            }
        }
    }
}

void placeFluorideCells(CA *ca, unsigned *amountFluoride, float weight, unsigned ppm, unsigned toothpasteVolume, unsigned amountBlood){
    unsigned volumeBlood = weight * BLOOD_PER_KG * 1000; // Average human blood volume in litres
    // Volume of fluoride expressed as a percentage of blood
    double percFluoride = 1000 * ((ppm * DENSITY_TOOTHPASTE) * (toothpasteVolume / 1000.0)) / DENSITY_FLUORIDE / (1000 * volumeBlood);
    // Calculate a concrete number of fluoride cells to be placed
    unsigned nFluoride = amountBlood * percFluoride; // Initial max number of fluoride cells
    // Percentage of fluoride cells relative to a right side area
    double percFluorideArea = nFluoride / (N_WIDTH * N_WIDTH / 2.0);
    
    unsigned cellCount = 0; // Meantime number of placed cells

    for(unsigned y = 0; y < N_WIDTH; y++){
        for(unsigned x = N_WIDTH / 2; x < N_WIDTH; x++){

            cellCount++;
            // Place another fluoride if not enough to meet the percentage
            if(1.0 * *amountFluoride / cellCount < percFluorideArea){
                int randX = simlib3::Random() * N_WIDTH / 2 + N_WIDTH / 2;
                int init = randX;

                // Try to randomly find the first fluoride-empty cell on a row
                while(ca->curr[y][getCellCoord(randX)] == CType::fluoride){
                    randX == (++randX) % (N_WIDTH / 2);
                    // Break if there was no space on a row
                    if(init == randX)
                        break;
                }
                if(ca->curr[y][getCellCoord(randX)] != CType::fluoride){
                    ca->curr[y][getCellCoord(randX)] = CType::fluoride;
                    (*amountFluoride)++;
                }
            }
        }
    }
}
