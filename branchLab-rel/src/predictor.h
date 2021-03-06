//========================================================//
//  predictor.h                                           //
//  Header file for the Branch Predictor                  //
//                                                        //
//  Includes function prototypes and global predictor     //
//  variables and defines                                 //
//========================================================//

#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <stdint.h>
#include <stdlib.h>

//
// Student Information
//
extern const char *studentName;
extern const char *studentID;
extern const char *email;

//------------------------------------//
//      Global Predictor Defines      //
//------------------------------------//
#define NOTTAKEN  0
#define TAKEN     1

// The Different Predictor Types
#define STATIC      0
#define GSHARE      1
#define LOCAL       2
#define TOURNAMENT  3
#define CUSTOM      4
extern const char *bpName[];

// Definitions for 2-bit counters
#define SN  0			// predict NT, strong not taken
#define WN  1			// predict NT, weak not taken
#define WT  2			// predict T, weak taken
#define ST  3			// predict T, strong taken

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//
extern int ghistoryBits; // Number of bits used for Global History
extern int lhistoryBits; // Number of bits used for Local History
extern int pcIndexBits;  // Number of bits used for PC index
extern int bpType;       // Branch Prediction Type

//------------------------------------//
//    Predictor Function Prototypes   //
//------------------------------------//

// Initialize the predictor
//
void init_predictor();

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc);

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome);

//Individual Predictor functions

//****************************************
//Local Predictor
void init_localPredictor();
uint8_t localPredictor(uint32_t);
void train_localPredictor(uint32_t, uint8_t);

//****************************************
//gshare Predictor
void init_gsharePredictor();
uint8_t gsharePredictor(uint32_t);
void train_gsharePredictor(uint32_t, uint8_t);

//****************************************
//tournament Predictor
void init_tournamentPredictor();
uint8_t tournamentPredictor(uint32_t);
void train_tournamentPredictor(uint32_t, uint8_t);

//****************************************
//custom Predictor
void init_customPredictor();
uint8_t customPredictor(uint32_t);
void train_customPredictor(uint32_t, uint8_t);

//------------------------------------//
//          Helper Functions          //
//------------------------------------//
int power(int,int);


#endif
