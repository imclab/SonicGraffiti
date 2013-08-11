/*
 *  pkmBinauralizerTree.cpp
 LICENCE
 
 pkmSonicGraffiti "The Software" © Parag K Mital, parag@pkmital.com
 
 The Software is and remains the property of Parag K Mital
 ("pkmital") The Licensee will ensure that the Copyright Notice set
 out above appears prominently wherever the Software is used.
 
 The Software is distributed under this Licence:
 
 - on a non-exclusive basis,
 
 - solely for non-commercial use in the hope that it will be useful,
 
 - "AS-IS" and in order for the benefit of its educational and research
 purposes, pkmital makes clear that no condition is made or to be
 implied, nor is any representation or warranty given or to be
 implied, as to (i) the quality, accuracy or reliability of the
 Software; (ii) the suitability of the Software for any particular
 use or for use under any specific conditions; and (iii) whether use
 of the Software will infringe third-party rights.
 
 pkmital disclaims:
 
 - all responsibility for the use which is made of the Software; and
 
 - any liability for the outcomes arising from using the Software.
 
 The Licensee may make public, results or data obtained from, dependent
 on or arising out of the use of the Software provided that any such
 publication includes a prominent statement identifying the Software as
 the source of the results or the data, including the Copyright Notice
 and stating that the Software has been made available for use by the
 Licensee under licence from pkmital and the Licensee provides a copy of
 any such publication to pkmital.
 
 The Licensee agrees to indemnify pkmital and hold them
 harmless from and against any and all claims, damages and liabilities
 asserted by third parties (including claims for negligence) which
 arise directly or indirectly from the use of the Software or any
 derivative of it or the sale of any products based on the
 Software. The Licensee undertakes to make no liability claim against
 any employee, student, agent or appointee of pkmital, in connection
 with this Licence or the Software.
 
 
 No part of the Software may be reproduced, modified, transmitted or
 transferred in any form or by any means, electronic or mechanical,
 without the express permission of pkmital. pkmital's permission is not
 required if the said reproduction, modification, transmission or
 transference is done without financial return, the conditions of this
 Licence are imposed upon the receiver of the product, and all original
 and amended source code is included in any transmitted product. You
 may be held legally responsible for any copyright infringement that is
 caused or encouraged by your failure to abide by these terms and
 conditions.
 
 You are not permitted under this Licence to use this Software
 commercially. Use for which any financial return is received shall be
 defined as commercial use, and includes (1) integration of all or part
 of the source code or the Software into a product for sale or license
 by or on behalf of Licensee to third parties or (2) use of the
 Software or any derivative of it for research with the final aim of
 developing software products for sale or license to a third party or
 (3) use of the Software or any derivative of it for research with the
 final aim of developing non-software products for sale or license to a
 third party, or (4) use of the Software to provide any service to an
 external organisation for which payment is received. If you are
 interested in using the Software commercially, please contact pkmital to
 negotiate a licence. Contact details are: parag@pkmital.com
 */

#include "pkmBinauralizerTree.h"
#include <Accelerate/Accelerate.h>

// HRTFs
ircam_hrtf_filter_set	pkmBinauralizerTree::m_hrtfs;		// frequency domain filters

// For kNN - using C-version of FlANN
double                  *pkmBinauralizerTree::dataset;		// positions dataset
                                                            //double					*pkmBinauralizerTree::query;		// position query 
                                                            //int						*pkmBinauralizerTree::nnIdx;		// near neighbor indices of the dataset
                                                            //double					*pkmBinauralizerTree::dists;		// near neighbor distances from the query to each dataset member
flann_index_t			pkmBinauralizerTree::kdTree;		// kd-tree reference representing all the feature-frames
struct FLANNParameters	pkmBinauralizerTree::flannParams;	// index parameters are stored here
int						pkmBinauralizerTree::k = 3;				// number of nearest neighbors
int						pkmBinauralizerTree::dim = 3;			// dimension of each point
int						pkmBinauralizerTree::pts = 187;			// number of points
bool					pkmBinauralizerTree::bBuiltIndex = false;	// did we build the kd-tree?


