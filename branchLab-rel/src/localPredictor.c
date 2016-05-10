#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>

int myPow(int x,int n)
{
    int i;
    int number = 1;
    for (i = 0; i < n; i++)
        number *= x;
    return number;
}

int main(void) {
    // Disable stdout buffering
    setvbuf(stdout, NULL, _IONBF, 0);

  int lHistoryBits = 6;
  int pcIndexBits = 2;

  printf("Local predictor---\n");
  printf("lHistoryBits: %d\n", lHistoryBits);
  printf("pcIndexBits: %d\n\n", pcIndexBits);

  //init_localPredictor
  int phtSize = myPow(2,pcIndexBits);
  uint32_t pht[phtSize];
  int bhtSize = myPow(2,lHistoryBits);
  uint32_t bht[bhtSize];


  //remove
  printf("Size of pht: %d \n", sizeof(pht)/sizeof(uint32_t));
  printf("Size of bht: %d \n", sizeof(bht)/sizeof(uint32_t));

  //initialize all patterns to NT
  int i; for(i=0; i<phtSize; i++) { pht[i] = 0; }
  //initilialize all predictors to weakly not taken
  for(i=0; i<bhtSize; i++) { bht[i] = 1; }

  //Test pht
  uint32_t pc = 0x40d7f9;
  uint8_t outcome = 1;

  printf("PC: %x\n", pc);
  printf("Outcome: %d\n\n",outcome);

  uint32_t mask = myPow(2,pcIndexBits) - 1;
  printf("Mask: %x\n", mask);

  uint32_t phtIndex = pc & mask;
  uint32_t phtValue = pht[phtIndex];

  printf("pht Index: %x\n", phtIndex);
  printf("pht Value: %x\n", phtValue);

  uint32_t bhtIndex = phtValue;
  uint32_t bhtValue = bht[bhtIndex];

  printf("bht Index: %x\n", bhtIndex);
  printf("bht Value: %x\n", bhtValue);

  //Decision
  int decision = (bhtValue >=2) ? 1:0;
  printf("Decision: %d\n", decision);

  //Train local predictor
  //Update BHT
  if(outcome) { if(bhtValue < 3) { bht[bhtIndex]++; } }
  else { if(bhtValue > 0) { bht[bhtIndex]--; } }

  printf("New bht value: %x\n", bht[bhtIndex]);
  //New pattern
  pht[phtIndex] = pht[phtIndex] >> 1 | outcome << lHistoryBits - 1;
  printf("New pht value: %x\n", pht[phtIndex]);
  printf("Corresponding bht value: %d\n", bht[pht[phtIndex]]);

    return 0;
}










