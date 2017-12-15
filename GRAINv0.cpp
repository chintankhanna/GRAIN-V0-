// GRAINv0.cpp : Defines the entry point for the console application.
//

#include "grainv0.h"


int main()
{
	
	GRAINV0 gv0;
	FILE *gv0keystream, *nbitkeystream, *lbitkeystream;
	const long long int lengthofseq= 200;   // CHANGE THIS VARIABLE TO APPROPRIATE LENGTH
	
	gv0keystream = fopen("gv0_sequence.bin", "wb");
	nbitkeystream = fopen("nbit_sequence.bin", "wb");
	lbitkeystream = fopen("lbit_sequence.bin", "wb");

	_int8 key[KEYSIZE / 8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
	_int8	iv[IVSIZE / 8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
	//_int8 key[KEYSIZE / 8] = { 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa };
	//_int8	iv[IVSIZE / 8] = { 0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb };
	// initialise internal state of GRAIN
	init_grainv0(&gv0, key, iv);
	//  Key Initialisation of GRAIN :Initialise LFSR and NFSR with IV and KEY, then CLOCK it 160 times
	init_key(&gv0);
	// grainv0 Key sequence generation
	grainv0_sequence_gen(&gv0, lengthofseq, gv0keystream, nbitkeystream, lbitkeystream);

	avalanche_gen(&gv0, key, iv);  // calculates avalanche effect over GrainV0
	fclose(gv0keystream);
	fclose(nbitkeystream);
	fclose(lbitkeystream);
	
    return 0;
}
// grainv0 Key sequence
GRAINV0OUT grainv0_onebit_gen(GRAINV0 *gv0)
{
	int count;
	
	GRAINV0OUT gout;
	
	gout.OBit = HFUNC ^ gv0->NFSR[0];
	
	gout.NBit = NFSR(gv0->LFSR, gv0->NFSR);
	
	gout.LBit = LFSR(gv0->LFSR);
	
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
				gv0->LFSR[count * 8 + (7- ebyte)] = (gv0->iv[count] >>(ebyte)) & 1;
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
		fprintf(keystream, "%c", (unsigned char)Outbyte);    // Saves keystream output
		fprintf(nbitkeystream, "%c", (unsigned char)Nbyte);  // In case analysis of outputs of NFSR and LFSR is desired
		fprintf(lbitkeystream, "%c", (unsigned char)Lbyte);
		Outbyte = 0;

	}

}

_int8* grainv0_sequence_gen1(GRAINV0 *gv0, long long int lengthofseq)
{
	GRAINV0OUT gout;
	long long int seqlen;
	int obit;
	_int8 Outbyte = 0;
	_int8 * output = (_int8 *) malloc(sizeof(_int8) * lengthofseq);
	for (seqlen = 0;seqlen < lengthofseq;seqlen++)
	{
		for (obit = 0; obit < 8; obit++) {
			Outbyte <<= 1; 
			gout = grainv0_onebit_gen(gv0);
			Outbyte |= gout.OBit;
			

			//Outbyte |= (grainv0_onebit_gen(gv0) << obit);
		}
		output[seqlen] = Outbyte;
		//	printf("%c", (unsigned char)Outbyte);
		
		Outbyte = 0;

	}
	return output;
}
int hamming(_int8 *input1, _int8 *input2, int len)
{
	_int8 temp;
	int count = 0;
	__int8 n;

	for (int i = 0; i < len; i++)
	{
		//temp = input1[i];
		temp = input1[i] ^ input2[i];
		n = (temp);

		for(int j=0; j< 8;j++)
		{
			count += n & 1;
			n >>= 1;
		}

	}


	return count;
}

void avalanche_gen(GRAINV0 *gv0, _int8 *key, _int8 *iv)
{
	int numiter = 10;
	bool overFlow = false;
	_int8 shift, temp;
	int long avalanchebits[200 * 8] = { 0 };
	const long long int lengthofseq = 200;   // CHANGE THIS VARIABLE TO APPROPRIATE LENGTH
	_int8 key1[KEYSIZE / 8];
	

	//Calculate Avalanche bits
	FILE *avalanchefp;
	avalanchefp = fopen("Avalanchebits.txt", "w");
	_int8* output1 = grainv0_sequence_gen1(gv0, lengthofseq);
	for (int g = 0; g<numiter; g++)
	{
		printf("\n iter = %ld", g);
		for (int k = 0;k < KEYSIZE / 8;k++)
		{
			key1[k] = key[k];
		}

		for (int j = KEYSIZE / 8; j >= 0; j--)  
		{
			shift = 0x01;
			while (shift)
			{
				key1[j] = key[j] ^ shift; // find next key1 with hamming distance 1 from key
				shift = shift << 1;
				init_grainv0(gv0, key1, iv);
				//  Key Initialisation of GRAIN :Initialise LFSR and NFSR with IV and KEY, then CLOCK it 160 times
				init_key(gv0);

				_int8* output2 = grainv0_sequence_gen1(gv0, lengthofseq);
				int currentHD = hamming(output1, output2, lengthofseq);
				avalanchebits[currentHD]++;
				key1[j] = key[j]; //restore the input1 to original as only a bit is to be flipped previous value to be restored
			}

		}

		// Increments key by 1
		for (int i = 0; i < KEYSIZE / 8 - 1; i++)
		{
			temp = key[i];     //increments input by 1
			key[i] = key[i] + 1;
			if (i == KEYSIZE / 8 - 1 && temp == 0xff && key[i] == 0x00)
				overFlow = true;
			if (temp< key[i])
				break;
		}

		if (overFlow)
			break;
	}

	int i;
	for (i = 0; i < lengthofseq * 8-1; i++)
	{
		fprintf(avalanchefp, "%s", "\"");
		fprintf(avalanchefp, "%ld", avalanchebits[i]);
		fprintf(avalanchefp, "%s", "\",");
	}
	fprintf(avalanchefp, "%s", "\"");
	fprintf(avalanchefp, "%ld", avalanchebits[i]);
	fprintf(avalanchefp, "%s", "\"");
	fclose(avalanchefp);
}