// For filtering
FFTSetup				pkmBinauralizerTree::fftSetup;
size_t					pkmBinauralizerTree::filterLength, 
						pkmBinauralizerTree::paddedFilterLength, 
						pkmBinauralizerTree::convolutionLength, 
						pkmBinauralizerTree::inputLength,
						pkmBinauralizerTree::fftSizeOver2,
						pkmBinauralizerTree::log2n, 
						pkmBinauralizerTree::log2nhalf;

float					*pkmBinauralizerTree::weightedLData1,
						*pkmBinauralizerTree::weightedRData1,
						*pkmBinauralizerTree::weightedLData2,
						*pkmBinauralizerTree::weightedRData2,
						*pkmBinauralizerTree::weightedLData3,
						*pkmBinauralizerTree::weightedRData3,
						*pkmBinauralizerTree::weightedLDataSummed,
						*pkmBinauralizerTree::weightedRDataSummed,
						*pkmBinauralizerTree::previousOutputLData,
						*pkmBinauralizerTree::previousOutputRData,
						*pkmBinauralizerTree::currentOutputLData,
						*pkmBinauralizerTree::currentOutputRData;

float					*pkmBinauralizerTree::paddedFilter_l, 
						*pkmBinauralizerTree::paddedFilter_r;
//COMPLEX_SPLIT			pkmBinauralizerTree::l_ir_fft,
//						pkmBinauralizerTree::r_ir_fft;

bool					pkmBinauralizerTree::bInitialized = false;


pkmBinauralizerTree::pkmBinauralizerTree()
{
	bMemberInitialized = false;
	prev_x = prev_y = prev_z = 0xFFFFFFFF;
    
}

pkmBinauralizerTree::~pkmBinauralizerTree()
{
    if(bMemberInitialized)
    {
        free(l_ir_fft.realp);
        free(l_ir_fft.imagp);
        free(r_ir_fft.realp);
        free(r_ir_fft.imagp);
        //free(paddedFilter_l);
		//free(paddedFilter_r);
		
		free(nnIdx);
		free(dists);
		free(query);
    }
}

void pkmBinauralizerTree::deallocate()
{
	if (bInitialized) {
		// clean up KDTree
		//free(nnIdx);
		//free(dists);
		//free(query);
		if(bBuiltIndex)
		{
			flann_free_index(kdTree, &flannParams);
			bBuiltIndex = false;
		}
				
		// clean up convolution
		vDSP_destroy_fftsetup(fftSetup);
		free(previousOutputLData);
		free(previousOutputRData);
		free(currentOutputLData);
		free(currentOutputRData);
		free(weightedLData1);
		free(weightedRData1);
		free(weightedLData2);
		free(weightedRData2);
		free(weightedLData3);
		free(weightedRData3);
		free(weightedLDataSummed);
		free(weightedRDataSummed);
		free(paddedFilter_l);
		free(paddedFilter_r);
		//free(l_ir_fft.realp);
		//free(l_ir_fft.imagp);
		//free(r_ir_fft.realp);
		//free(r_ir_fft.imagp);
		
		bInitialized = false;
	}
	
}

