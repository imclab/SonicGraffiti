/*
 *  pkmMatrix.cpp
 *  
 
 row-major floating point matrix utility class
 utilizes Apple Accelerate's vDSP functions for SSE optimizations
 
 Copyright (C) 2011 Parag K. Mital
 
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
 
 *
 */

#include "pkmMatrix.h"
#include <math.h>

using namespace pkm;

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

Mat::Mat()
{
	bUserData = false;
	rows = cols = 0;
	data = temp_data = NULL;
	bAllocated = false;
	current_row = 0;
	bCircularInsertionFull = false;
}

// destructor
Mat::~Mat()
{
	//printf("destruction\n");
	releaseMemory();
    
	rows = cols = 0;
	current_row = 0;
	bCircularInsertionFull = false;
	bAllocated = false;
}

Mat::Mat(vector<float> m)
{
    rows = 1;
    cols = m.size();
    data = (float *)malloc(sizeof(float)*cols);
	
    // sacrifice memory w/ speed, by pre-allocating a temporary buffer
	temp_data = (float *)malloc(cols * sizeof(float));
    
    cblas_scopy(cols, &m[0], 1, data, 1);
	current_row = 0;
	bCircularInsertionFull = false;
	bUserData = false;
	bAllocated = true;
}

Mat::Mat(vector<vector<float> > m)
{
    rows = m.size();
    cols = m[0].size();
    data = (float *)malloc(sizeof(float)*rows*cols);
    // sacrifice memory w/ speed, by pre-allocating a temporary buffer
	temp_data = (float *)malloc(rows * cols * sizeof(float));
    
    for(int i = 0; i < rows; i++)
        cblas_scopy(cols, &(m[i][0]), 1, data+i*cols, 1);
    
	current_row = 0;
	bCircularInsertionFull = false;
	bUserData = false;
	bAllocated = true;
}

#ifdef HAVE_OPENCV
Mat::Mat(cv::Mat m)
{
    rows = m.rows;
    cols = m.cols;
    data = (float *)malloc(sizeof(float)*rows*cols);
    // sacrifice memory w/ speed, by pre-allocating a temporary buffer
	temp_data = (float *)malloc(rows * cols * sizeof(float));
    
    for(int i = 0; i < rows; i++)
        cblas_scopy(cols, m.ptr<float>(i), 1, data+i*cols, 1);
    
	current_row = 0;
	bCircularInsertionFull = false;
	bUserData = false;
	bAllocated = true;
}
#endif
// allocate data
Mat::Mat(int r, int c, bool clear)
{
	data = temp_data = NULL;
	
	bUserData = false;
	rows = r;
	cols = c;
	current_row = 0;
	bCircularInsertionFull = false;
	data = (float *)malloc(rows * cols * sizeof(float));
	
	// sacrifice memory w/ speed, by pre-allocating a temporary buffer
	temp_data = (float *)malloc(rows * cols * sizeof(float));

	bAllocated = true;
	
	// set every element to 0
	if(clear)
	{
		vDSP_vclr(data, 1, rows*cols);
	}
}

// pass in existing data
// non-destructive by default
// this WILL destroy the passed in data when object leaves scope if
// with copy is not true
Mat::Mat(int r, int c, float *existing_buffer, bool withCopy)
{
	data = temp_data = NULL;
	
	bUserData = false;
	rows = r;
	cols = c;
	current_row = 0;
	bCircularInsertionFull = false;
	
	// sacrifice memory w/ speed, by pre-allocating a temporary buffer
	temp_data = (float *)malloc(rows * cols * sizeof(float));
	
	if(withCopy)
	{
		data = (float *)malloc(rows * cols * sizeof(float));
		
		cblas_scopy(rows*cols, existing_buffer, 1, data, 1);
		//memcpy(data, existing_buffer, sizeof(float)*r*c);
	}
	else {
		// user gave us data, don't free it.
		bUserData = true;
		data = existing_buffer;
	}
	
	bAllocated = true;
}

// set every element to a value
Mat::Mat(int r, int c, float val)
{
	data = temp_data = NULL;
	
	bUserData = false;
	rows = r;
	cols = c;
	current_row = 0;
	bCircularInsertionFull = false;
	
	data = (float *)malloc(rows * cols * sizeof(float));
	// sacrifice memory w/ speed, by pre-allocating a temporary buffer
	temp_data = (float *)malloc(rows * cols * sizeof(float));
	
	bAllocated = true;
	
	// set every element to val
	vDSP_vfill(&val, data, 1, rows * cols);
	
}

