/**
 *   @file  testRangeProc.c
 *
 *   @brief
 *      Unit Test bench for the range processing on DSP
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/ 
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <swpform.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <modules/rangeProc/rangeProc/api/RADARDEMO_rangeProc.h>
#include <modules/utilities/cycle_measure.h>
#include <modules/utilities/radarOsal_malloc.h>

/* Osal functions for DSP BIOS*/

#define MAXNUMHEAPS (2)
#define L2HEAPSIZE (0x20000)
#define L2SCRATCHSIZE (0x10000)
#define DDRSCRATCHSIZE (0x10)
#define DDRHEAPSIZE (0x400000)
#pragma DATA_SECTION(memHeapL2, ".L2heap")
uint8_t memHeapL2[L2HEAPSIZE];
#pragma DATA_SECTION(ddrScratchMem, ".ddrScratchSect")
uint8_t ddrScratchMem[DDRSCRATCHSIZE];
#pragma DATA_SECTION(ddrHeapMem, ".ddrHeap")
uint8_t ddrHeapMem[DDRHEAPSIZE];
#pragma DATA_SECTION(l2ScratchMem, ".L2ScratchSect")
uint8_t l2ScratchMem[L2SCRATCHSIZE];

radarOsal_heapConfig heapconfig[MAXNUMHEAPS];

#define MAXFXPERROR (12.f)
#define MAXFLTPERROR (7.f)


//glabal test vectors
#define MAX_NUM_TESTS (100)
#define MAX_FFTSIZE (4096)
#define MAX_WINSIZE (4096)

#pragma DATA_SECTION(fullTestInput, ".testVec_input")
cplx16_t fullTestInput[2 * MAX_FFTSIZE];
#pragma DATA_SECTION(win1D, ".testVec_input")
int16_t  win1D[MAX_WINSIZE];
#pragma DATA_SECTION(fullWin1D, ".testVec_input")
int16_t  fullWin1D[MAX_WINSIZE];
#pragma DATA_SECTION(fullWin2D, ".testVec_input")
int16_t  fullWin2D[MAX_WINSIZE];
#pragma DATA_SECTION(rangeProcOutputFull, ".testVec_input")
int32_t rangeProcOutputFull[2 * MAX_FFTSIZE];
#pragma DATA_SECTION(rangeFFTOrder, ".testVec_input")
int32_t rangeFFTOrder[MAX_NUM_TESTS];
#pragma DATA_SECTION(rangeInput, ".testVec_input")
RADARDEMO_rangeProc_input rangeInput;
#pragma DATA_SECTION(refTestOutput, ".testVec_input")
float refTestOutput[2 * MAX_FFTSIZE];