void pkmBinauralizerTree::initialize()
{
	
	// -----  Convolution Parameters ------
	filterLength = 512;
	inputLength = 512;
	
	// pad by one so convolution overlap is the same size as the signal (n + m - 1; overlap is (n <- m = n))
	paddedFilterLength = inputLength + 1;
	convolutionLength = inputLength + paddedFilterLength - 1;
	fftSizeOver2 = convolutionLength/2.0;
	log2n = log2f(convolutionLength);
	log2nhalf = log2n/2.0;
	fftSetup = vDSP_create_fftsetup(log2n, FFT_RADIX2);
	
	// allocate data (16 byte allignment)
	previousOutputLData = (float *)malloc(sizeof(float) * convolutionLength);
	previousOutputRData = (float *)malloc(sizeof(float) * convolutionLength);
	currentOutputLData = (float *)malloc(sizeof(float) * convolutionLength);
	currentOutputRData = (float *)malloc(sizeof(float) * convolutionLength);
	
	memset(previousOutputLData, 0, sizeof(float) * convolutionLength);
	memset(previousOutputRData, 0, sizeof(float) * convolutionLength);
	memset(currentOutputLData, 0, sizeof(float) * convolutionLength);
	memset(currentOutputRData, 0, sizeof(float) * convolutionLength);
	
	// more data (256 * 8 * 10 / 1024 = 20 KB)
	// these allow us to interpolate between filters
	weightedLData1 = (float *)malloc(sizeof(float) * paddedFilterLength);
	weightedRData1 = (float *)malloc(sizeof(float) * paddedFilterLength);
	weightedLData2 = (float *)malloc(sizeof(float) * paddedFilterLength);
	weightedRData2 = (float *)malloc(sizeof(float) * paddedFilterLength);
	weightedLData3 = (float *)malloc(sizeof(float) * paddedFilterLength);
	weightedRData3 = (float *)malloc(sizeof(float) * paddedFilterLength);
	weightedLDataSummed = (float *)malloc(sizeof(float) * paddedFilterLength);
	weightedRDataSummed = (float *)malloc(sizeof(float) * paddedFilterLength);
	
	memset(weightedLData1, 0, sizeof(float) * paddedFilterLength);
	memset(weightedRData1, 0, sizeof(float) * paddedFilterLength);
	memset(weightedLData2, 0, sizeof(float) * paddedFilterLength);
	memset(weightedRData2, 0, sizeof(float) * paddedFilterLength);
	memset(weightedLData3, 0, sizeof(float) * paddedFilterLength);
	memset(weightedRData3, 0, sizeof(float) * paddedFilterLength);
	memset(weightedLDataSummed, 0, sizeof(float) * paddedFilterLength);
	memset(weightedRDataSummed, 0, sizeof(float) * paddedFilterLength);
	
	// the final filter
	paddedFilter_l = (float *)malloc(sizeof(float)*convolutionLength);
	paddedFilter_r = (float *)malloc(sizeof(float)*convolutionLength);
	
	// ------ KD-tree ---------
	
	//k				= 3;						// number of nearest neighbors
	//dim				= 3;						// dimension of data (x,y,z)
	//pts				= 187;						// maximum number of data points
	
	//nnIdx			= (int *)malloc(sizeof(int)*k);			// allocate near neighbor indices
	//dists			= (double *)malloc(sizeof(double)*k);	// allocate near neighbor dists	
	//query			= (double *)malloc(sizeof(double)*dim);	// pre-allocate a query frame
	dataset         = (double *)malloc(sizeof(double)*pts*dim);							// allocate our dataset
	
	for (int i = 0; i < pts; i++) {
		for (int j = 0; j < dim; j++) {
			dataset[i*dim + j] = hrtf_positions[i][j];
		}
	}
	
	flannParams = DEFAULT_FLANN_PARAMETERS;
	flannParams.algorithm = FLANN_INDEX_AUTOTUNED; // or FLANN_INDEX_KDTREE, FLANN_INDEX_KMEANS
	flannParams.target_precision = 0.9;
	
	if(bBuiltIndex)
	{
		flann_free_index_double(kdTree, &flannParams);
		bBuiltIndex = false;
	}
	
	float speedup = 2.0;
	kdTree = flann_build_index_double(dataset, pts, dim, &speedup, &flannParams);
	bBuiltIndex = true;	
    bInitialized = true;
}