// copy-constructor, called during:
//		pkm::Mat a = rhs;
//		pkm::Mat a(rhs);
Mat::Mat(const Mat &rhs)
{		
	if(rhs.bAllocated)
	{
		bUserData = false;
		
		rows = rhs.rows;
		cols = rhs.cols;
		current_row = rhs.current_row;
		bCircularInsertionFull = rhs.bCircularInsertionFull;
		
		data = (float *)malloc(rows * cols * sizeof(float));
		
		// sacrifice memory w/ speed, by pre-allocating a temporary buffer
		temp_data = (float *)malloc(rows * cols * sizeof(float));
		
		bAllocated = true;
		
		cblas_scopy(rows*cols, rhs.data, 1, data, 1);
		//memcpy(data, rhs.data, sizeof(float)*rows*cols);
	}
	else {
		rows = 0;
		cols = 0;
		current_row = 0;
		bCircularInsertionFull = false;
		
		data = NULL;
		temp_data = NULL;
		bUserData = false;
		bAllocated = false;
	}
}

Mat & Mat::operator=(const Mat &rhs)
{	
	
	if(data == rhs.data)
		return *this;
	
	if(rhs.bAllocated)
	{
		bUserData = false;
		
		if (rows != rhs.rows || cols != rhs.cols) {

			rows = rhs.rows;
			cols = rhs.cols;
			current_row = rhs.current_row;
			bCircularInsertionFull = rhs.bCircularInsertionFull;
			
            releaseMemory();
			
            
            data = (float *)malloc(rows * cols * sizeof(float));
            temp_data = (float *)malloc(rows * cols * sizeof(float));
            bAllocated = true;
        
		}
		
		cblas_scopy(rows*cols, rhs.data, 1, data, 1);
		//memcpy(data, rhs.data, sizeof(float)*rows*cols);
		
		return *this;
	}
	else 
	{
        releaseMemory();
        
		bUserData = false;
		rows = 0;
		cols = 0;
		current_row = 0;
		bCircularInsertionFull = false;
		data = NULL;
		temp_data = NULL;
		
		bAllocated = false;
		return *this;
	}			
}


Mat & Mat::operator=(const vector<float> &rhs)
{	
	
	if(rhs.size() != 0)
	{
		bUserData = false;
		
		if (rows != 1 || cols != rhs.size()) {
            
			rows = 1;
			cols = rhs.size();
			current_row = 0;
			bCircularInsertionFull = false;
			
            releaseMemory();
			
			data = (float *)malloc(rows * cols * sizeof(float));
			temp_data = (float *)malloc(rows * cols * sizeof(float));
			
			bAllocated = true;
		}
		
		cblas_scopy(rows*cols, &(rhs[0]), 1, data, 1);
		//memcpy(data, rhs.data, sizeof(float)*rows*cols);
		
		return *this;
	}
	else 
	{
        releaseMemory();
        
		bUserData = false;
		rows = 0;
		cols = 0;
		current_row = 0;
		bCircularInsertionFull = false;
		data = NULL;
		temp_data = NULL;
		
		bAllocated = false;
		return *this;
	}			
}


Mat & Mat::operator=(const vector<vector<float> > &rhs)
{	
	
	if(rhs.size() != 0)
	{
		bUserData = false;
		
		if (rows != rhs.size() || cols != rhs[0].size()) {
            
			rows = rhs.size();
			cols = rhs[0].size();
			current_row = 0;
			bCircularInsertionFull = false;
			
            releaseMemory();
			
			data = (float *)malloc(rows * cols * sizeof(float));
			temp_data = (float *)malloc(rows * cols * sizeof(float));
			
			bAllocated = true;
		}
		
        for(int i = 0; i < rows; i++)
            cblas_scopy(cols, &(rhs[i][0]), 1, data+i*cols, 1);

		//memcpy(data, rhs.data, sizeof(float)*rows*cols);
		
		return *this;
	}
	else 
	{
        releaseMemory();
        
		bUserData = false;
		rows = 0;
		cols = 0;
		current_row = 0;
		bCircularInsertionFull = false;
		data = NULL;
		temp_data = NULL;
		
		bAllocated = false;
		return *this;
	}			
}