int main()
{
	int32_t i, testIdx, error, totalError, itemp, scale1D;
	int32_t totalNumTests, nChirpPerFrame, largestFFTSize2Test;
	RADARDEMO_rangeProc_config rangeProgConfig;
	RADARDEMO_rangeProc_input *rangeProgInput = &rangeInput;
	RADARDEMO_rangeProc_errorCode rangeProcErrorCode;
	void *rangeProcOutput;
	void * handle;
	float fscale;
	FILE * fin;
	uint8_t testFlagCases[5] ={1, 4, 4, 6, 6};
	//uint8_t testFlagCases[5] ={1, 1, 1, 1, 1};
	//uint8_t testFlagCases[5] ={2, 2, 2, 2, 2};
	//uint8_t testFlagCases[5] ={3, 3, 3, 3, 3};
	//uint8_t testFlagCases[5] ={4, 4, 4, 4, 4};
	//uint8_t testFlagCases[5] ={6, 6, 6, 6, 6};

	int32_t       t1, t2;
	int32_t         tsc_overhead;

    cache_setL1PSize(CACHE_L1_32KCACHE);
    cache_setL1DSize(CACHE_L1_32KCACHE);
    cache_setL2Size(CACHE_0KCACHE);
#ifndef _TMS320C6600 //C674x
    cache_setMar((unsigned int *)0xC0000000, 0x20000000, Cache_PC | Cache_PFX);
#else //C66x
//    Cache_setMar((unsigned int *)0x5B000000, 0x40000, Cache_PC | Cache_PFX);
    cache_setMar((unsigned int *)0x80000000, 0x20000000, Cache_PC | Cache_PFX);
#endif
    startClock();

	t1 = ranClock();
	t2 = ranClock();
	tsc_overhead = t2 - t1;

	/* parsing test vectors */
#ifdef _WIN32
	fin=fopen("..\\rangeProctestvectors.bin","rb");
#else
	fin=fopen("..\\..\\..\\rangeProctestvectors.bin","rb");
#endif
	if(fin == NULL)
	{
		printf("Fail open file rangeProctestvectors.bin\n");
		exit(1);
	}
	fread(&totalNumTests, sizeof(int32_t), 1, fin);
	fread(rangeFFTOrder, sizeof(int32_t), totalNumTests, fin);
	fread(&nChirpPerFrame, sizeof(int32_t), 1, fin);
	largestFFTSize2Test	=	1<<(rangeFFTOrder[totalNumTests-1]);
	fread(fullTestInput, sizeof(cplx16_t), largestFFTSize2Test, fin);
	fread(fullWin1D, sizeof(int16_t), largestFFTSize2Test/2, fin);
	fread(fullWin2D, sizeof(int16_t), nChirpPerFrame/2, fin);


	/*initialize input */
	rangeProgConfig.win1D			=	(int16_t *)	win1D;
	

	totalError		=	0;
	for (testIdx = 0; testIdx < totalNumTests; testIdx++ )
	{
		memset(heapconfig, 0, sizeof(heapconfig));
		heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].heapType 	= 	RADARMEMOSAL_HEAPTYPE_DDR_CACHED;
		heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].heapAddr   = 	(int8_t *) &ddrHeapMem[0];
		heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].heapSize   = 	DDRHEAPSIZE;
		heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].scratchAddr= 	NULL; 	/* not DDR scratch for TM demo  */
		heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].scratchSize= 	0; 	/* not DDR scratch for TM demo  */

		heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].heapType 			= 	RADARMEMOSAL_HEAPTYPE_LL2;
		heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].heapAddr   		= 	(int8_t *) &memHeapL2[0];
		heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].heapSize   		= 	L2HEAPSIZE;
		heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].scratchAddr   	= 	(int8_t *)&l2ScratchMem[0];;
		heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].scratchSize   	= 	L2SCRATCHSIZE;
		if(radarOsal_memInit(&heapconfig[0], MAXNUMHEAPS) == RADARMEMOSAL_FAIL)
		{
			printf("Error: radarOsal_memInit fail\n");
			return(1);
		}

		if(testIdx == 5)
			testIdx = 5;
		/* module creation */
		rangeProgConfig.fft1DSize		=	1<<rangeFFTOrder[testIdx]; 
		fread(&itemp, sizeof(int32_t), 1, fin);
		rangeProgConfig.nSamplesPerChirp	=	itemp; 
		rangeProgConfig.nRxAnt				=	0;    //no used may remove later
		rangeProgConfig.numChirpsPerFrame	=	32;   //hardcoded just for testing
		fread(&itemp, sizeof(int32_t), 1, fin);
		rangeProgConfig.adcNumOutputBits=	itemp; 
		fread(&itemp, sizeof(int32_t), 1, fin);
		rangeProgConfig.adjustDCforInput=	(RADARDEMO_rangeProc_ADCAdjustType)itemp; 
		memcpy(rangeProgConfig.win1D, fullWin1D, rangeProgConfig.nSamplesPerChirp * sizeof(int16_t));
		rangeProgConfig.win1DLength		=	(rangeProgConfig.nSamplesPerChirp >> 2) << 1; 
		rangeProgConfig.win2D			=	fullWin2D; 
		rangeProgInput->inputSignal		=	(cplx16_t *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, rangeProgConfig.fft1DSize*sizeof(cplx16_t), 8);

		itemp							=	testFlagCases[testIdx%5];
		rangeProgConfig.fft1DType		=	(RADARDEMO_rangeProc_1DFFTType) ((itemp >> 2) & 1); //RADARDEMO_RANGEPROC_1DFFT_SPxSP;
		rangeProgConfig.include2DwinFlag=	(itemp >> 1) & 1; 
		rangeProgConfig.outputFixedpFlag=	itemp & 1;

		/* to match MATLAB code */
		if (rangeProgConfig.fft1DType == RADARDEMO_RANGEPROC_1DFFT_SPxSP)
		{
			rangeProgConfig.win1DLength		=	(rangeProgConfig.nSamplesPerChirp >> 1) + 1; 
			if (((int32_t)(rangeProgConfig.nSamplesPerChirp & 0x3) == 1) || ((int32_t)(rangeProgConfig.nSamplesPerChirp & 0x3) == 2))
				rangeProgConfig.win1D[((rangeProgConfig.nSamplesPerChirp >> 2) << 1)] = 32767;
			else if ((int32_t)(rangeProgConfig.nSamplesPerChirp & 0x3) > 1)
			{
				rangeProgConfig.win1D[((rangeProgConfig.nSamplesPerChirp >> 2) << 1)] = 32767;
				rangeProgConfig.win1D[((rangeProgConfig.nSamplesPerChirp >> 2) << 1) + 1] = 32767;
			}
		}

