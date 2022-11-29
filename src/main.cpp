#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include <iostream>
#include <tuple>
#include <simlib.h>
#include <getopt.h>
#include <algorithm> 

#include "cellular_automata.hpp"
#include "grid.hpp"

using namespace cv;
using namespace std;

#define EXCRETE_MINUTES 120     ///< Average time until the fluoride starts excretion to livers
#define ITERS_PER_MINUTE 10     ///< How many iterations is approximately 1 minute

unsigned fps = 2;               ///< FPS 
float weight = 40;              ///< Person weight in kg
unsigned ppm = 1500;            ///< PPM toothpaste units
float amountEaten = 1.0;        ///< Eaten amount of a whole toothpaste percentile
float fullness = 0.25;          ///< Approximate food stomach fullness percentile

int main(int argc, char **argv){
    int c;
    try{
        while ((c = getopt(argc, argv, "s:w:p:e:f:")) != -1){
            switch (c){
                case 's': // Speed of drawing
                    fps = stoi(optarg);
                    break;
                case 'w': // Weight of a person
                    weight = atof(optarg);
                    break;
                case 'p': // PPM of a toothpaste
                    ppm = stoi(optarg);
                    break;
                case 'e': // Eaten amount of the toothpaste
                    amountEaten = atof(optarg);
                    break;
                case 'f': // Fullness
                    fullness = atof(optarg);
                    break;

                default:
                    throw 99;
            }
        }
    }
    catch(int err){
        cout << "Error: Invalid argument" << endl;
        exit(err);
    }

    char window[] = "Grid";                         // Graphic window
    Mat plane = Mat::zeros(SIZE, SIZE, CV_8UC3);    // 2D matrix of (8-bit cells with 3 channels)
    CA *ca = new CA();                              // Cellular automata object with plane states
    
    // Init a random generator
    simlib3::RandomSeed(time(NULL));

    // Prepare a background (tissues|stomach) a bit jagged on a border
    // Background: Left side will be tissues and veins, Right side stomach 
    initCellularMatrix(ca);

    // Left side blood veins placement distribution
    placeBloodCells(ca);

    // Left side oxygen distribution in veins
    unsigned amountBlood = 0;   // Total amount of blood with oxygen at the start
    unsigned amountOxygen = 0;  // Amount of oxygen cells at the start
    placeOxygenCells(ca, &amountBlood, &amountOxygen);

    // Right side random placement of a certain number of fluorides
    unsigned amountFluoride = 0; // Amount of fluoride cells at the start
    placeFluorideCells(ca, &amountFluoride, weight, ppm, amountEaten, amountBlood);

    unsigned iters = 0;         // Counter of iterations
    unsigned cntFluoride = 0;   // Counter of fluoride cells
    unsigned cntOxygen = 0;     // Counter of oxygen cells
    unsigned cntBlood = 0;      // Counter of blood cells    
    unsigned cntToxic = 0;      // Counter of toxic cells
    unsigned cntWeak = 0;       // Counter of weak cells

    printf("%d FPS, %.1f kg, %d ppm, %.1f %% eaten, %.1f %% food fullness\n", fps, weight, ppm, amountEaten * 100, fullness * 100);

    // Main loop
    while(true){
        for(unsigned y = 0; y < N_WIDTH; y++){
            for(unsigned x = 0; x < N_WIDTH; x++){
                // Draw all cells
                drawCell(plane, x, y, ca->curr[y][x]);

                // Count cells
                switch(ca->curr[y][x]){
                    case(CType::fluoride): cntFluoride++; break;
                    case(CType::oxygen):   cntOxygen++;   break;
                    case(CType::blood):    cntBlood++;    break;
                    case(CType::toxic):    cntToxic++;    break;
                    case(CType::weak):     cntWeak++;     break;
                }
            }
        }
        // The blood changes over time, so the total is a sum of these cells
        cntBlood += cntOxygen + cntWeak;

        // Print the stats every X minutes (X * ITERS_PER_MINUTE) iterations
        if(!(iters % (20 * ITERS_PER_MINUTE)) || iters == 0){
            printf("--------------------------- %3d min --------------------------\n", iters / ITERS_PER_MINUTE);
            printf("Iteration: %d\n", iters);
            printf("Oxygen: %.2f %% of blood volume\n", 100.0 * cntOxygen/cntBlood);
            printf("Oxygen saturation: %.2f %%\n", min(100.0, 100.0 * cntOxygen/amountOxygen));
            printf("Fluoride in tissues %.2f mg F/kg body weight\n", (cntToxic * ppm / PPM_MG_DIVIDER) / amountFluoride / weight);
        }

        // Random movement of fluoride cells
        ca->randomMove(CType::fluoride, fullness);
        
        // Clear temp matrix with none states
        ca->temp.assign(N_WIDTH, vector<CType>(N_WIDTH, CType::none));

        double probToExcrete = 0; // Probability to excrete a current specific cell
        static const unsigned itersExcretStart = EXCRETE_MINUTES * ITERS_PER_MINUTE; // Time to start fluoride blood excretion
        static const unsigned reduceTimeFactor = 5 * ITERS_PER_MINUTE; // Every Y minutes the probability to excrete the fluoride increases
        
        // Start the excretion probability after X minutes specified using itersExcretStart iterations
        if(iters >= itersExcretStart){
            // Every Y minutes increase the excrete probability by reducing the time difference from the excretion start
            if(!(iters % (reduceTimeFactor))){
                // Excretion probability is clamped and increased using an exponential function 0.5^x to 0.0 - 1.0
                probToExcrete = 1 - pow(0.5, (iters - itersExcretStart) / (reduceTimeFactor));
            }
        }

        // Compare all the cells with reference rules
        for(int y=0; y<N_WIDTH; y++){
            for(int x=0; x<N_WIDTH; x++){
                // Adaptation of probabilities to a current number of relevant cells
                static const double fracToxic = 0.2; // Removal speed adjustment for fluoride excretion probability
                static const double fracWeak = 0.02; // Removal speed adjustment for weak excretion probability

                // If an excretion has already started
                if(probToExcrete > 0){
                    // For each toxic/weak/fluoride cell test it's removal using probToExcrete adjusted to a number of all the specific cells
                    if(ca->curr[y][x] == CType::toxic && simlib3::Random() <= probToExcrete / (fracToxic * (cntToxic + 1)))
                        ca->curr[y][x] = CType::oxygen;
                    else if(ca->curr[y][x] == CType::weak && simlib3::Random() <= probToExcrete / (fracWeak * (cntWeak + 1)))
                        ca->curr[y][x] = CType::blood;
                    else if(ca->curr[y][x] == CType::fluoride && simlib3::Random() <= probToExcrete / (cntFluoride + 1))
                        ca->curr[y][x] = CType::stomach;
                }

                // Apply the rules
                ca->applyRulesToTemp(x, y);
            }
        }
        // Reassign the new matrix to a current one
        ca->curr = ca->temp;

        // Reset the counters
        cntFluoride = cntToxic = cntOxygen = cntBlood = cntWeak = 0;
        iters++;    

        // Show the image
        imshow(window, plane);
        moveWindow(window, 200, 200);
        // Clear the plane matrix
        plane.setTo(Scalar(0,0,0));
        // Wait (1000 ms / fps) seconds and continue or exit by a key press
        if(waitKey(1000 / fps) >= 0)
            break;
     }

    return(0);
}