#ifdef HAVE_OPENCV
Mat & Mat::operator=(const cv::Mat &rhs)
{	
	
	if(rhs.rows > 0 && rhs.cols > 0)
	{
		bUserData = false;
		
		if (rows != rhs.rows || cols != rhs.cols) {
            
			rows = rhs.rows;
			cols = rhs.cols;
			current_row = 0;
			bCircularInsertionFull = false;
			
            releaseMemory();
			
			data = (float *)malloc(rows * cols * sizeof(float));
			temp_data = (float *)malloc(rows * cols * sizeof(float));
			
			bAllocated = true;
		}
		
        for(int i = 0; i < rows; i++)
            cblas_scopy(cols, rhs.ptr<float>(i), 1, data+i*cols, 1);
        
		//memcpy(data, rhs.data, sizeof(float)*rows*cols);
		
		return *this;
	}
	else 
	{
        releaseMemory();
        
		bUserData = false;
		rows = 0;
		cols = 0;
		current_row = 0;
		bCircularInsertionFull = false;
		data = NULL;
		temp_data = NULL;
		
		bAllocated = false;
		return *this;
	}			
}


cv::Mat Mat::cvMat()
{
    cv::Mat cvm(rows, cols, CV_32FC1, data);
    return cvm;
}

#endif

/////////////////////////////////////////



/////////////////////////////////////////


Mat Mat::getTranspose()
{
#ifndef DEBUG			
	assert(data != NULL);
#endif	
	Mat transposedMatrix(cols, rows);
	
	if (rows == 1 || cols == 1) {
		cblas_scopy(rows*cols, data, 1, transposedMatrix.data, 1);
		//memcpy(transposedMatrix.data, data, sizeof(float)*rows*cols);
	}
	else {
		vDSP_mtrans(data, 1, transposedMatrix.data, 1, cols, rows);
	}
	
	return transposedMatrix;
}


// get a diagonalized version of the current vector (non-destructive)
Mat Mat::getDiag()
{
#ifndef DEBUG
	assert(data != NULL);
#endif	
	if((rows == 1 && cols > 1) || (cols == 1 && rows > 1))
	{
		int diagonal_elements = MAX(rows,cols);
		
		// create a square matrix
		Mat diagonalMatrix(diagonal_elements, diagonal_elements, true);
		
		// set diagonal elements to the current vector in data
		for (int i = 0; i < diagonal_elements; i++) {
			diagonalMatrix.data[i*diagonal_elements+i] = data[i];
		}
		return diagonalMatrix;
	}
	else if(rows == 1 && cols == 1)
	{
		return *this;
	}
	else {
		printf("[ERROR]: Cannot diagonalize a matrix. Either rows or cols must be == 1.");
		Mat A;
		return A;
	}
}

Mat Mat::diag(Mat &A)
{
	if((A.rows == 1 && A.cols > 1) || (A.cols == 1 && A.rows > 1))
	{
		int diagonal_elements = MAX(A.rows,A.cols);
		
		// create a square matrix
		Mat diagonalMatrix(diagonal_elements,diagonal_elements, true);
		
		// set diagonal elements to the current vector in data
		for (int i = 0; i < diagonal_elements; i++) {
			diagonalMatrix.data[i*diagonal_elements+i] = A.data[i];
		}
		return diagonalMatrix;
	}
	else {
		printf("[ERROR]: Cannot diagonalize a matrix. Either rows or cols must be == 1.");
		Mat A;
		return A;
	}
}

Mat Mat::abs(Mat &A)
{
#ifdef DEBUG			
    assert(A.data != NULL);
    assert(A.rows >0 &&
           A.cols >0);
#endif	
	Mat newMat(A.rows, A.cols);
    vDSP_vabs(A.data, 1, newMat.data, 1, A.rows * A.cols);
    return newMat;
}


Mat Mat::log(Mat &A)
{
#ifdef DEBUG			
    assert(A.data != NULL);
    assert(A.rows >0 &&
           A.cols >0);
#endif	
	Mat newMat(A.rows, A.cols);
	for(int i = 0; i < A.rows*A.cols; i++)
	{
		newMat.data[i] = logf(A.data[i]);
	}
	return newMat;
}

Mat Mat::exp(Mat &A)
{
#ifdef DEBUG			
    assert(A.data != NULL);
    assert(A.rows >0 &&
           A.cols >0);
#endif	
	Mat newMat(A.rows, A.cols);
	for(int i = 0; i < A.rows*A.cols; i++)
	{
		newMat.data[i] = expf(A.data[i]);
	}
	return newMat;
}

Mat Mat::identity(int dim)
{
	
	// create a square matrix
	Mat identityMatrix(dim,dim, true);
	
	// set diagonal elements to the current vector in data
	for (int i = 0; i < dim; i++) {
		identityMatrix.data[i*dim+i] = 1;
	}
	
	return identityMatrix;
}


