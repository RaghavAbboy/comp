//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[5] = { "Static", "Gshare", "Local",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

//Miscellaneous Variables
int i;

//****************************************
//Local Predictor
int phtSize;
int bhtSize;
uint32_t *pht;
uint32_t *bht;

uint32_t mask;
uint32_t phtIndex;
uint32_t phtValue;
uint32_t bhtIndex;
uint32_t bhtValue;

//****************************************
//gshare Predictor
uint32_t ghr;
uint32_t ghrMask;  //power(2,ghistoryBits) - 1

uint32_t pcMask;   //ghrMask
uint32_t maskedPC;

//already defined above, commented for reference
// uint32_t *bht
// int bhtSize

// uint32_t bhtIndex;
// uint32_t bhtValue;



//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor()
{
  // Initialize based on the bpType
  switch (bpType) {
    case STATIC:
      return;
    case GSHARE:
      init_gsharePredictor();
      break;
    case LOCAL:
      init_localPredictor();
      break;
    case TOURNAMENT:
      return;
    case CUSTOM:
      return;
    default:
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gsharePredictor(pc);
    case LOCAL:
      return localPredictor(pc);
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome)
{
  switch (bpType) {
    case STATIC:
      return;
    case GSHARE:
      train_gsharePredictor(pc, outcome);
      return;
    case LOCAL:
      train_localPredictor(pc, outcome);
      return;
    case TOURNAMENT:
      return;
    case CUSTOM:
      return;
    default:
      break;
  }
}

//------------------------------------//
//          Helper Functions          //
//------------------------------------//
int power(int x, int n) {
  int i;
  int result = 1;
  for(i=0; i<n; i++) { result *= x; }
  return result;
}

//------------------------------------//
//   Individual Predictor Functions   //
//------------------------------------//

//****************************************
//Local Predictor

void init_localPredictor() {
  //remove
  // printf("init_localPredictor called---------\n");
  // printf("lHistoryBits: %d\n", lhistoryBits);
  // printf("pcIndexBits: %d\n", pcIndexBits);

  //Initialize pht and bht
  phtSize = power(2,pcIndexBits);
  pht = malloc(sizeof(uint32_t) * phtSize);

  bhtSize = power(2,lhistoryBits);
  bht = malloc(sizeof(uint32_t) * bhtSize);

  // printf("phtSize: %d \n", phtSize);
  // printf("bhtSize: %d \n\n", bhtSize);

  //Initialize all patterns to NT
  for(i=0; i<phtSize; i++) { pht[i] = 0; }
  //Initilialize all predictors to weakly not taken
  for(i=0; i<bhtSize; i++) { bht[i] = 1; }
}


uint8_t localPredictor(uint32_t pc) {
  // printf("localPredictor called---------\n");
  // printf("PC: %x\n", pc);

  //Make a mask
  mask = power(2,pcIndexBits) - 1;
  // printf("Mask: %x\n", mask);

  phtIndex = pc & mask;
  phtValue = pht[phtIndex];

  // printf("pht Index: %x\n", phtIndex);
  // printf("pht Value: %x\n", phtValue);

  bhtIndex = phtValue;
  bhtValue = bht[bhtIndex];

  // printf("bht Index: %x\n", bhtIndex);
  // printf("bht Value: %x\n", bhtValue);

  //Decision
  int decision = (bhtValue >=2) ? TAKEN:NOTTAKEN;
  // printf("Decision: %d\n", decision);

  return decision;
}


void train_localPredictor(uint32_t pc, uint8_t outcome) {
//   printf("train_localPredictor called---------\n");
//   printf("PC: %x\n", pc);
//   printf("Outcome: %d\n", outcome);

  if(outcome) { if(bhtValue < 3) { bht[bhtIndex]++; } }
  else { if(bhtValue > 0) { bht[bhtIndex]--; } }

  // printf("New bht value: %x\n", bht[bhtIndex]);

  //New pattern
  pht[phtIndex] = pht[phtIndex] << 1 | outcome;
  pht[phtIndex] = phtIndex & (power(2,lhistoryBits)-1);
  // printf("New pht value: %x\n", pht[phtIndex]);
  // printf("Corresponding bht value: %d\n", bht[pht[phtIndex]]);
}



//****************************************
//gshare Predictor
void init_gsharePredictor() {
  // printf("init_gsharePredictor called---------\n");
  // printf("ghistoryBits: %d\n", ghistoryBits);

  //Initialize ghr to NT
  ghr = 0;
  ghrMask = power(2,ghistoryBits) - 1;
  pcMask = ghrMask;

  // printf("GHR and PC mask: %x\n", ghrMask);

  //Create a BHT
  bhtSize = power(2, ghistoryBits);
  bht = malloc(sizeof(uint32_t) * bhtSize);
  // printf("bhtSize: %d\n", bhtSize);

  //Initilialize all predictors to weakly not taken
  for(i=0; i<bhtSize; i++) { bht[i] = 1; }
};

uint8_t gsharePredictor(uint32_t pc) {
  // printf("gsharePredictor called---------\n");
  // printf("PC: %x\n", pc);

  maskedPC = pc & pcMask;
  // printf("Masked PC: %x\n", maskedPC);
  // printf("GHR: %x\n", ghr);

  bhtIndex = maskedPC ^ ghr;
  // printf("bhtIndex: %x\n", bhtIndex);

  bhtValue = bht[bhtIndex];
  // printf("bhtValue: %x\n", bhtValue);

  //Decision
  int decision = (bhtValue >=2) ? TAKEN:NOTTAKEN;
  // printf("Decision: %d\n", decision);

  return decision;
}


void train_gsharePredictor(uint32_t pc, uint8_t outcome) {
  // printf("train_gsharePredictor called---------\n");
  // printf("PC: %x\n", pc);
  // printf("Outcome: %d\n", outcome);

  if(outcome) { if(bhtValue < 3) { bht[bhtIndex]++; } }
  else { if(bhtValue > 0) { bht[bhtIndex]--; } }

  // printf("New bht value: %x\n", bht[bhtIndex]);

  //Update lhr and mask it
  ghr = ghr << 1 | outcome;
  ghr = ghr & ghrMask;
};
