/*
		rangeProgConfig.fft1DType		=	RADARDEMO_RANGEPROC_1DFFT_16x16; //(RADARDEMO_rangeProc_1DFFTType) ((itemp >> 2) & 1); //RADARDEMO_RANGEPROC_1DFFT_SPxSP; 
		rangeProgConfig.include2DwinFlag=	1; 
		rangeProgConfig.outputFixedpFlag=	0; 
*/

		handle = (void *) RADARDEMO_rangeProc_create(&rangeProgConfig, &rangeProcErrorCode);
		if (rangeProcErrorCode > RADARDEMO_RANGEPROC_NO_ERROR)
		{
			printf("Failed to create RADARDEMO_rangeProc module! Exit the test now!\n");
			exit(1);
		}

		rangeProgInput->chirpNumber =	testIdx;
		itemp	=	1 << (rangeProgConfig.adcNumOutputBits - 1);
		itemp	=	_pack2(itemp, itemp);
		if (rangeProgConfig.adjustDCforInput != RADARDEMO_RANGEPROC_ADC_NO_ADJUST)
		{
			for (i = 0; i < (int32_t)rangeProgConfig.nSamplesPerChirp; i++ )
			{
				_amem4(&rangeProgInput->inputSignal[i])  =	_shr2(_amem4(&fullTestInput[i]), 14 - rangeProgConfig.adcNumOutputBits);
			}
		}
		else 
		{
			for (i = 0; i < (int32_t)rangeProgConfig.nSamplesPerChirp; i++ )
			{
				_amem4(&rangeProgInput->inputSignal[i])  =	_ssub2(_shr2(_amem4(&fullTestInput[i]), 14 - rangeProgConfig.adcNumOutputBits), itemp);
			}
		}
		rangeProcOutput				=	(void *)rangeProcOutputFull;

		/*module run */
		t1 = ranClock();
		RADARDEMO_rangeProc_run(
                            handle,
							rangeProgInput,
							rangeProcOutput);
		t2 = ranClock() - t1 - tsc_overhead;
		if (rangeProcErrorCode > RADARDEMO_RANGEPROC_NO_ERROR)
		{
			printf("fatal error in RADARDEMO_rangeProc_run! Exit!");
			exit(1);
		}


		/*check results*/
		if(rangeProgConfig.fft1DType == RADARDEMO_RANGEPROC_1DFFT_16x16)
		{
			//scale1D = ((31 - _norm(rangeProgConfig.fft1DSize)) >> 1) - 1;
			scale1D = ((31 - _norm(rangeProgConfig.fft1DSize)) >> 1) + 1;
			if (scale1D &1)
				fscale = 1.4141f;
			else
				fscale = 2.f;
			/*
			if (scale1D - 15 + rangeProgConfig.adcNumOutputBits >= 0 )
				fscale = sqrtf((float)rangeProgConfig.fft1DSize)/(float)(1 << (scale1D - 16 + rangeProgConfig.adcNumOutputBits));
			else
				fscale = sqrtf((float)rangeProgConfig.fft1DSize) * (float)(1 << ( - scale1D + 16 - rangeProgConfig.adcNumOutputBits));*/
		}
		else
		{
			fscale	=	sqrtf((float)rangeProgConfig.fft1DSize);
		}
		fread(refTestOutput, sizeof(float), 2 * rangeProgConfig.nSamplesPerChirp, fin); //skip win1D ref out
		fread(refTestOutput, sizeof(float), 2 * rangeProgConfig.fft1DSize, fin); //FFT1D refout
		error = 0;
		if(rangeProgConfig.include2DwinFlag == 1)
		{
			fread(refTestOutput, sizeof(float), 2 * rangeProgConfig.fft1DSize, fin); //WIN2D refout
		}
		if(rangeProgConfig.outputFixedpFlag == 1)
		{
			cplx16_t *tempOutPtr = (cplx16_t *) rangeProcOutput;
			for (i = 1; i < (int32_t)rangeProgConfig.fft1DSize; i++ )
			{
				if (_fabs( ((float)tempOutPtr[i].real)/fscale - refTestOutput[2 * i + 1] ) > MAXFXPERROR)
					error++;
				if (_fabs( ((float)tempOutPtr[i].imag)/fscale - refTestOutput[2 * i + 0] ) > MAXFXPERROR)
					error++;
			}
		}
		else
		{
			float *tempOutPtr = (float *) rangeProcOutput;
			for (i = 1; i < (int32_t)rangeProgConfig.fft1DSize; i++ )
			{
				if (_fabs(tempOutPtr[2*i+0]/fscale - refTestOutput[2*i+1]) > MAXFLTPERROR)
					error++;
				if (_fabs(tempOutPtr[2*i+1]/fscale - refTestOutput[2*i+0]) > MAXFLTPERROR)
					error++;
			}
		}
		if (error)
		{
			printf("test %d failed!\n", testIdx);
			totalError++;
		}
		else
			printf("test %d passed! Cycle cost = %7d FFTSize = %4d Fixed16x16PFFTDisabled = %d win2Denabled = %d\n",
				testIdx, t2, rangeProgConfig.fft1DSize, (int32_t) rangeProgConfig.fft1DType, rangeProgConfig.include2DwinFlag);
		if(rangeProgConfig.include2DwinFlag == 0)
		{
			fread(refTestOutput, sizeof(float), 2 * rangeProgConfig.fft1DSize, fin); //skip WIN2D refout
		}
		
		/*reset the memory for next test*/
		RADARDEMO_rangeProc_delete(handle);
		radarOsal_memFree(rangeProgInput->inputSignal, rangeProgConfig.fft1DSize*sizeof(cplx16_t));
		radarOsal_memDeInit();
	}
	if (totalError)
		printf("Overall test failed\n");
	else
		printf("Overall test passed\n");

	fclose(fin);
}