// set every element to a random value between low and high
void Mat::setRand(float low, float high)
{
	float width = (high-low);
	float *ptr = data;
	for (int i = 0; i < rows*cols; i++) {
		*ptr = low + (float(::random())/float(RAND_MAX))*width;
		++ptr;
	}
}

// create a random matrix
Mat Mat::rand(int r, int c, float low, float high)
{
	Mat randomMatrix(r, c);
	randomMatrix.setRand(low, high);
	return randomMatrix;
}

Mat Mat::sum(bool across_rows)
{
	// sum across rows
	if(across_rows)
	{
		Mat result(1, cols);
		for (int i = 0; i < cols; i++) {
			vDSP_sve(data+i, cols, result.data+i, rows);
		}				
		return result;
	}
	// cols
	else
	{
		Mat result(rows, 1);
		for (int i = 0; i < rows; i++) {
			vDSP_sve(data+(i*cols), 1, result.data+i, cols);
		}
		return result;
	}
	
}

// normalize the values for each row-vector
void Mat::setNormalize(bool row_major)
{
	if (row_major) {
		for (int r = 0; r < rows; r++) {
			float min, max;
			vDSP_minv(&(data[r*cols]), 1, &min, cols);
			vDSP_maxv(&(data[r*cols]), 1, &max, cols);
			float height = max-min;
			min = -min;
			vDSP_vsadd(&(data[r*cols]), 1, &min, &(data[r*cols]), 1, cols);
			if (height != 0) {
				vDSP_vsdiv(&(data[r*cols]), 1, &height, &(data[r*cols]), 1, cols);	
			}
		}			
	}
	// or for each column
	else {
		for (int c = 0; c < cols; c++) {
			float min, max;
			vDSP_minv(&(data[c]), cols, &min, rows);
			vDSP_maxv(&(data[c]), cols, &max, rows);
			float height = max-min;
			min = -min;
			vDSP_vsadd(&(data[c]), cols, &min, &(data[c]), cols, rows);
			if (height != 0) {
				vDSP_vsdiv(&(data[c]), cols, &height, &(data[c]), cols, rows);	
			}
		}
	}
}

void Mat::divideEachVecByMaxVecElement(bool row_major)
{
	if (row_major) {
		for (int r = 0; r < rows; r++) {
			int idx = cblas_isamax(cols, data+r*cols, 1);
			float val = *(data+r*cols+idx);
			if (val != 0.0f) {
				vDSP_vsdiv(&(data[r*cols]), 1, &val, &(data[r*cols]), 1, cols);	
			}
		}
	}
	else {
		for (int c = 0; c < cols; c++) {
			int idx = cblas_isamax(rows, data+c, cols);
			float val = *(data+c+idx);
			if (val != 0.0f) {
				vDSP_vsdiv(&(data[c]), cols, &val, &(data[c]), cols, rows);	
			}
		}
	}
}

void Mat::divideEachVecBySum(bool row_major)
{
	if (row_major) {
		for (int r = 0; r < rows; r++) {
			float val;
			vDSP_sve(data+r*cols, 1, &val, cols);
			if (val != 0.0f) {
				vDSP_vsdiv(data+r*cols, 1, &val, data+r*cols, 1, cols);	
			}
		}
	}
	else {
		for (int c = 0; c < cols; c++) {
			float val;
			vDSP_sve(data+c, cols, &val, rows);
			if (val != 0.0f) {
				vDSP_vsdiv(data+c, cols, &val, data+c, cols, rows);	
			}
		}
	}
}

void Mat::printAbbrev(bool row_major)
{
	
	printf("r: %d, c: %d\n", rows, cols);
	
	if(row_major)
	{
		for (int r = 0; r < MIN(rows,5); r++) {
			for (int c = 0; c < MIN(cols,5); c++) {
				printf("%8.2f ", data[r*cols + c]);
			}
			printf("\n");
		}
		printf("\n");
	}
	else {
		for (int r = 0; r < MIN(rows,5); r++) {
			for (int c = 0; c < MIN(cols,5); c++) {
				printf("%8.2f ", data[c*rows + r]);
			}
			printf("\n");
		}
		printf("\n");
	}
	
}

void Mat::print(bool row_major)
{
	
	printf("r: %d, c: %d\n", rows, cols);
	
	if(row_major)
	{
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				printf("%8.2f ", data[r*cols + c]);
			}
			printf("\n");
		}
		printf("\n");
	}
	else {
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				printf("%8.2f ", data[c*rows + r]);
			}
			printf("\n");
		}
		printf("\n");
	}
	
}




