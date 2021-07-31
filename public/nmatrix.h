//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef NMATRIX_H
#define NMATRIX_H
#ifdef _WIN32
#pragma once
#endif


#include <assert.h>
#include "nvector.h"


#define NMatrixMN NMatrix<M,N>


template<int M, int N>
class NMatrix
{
public:

                        NMatrixMN() {}

    static NMatrixMN    SetupNMatrixNull();                 // Return a matrix of all zeros.
    static NMatrixMN    SetupNMatrixIdentity();             // Return an identity matrix.

    NMatrixMN const&    operator=( NMatrixMN const &other );

    NMatrixMN           operator+( NMatrixMN const &v ) const;
    NMatrixMN const&    operator+=( NMatrixMN const &v );

    NMatrixMN           operator-() const;
    NMatrixMN           operator-( NMatrixMN const &v ) const;

    // Multiplies the column vector on the right-hand side.
    NVector<M>          operator*( NVector<N> const &v ) const;

    // Can't get the compiler to work with a real MxN * NxR matrix multiply...
    NMatrix<M,M>        operator*( NMatrix<N,M> const &b ) const;

    NMatrixMN           operator*( float val ) const;

    bool                InverseGeneral( NMatrixMN &mInverse ) const;
    NMatrix<N,M>        Transpose() const;


public:

    float               m[M][N];
};



// Return the matrix generated by multiplying a column vector 'a' by row vector 'b'.
template<int N>
inline NMatrix<N,N> OuterProduct( NVectorN const &a, NVectorN const &b )
{
    NMatrix<N,N> ret;

    for( int i=0; i < N; i++ )
        for( int j=0; j < N; j++ )
            ret.m[i][j] = a.v[i] * b.v[j];

    return ret;
}


// -------------------------------------------------------------------------------- //
// NMatrix inlines.
// -------------------------------------------------------------------------------- //

template<int M, int N>
inline NMatrixMN NMatrixMN::SetupNMatrixNull()
{
    NMatrix ret;
    memset( ret.m, 0, sizeof(float)*M*N );
    return ret;
}


template<int M, int N>
inline NMatrixMN NMatrixMN::SetupNMatrixIdentity()
{
    assert( M == N );   // Identity matrices must be square.

    NMatrix ret;
    memset( ret.m, 0, sizeof(float)*M*N );
    for( int i=0; i < N; i++ )
        ret.m[i][i] = 1;
    return ret;
}


template<int M, int N>
inline NMatrixMN const &NMatrixMN::operator=( NMatrixMN const &v )
{
    memcpy( m, v.m, sizeof(float)*M*N );
    return *this;
}


template<int M, int N>
inline NMatrixMN NMatrixMN::operator+( NMatrixMN const &v ) const
{
    NMatrixMN ret;
    for( int i=0; i < M; i++ )
        for( int j=0; j < N; j++ )
            ret.m[i][j] = m[i][j] + v.m[i][j];

    return ret;
}


template<int M, int N>
inline NMatrixMN const &NMatrixMN::operator+=( NMatrixMN const &v )
{
    for( int i=0; i < M; i++ )
        for( int j=0; j < N; j++ )
            m[i][j] += v.m[i][j];

    return *this;
}


template<int M, int N>
inline NMatrixMN NMatrixMN::operator-() const
{
    NMatrixMN ret;

    for( int i=0; i < M*N; i++ )
        ((float*)ret.m)[i] = -((float*)m)[i];

    return ret;
}


template<int M, int N>
inline NMatrixMN NMatrixMN::operator-( NMatrixMN const &v ) const
{
    NMatrixMN ret;
    for( int i=0; i < M; i++ )
        for( int j=0; j < N; j++ )
            ret.m[i][j] = m[i][j] - v.m[i][j];
    return ret;
}


template<int M, int N>
inline NVector<M> NMatrixMN::operator*( NVectorN const &v ) const
{
    NVectorN ret;

    for( int i=0; i < M; i++ )
    {
        ret.v[i] = 0;

        for( int j=0; j < N; j++ )
            ret.v[i] += m[i][j] * v.v[j];
    }

    return ret;
}


