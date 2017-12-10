// GRAINv0.cpp : Defines the entry point for the console application.
//

#include "grainv0.h"


int main()
{
	GRAINV0 gv0;
	FILE *gv0keystream, *nbitkeystream, *lbitkeystream;
	long long int lengthofseq= 100;   // CHANGE THIS VARIABLE TO APPROPRIATE LENGTH
	gv0keystream = fopen("gv0_sequence.bin", "wb");
	nbitkeystream = fopen("nbit_sequence.bin", "wb");
	lbitkeystream = fopen("lbit_sequence.bin", "wb");

	//_int8 key[KEYSIZE / 8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
	//_int8	iv[IVSIZE / 8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
	_int8 key[KEYSIZE / 8] = { 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa };
	_int8	iv[IVSIZE / 8] = { 0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb };
	// initialise internal state of GRAIN
	init_grainv0(&gv0, key, iv);
	//  Key Initialisation of GRAIN :Initialise LFSR and NFSR with IV and KEY, then CLOCK it 160 times
	init_key(&gv0);
	// grainv0 Key sequence generation
	grainv0_sequence_gen(&gv0, lengthofseq, gv0keystream, nbitkeystream, lbitkeystream);
	fclose(gv0keystream);
	fclose(nbitkeystream);
	fclose(lbitkeystream);
    return 0;
}
// grainv0 Key sequence
GRAINV0OUT grainv0_onebit_gen(GRAINV0 *gv0)
{
	int count;
	//_int8 LBit, NBit, OBit;
	GRAINV0OUT gout;
	
	gout.OBit = HFUNC ^ gv0->NFSR[0];
	
	gout.NBit = NFSR(gv0->LFSR, gv0->NFSR);
	
	gout.LBit = LFSR(gv0->LFSR);
	
	//printf("%02x ", LBit);
	// upon trigger update the state of LFSR and NFSR
	for (count = 1;count<(KEYSIZE);count++)
	{
		gv0->NFSR[count - 1] = gv0->NFSR[count];
		gv0->LFSR[count - 1] = gv0->LFSR[count];
	}

	gv0->LFSR[79] = gout.LBit;
	gv0->NFSR[79] = gout.NBit;

	return gout;
}

void init_grainv0(GRAINV0 *gv0, _int8 *key, _int8 *iv)
{
	gv0->iv = iv;
	gv0->key = key;
}

void init_key(GRAINV0 *gv0)
{
	int count,ebyte;

	for (count = 0; count < KEYSIZE / 8; count++)
	{
		for (ebyte = 7; ebyte >=0; ebyte--)
		{
			if(count < 8)
				gv0->LFSR[count * 8 + (7- ebyte)] = (gv0->iv[count] >>( ebyte)) & 1;
			else 
				gv0->LFSR[count * 8 + (7- ebyte)] = (1) & 1;

			gv0->NFSR[count * 8 + (7 - ebyte)] = (gv0->key[count] >> (ebyte)) & 1;
		}
	}

	grainv0_clock160(gv0);
}

void grainv0_clock160(GRAINV0 *gv0)
{
	GRAINV0OUT gout;
	int count;
	__int8 OBit;
	for (count = 0; count < 160; count++)
	{
		gout = grainv0_onebit_gen(gv0);
		OBit = gout.OBit;      // gets o/p of Grain cipher
		gv0->LFSR[79] = gv0->LFSR[79] ^ OBit;  // feedback output to input of LFSR and NFSR
		gv0->NFSR[79] = gv0->NFSR[79] ^ OBit;
	}
}

void grainv0_sequence_gen(GRAINV0 *gv0, long long int lengthofseq, FILE *keystream, FILE *nbitkeystream, FILE *lbitkeystream)
{
	GRAINV0OUT gout;
	long long int seqlen;
	int obit;
	_int8 Outbyte=0, Nbyte = 0, Lbyte = 0;
	for (seqlen = 0;seqlen < lengthofseq;seqlen++)
	{
		for (obit = 0; obit < 8; obit++) {
			Outbyte <<= 1; Nbyte <<= 1; Lbyte <<= 1;
			gout = grainv0_onebit_gen(gv0);
			Outbyte |= gout.OBit;
			Nbyte |= gout.NBit;
			Lbyte |= gout.LBit;

			//Outbyte |= (grainv0_onebit_gen(gv0) << obit);
		}
	//	printf("%c", (unsigned char)Outbyte);
		fprintf(keystream, "%c", (unsigned char)Outbyte);
		fprintf(nbitkeystream, "%c", (unsigned char)Nbyte);
		fprintf(lbitkeystream, "%c", (unsigned char)Lbyte);
		Outbyte = 0;

	}

}