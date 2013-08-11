/*
 *  pkmMatrix.h
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

#pragma once

//#define HAVE_OPENCV

#include <iostream>
#include <assert.h>
#include <Accelerate/Accelerate.h>
#include <vector>

#ifdef HAVE_OPENCV
#include <opencv2/opencv.hpp>
#endif

using namespace std;
#ifndef DEBUG
#define DEBUG 1
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

//typedef pkm::Mat pkmMatrix;

namespace pkm
{	
	// row-major floating point matrix
	class Mat
	{
		/////////////////////////////////////////
	public:
		// default constructor
		Mat();
		
		// destructor
		~Mat();
		
        Mat(vector<float> m);
        
        Mat(vector<vector<float> > m);
#ifdef HAVE_OPENCV
        Mat(cv::Mat m);
#endif
		// allocate data
		Mat(int r, int c, bool clear = false);
		
		// pass in existing data
		// non-destructive by default
		Mat(int r, int c, float *existing_buffer, bool withCopy);
		
		// set every element to a value
		Mat(int r, int c, float val);
		
		// copy-constructor, called during:
		//		pkm::Mat a(rhs);
		Mat(const Mat &rhs);
		Mat & operator=(const Mat &rhs);
		Mat & operator=(const vector<float> &rhs);
		Mat & operator=(const vector<vector<float> > &rhs);
#ifdef HAVE_OPENCV
        Mat & operator=(const cv::Mat &rhs);
        cv::Mat cvMat();
#endif
		
		inline Mat operator+(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows &&
				   cols == rhs.cols);
#endif			
			Mat newMat(rows, cols);
			vDSP_vadd(data, 1, rhs.data, 1, newMat.data, 1, rows*cols);
			//cblas_scopy(rows*cols, temp_data, 1, data, 1);
			return newMat;
		}
		
		
		inline Mat operator+(float rhs)
		{	
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat newMat(rows, cols);
			vDSP_vsadd(data, 1, &rhs, newMat.data, 1, rows*cols);
			return newMat;
		}
		
		inline Mat operator-(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows &&
				   cols == rhs.cols);
#endif			
			Mat newMat(rows, cols);
			vDSP_vsub(data, 1, rhs.data, 1, newMat.data, 1, rows*cols);
			//cblas_scopy(rows*cols, temp_data, 1, data, 1);
			return newMat;
		}
		

		
		inline Mat operator-(const float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat newMat(rows, cols);
			float rhs = -scalar;
			vDSP_vsadd(data, 1, &rhs, newMat.data, 1, rows*cols);
			return newMat;
		}
		
		
		inline Mat operator*(const pkm::Mat &rhs)
		{
#ifdef DEBUG
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(cols == rhs.rows);
#endif
			
			Mat gemmResult(rows, rhs.cols);
			//ldb must be >= MAX(N,1): ldb=30 N=3533Parameter 11 to routine cblas_sgemm was incorrect
			cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, gemmResult.rows, gemmResult.cols, cols, 1.0f, data, cols, rhs.data, rhs.cols, 0.0f, gemmResult.data, gemmResult.cols);
			//vDSP_mmul(data, 1, rhs.data, 1, gemmResult.data, 1, gemmResult.rows, gemmResult.cols, cols);
			return gemmResult;
		}
		

		
		inline Mat operator*(float scalar)
		{
#ifdef DEBUG
			assert(data != NULL);
#endif
			
			Mat gemmResult(rows, cols);
			vDSP_vsmul(data, 1, &scalar, gemmResult.data, 1, rows*cols);
			
			return gemmResult;
		}
		
		
		inline Mat operator/(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif			
			Mat result(rows, cols);
			vDSP_vdiv(rhs.data, 1, data, 1, result.data, 1, rows*cols);
			return result;
		}
		
		inline Mat operator/(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat result(rows, cols);
			vDSP_vsdiv(data, 1, &scalar, result.data, 1, rows*cols);
			return result;
			
		}
		
		inline Mat operator>(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] > rhs.data[i];
			return result;
		}
		
		inline Mat operator>(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] > scalar;
			return result;
		}
		
		inline Mat operator>=(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] >= rhs.data[i];
			return result;
		}
		
		inline Mat operator>=(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] >= scalar;
			return result;
		}
		
		inline Mat operator<(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] < rhs.data[i];
			return result;
		}
		
		inline Mat operator<(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] < scalar;
			return result;
		}
		
		inline Mat operator<=(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] <= rhs.data[i];
			return result;
		}
		
		inline Mat operator<=(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] <= scalar;
			return result;
		}
		
		inline Mat operator==(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] == rhs.data[i];
			return result;
		}
		
		inline Mat operator==(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] == scalar;
			return result;
		}
		
		inline Mat operator!=(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] != rhs.data[i];
			return result;
		}
		
		inline Mat operator!=(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			Mat result(rows, cols);
			for(int i = 0; i < rows*cols; i++)
				result.data[i] = data[i] != scalar;
			return result;
		}
		
		inline float & operator[](int idx)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rows*cols >= idx);
#endif	
			return data[idx];
		}
		
		// return a vector composed on non-zero indices of logicalMat
		inline Mat operator[](Mat rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif	
			std::vector<float> newMat;
			for(int i = 0; i < rows*cols; i++)
			{
				if (rhs.data[i] > 0) {
					newMat.push_back(data[i]);
				}
			}
			if (newMat.size() > 0) {
				Mat result(1,newMat.size());
				for(int i = 0; i < newMat.size(); i++)
				{
					result.data[i] = newMat[i];
				}
				return result;
			}
			else {
				Mat empty;
				return empty;
			}
		}
		

		
		friend Mat operator-(float lhs, const Mat &rhs)
		{
#ifdef DEBUG			
			assert(rhs.data != NULL);
#endif			
			Mat newMat(rhs.rows, rhs.cols);
			float scalar = -lhs;
			vDSP_vsadd(rhs.data, 1, &scalar, newMat.data, 1, rhs.rows*rhs.cols);
			return newMat;
		}
		
		friend Mat operator*(float lhs, const Mat &rhs)
		{
#ifdef DEBUG
			assert(rhs.data != NULL);
#endif
			
			Mat gemmResult(rhs.rows, rhs.cols);
			vDSP_vsmul(rhs.data, 1, &lhs, gemmResult.data, 1, rhs.rows*rhs.cols);
			
			return gemmResult;
		}
		friend Mat operator+(float lhs, const Mat &rhs)
		{
#ifdef DEBUG			
			assert(rhs.data != NULL);
#endif			
			Mat newMat(rhs.rows, rhs.cols);
			vDSP_vsadd(rhs.data, 1, &lhs, newMat.data, 1, rhs.rows*rhs.cols);
			//cblas_scopy(rows*cols, temp_data, 1, data, 1);
			return newMat;
		}
        
        bool isNaN()
        {
            for(int i = 0; i < rows*cols; i++)
            {
                if (isnan(data[i])) {
                    return true;
                }
            }
            return false;
        }
		
        void resize(int r, int c, bool clear = false)
        {
            cblas_scopy(rows*cols, data, 1, temp_data, 1);
            
            if (r >= rows && c >= cols) {
                if (bUserData) {
                    data = (float *)malloc(r * c * sizeof(float));
                    temp_data = (float *)realloc(temp_data, r * c * sizeof(float));
                }
                else
                {
                    data = (float *)realloc(data, r * c * sizeof(float));
                    cblas_scopy(rows*cols, temp_data, 1, data, 1);
                    temp_data = (float *)realloc(temp_data, r * c * sizeof(float));
                }
                
                if(clear)
                {
                    vDSP_vclr(data + rows*cols, 1, r*c - rows*cols);
                }
                
                rows = r;
                cols = c;
                
                bAllocated = true;
                bUserData = false;
            }
            else {
                printf("[ERROR: pkmMatrix::resize()] Cannot resize to a smaller matrix (yet).\n");
                return;
            }

            
        }
		
		// can be used to create an already declared matrix without a copy constructor
		void reset(int r, int c, bool clear = false)
		{
			rows = r;
			cols = c;
			current_row = 0;
			bCircularInsertionFull = false;
			
            releaseMemory();
            
            data = (float *)malloc(rows * cols * sizeof(float));
            // sacrifice memory w/ speed, by pre-allocating a temporary buffer
            temp_data = (float *)malloc(rows * cols * sizeof(float));
        
			bAllocated = true;
			bUserData = false;
			
			// set every element to 0
			if(clear)
			{
				vDSP_vclr(data, 1, rows*cols);
			}
		}
		
		// set every element to a value
		inline void setTo(float val)
		{
#ifdef DEBUG
			assert(data != NULL);
#endif	
			vDSP_vfill(&val, data, 1, rows * cols);
		}	
		
		inline void clear()
		{
			if (rows == 0 || cols == 0) {
				return;
			}

			vDSP_vclr(data, 1, rows * cols);
		}
		
		/////////////////////////////////////////
		
		inline float * row(int r)
		{
#ifdef DEBUG
			assert(data != NULL);
#endif			
			return (data + r*cols);
		}
		
		inline void insertRow(float *buf, int row_idx)
		{
			float * rowData = row(row_idx);
			cblas_scopy(cols, buf, 1, rowData, 1);
		}
		
        void push_back(Mat m)
        {
            if (bAllocated && (rows > 0) && (cols > 0)) {
                if (m.cols != cols) {
                    printf("[ERROR]: pkm::Mat push_back(Mat m) requires same number of columns!\n");
                    return;
                }
                if (m.bAllocated && (m.cols > 0) && (m.rows > 0)) {
                    cblas_scopy(rows*cols, data, 1, temp_data, 1);
                    assert(data != 0);
                    free(data); data = NULL;
                    data = (float *)malloc(sizeof(float) * (rows + m.rows) * cols);
                    //data = (float *)realloc(data, (rows+m.rows)*cols*sizeof(float));
                    cblas_scopy(rows*cols, temp_data, 1, data, 1);
                    cblas_scopy(m.rows*cols, m.data, 1, data + (rows*cols), 1);
                    rows+=m.rows;
                    assert(temp_data != 0);
                    free(temp_data); temp_data = NULL;
                    temp_data = (float *)malloc(sizeof(float) * rows * cols);
                    //temp_data = (float *)realloc(temp_data, rows*cols*sizeof(float));
                }
                else {
                    printf("[ERROR]: pkm::Mat push_back(Mat m), matrix m is empty!\n");
                    return;
                }
            }
            else {
                *this = m;
            }

        }
        
        void push_back(float *m, int size)
        {
            if(size > 0)
            {
                if (bAllocated && (rows > 0) && (cols > 0)) {
                    if (size != cols) {
                        printf("[ERROR]: pkm::Mat push_back(float *m) requires same number of columns in Mat as length of vector!\n");
                        return;
                    }
                    cblas_scopy(rows*cols, data, 1, temp_data, 1);
                    assert(data != 0);
                    free(data);
                    data = NULL;
                    data = (float *)malloc((rows+1)*cols*sizeof(float));
                    cblas_scopy(rows*cols, temp_data, 1, data, 1);
                    cblas_scopy(cols, m, 1, data + (rows*cols), 1);
                    rows++;
                    assert(temp_data != 0);
                    free(temp_data);
                    temp_data = NULL;
                    temp_data = (float *)malloc(rows*cols*sizeof(float));
                }
                else {
                    cols = size;
                    data = (float *)malloc(sizeof(float) * cols);
                    cblas_scopy(cols, m, 1, data, 1);
                    temp_data = (float *)malloc(sizeof(float) * cols);
                    rows = 1;
                    bAllocated = true;
                }
            }
        }
        
        inline void push_back(vector<float> &m)
        {
            if (bAllocated && rows > 0 && cols > 0) {
                if (m.size() != cols) {
                    printf("[ERROR]: pkm::Mat push_back(vector<float> m) requires same number of columns in Mat as length of vector!\n");
                    return;
                }
                cblas_scopy(rows*cols, data, 1, temp_data, 1);
                assert(data != 0);
                free(data); data = NULL;
                data = (float *)malloc((rows+1)*cols*sizeof(float));
                cblas_scopy(rows*cols, temp_data, 1, data, 1);
                cblas_scopy(cols, &(m[0]), 1, data + (rows*cols), 1);
                rows++;
                assert(temp_data != 0);
                free(temp_data); temp_data = NULL;
                temp_data = (float *)malloc(rows*cols*sizeof(float));
            }
            else {
                *this = m;
            }
            
        }
        
        inline void push_back(vector<vector<float> > &m)
        {
            if (rows > 0 && cols > 0) {
                if (m[0].size() != cols) {
                    printf("[ERROR]: pkm::Mat push_back(vector<vector<float> > m) requires same number of cols in Mat as length of each vector!\n");
                    return;
                }
                cblas_scopy(rows*cols, data, 1, temp_data, 1);
                assert(data != 0);
                free(data); data = NULL;
                data = (float *)malloc((rows+m.size())*cols*sizeof(float));
                cblas_scopy(rows*cols, temp_data, 1, data, 1);
                for (int i = 0; i < m.size(); i++) {
                    cblas_scopy(cols, &(m[i][0]), 1, data + ((rows+i)*cols), 1);
                }
                rows+=m.size();
                assert(temp_data != 0);
                free(temp_data); temp_data = NULL;
                temp_data = (float *)malloc(rows*cols*sizeof(float));
            }
            else {
                *this = m;
            }
            
        }
        
		inline void insertRowCircularly(float *buf)
		{
			insertRow(buf, current_row);
			current_row = (current_row + 1) % rows;
			if (current_row == 0) {
				bCircularInsertionFull = true;
			}
		}
		
		// inclusive of start, exclusive of end
		// can be a copy of the original matrix, or a way of editing the original
		// one by not copying the values (default)
		inline Mat rowRange(int start, int end, bool withCopy = true)
		{
#ifdef DEBUG
			assert(rows >= end);
#endif
			Mat submat(end-start, cols, row(start), withCopy);
			return submat;
		}
		
		
		inline Mat colRange(int start, int end, bool withCopy = true)
		{
#ifdef DEBUG
			assert(cols >= end);
#endif
			setTranspose();
			Mat submat(end-start, cols, row(start), withCopy);
			setTranspose();
			submat.setTranspose();
			return submat;
		}
		
		// copy data into the matrix
		void copy(Mat rhs)
		{
#ifdef DEBUG
			assert(rhs.rows == rows);
			assert(rhs.cols == cols);
#endif
			cblas_scopy(rows*cols, rhs.data, 1, data, 1);
			
		}
		
		void copy(Mat &rhs, Mat &indx)
		{
#ifdef DEBUG
			assert(indx.rows == rows);
			assert(indx.cols == cols);
#endif
			int idx = 0;
			for(int i = 0; i < rows; i++)
			{
				for(int j = 0; j < cols; j++)
				{
					if (indx.data[i*cols + j]) {
						data[i*cols + j] = rhs[idx];
						idx++;
					}
				}
				
			}
		}
		
		/////////////////////////////////////////
		
		// element-wise multiplication
		inline void multiply(const Mat &rhs, Mat &result) const 
		{
#ifdef DEBUG
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(result.data != NULL);
			assert(rows == rhs.rows && 
				   rhs.rows == result.rows &&
				   cols == rhs.cols && 
				   rhs.cols == result.cols);
#endif
			vDSP_vmul(data, 1, rhs.data, 1, result.data, 1, rows*cols);
			
		}		
		// element-wise multiplication
		// result stored in newly created matrix
		inline Mat multiply(const Mat &rhs)
		{
#ifdef DEBUG
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows && 
				   cols == rhs.cols);
#endif			
			Mat multiplied_matrix(rows, cols);
			
			vDSP_vmul(data, 1, rhs.data, 1, multiplied_matrix.data, 1, rows*cols);
			return multiplied_matrix;
		}		
		
		inline void multiply(float scalar, Mat &result) const 
		{
#ifdef DEBUG
			assert(data != NULL);
			assert(result.data != NULL);
			assert(rows == result.rows &&
				   cols == result.cols);
#endif			
			vDSP_vsmul(data, 1, &scalar, result.data, 1, rows*cols);
			
		}
		
		inline void multiply(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif
			vDSP_vsmul(data, 1, &scalar, data, 1, rows*cols);
		}
		

		
		
		inline void divide(const Mat &rhs, Mat &result) const 
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(result.data != NULL);
			assert(rows == rhs.rows && 
				   rhs.rows == result.rows &&
				   cols == rhs.cols && 
				   rhs.cols == result.cols);
#endif			
			vDSP_vdiv(rhs.data, 1, data, 1, result.data, 1, rows*cols);
			
		}
		
		inline void divide(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows &&
				   cols == rhs.cols);
#endif			
			vDSP_vdiv(rhs.data, 1, data, 1, temp_data, 1, rows*cols);
			//cblas_scopy(rows*cols, temp_data, 1, data, 1);
			std::swap(data, temp_data);
		}
		
		inline void divide(float scalar, Mat &result) const 
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(result.data != NULL);
			assert(rows == result.rows &&
				   cols == result.cols);
#endif	
			
			vDSP_vsdiv(data, 1, &scalar, result.data, 1, rows*cols);
		}
		
		inline void divide(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif
			vDSP_vsdiv(data, 1, &scalar, data, 1, rows*cols);
		}
		
		inline void add(const Mat &rhs, Mat &result) const 
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(result.data != NULL);
			assert(rows == rhs.rows && 
				   rhs.rows == result.rows &&
				   cols == rhs.cols && 
				   rhs.cols == result.cols);
#endif			
			vDSP_vadd(data, 1, rhs.data, 1, result.data, 1, rows*cols);
		}
		
		inline void add(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows &&
				   cols == rhs.cols);
#endif			
			vDSP_vadd(data, 1, rhs.data, 1, temp_data, 1, rows*cols);
			//cblas_scopy(rows*cols, temp_data, 1, data, 1);
			std::swap(data, temp_data);
		}
		
		inline void add(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			vDSP_vsadd(data, 1, &scalar, data, 1, rows*cols);
		}
		
		inline void subtract(const Mat &rhs, Mat &result) const 
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(result.data != NULL);
			assert(rows == rhs.rows && 
				   rhs.rows == result.rows &&
				   cols == rhs.cols && 
				   rhs.cols == result.cols);
#endif			
			vDSP_vsub(data, 1, rhs.data, 1, data, 1, rows*cols);
			
		}
		
		inline void subtract(const Mat &rhs)
		{
#ifdef DEBUG			
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(rows == rhs.rows &&
				   cols == rhs.cols);
#endif			
			vDSP_vsub(data, 1, rhs.data, 1, temp_data, 1, rows*cols);
			//cblas_scopy(rows*cols, temp_data, 1, data, 1);
			std::swap(data, temp_data);
		}
		
		inline void subtract(float scalar)
		{
#ifdef DEBUG			
			assert(data != NULL);
#endif			
			float rhs = -scalar;
			vDSP_vsadd(data, 1, &rhs, data, 1, rows*cols);
		}
			
		
		inline void GEMM(const Mat &rhs, Mat &result) const 
		{
#ifdef DEBUG
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(result.data != NULL);
			assert(rows == result.rows &&
				   rhs.cols == result.cols &&
				   cols == rhs.rows);
#endif
			
			cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, result.rows, result.cols, cols, 1.0f, data, rows, rhs.data, rhs.rows, 0.0f, result.data, result.cols);
			//vDSP_mmul(data, 1, rhs.data, 1, result.data, 1, result.rows, result.cols, cols);
			
		}
		
		inline Mat GEMM(const pkm::Mat &rhs)
		{
#ifdef DEBUG
			assert(data != NULL);
			assert(rhs.data != NULL);
			assert(cols == rhs.rows);
#endif
			
			Mat gemmResult(rows, rhs.cols);
			
			printf("lda: %d\nldb: %d\nldc: %d\n", rows, rhs.rows, gemmResult.rows); 
			cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, gemmResult.rows, gemmResult.cols, cols, 1.0f, data, cols, rhs.data, rhs.cols, 0.0f, gemmResult.data, gemmResult.cols);
			//vDSP_mmul(data, 1, rhs.data, 1, gemmResult.data, 1, gemmResult.rows, gemmResult.cols, cols);
			return gemmResult;
			
		}
				
		inline void setTranspose()
		{
#ifdef DEBUG      
			assert(data != NULL);
#endif      
			vDSP_mtrans(data, 1, temp_data, 1, cols, rows);
			cblas_scopy(rows*cols, temp_data, 1, data, 1);
                //std::swap(data, temp_data);					// swap will break certain operations for col/row range as their pointers will have changed. :(
                //std::swap(rows, cols);
            int tempvar = cols;
            cols = rows;
            rows = tempvar;
		}
		
		Mat getTranspose();
		
		
		// diagonalize the vector into a square matrix with 
		// the current data vector along the diagonal
		inline void setDiag()
		{
#ifdef DEBUG
			assert(data != NULL);
#endif	
			if((rows == 1 && cols > 1) || (cols == 1 && rows > 1))
			{
				int diagonal_elements = MAX(rows,cols);
				
				// create a square matrix
				temp_data = (float *)realloc(temp_data, diagonal_elements*diagonal_elements*sizeof(float));
				
				// set values to 0
				vDSP_vclr(temp_data, 1, diagonal_elements*diagonal_elements);
				
				// set diagonal elements to the current vector in data
				for (int i = 0; i < diagonal_elements; i++) {
					temp_data[i*diagonal_elements+i] = data[i];
				}
				
				// store in data
				rows = cols = diagonal_elements;
				//free(data);
				//data = (float *)malloc(rows*cols*sizeof(float));
				//cblas_scopy(rows*cols, temp_data, 1, data, 1);
				std::swap(data, temp_data);
				
				// reallocate temp data for future processing
				temp_data = (float *)realloc(temp_data, diagonal_elements*diagonal_elements*sizeof(float));
				
				// save dimensions
			}
		}
		Mat getDiag();
		
        // returns a new matrix with each el the abs(el)
        static Mat abs(Mat &A);
		
        // returns a new matrix with each el the log(el)
		static Mat log(Mat &A);
		
		// returns a new matrix with each el the exp(el)
		static Mat exp(Mat &A);
		
		// returns a new diagonalized matrix version of A
		static Mat diag(Mat &A);
		
		// get a new identity matrix of size dim x dim
		static Mat identity(int dim);
		
		static Mat zeros(int rows, int cols)
		{
			return Mat(rows, cols, true);
		}
		
		// set every element to a random value between low and high
		void setRand(float low = 0.0, float high = 1.0);
		
		// create a random matrix
		static Mat rand(int r, int c, float low = 0.0, float high = 1.0);
		
		// sum across rows or columns creating a vector from a matrix, or a scalar from a vector
		Mat sum(bool across_rows = true);
		
		// repeat a vector for size times
		static Mat repeat(Mat &m, int size)
		{
			// repeat a column vector across cols
			if(m.rows > 1 && m.cols == 1 && size > 1)
			{
				Mat repeated_matrix(size, m.rows);
				for (int i = 0; i < size; i++) {
					cblas_scopy(m.rows, m.data, 1, repeated_matrix.data + (i*m.rows), 1);
				}
				repeated_matrix.setTranspose();
				return repeated_matrix;
			}
			else if( m.rows == 1 && m.cols > 1 && size > 1)
			{
				Mat repeated_matrix(size, m.cols, 5.0f);
				
				for (int i = 0; i < size; i++) {
					cblas_scopy(m.cols, m.data, 1, repeated_matrix.data + (i*m.cols), 1);
				}
				return repeated_matrix;
			}
			else {
				printf("[ERROR]: repeat requires a vector and a size to repeat on.");
				Mat a;
				return a;
			}

		}
		
		// repeat a vector for size times
		static void repeat(Mat &dst, const Mat &m, int size)
		{
			// repeat a column vector across cols
			if(m.rows > 1 && m.cols == 1 && size > 1)
			{
				dst.reset(size, m.rows);
				for (int i = 0; i < size; i++) {
					cblas_scopy(m.rows, m.data, 1, dst.data + (i*m.rows), 1);
				}
				dst.setTranspose();
			}
			else if( m.rows == 1 && m.cols > 1 && size > 1)
			{
				dst.reset(size, m.cols);
				
				for (int i = 0; i < size; i++) {
					cblas_scopy(m.cols, m.data, 1, dst.data + (i*m.cols), 1);
				}
			}
			else {
				printf("[ERROR]: repeat requires a vector and a size to repeat on.");
				
			}
			
		}
		
		static float meanMagnitude(float *buf, int size)
		{
			float mean;
			vDSP_meamgv(buf, 1, &mean, size);
			return mean;
		}
		
		static float sumOfAbsoluteDifferences(float *buf1, float *buf2, int size)
		{
			int a = size;
			float diff = 0;
			float *p1 = buf1, *p2 = buf2;
			while (a) {
				diff += fabs(*p1++ - *p2++);
				a--;
			}
			return diff/(float)size;
		}
		
		static float mean(float *buf, int size, int stride = 1)
		{
			float val;
			vDSP_meanv(buf, stride, &val, size);
			return val;
		}
		
		static float var(float *buf, int size, int stride = 1)
		{
			float m = mean(buf, size, stride);
			float v = 0;
			float sqr = 0;
			float *p = buf;
			int a = size;
			while (a) {
				sqr = (*p - m);
				p += stride;
				v += sqr*sqr;
				a--;
			}
			return v/(float)size;
		}
		
		static float rms(float *buf, int size)
		{
			float val;
			vDSP_rmsqv(buf, 1, &val, size);
			return val;
		}
		
		static float min(Mat &A)
		{
			float minval;
			vDSP_minv(A.data, 1, &minval, A.rows*A.cols);
			return minval;
		}
        
        static unsigned long minIndex(Mat &A)
        {
            float maxval;
            unsigned long maxidx;
            vDSP_minvi(A.data, 1, &maxval, &maxidx, A.rows*A.cols);
            return maxidx;
        }
		
		static float max(Mat &A)
		{
			float maxval;
			vDSP_maxv(A.data, 1, &maxval, A.rows*A.cols);
			return maxval;
		}
        
        static unsigned long maxIndex(Mat &A)
        {
            float maxval;
            unsigned long maxidx;
            vDSP_maxvi(A.data, 1, &maxval, &maxidx, A.rows*A.cols);
            return maxidx;
        }
		
		static float sum(Mat &A)
		{
			float sumval;
			vDSP_sve(A.data, 1, &sumval, A.rows*A.cols);
			return sumval;
		}
		
		Mat var(bool row_major = true)
		{
#ifdef DEBUG			
            assert(data != NULL);
            assert(rows >0 &&
                   cols >0);
#endif	
            if (row_major) {
                if (rows == 1) {
                    return *this;
                }
                Mat newMat(1, cols);
                
                for(int i = 0; i < cols; i++)
                {
                    newMat.data[i] = var(data + i, rows, cols);
                }
                return newMat;
            }
            else {
                if (cols == 1) {
                    return *this;
                }
                Mat newMat(rows, 1);
                for(int i = 0; i < rows; i++)
                {
                    newMat.data[i] = var(data + i*cols, cols, 1);
                }
                return newMat;
            }	
		}
		
        
        Mat mean(bool row_major = true)
        {
#ifdef DEBUG			
            assert(data != NULL);
            assert(rows >0 &&
                   cols >0);
#endif	
            if (row_major) {
                if (rows == 1) {
                    return *this;
                }
                Mat newMat(1, cols);
                
                for(int i = 0; i < cols; i++)
                {
                    newMat.data[i] = mean(data + i, rows, cols);
                }
                return newMat;
            }
            else {
                if (cols == 1) {
                    return *this;
                }
                Mat newMat(rows, 1);
                for(int i = 0; i < rows; i++)
                {
                    newMat.data[i] = mean(data + i*cols, cols, 1);
                }
                return newMat;
            }

        }
		
		// rescale the values in each row to their maximum
		void setNormalize(bool row_major = true);
		
		void normalizeRow(int r)
		{
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
		
		void divideEachVecByMaxVecElement(bool row_major);
		void divideEachVecBySum(bool row_major);
        
        
        bool save(string filename)
        {
            FILE *fp;
            fp = fopen(filename.c_str(), "w");
            fprintf(fp, "%d %d\n", rows, cols);
            for(int i = 0; i < rows; i++)
            {
                for(int j = 0; j < cols; j++)
                {
                    fprintf(fp, "%f, ", data[i*cols + j]);
                }
                fprintf(fp,"\n");
            }
            fclose(fp);
            return true;
        }
        
        bool load(string filename)
        {
            if (bAllocated) {
                free(data); data = NULL;
                free(temp_data); temp_data = NULL;
                rows = cols = 0;
            }
            FILE *fp;
            fp = fopen(filename.c_str(), "r");
            fscanf(fp, "%d %d\n", &rows, &cols);
            data = (float *)malloc(sizeof(float) * rows * cols);
            temp_data = (float *)malloc(sizeof(float) * rows * cols);
            for(int i = 0; i < rows; i++)
            {
                for(int j = 0; j < cols; j++)
                {
                    fscanf(fp, "%f, ", &(data[i*cols + j]));
                }
                fscanf(fp, "\n");
            }
            fclose(fp);
            return true;
        }
		
		// simple print output (be careful with large matrices!)
		void print(bool row_major = true);
		// only prints maximum of 5 rows/cols
		void printAbbrev(bool row_major = true);
		
		/////////////////////////////////////////
		
		int current_row;	// for circular insertion
		bool bCircularInsertionFull;
		int rows;
		int cols;
		
		float *data;
		float *temp_data;
		
		bool bAllocated;
		bool bUserData;
        
    protected:
        void releaseMemory()
        {
            if(bAllocated)
            {
                if (!bUserData) {
                    assert(data != 0);
                    free(data);
                    data = NULL;
                }
                assert(temp_data != 0);
                free(temp_data);
                temp_data = NULL;
            }
        }
        
	};
    
	
};