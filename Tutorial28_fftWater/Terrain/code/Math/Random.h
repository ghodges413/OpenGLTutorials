//
//  Random.h
//
#pragma once


/*
 ================================
 Random
 ================================
 */
class Random {
public:
                        ~Random();
	
    static  float       RangedNumber( const float & min, const float & max );
    static  float       GetMaxFloat();
    static  int         GetMaxInt();
    static  int         GetInt();

	// Generating gaussian random number with mean 0 and standard deviation 1.
	static	float		Gaussian( const float mean = 0.0f, const float stdDeviation = 1.0f, const float epsilon = 1e-6f );
    
private:
                        Random();
                        Random( const Random & rhs );
    const   Random &  operator = ( const Random & rhs );
};
