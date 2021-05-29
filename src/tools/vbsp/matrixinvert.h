#ifndef MATRIXINVERT_H_
#define MATRIXINVERT_H_ 1
#include "mathlib/mathlib.h"

// Although mathlib has MatrixInvert, it doesn't actually do
// all the math behind the inversion (for some reason...).
// So here's modified code from VDC originally made by Jason Yu-Tseh Chi

static void MI_Minor( float **pSrc, float **pDest, int iRow, int iCol, int iOrder )
{
    int iColCount = 0, iRowCount = 0;
    for( int i = 0; i < iOrder; i++ ) {
        if( i != iRow ) {
            iColCount = 0;
            for( int j = 0; j < iOrder; j++ ) {
                if( j != iCol ) {
                    pDest[iRowCount][iColCount] = pSrc[i][j];
                    iColCount++;
                }
            }
            iRowCount++;
        }
    }
}

static float MI_Determinant( float **pMatrix, int iOrder )
{
    if( iOrder == 1 )
        return pMatrix[0][0];

    float flDet = 0;
    float **pMinor;

    pMinor = new float *[iOrder - 1];
    for( int i = 0; i < iOrder - 1; i++ )
        pMinor[i] = new float[iOrder - 1];

    for( int i = 0; i < iOrder; i++ ) {
        MI_Minor( pMatrix, pMinor, 0, i, iOrder );
        flDet += ( i % 2 == 1 ? -1.0f : 1.0f ) * pMatrix[0][i] * MI_Determinant( pMinor, iOrder - 1 );
    }

    for( int i = 0; i < iOrder - 1; i++ )
        delete[] pMinor[i];
    delete[] pMinor;

    return flDet;
}

static void MI_Invert( matrix3x4_t &in, matrix3x4_t &out )
{
    const int iOrder = 4;
    float **pMatrix, *pTemp, **pMinor;

    pMatrix = new float *[iOrder];
    for( int i = 0; i < iOrder; i++ ) {
        pMatrix[i] = new float[iOrder];
        if( i < 3 ) {
            for( int j = 0; j < iOrder; j++ ) {
                pMatrix[i][j] = in[i][j];
            }
        }
    }
    
    pTemp = new float[( iOrder - 1 ) * ( iOrder - 1 )];
    pMinor = new float *[iOrder - 1];
    for( int i = 0; i < iOrder - 1; i++ )
        pMinor[i] = pTemp + i * ( iOrder - 1 );

    pMatrix[3][0] = pMatrix[3][1] = pMatrix[3][2] = 0;
    pMatrix[3][3] = 1;

    const float flDet = 1.0 / MI_Determinant( pMatrix, iOrder );

    for( int j = 0; j < iOrder; j++ ) {
        for( int i = 0; i < iOrder; i++ ) {
            MI_Minor( pMatrix, pMinor, j, i, iOrder );
            out[i][j] = ( ( i + j ) % 2 == 1 ? -1.0f : 1.0f ) * flDet * MI_Determinant( pMinor, iOrder - 1 );
        }
    }

    delete[] pMinor;
    delete[] pTemp;

    for( int i = 0; i < 4; i++ )
        delete pMatrix[i];
    delete[] pMatrix;
}

#endif