void pkmBinauralizerTree::initializeMember()
{
    if(bInitialized)
    {
        l_ir_fft.realp = (float *)malloc(sizeof(float)*convolutionLength);
        l_ir_fft.imagp = (float *)malloc(sizeof(float)*convolutionLength);
        r_ir_fft.realp = (float *)malloc(sizeof(float)*convolutionLength);
        r_ir_fft.imagp = (float *)malloc(sizeof(float)*convolutionLength);	
        nnIdx			= (int *)malloc(sizeof(int)*k);			// allocate near neighbor indices
        dists			= (double *)malloc(sizeof(double)*k);	// allocate near neighbor dists	
        query			= (double *)malloc(sizeof(double)*dim);	// pre-allocate a query frame
        
        bMemberInitialized = true;
    }
    else
    {
        cerr << "[ERROR] call pkmBinauralizerTree::initialize(); first!" << endl;
        std::exit(1);
        return;
    }
}

void pkmBinauralizerTree::updateNearestFilters(double x, double y, double z)
{
    if (!bMemberInitialized)
    {
        cerr << "[ERROR] call initialize(); first!" << endl;
        std::exit(1);
        return;
    }
    if (!bInitialized)
    {
        cerr << "[ERROR] call pkmBinauralizerTree::initialize(); first!" << endl;
        std::exit(1);
        return;
    }
	if (prev_x == x & prev_y == y && prev_z == z) {
		return;
	}
	prev_x = x;
	prev_y = y;
	prev_z = z;
	
	// convert to meters (this doesn't belong here but just for testing)
	x /= 100.0;
	y /= 100.0;
	z /= 100.0;
	
	//printf("search for neighbors at: %2.2f, %2.2f, %2.2f\n", x, y, z);
	
	// check for singularity
	// de-prioritize first by elevation
	if( x == 0.0 && y == 0.0 || x == 0.0 && z == 0.0)
	{
		x = 1.0; z = 1.0;
	}
	// so that panning is not affected
	
	// search for the nn of the query
	query[0] = x;
	query[1] = y;
	query[2] = z;	
	if (flann_find_nearest_neighbors_index_double(kdTree, 
												 query, 
												 1, 
												 nnIdx, 
												 dists, 
												 k, 
												 &flannParams) < 0)
	{
		printf("[ERROR] No frames found for Nearest Neighbor Search!\n");
		return;
	}
	
	if (k == 1) {
        cblas_scopy(filterLength, irc_1037.filters[nnIdx[0]].left, 1, weightedLDataSummed, 1);
        cblas_scopy(filterLength, irc_1037.filters[nnIdx[0]].right, 1, weightedRDataSummed, 1);
		//memcpy(weightedLDataSummed, irc_1037.filters[nnIdx[0]].left, sizeof(float)*filterLength);
		//memcpy(weightedRDataSummed, irc_1037.filters[nnIdx[0]].right, sizeof(float)*filterLength);
		
		float attenutation = powf(1.95/(dists[0] + 0.0001),3);
		vDSP_vsdiv(weightedLDataSummed, 1, &attenutation, weightedLDataSummed, 1, filterLength);
		vDSP_vsdiv(weightedRDataSummed, 1, &attenutation, weightedRDataSummed, 1, filterLength);
		
		
	}
	else if(k == 2) {
		float sumDists = dists[0] + dists[1];
		float dist1 = dists[0] / sumDists;
		float dist2 = dists[1] / sumDists;
		
		vDSP_vsmul(irc_1037.filters[nnIdx[0]].left,  1, &dist1, weightedLData1,	1, filterLength);
		vDSP_vsmul(irc_1037.filters[nnIdx[0]].right, 1, &dist1, weightedRData1,	1, filterLength);
		vDSP_vsmul(irc_1037.filters[nnIdx[1]].left,  1, &dist2, weightedLData2,	1, filterLength);
		vDSP_vsmul(irc_1037.filters[nnIdx[1]].right, 1, &dist2, weightedRData2,	1, filterLength);
		
		// add all the ffts for each the left and right channels
		vDSP_vadd(weightedLData1, 1, weightedLData2, 1, weightedLDataSummed, 1, filterLength);
		vDSP_vadd(weightedRData1, 1, weightedRData2, 1, weightedRDataSummed, 1, filterLength);
		
		// attenuate volume based on distance
		float distance = ((dists[0] + dists[1]) / 3.0) - 1.0f;
		float attenutation = powf(1.95/distance,3);
		vDSP_vsdiv(weightedLDataSummed, 1, &attenutation, weightedLDataSummed, 1, filterLength);
		vDSP_vsdiv(weightedRDataSummed, 1, &attenutation, weightedRDataSummed, 1, filterLength);
		
	}
	else if(k == 3) {	
		// turn distances into weights
		double sumDists = dists[0] + dists[1] + dists[2];
		float dist1 = dists[0] / sumDists;
		float dist2 = dists[1] / sumDists;
		float dist3 = dists[2] / sumDists;
		
		float distance = ((dists[0] + dists[1] + dists[2]) / 3.0) - 1.0f;
        vDSP_vsmul(irc_1037.filters[nnIdx[0]].left,  1, &dist1,	weightedLDataSummed, 1, filterLength);
        vDSP_vsmul(irc_1037.filters[nnIdx[0]].right, 1, &dist1,	weightedRDataSummed, 1, filterLength);
        vDSP_vsmul(irc_1037.filters[nnIdx[1]].left,  1, &dist2,	weightedLData2,	1, filterLength);
        vDSP_vsmul(irc_1037.filters[nnIdx[1]].right, 1, &dist2,	weightedRData2,	1, filterLength);
        vDSP_vsmul(irc_1037.filters[nnIdx[2]].left,  1, &dist3,	weightedLData3,	1, filterLength);
        vDSP_vsmul(irc_1037.filters[nnIdx[2]].right, 1, &dist3,	weightedRData3,	1, filterLength);
        
        vDSP_vadd(weightedLDataSummed, 1, weightedLData2, 1, weightedLData1, 1, filterLength);
        vDSP_vadd(weightedLData3, 1, weightedLData1, 1, weightedLDataSummed, 1, filterLength);
        
        vDSP_vadd(weightedRDataSummed, 1, weightedRData2, 1, weightedRData1, 1, filterLength);
        vDSP_vadd(weightedRData3, 1, weightedRData1, 1, weightedRDataSummed, 1, filterLength);
        
        
        // attenuate volume based on distance
        float attenutation = powf(1.95/(distance+0.0001),3);
        vDSP_vsdiv(weightedLDataSummed, 1, &attenutation, weightedLDataSummed, 1, filterLength);
        vDSP_vsdiv(weightedRDataSummed, 1, &attenutation, weightedRDataSummed, 1, filterLength);
		
	}
	
	vDSP_vclr(paddedFilter_l, 1, convolutionLength);
	vDSP_vclr(paddedFilter_r, 1, convolutionLength);
    cblas_scopy(filterLength, weightedLDataSummed, 1, paddedFilter_l, 1);
    cblas_scopy(filterLength, weightedRDataSummed, 1, paddedFilter_r, 1);
	//memcpy(paddedFilter_l, weightedLDataSummed, sizeof(float)*filterLength);
	//memcpy(paddedFilter_r, weightedRDataSummed, sizeof(float)*filterLength);
	
	// convert to split complex
	vDSP_ctoz((COMPLEX *)paddedFilter_l, 2, &l_ir_fft, 1, fftSizeOver2); 
	
	// fft
	vDSP_fft_zrip(fftSetup, &l_ir_fft, 1, log2n, FFT_FORWARD); 
	
	// convert to split complex
	vDSP_ctoz((COMPLEX *)paddedFilter_r, 2, &r_ir_fft, 1, fftSizeOver2); 
	
	// fft
	vDSP_fft_zrip(fftSetup, &r_ir_fft, 1, log2n, FFT_FORWARD); 
}