template<int M, int N>
inline NMatrix<M,M> NMatrixMN::operator*( NMatrix<N,M> const &b ) const
{
    NMatrix<M,M> ret;

    for( int myRow=0; myRow < M; myRow++ )
    {
        for( int otherCol=0; otherCol < M; otherCol++ )
        {
            ret[myRow][otherCol] = 0;
            for( int i=0; i < N; i++ )
                ret[myRow][otherCol] += a.m[myRow][i] * b.m[i][otherCol];
        }
    }

    return ret;
}


template<int M, int N>
inline NMatrixMN NMatrixMN::operator*( float val ) const
{
    NMatrixMN ret;

    for( int i=0; i < N*M; i++ )
        ((float*)ret.m)[i] = ((float*)m)[i] * val;

    return ret;
}


template<int M, int N>
bool NMatrixMN::InverseGeneral( NMatrixMN &mInverse ) const
{
    int iRow, i, j, iTemp, iTest;
    float mul, fTest, fLargest;
    float mat[N][2*N];
    int rowMap[N], iLargest;
    float *pOut, *pRow, *pScaleRow;


    // Can only invert square matrices.
    if( M != N )
    {
        assert( !"Tried to invert a non-square matrix" );
        return false;
    }


    // How it's done.
    // AX = I
    // A = this
    // X = the matrix we're looking for
    // I = identity

    // Setup AI
    for(i=0; i < N; i++)
    {
        const float *pIn = m[i];
        pOut = mat[i];

        for(j=0; j < N; j++)
        {
            pOut[j] = pIn[j];
        }

        for(j=N; j < 2*N; j++)
            pOut[j] = 0;

        pOut[i+N] = 1.0f;

        rowMap[i] = i;
    }

    // Use row operations to get to reduced row-echelon form using these rules:
    // 1. Multiply or divide a row by a nonzero number.
    // 2. Add a multiple of one row to another.
    // 3. Interchange two rows.

    for(iRow=0; iRow < N; iRow++)
    {
        // Find the row with the largest element in this column.
        fLargest = 0.001f;
        iLargest = -1;
        for(iTest=iRow; iTest < N; iTest++)
        {
            fTest = (float)fabs(mat[rowMap[iTest]][iRow]);
            if(fTest > fLargest)
            {
                iLargest = iTest;
                fLargest = fTest;
            }
        }

        // They're all too small.. sorry.
        if(iLargest == -1)
        {
            return false;
        }

        // Swap the rows.
        iTemp = rowMap[iLargest];
        rowMap[iLargest] = rowMap[iRow];
        rowMap[iRow] = iTemp;

        pRow = mat[rowMap[iRow]];

        // Divide this row by the element.
        mul = 1.0f / pRow[iRow];
        for(j=0; j < 2*N; j++)
            pRow[j] *= mul;

        pRow[iRow] = 1.0f; // Preserve accuracy...

        // Eliminate this element from the other rows using operation 2.
        for(i=0; i < N; i++)
        {
            if(i == iRow)
                continue;

            pScaleRow = mat[rowMap[i]];

            // Multiply this row by -(iRow*the element).
            mul = -pScaleRow[iRow];
            for(j=0; j < 2*N; j++)
            {
                pScaleRow[j] += pRow[j] * mul;
            }

            pScaleRow[iRow] = 0.0f; // Preserve accuracy...
        }
    }

    // The inverse is on the right side of AX now (the identity is on the left).
    for(i=0; i < N; i++)
    {
        const float *pIn = mat[rowMap[i]] + N;
        pOut = mInverse.m[i];

        for(j=0; j < N; j++)
        {
            pOut[j] = pIn[j];
        }
    }

    return true;
}


template<int M, int N>
inline NMatrix<N,M> NMatrixMN::Transpose() const
{
    NMatrix<N,M> ret;

    for( int i=0; i < M; i++ )
        for( int j=0; j < N; j++ )
            ret.m[j][i] = m[i][j];

    return ret;
}

#endif // NMATRIX_H
