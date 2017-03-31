#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "oRS.h"
#include "append.h"
#define NULL 0
#define NPAR23 23
unsigned char genPoly23[23];

rsEncoder::rsEncoder()
{LFSR = NULL;}
rsEncoder::rsEncoder(int par)
{
	init(par);
}

rsEncoder::~rsEncoder()
{
	if (LFSR)
	 free(LFSR);
}

void rsEncoder::init(int par)
{
		NPAR = par;

		init_exp_table();
		LFSR = (unsigned char*)malloc(MAX_NPAR);
		resetData();
}
void rsEncoder::resetData()
{
	memset(LFSR, 0, NPAR);
}
void rsEncoder::output(unsigned char *dst)
{
	memcpy(dst, LFSR, NPAR);
}


void rsEncoder::append_data (unsigned char msg[], int nbytes)
{
	rs_append(msg, nbytes, LFSR, NPAR);
}

//rsDecoder
rsDecoder::rsDecoder()
{synBytes = NULL;}
rsDecoder::rsDecoder(int par)
{
	init(par);
}

void rsDecoder::init(int par)
{
	NPAR = par;
	init_exp_table();
	synBytes = new unsigned char[MAXDEG];
}
rsDecoder::~rsDecoder()
{
	if (synBytes)
	delete [] synBytes;
}
int rsDecoder::decode_data(unsigned char *data, int cwsize)
{
	int i, j;
	unsigned int sum;
	
	
	for (j = 0; j < NPAR;  j++) {
		sum	= 0;
		for (i = 0; i < cwsize; i++) {
			sum = data[i] ^ gmult(gexp[j+1], sum);
			//sum = data[i] ^ decode_mult_table[j][sum];
		}
		synBytes[j]  = sum;
	}	
	
	

	/*
	memset(synBytes, 0, 23);
	for (i=0; i<cwsize; i++)
	{
		synBytes[0] = data[i] ^ gmult(gexp[1],synBytes[0]);
		synBytes[1] = data[i] ^ gmult(gexp[2],synBytes[1]);
		synBytes[2] = data[i] ^ gmult(gexp[3],synBytes[2]);
		synBytes[3] = data[i] ^ gmult(gexp[4],synBytes[3]);
		synBytes[4] = data[i] ^ gmult(gexp[5],synBytes[4]);
		synBytes[5] = data[i] ^ gmult(gexp[6],synBytes[5]);
		synBytes[6] = data[i] ^ gmult(gexp[7],synBytes[6]);
		synBytes[7] = data[i] ^ gmult(gexp[8],synBytes[7]);
		synBytes[8] = data[i] ^ gmult(gexp[9],synBytes[8]);
		synBytes[9] = data[i] ^ gmult(gexp[10],synBytes[9]);
		synBytes[10] = data[i] ^ gmult(gexp[11],synBytes[10]);
		synBytes[11] = data[i] ^ gmult(gexp[12],synBytes[11]);
		synBytes[12] = data[i] ^ gmult(gexp[13],synBytes[12]);
		synBytes[13] = data[i] ^ gmult(gexp[14],synBytes[13]);
		synBytes[14] = data[i] ^ gmult(gexp[15],synBytes[14]);
		synBytes[15] = data[i] ^ gmult(gexp[16],synBytes[15]);
		synBytes[16] = data[i] ^ gmult(gexp[17],synBytes[16]);
		synBytes[17] = data[i] ^ gmult(gexp[18],synBytes[17]);
		synBytes[18] = data[i] ^ gmult(gexp[19],synBytes[18]);
		synBytes[19] = data[i] ^ gmult(gexp[20],synBytes[19]);
		synBytes[20] = data[i] ^ gmult(gexp[21],synBytes[20]);
		synBytes[21] = data[i] ^ gmult(gexp[22],synBytes[21]);
		synBytes[22] = data[i] ^ gmult(gexp[23],synBytes[22]);
	}
	*/
	
		
	for (int i =0 ; i < NPAR; i++) 
	{
		if (synBytes[i] != 0) return 1;
	}
	return 0;
}
int rsDecoder::decode_data(unsigned char *msg, int msg_size, unsigned char *parity)
{
	unsigned char data[256];
	memcpy(data, msg, msg_size);
	memcpy(data+msg_size, parity, NPAR);

	return decode_data(data, msg_size+NPAR);
}

	
int rsDecoder::correct_errors_erasures (unsigned char codeword[], 
			 int csize,
			 int nerasures,
			 int erasures[])
{
	int r, i, j, err;

	int decode_result = decode_data(codeword, csize);
	if (nerasures == 0 && decode_result ==0) return 1;

	unsigned char Lambda[256];
	unsigned char Omega[256];
	unsigned char psi[256];
	unsigned char psi2[256];
	unsigned char D[256];
	unsigned char gamma[256];
	unsigned char tmp[256];
	unsigned char product[512];


	int ErrorLocs[256];
	int NErrors;

	int ErasureLocs[256];
	int NErasures;

	int ret = 0;



	// If you want to take advantage of erasure correction, be sure to
	//	set NErasures and ErasureLocs[] with the locations of erasures.
	NErasures = nerasures;
	for (i = 0; i < NErasures; i++)
		ErasureLocs[i] = csize - 1 - erasures[i];

	////////////////////////////////////////////////////////////////////
	//Modified_Berlekamp_Massey();
	////////////////////////////////////////////////////////////////////
	int n, L, L2, k, d;
	// initialize Gamma, the erasure locator polynomial
	int e;
	zero_poly(gamma, MAXDEG);
	zero_poly(tmp, MAXDEG);
	gamma[0] = 1;

	for (e = 0; e < NErasures; e++) 
	{
		copy_poly(tmp, gamma, MAXDEG);
		scale_poly(gexp[ErasureLocs[e]], tmp, MAXDEG);
		mul_z_poly(tmp, MAXDEG);
		add_polys(gamma, tmp, MAXDEG);
	}

	// initialize to z
	copy_poly(D, gamma, MAXDEG);
	mul_z_poly(D, MAXDEG);

	copy_poly(psi, gamma, MAXDEG);
	k = -1; L = NErasures;

	for (n = NErasures; n < NPAR; n++)
	{
		//d = compute_discrepancy(psi, synBytes, L, n);
		d = 0;
		for (i = 0; i <= L; i++) 
			d ^= gmult(psi[i], synBytes[n-i]);

		if (d != 0) 
		{				
			// psi2 = psi - d*D 
			for (i = 0; i < MAXDEG; i++)
				psi2[i] = psi[i] ^ gmult(d, D[i]);
				
			if (L < (n-k)) 
			{
				L2 = n-k;
				k = n-L;
				// D = psi / ginv(d); scale_poly(ginv(d), psi);
				for (i = 0; i < MAXDEG; i++)
					D[i] = gmult(psi[i], ginv(d));
				L = L2;
			}
					
			// psi = psi2
			for (i = 0; i < MAXDEG; i++)
				psi[i] = psi2[i];
		}
		mul_z_poly(D, MAXDEG);
	}

	// Lambda = psi
	for(i = 0; i < MAXDEG; i++)
		Lambda[i] = psi[i];

	//compute_modified_omega();
	mult_polys(product, Lambda, synBytes, MAXDEG);
	zero_poly(Omega, MAXDEG);
	for(i = 0; i < NPAR; i++)
		Omega[i] = product[i];

	//Find_Roots();
	NErrors = 0;
	
	unsigned char sum[256];
	memset(sum, 0, 256);
	for(k=0; k<NPAR+1; k++)
	{
		sum[1] ^= gmult(Lambda[k], EKR255_table[k][1]);
		sum[2] ^= gmult(Lambda[k], EKR255_table[k][2]);
		sum[3] ^= gmult(Lambda[k], EKR255_table[k][3]);
		sum[4] ^= gmult(Lambda[k], EKR255_table[k][4]);
		sum[5] ^= gmult(Lambda[k], EKR255_table[k][5]);
		sum[6] ^= gmult(Lambda[k], EKR255_table[k][6]);
		sum[7] ^= gmult(Lambda[k], EKR255_table[k][7]);
		sum[8] ^= gmult(Lambda[k], EKR255_table[k][8]);
		sum[9] ^= gmult(Lambda[k], EKR255_table[k][9]);
		sum[10] ^= gmult(Lambda[k], EKR255_table[k][10]);
		sum[11] ^= gmult(Lambda[k], EKR255_table[k][11]);
		sum[12] ^= gmult(Lambda[k], EKR255_table[k][12]);
		sum[13] ^= gmult(Lambda[k], EKR255_table[k][13]);
		sum[14] ^= gmult(Lambda[k], EKR255_table[k][14]);
		sum[15] ^= gmult(Lambda[k], EKR255_table[k][15]);
		sum[16] ^= gmult(Lambda[k], EKR255_table[k][16]);
		sum[17] ^= gmult(Lambda[k], EKR255_table[k][17]);
		sum[18] ^= gmult(Lambda[k], EKR255_table[k][18]);
		sum[19] ^= gmult(Lambda[k], EKR255_table[k][19]);
		sum[20] ^= gmult(Lambda[k], EKR255_table[k][20]);
		sum[21] ^= gmult(Lambda[k], EKR255_table[k][21]);
		sum[22] ^= gmult(Lambda[k], EKR255_table[k][22]);
		sum[23] ^= gmult(Lambda[k], EKR255_table[k][23]);
		sum[24] ^= gmult(Lambda[k], EKR255_table[k][24]);
		sum[25] ^= gmult(Lambda[k], EKR255_table[k][25]);
		sum[26] ^= gmult(Lambda[k], EKR255_table[k][26]);
		sum[27] ^= gmult(Lambda[k], EKR255_table[k][27]);
		sum[28] ^= gmult(Lambda[k], EKR255_table[k][28]);
		sum[29] ^= gmult(Lambda[k], EKR255_table[k][29]);
		sum[30] ^= gmult(Lambda[k], EKR255_table[k][30]);
		sum[31] ^= gmult(Lambda[k], EKR255_table[k][31]);
		sum[32] ^= gmult(Lambda[k], EKR255_table[k][32]);
		sum[33] ^= gmult(Lambda[k], EKR255_table[k][33]);
		sum[34] ^= gmult(Lambda[k], EKR255_table[k][34]);
		sum[35] ^= gmult(Lambda[k], EKR255_table[k][35]);
		sum[36] ^= gmult(Lambda[k], EKR255_table[k][36]);
		sum[37] ^= gmult(Lambda[k], EKR255_table[k][37]);
		sum[38] ^= gmult(Lambda[k], EKR255_table[k][38]);
		sum[39] ^= gmult(Lambda[k], EKR255_table[k][39]);
		sum[40] ^= gmult(Lambda[k], EKR255_table[k][40]);
		sum[41] ^= gmult(Lambda[k], EKR255_table[k][41]);
		sum[42] ^= gmult(Lambda[k], EKR255_table[k][42]);
		sum[43] ^= gmult(Lambda[k], EKR255_table[k][43]);
		sum[44] ^= gmult(Lambda[k], EKR255_table[k][44]);
		sum[45] ^= gmult(Lambda[k], EKR255_table[k][45]);
		sum[46] ^= gmult(Lambda[k], EKR255_table[k][46]);
		sum[47] ^= gmult(Lambda[k], EKR255_table[k][47]);
		sum[48] ^= gmult(Lambda[k], EKR255_table[k][48]);
		sum[49] ^= gmult(Lambda[k], EKR255_table[k][49]);
		sum[50] ^= gmult(Lambda[k], EKR255_table[k][50]);
		sum[51] ^= gmult(Lambda[k], EKR255_table[k][51]);
		sum[52] ^= gmult(Lambda[k], EKR255_table[k][52]);
		sum[53] ^= gmult(Lambda[k], EKR255_table[k][53]);
		sum[54] ^= gmult(Lambda[k], EKR255_table[k][54]);
		sum[55] ^= gmult(Lambda[k], EKR255_table[k][55]);
		sum[56] ^= gmult(Lambda[k], EKR255_table[k][56]);
		sum[57] ^= gmult(Lambda[k], EKR255_table[k][57]);
		sum[58] ^= gmult(Lambda[k], EKR255_table[k][58]);
		sum[59] ^= gmult(Lambda[k], EKR255_table[k][59]);
		sum[60] ^= gmult(Lambda[k], EKR255_table[k][60]);
		sum[61] ^= gmult(Lambda[k], EKR255_table[k][61]);
		sum[62] ^= gmult(Lambda[k], EKR255_table[k][62]);
		sum[63] ^= gmult(Lambda[k], EKR255_table[k][63]);
		sum[64] ^= gmult(Lambda[k], EKR255_table[k][64]);
		sum[65] ^= gmult(Lambda[k], EKR255_table[k][65]);
		sum[66] ^= gmult(Lambda[k], EKR255_table[k][66]);
		sum[67] ^= gmult(Lambda[k], EKR255_table[k][67]);
		sum[68] ^= gmult(Lambda[k], EKR255_table[k][68]);
		sum[69] ^= gmult(Lambda[k], EKR255_table[k][69]);
		sum[70] ^= gmult(Lambda[k], EKR255_table[k][70]);
		sum[71] ^= gmult(Lambda[k], EKR255_table[k][71]);
		sum[72] ^= gmult(Lambda[k], EKR255_table[k][72]);
		sum[73] ^= gmult(Lambda[k], EKR255_table[k][73]);
		sum[74] ^= gmult(Lambda[k], EKR255_table[k][74]);
		sum[75] ^= gmult(Lambda[k], EKR255_table[k][75]);
		sum[76] ^= gmult(Lambda[k], EKR255_table[k][76]);
		sum[77] ^= gmult(Lambda[k], EKR255_table[k][77]);
		sum[78] ^= gmult(Lambda[k], EKR255_table[k][78]);
		sum[79] ^= gmult(Lambda[k], EKR255_table[k][79]);
		sum[80] ^= gmult(Lambda[k], EKR255_table[k][80]);
		sum[81] ^= gmult(Lambda[k], EKR255_table[k][81]);
		sum[82] ^= gmult(Lambda[k], EKR255_table[k][82]);
		sum[83] ^= gmult(Lambda[k], EKR255_table[k][83]);
		sum[84] ^= gmult(Lambda[k], EKR255_table[k][84]);
		sum[85] ^= gmult(Lambda[k], EKR255_table[k][85]);
		sum[86] ^= gmult(Lambda[k], EKR255_table[k][86]);
		sum[87] ^= gmult(Lambda[k], EKR255_table[k][87]);
		sum[88] ^= gmult(Lambda[k], EKR255_table[k][88]);
		sum[89] ^= gmult(Lambda[k], EKR255_table[k][89]);
		sum[90] ^= gmult(Lambda[k], EKR255_table[k][90]);
		sum[91] ^= gmult(Lambda[k], EKR255_table[k][91]);
		sum[92] ^= gmult(Lambda[k], EKR255_table[k][92]);
		sum[93] ^= gmult(Lambda[k], EKR255_table[k][93]);
		sum[94] ^= gmult(Lambda[k], EKR255_table[k][94]);
		sum[95] ^= gmult(Lambda[k], EKR255_table[k][95]);
		sum[96] ^= gmult(Lambda[k], EKR255_table[k][96]);
		sum[97] ^= gmult(Lambda[k], EKR255_table[k][97]);
		sum[98] ^= gmult(Lambda[k], EKR255_table[k][98]);
		sum[99] ^= gmult(Lambda[k], EKR255_table[k][99]);
		sum[100] ^= gmult(Lambda[k], EKR255_table[k][100]);
		sum[101] ^= gmult(Lambda[k], EKR255_table[k][101]);
		sum[102] ^= gmult(Lambda[k], EKR255_table[k][102]);
		sum[103] ^= gmult(Lambda[k], EKR255_table[k][103]);
		sum[104] ^= gmult(Lambda[k], EKR255_table[k][104]);
		sum[105] ^= gmult(Lambda[k], EKR255_table[k][105]);
		sum[106] ^= gmult(Lambda[k], EKR255_table[k][106]);
		sum[107] ^= gmult(Lambda[k], EKR255_table[k][107]);
		sum[108] ^= gmult(Lambda[k], EKR255_table[k][108]);
		sum[109] ^= gmult(Lambda[k], EKR255_table[k][109]);
		sum[110] ^= gmult(Lambda[k], EKR255_table[k][110]);
		sum[111] ^= gmult(Lambda[k], EKR255_table[k][111]);
		sum[112] ^= gmult(Lambda[k], EKR255_table[k][112]);
		sum[113] ^= gmult(Lambda[k], EKR255_table[k][113]);
		sum[114] ^= gmult(Lambda[k], EKR255_table[k][114]);
		sum[115] ^= gmult(Lambda[k], EKR255_table[k][115]);
		sum[116] ^= gmult(Lambda[k], EKR255_table[k][116]);
		sum[117] ^= gmult(Lambda[k], EKR255_table[k][117]);
		sum[118] ^= gmult(Lambda[k], EKR255_table[k][118]);
		sum[119] ^= gmult(Lambda[k], EKR255_table[k][119]);
		sum[120] ^= gmult(Lambda[k], EKR255_table[k][120]);
		sum[121] ^= gmult(Lambda[k], EKR255_table[k][121]);
		sum[122] ^= gmult(Lambda[k], EKR255_table[k][122]);
		sum[123] ^= gmult(Lambda[k], EKR255_table[k][123]);
		sum[124] ^= gmult(Lambda[k], EKR255_table[k][124]);
		sum[125] ^= gmult(Lambda[k], EKR255_table[k][125]);
		sum[126] ^= gmult(Lambda[k], EKR255_table[k][126]);
		sum[127] ^= gmult(Lambda[k], EKR255_table[k][127]);
		sum[128] ^= gmult(Lambda[k], EKR255_table[k][128]);
		sum[129] ^= gmult(Lambda[k], EKR255_table[k][129]);
		sum[130] ^= gmult(Lambda[k], EKR255_table[k][130]);
		sum[131] ^= gmult(Lambda[k], EKR255_table[k][131]);
		sum[132] ^= gmult(Lambda[k], EKR255_table[k][132]);
		sum[133] ^= gmult(Lambda[k], EKR255_table[k][133]);
		sum[134] ^= gmult(Lambda[k], EKR255_table[k][134]);
		sum[135] ^= gmult(Lambda[k], EKR255_table[k][135]);
		sum[136] ^= gmult(Lambda[k], EKR255_table[k][136]);
		sum[137] ^= gmult(Lambda[k], EKR255_table[k][137]);
		sum[138] ^= gmult(Lambda[k], EKR255_table[k][138]);
		sum[139] ^= gmult(Lambda[k], EKR255_table[k][139]);
		sum[140] ^= gmult(Lambda[k], EKR255_table[k][140]);
		sum[141] ^= gmult(Lambda[k], EKR255_table[k][141]);
		sum[142] ^= gmult(Lambda[k], EKR255_table[k][142]);
		sum[143] ^= gmult(Lambda[k], EKR255_table[k][143]);
		sum[144] ^= gmult(Lambda[k], EKR255_table[k][144]);
		sum[145] ^= gmult(Lambda[k], EKR255_table[k][145]);
		sum[146] ^= gmult(Lambda[k], EKR255_table[k][146]);
		sum[147] ^= gmult(Lambda[k], EKR255_table[k][147]);
		sum[148] ^= gmult(Lambda[k], EKR255_table[k][148]);
		sum[149] ^= gmult(Lambda[k], EKR255_table[k][149]);
		sum[150] ^= gmult(Lambda[k], EKR255_table[k][150]);
		sum[151] ^= gmult(Lambda[k], EKR255_table[k][151]);
		sum[152] ^= gmult(Lambda[k], EKR255_table[k][152]);
		sum[153] ^= gmult(Lambda[k], EKR255_table[k][153]);
		sum[154] ^= gmult(Lambda[k], EKR255_table[k][154]);
		sum[155] ^= gmult(Lambda[k], EKR255_table[k][155]);
		sum[156] ^= gmult(Lambda[k], EKR255_table[k][156]);
		sum[157] ^= gmult(Lambda[k], EKR255_table[k][157]);
		sum[158] ^= gmult(Lambda[k], EKR255_table[k][158]);
		sum[159] ^= gmult(Lambda[k], EKR255_table[k][159]);
		sum[160] ^= gmult(Lambda[k], EKR255_table[k][160]);
		sum[161] ^= gmult(Lambda[k], EKR255_table[k][161]);
		sum[162] ^= gmult(Lambda[k], EKR255_table[k][162]);
		sum[163] ^= gmult(Lambda[k], EKR255_table[k][163]);
		sum[164] ^= gmult(Lambda[k], EKR255_table[k][164]);
		sum[165] ^= gmult(Lambda[k], EKR255_table[k][165]);
		sum[166] ^= gmult(Lambda[k], EKR255_table[k][166]);
		sum[167] ^= gmult(Lambda[k], EKR255_table[k][167]);
		sum[168] ^= gmult(Lambda[k], EKR255_table[k][168]);
		sum[169] ^= gmult(Lambda[k], EKR255_table[k][169]);
		sum[170] ^= gmult(Lambda[k], EKR255_table[k][170]);
		sum[171] ^= gmult(Lambda[k], EKR255_table[k][171]);
		sum[172] ^= gmult(Lambda[k], EKR255_table[k][172]);
		sum[173] ^= gmult(Lambda[k], EKR255_table[k][173]);
		sum[174] ^= gmult(Lambda[k], EKR255_table[k][174]);
		sum[175] ^= gmult(Lambda[k], EKR255_table[k][175]);
		sum[176] ^= gmult(Lambda[k], EKR255_table[k][176]);
		sum[177] ^= gmult(Lambda[k], EKR255_table[k][177]);
		sum[178] ^= gmult(Lambda[k], EKR255_table[k][178]);
		sum[179] ^= gmult(Lambda[k], EKR255_table[k][179]);
		sum[180] ^= gmult(Lambda[k], EKR255_table[k][180]);
		sum[181] ^= gmult(Lambda[k], EKR255_table[k][181]);
		sum[182] ^= gmult(Lambda[k], EKR255_table[k][182]);
		sum[183] ^= gmult(Lambda[k], EKR255_table[k][183]);
		sum[184] ^= gmult(Lambda[k], EKR255_table[k][184]);
		sum[185] ^= gmult(Lambda[k], EKR255_table[k][185]);
		sum[186] ^= gmult(Lambda[k], EKR255_table[k][186]);
		sum[187] ^= gmult(Lambda[k], EKR255_table[k][187]);
		sum[188] ^= gmult(Lambda[k], EKR255_table[k][188]);
		sum[189] ^= gmult(Lambda[k], EKR255_table[k][189]);
		sum[190] ^= gmult(Lambda[k], EKR255_table[k][190]);
		sum[191] ^= gmult(Lambda[k], EKR255_table[k][191]);
		sum[192] ^= gmult(Lambda[k], EKR255_table[k][192]);
		sum[193] ^= gmult(Lambda[k], EKR255_table[k][193]);
		sum[194] ^= gmult(Lambda[k], EKR255_table[k][194]);
		sum[195] ^= gmult(Lambda[k], EKR255_table[k][195]);
		sum[196] ^= gmult(Lambda[k], EKR255_table[k][196]);
		sum[197] ^= gmult(Lambda[k], EKR255_table[k][197]);
		sum[198] ^= gmult(Lambda[k], EKR255_table[k][198]);
		sum[199] ^= gmult(Lambda[k], EKR255_table[k][199]);
		sum[200] ^= gmult(Lambda[k], EKR255_table[k][200]);
		sum[201] ^= gmult(Lambda[k], EKR255_table[k][201]);
		sum[202] ^= gmult(Lambda[k], EKR255_table[k][202]);
		sum[203] ^= gmult(Lambda[k], EKR255_table[k][203]);
		sum[204] ^= gmult(Lambda[k], EKR255_table[k][204]);
		sum[205] ^= gmult(Lambda[k], EKR255_table[k][205]);
		sum[206] ^= gmult(Lambda[k], EKR255_table[k][206]);
		sum[207] ^= gmult(Lambda[k], EKR255_table[k][207]);
		sum[208] ^= gmult(Lambda[k], EKR255_table[k][208]);
		sum[209] ^= gmult(Lambda[k], EKR255_table[k][209]);
		sum[210] ^= gmult(Lambda[k], EKR255_table[k][210]);
		sum[211] ^= gmult(Lambda[k], EKR255_table[k][211]);
		sum[212] ^= gmult(Lambda[k], EKR255_table[k][212]);
		sum[213] ^= gmult(Lambda[k], EKR255_table[k][213]);
		sum[214] ^= gmult(Lambda[k], EKR255_table[k][214]);
		sum[215] ^= gmult(Lambda[k], EKR255_table[k][215]);
		sum[216] ^= gmult(Lambda[k], EKR255_table[k][216]);
		sum[217] ^= gmult(Lambda[k], EKR255_table[k][217]);
		sum[218] ^= gmult(Lambda[k], EKR255_table[k][218]);
		sum[219] ^= gmult(Lambda[k], EKR255_table[k][219]);
		sum[220] ^= gmult(Lambda[k], EKR255_table[k][220]);
		sum[221] ^= gmult(Lambda[k], EKR255_table[k][221]);
		sum[222] ^= gmult(Lambda[k], EKR255_table[k][222]);
		sum[223] ^= gmult(Lambda[k], EKR255_table[k][223]);
		sum[224] ^= gmult(Lambda[k], EKR255_table[k][224]);
		sum[225] ^= gmult(Lambda[k], EKR255_table[k][225]);
		sum[226] ^= gmult(Lambda[k], EKR255_table[k][226]);
		sum[227] ^= gmult(Lambda[k], EKR255_table[k][227]);
		sum[228] ^= gmult(Lambda[k], EKR255_table[k][228]);
		sum[229] ^= gmult(Lambda[k], EKR255_table[k][229]);
		sum[230] ^= gmult(Lambda[k], EKR255_table[k][230]);
		sum[231] ^= gmult(Lambda[k], EKR255_table[k][231]);
		sum[232] ^= gmult(Lambda[k], EKR255_table[k][232]);
		sum[233] ^= gmult(Lambda[k], EKR255_table[k][233]);
		sum[234] ^= gmult(Lambda[k], EKR255_table[k][234]);
		sum[235] ^= gmult(Lambda[k], EKR255_table[k][235]);
		sum[236] ^= gmult(Lambda[k], EKR255_table[k][236]);
		sum[237] ^= gmult(Lambda[k], EKR255_table[k][237]);
		sum[238] ^= gmult(Lambda[k], EKR255_table[k][238]);
		sum[239] ^= gmult(Lambda[k], EKR255_table[k][239]);
		sum[240] ^= gmult(Lambda[k], EKR255_table[k][240]);
		sum[241] ^= gmult(Lambda[k], EKR255_table[k][241]);
		sum[242] ^= gmult(Lambda[k], EKR255_table[k][242]);
		sum[243] ^= gmult(Lambda[k], EKR255_table[k][243]);
		sum[244] ^= gmult(Lambda[k], EKR255_table[k][244]);
		sum[245] ^= gmult(Lambda[k], EKR255_table[k][245]);
		sum[246] ^= gmult(Lambda[k], EKR255_table[k][246]);
		sum[247] ^= gmult(Lambda[k], EKR255_table[k][247]);
		sum[248] ^= gmult(Lambda[k], EKR255_table[k][248]);
		sum[249] ^= gmult(Lambda[k], EKR255_table[k][249]);
		sum[250] ^= gmult(Lambda[k], EKR255_table[k][250]);
		sum[251] ^= gmult(Lambda[k], EKR255_table[k][251]);
		sum[252] ^= gmult(Lambda[k], EKR255_table[k][252]);
		sum[253] ^= gmult(Lambda[k], EKR255_table[k][253]);
		sum[254] ^= gmult(Lambda[k], EKR255_table[k][254]);
		sum[255] ^= gmult(Lambda[k], EKR255_table[k][255]);
	}
	for(r=1; r<256; r++)
		if (sum[r] == 0)
			ErrorLocs[NErrors++] = (255-r);

	/*  original code
	int sum;	
	for (r = 1; r < 256; r++) 
	{
		sum = 0;
		// evaluate lambda at r
		for (k = 0; k < static_cast<int>(NPAR+1); k++) 
		{
			#ifdef USE_MULT
			sum ^= gmult(gexp[(k*r)%255], Lambda[k]);
			#else
			sum ^= gmult(EKR255_table[r][k], Lambda[k]);
			#endif
		}
		if (sum == 0)
			ErrorLocs[NErrors++] = (255-r);
	}
	*/
	  

	if ((NErrors <= NPAR) && NErrors > 0)
	{
		// first check for illegal error locs
		for (r = 0; r < NErrors; r++)
		{
			if (ErrorLocs[r] >= csize) 
			{
				ret=0;
				goto end;
			}
		}

		for (r = 0; r < NErrors; r++) 
		{
			int num, denom;
			i = ErrorLocs[r];
			
			/*
			// evaluate Omega at alpha^(-i)
			num = 0;
			for (j = 0; j < MAXDEG; j++) 
				#ifdef USE_MULT
				num ^= gmult(Omega[j], gexp[((255-i)*j)%255]);
				#else
				num ^= gmult(Omega[j], EKR255_table[255-i][j]);
				#endif

			// evaluate Lambda' (derivative) at alpha^(-i) ; all odd powers disappear
			denom = 0;
			for (j = 1; j < MAXDEG; j += 2)
				#ifdef USE_MULT
				denom ^= gmult(Lambda[j], gexp[((255-i)*(j-1)) % 255]);
				#else
				denom ^= gmult(Lambda[j], EKR255_table[255-i][j-1]);
				#endif
			*/
			
			num = denom = 0;
			for (j=0; j<MAXDEG; j+=2)
			{
				num ^= gmult(Omega[j], EKR255_table[255-i][j]);
				num ^= gmult(Omega[j+1],EKR255_table[255-i][j+1]);
				denom ^=gmult(Lambda[j+1], EKR255_table[255-i][j]);
			}
			
			err = gmult(num, ginv(denom));
			codeword[csize-i-1] ^= err;
		}

		ret = 1;
		goto end;
	}
	else 
	{
		ret=0;
		goto end;
	}

end:
	return ret;
}
