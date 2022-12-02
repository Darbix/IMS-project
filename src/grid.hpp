#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include <cstdint>
#include <simlib.h>

#include "cellular_automata.hpp"

using namespace std;
using namespace cv;

#define DENSITY_FLUORIDE 1.696  ///< Fluoride substance density [g/l]
#define BOUNDED_OXYGEN 0.2      ///< Oxygen bounded to blood hemoglobin cells [l per l or percentile of the whole volume]
#define BLOOD_PER_KG 0.08       ///< Human voulume of blood per 1 kg of their weight
#define DENSITY_TOOTHPASTE 1.3  ///< Toothpaste density in g/ml
#define WATER_PERC 0.01         ///< Percentile of the right side which are water cells

/**
 * @brief Draw a rectangle cell at the x, y position on the plane 
 * @param plane Canvas plane window to draw at
 * @param x X coordinate
 * @param y Y coordinate
 * @param cType Cell state representing the specific color
 */
void drawCell(Mat &plane, unsigned x, unsigned y, CType cType);

/**
 * @brief Prepare the cellular plane matrix divided into two halves 
 * @param ca Cellular automata object with matrices
 */
void initCellularMatrix(CA *ca);

/**
 * @brief Place the initial blood cells into a 2D matrix 
 * @param ca Cellular automata object with matrices
 */
void placeBloodCells(CA *ca);

/**
 * @brief Place the initial oxygen cells into blood in a 2D matrix 
 * @param ca Cellular automata object with matrices
 * @param amountBlood Number of blood cells before oxygen is placed
 * @param amountOxygen Number of oxygen cells to be placed
 */
void placeOxygenCells(CA *ca, unsigned *amountBlood, unsigned *amountOxygen);

/**
 * @brief Place the initial fluoride cells into a 2D matrix 
 * @param ca Cellular automata object with matrices
 * @param amountFluoride Number of fluoride cells to be placed
 * @param weight Human weight in kg
 * @param ppm Toothpaste ppm number of fluorides
 * @param toothpasteVolume Volume of toothpaste eaten, in ml
 * @param amountBlood Number of blood + oxygen cells
 */
void placeFluorideCells(CA *ca, unsigned *amountFluoride, float weight, unsigned ppm, unsigned toothpasteVolume, unsigned amountBlood);
