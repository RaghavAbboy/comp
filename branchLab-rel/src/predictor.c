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
const char *studentName = "Raghav Abboy";
const char *studentID   = "A53081298";
const char *email       = "rabboy@eng.ucsd.edu";

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

// Already defined above, commented for reference
// uint32_t *bht
// int bhtSize

// uint32_t bhtIndex;
// uint32_t bhtValue;

//****************************************
//tournament Predictor

//Global history variables
// uint32_t ghr;
// uint32_t ghrMask;

uint32_t *globalBHT;
int globalBHTSize;

//Chooser Table variables
uint32_t *chooserT;
int chooserTSize;

//Local History variables
uint32_t *localPHT;
int localPHTSize;
uint32_t *localBHT;
int localBHTSize;
uint32_t localPHTMask;

//Helper variables
// uint32_t pcMask;
uint32_t localPHTIndex;
uint32_t localBHTIndex;

uint32_t localPrediction;
int localDecision;

uint32_t globalPrediction;
int globalDecision;

uint32_t chooserPrediction;
int chooserDecision;

//****************************************
//custom Predictor
// uint32_t ghr;
const int k = 5;

const int h = 10; //gHistoryBits for custom predictor

uint8_t ga[10];  //Global address register ga[h]
int theta = 44; //2.14*(h+1) + 20.58;

//weights array
int Wdimension = 32; //2^5
int W[32][32][10];  //[Wdimension][Wdimension][h]

uint8_t address;

uint32_t mask32;
uint8_t mask8;

int customDecision = 0;
uint8_t predict = 0;

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
      init_tournamentPredictor();
      break;
    case CUSTOM:
      init_customPredictor();
      break;
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

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gsharePredictor(pc);
    case LOCAL:
      return localPredictor(pc);
    case TOURNAMENT:
      return tournamentPredictor(pc);
    case CUSTOM:
      return customPredictor(pc);
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
      train_tournamentPredictor(pc, outcome);
      return;
    case CUSTOM:
      train_customPredictor(pc, outcome);
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


//****************************************
//tournament Predictor
void init_tournamentPredictor() {
  // printf("init_tournamentPredictor called---------\n");
  // printf("ghistoryBits: %d\n", ghistoryBits);
  // printf("lhistoryBits: %d\n", lhistoryBits);
  // printf("pcIndexBits: %d\n", pcIndexBits);

  pcMask = power(2, pcIndexBits) - 1;

  //Initialize ghr to NT
  ghr = 0;
  ghrMask = power(2,ghistoryBits) - 1;

  //Create local PHT
  localPHTMask = power(2,lhistoryBits) - 1;

  localPHTSize = power(2,pcIndexBits);
  localPHT = malloc(sizeof(uint32_t) * localPHTSize);

  // printf("\nlocalPHTMask: %x\n", localPHTMask);
  // printf("ghrMask: %x\n", ghrMask);
  // printf("pcMask: %x\n", pcMask);


  //Initialize localPHT table with NT
  for(i=0; i<localPHTSize; i++) { localPHT[i] = 0; }

  //Create local BHT
  localBHTSize = power(2, lhistoryBits);
  localBHT = malloc(sizeof(uint32_t) * localBHTSize);

  // printf("localPHTSize: %d\n", localPHTSize);
  // printf("localBHTSize: %d\n", localBHTSize);

  //Initialize localBHT with WNT (1)
  for(i=0; i<localBHTSize; i++) { localBHT[i] = 1; }

  //Create global BHT
  globalBHTSize = power(2, ghistoryBits);
  globalBHT = malloc(sizeof(uint32_t) * globalBHTSize);

  //Initialize global BHT with WNT (1)
  for(i=0; i<globalBHTSize; i++) { globalBHT[i] = 1; }

  //Create chooserT
  chooserTSize = power(2, ghistoryBits);
  chooserT = malloc(sizeof(uint32_t) * chooserTSize);


  //Initialize chooserT with weakly global (WG)
  /*
    3 - 11 - Strongly Local (SL)
    2 - 10 - Weakly Local (WL)
    1 - 01 - Weakly Global (WG)
    0 - 00 - Strongly Global (SG)
  */
  for(i=0; i<chooserTSize; i++) { chooserT[i] = 1; }

  // printf("globalBHTSize: %d\n", globalBHTSize);
  // printf("chooserTSize: %d\n", chooserTSize);

}

