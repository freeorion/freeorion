#ifndef __LIN_QUADTREE_EXP_H_
#define __LIN_QUADTREE_EXP_H_

#include "LinearQuadtree.h"

namespace ogdf {

class LinearQuadtreeExpansion
{
public:
	//! constructor
	LinearQuadtreeExpansion(__uint32 precision, const LinearQuadtree& tree);
	
	//! destructor
	~LinearQuadtreeExpansion(void);

	//! adds a point with the given charge to the receiver expansion
	void P2M(__uint32 point, __uint32 receiver);

	//! shifts the source multipole coefficient to the center of the receiver and adds them 
	void M2M(__uint32 source, __uint32 receiver);

	//! converts the source multipole coefficient in to a local coefficients at the center of the receiver and adds them
	void M2L(__uint32 source, __uint32 receiver);

	//! shifts the source local coefficient to the center of the receiver and adds them
	void L2L(__uint32 source, __uint32 receiver);

	//! evaluates the derivate of the local expansion at the point and adds the forces to fx fy
	void L2P(__uint32 source, __uint32 point, float& fx, float& fy);

	//! returns the size in bytes
	__uint32 sizeInBytes() const { return m_numExp*m_numCoeff*sizeof(double)*4; };

	//! returns the array with multipole coefficients 
	inline double* multiExp() const { return m_multiExp; };

	//! returns the array with local coefficients 
	inline double* localExp() const { return m_localExp; };

	//! number of coefficients per expansions
	inline __uint32 numCoeff() const { return m_numCoeff; };

	//! the quadtree
	const LinearQuadtree& tree() { return m_tree; }; 
private:

	//! allocates the space for the coeffs
	void allocate();

	//! releases the memory for the coeffs
	void deallocate();

	//! the Quadtree reference
	const LinearQuadtree& m_tree;
public:	
	//! the big multipole expansione coeff array
	double* m_multiExp;

	//! the big local expansion coeff array
	double* m_localExp;

public:
	//! the number of multipole (locale) expansions
	__uint32 m_numExp;

	//! the number of coeff per expansions
	__uint32 m_numCoeff;

	BinCoeff<double> binCoef;
};


} // end of namespace ogdf

#endif
