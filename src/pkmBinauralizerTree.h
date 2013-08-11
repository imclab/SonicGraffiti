/*
 *  pkmBinauralizerTree.h
 *  autoGraffitiFLANN
 *
 *  Created by Mr. Magoo on 7/9/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "pkmIRCAM_HRTF_DATABASE.h"			// HRTFs
#include <flann.h>
#include <Accelerate/Accelerate.h>
#include "pkmMatrix.h"

class pkmBinauralizerTree
{
public:
	
	pkmBinauralizerTree();
	~pkmBinauralizerTree();
    
	// setup binauralizer - only works for a signal of 512 for now, so you have
	// to window in order to binauralize longer samples.
	static void initialize				();
    
    void initializeMember();
	
	// call upon exit of program to free memory
	static void deallocate				();
	
	// Get the hrtf's closest to a 3D pt
	// 
	// input takes a floating pt x,y,z (in meters)
	//
	// results are in: 
	//	* nnIdx (indices of 3 nearest filters)
	//	* dists (float pt distances of these filters from pt x,y,z)
	//
	void updateNearestFilters	(double x, double y, double z);

	COMPLEX_SPLIT getLeftFilter()
	{ 
		return l_ir_fft;
	}

	COMPLEX_SPLIT getRightFilter()
	{ 
		return r_ir_fft;
	}
	

	// HRTFs
	static ircam_hrtf_filter_set	m_hrtfs;		// frequency domain filters
	
	// For kNN - using C-version of FlANN
	static double					*dataset;		// positions dataset
	double                          *query;			// position query 
	int                             *nnIdx;			// near neighbor indices of the dataset
	double                          *dists;			// near neighbor distances from the query to each dataset member
	static flann_index_t			kdTree;			// kd-tree reference representing all the feature-frames
	static struct FLANNParameters	flannParams;	// index parameters are stored here
	static int						k;				// number of nearest neighbors
	static int						dim;			// dimension of each point
	static int						pts;			// number of points
	static bool						bBuiltIndex;	// did we build the kd-tree?
	
	
	// For filtering
	static FFTSetup					fftSetup;
	static size_t					filterLength, 
									paddedFilterLength, 
									convolutionLength, 
									inputLength,
									fftSizeOver2,
									log2n, 
									log2nhalf;
	
	double                          prev_x, prev_y, prev_z;
	
	static float					*weightedLData1,
									*weightedRData1,
									*weightedLData2,
									*weightedRData2,
									*weightedLData3,
									*weightedRData3,
									*weightedLDataSummed,
									*weightedRDataSummed,
									*previousOutputLData,
									*previousOutputRData,
									*currentOutputLData,
									*currentOutputRData;
	
	static float                    *paddedFilter_l, 
									*paddedFilter_r;
	COMPLEX_SPLIT                   l_ir_fft,
									r_ir_fft;
	
	static bool						bInitialized;
    bool                            bMemberInitialized;
};