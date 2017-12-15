#include <stdio.h>
#include <stdlib.h>
#define KEYSIZE 80  // in bits
#define IVSIZE  64  // in bits

#define x0 gv0->LFSR[3]
#define x1 gv0->LFSR[25]
#define x2 gv0->LFSR[46]
#define x3 gv0->LFSR[64]
#define x4 gv0->NFSR[63]

#define LFSR(L)		(L[62] ^ L[51] ^ L[38] ^ L[23] ^ L[13] ^ L[0])

#define NFSR(L,N)	(L[0] ^ N[63] ^ N[60] ^ N[52] ^ N[45] ^ N[37] ^ \
					N[33] ^ N[28] ^ N[21] ^ N[15] ^ N[9 ] ^ N[0 ] ^ \
					(N[63] & N[60]) ^ (N[37] & N[33]) ^ (N[15] & N[9]) ^ \
					(N[60] & N[52] & N[45]) ^ (N[33] & N[28] & N[21]) ^ \
					(N[63] & N[45] & N[28] & N[9]) ^ (N[60] & N[52] & N[37] & N[33]) ^ \
					(N[63] & N[60] & N[21] & N[15] ) ^	\
					(N[63] & N[60] & N[52] & N[45] & N[37]) ^	\
					(N[33] & N[28] & N[21] & N[15] & N[9])	^	\
					(N[52] & N[45] & N[37] & N[33] & N[28] & N[21]) )

#define HFUNC		(x1 ^ x4 ^ (x0 & x3) ^ (x2 & x3) ^ (x3 & x4) ^	\
					(x0 & x1 & x2) ^ (x0 & x2 & x3) ^ (x0 & x2 & x4) ^  \
					(x1 & x2 & x4) ^ (x2 & x3 & x4))
typedef struct
{
	int LFSR[80];
	int NFSR[80];
	_int8 *key;
	_int8 *iv;
} GRAINV0;

typedef struct
{
	_int8 NBit;
	_int8 LBit;
	_int8 OBit;
} GRAINV0OUT;

void init_grainv0(GRAINV0 *gv0, _int8 *key, _int8 *iv);
void init_key(GRAINV0 *gv0);
void grainv0_clock160(GRAINV0 *gv0);
GRAINV0OUT grainv0_onebit_gen(GRAINV0 *gv0);
void grainv0_sequence_gen(GRAINV0 *gv0, long long int lengthofseq, FILE *gv0keystream, FILE *nbitkeystream, FILE *lbitkeystream);
_int8* grainv0_sequence_gen1(GRAINV0 *gv0, long long int lengthofseq);
int hamming(_int8 *input1, _int8 *input2, int len);
void avalanche_gen(GRAINV0 *gv0, _int8 *key, _int8 *iv);