uint8_t tournamentPredictor(uint32_t pc) {
  // printf("tournamentPredictor called---------\n");
  // printf("PC: %x\n", pc);

  //Fetch local prediction, decision
  localPHTIndex = pc & pcMask;
  localBHTIndex = localPHT[localPHTIndex];
  localPrediction = localBHT[localBHTIndex];
  localDecision = (localPrediction >= 2) ? TAKEN : NOTTAKEN;

  //Fetch global prediction, decision
  globalPrediction = globalBHT[ghr];
  globalDecision = (globalPrediction >= 2) ? TAKEN : NOTTAKEN;

  //Fetch chooserPrediction, decision
  chooserPrediction = chooserT[ghr];
  chooserDecision = (chooserPrediction >= 2) ? localDecision : globalDecision;

  // printf("localPrediction: %d\n", localPrediction);
  // printf("globalPrediction: %d\n", globalPrediction);
  // printf("chooserPrediction: %d\n", chooserPrediction);

  return chooserDecision;
}

void train_tournamentPredictor(uint32_t pc, uint8_t outcome) {
  // printf("train_tournamentPredictor called---------\n");
  // printf("PC: %x\n", pc);
  // printf("Outcome: %d\n", outcome);

  //train chooser
  if(localDecision != globalDecision) {
    if(localDecision == outcome) {
      if(chooserPrediction < 3) { chooserT[ghr]++; }
    }
    else {
      if(chooserPrediction > 0) { chooserT[ghr]--; }
    }
  }

  //Update globalBHT
  if(outcome == TAKEN) {
    if(globalBHT[ghr] < 3) { globalBHT[ghr]++; }
  }
  else {
    if(globalBHT[ghr] > 0) { globalBHT[ghr]--; }
  }

  //Update ghr
  ghr = ghr<<1 | outcome;
  ghr = ghr & ghrMask;

  //Update local BHT
  if(outcome == TAKEN) {
    if(localBHT[localBHTIndex] < 3) { localBHT[localBHTIndex]++; }
  }
  else {
    if(localBHT[localBHTIndex] > 0) { localBHT[localBHTIndex]--; }
  }

  //Update local PHT
  localPHT[localPHTIndex] = localPHT[localPHTIndex]<<1 | outcome;
  localPHT[localPHTIndex] = localPHT[localPHTIndex] & localPHTMask;

}


//****************************************
//custom Predictor - Piecewise linear predictor
void init_customPredictor() {
  // printf("init_customPredictor called---------\n");

  // printf("h - ghrLength: %d\n", h);
  // printf("k - number of address bits: %d\n", k);

  //Initialize all ga[i] to 0
  for(i=0; i<h; i++) { ga[i]=0; }

  //Initialize ghr to NT
  ghr = 0;

  //Initialize all W[][][] to 0
  int j,k;

  for(i=0; i<Wdimension; i++)
    for(j=0; j<Wdimension; j++)
      for(k=0; k<h; k++)
        { W[i][j][k] = 0; }

  //Initialize masks
  k=5;
  mask8 = power(2,k) - 1;
  mask32 = power(2,k) - 1;
}

uint8_t customPredictor(uint32_t pc) {
  // printf("customPredictor called---------\n");
  // printf("PC: %x Mask:%x\n", pc, mask32);

  address = pc & mask32;  //8 bit address
  // printf("Address: %x\n", address);

  customDecision = W[address][0][0];

  for(i=0; i<h; i++) {
    uint32_t ghr_i = ghr>>i & 0x00000001;

    if(ghr_i) {
      customDecision = customDecision + W[address][ga[i]][i];
    }
    else {
      customDecision = customDecision - W[address][ga[i]][i];
    }
  }

  // printf("\nDecision: %d\n", customDecision);
  predict = customDecision >= 0;
  return predict;
}

void train_customPredictor(uint32_t pc, uint8_t outcome) {
  // printf("train_customPredictor called---------\n");
  // printf("PC: %x\n", pc);
  // printf("Predict: %d\n", predict);
  // printf("Decision: %d\n", customDecision);
  // printf("Outcome: %d\n", outcome);
  // printf("Theta: %d\n\n", theta);

  int abs_customDecision = (customDecision>=0)? customDecision:-customDecision;
  // printf("abs(Decision): %d\n", abs_customDecision);

  if(abs_customDecision < theta || predict != outcome) {
    if(outcome == 1) { if(W[address][0][0] < 127) W[address][0][0] += 1; }
    else { if(W[address][0][0] > -128) W[address][0][0] -= 1; }

    for(i=0; i<h; i++) {
      uint32_t ghr_i = ghr>>i & 0x00000001;
      if(ghr_i == outcome) { if(W[address][0][0] < 127) W[address][ga[i]][i] += 1; }
      else { if(W[address][0][0] > -128) W[address][ga[i]][i] -= 1; }
    }
  }

  //Shift in the new address into ga[]
  uint8_t newaddress = pc & mask32;

  for(i=h-1; i>0; i--) { ga[i] = ga[i-1]; }
  ga[0] = newaddress;

  //Update ghr
  // printf("ghr before update: %x\n", ghr);
  ghr = ghr<<1 | outcome;
  ghr = ghr & mask32;
  // printf("ghr after update: %x\n", ghr);
}




























