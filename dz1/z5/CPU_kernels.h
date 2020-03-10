#include "stdio.h"
#include "UDTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif
	void calculateLUT(float beta, float width, float **LUT, unsigned int *sizeLUT);

	int gridding_Gold(unsigned int n, parameters params, ReconstructionSample *sample, float *LUT, unsigned int sizeLUT, cmplx *gridData, float *sampleDensity);
	int gridding_Gold_Parallel(unsigned int n, parameters params, ReconstructionSample *sample, float *LUT, unsigned int sizeLUT, cmplx *gridData, float *sampleDensity);

#ifdef __cplusplus
}
#endif
