
#include <cuda.h>
#include <cuda_runtime.h>

__constant__ float q = 1.60217646e-19;
__constant__ float m = 9.10938188e-31;
__constant__ float B0    = 1e-12;
__constant__ float alpha = 250000;

__global__ void kernel(float* x, float* y, float* z,
	float* vx, float* vy, float* vz, int count, float tau)
{
	int i = blockDim.x * blockIdx.x + threadIdx.x;
	if(i < count)
	{
		float r = sqrtf(x[i]*x[i]+y[i]*y[i]+z[i]*z[i]);

		float Bx = 0;
		float By = 0;
		float Bz = -B0 * expf(-r*r / alpha);

		float vx1 = vx[i]; float vy1 = vy[i]; float vz1 = vz[i];
		vx[i] = vx1 + tau * q * (vy1 * Bz - vz1 * By) / m;
		vy[i] = vy1 + tau * q * (vz1 * Bx - vx1 * Bz) / m;
		vz[i] = vz1 + tau * q * (vx1 * By - vy1 * Bx) / m;

		x[i] += vx[i] * tau;
		y[i] += vy[i] * tau;
		z[i] += vz[i] * tau;
	}
}

static float *d_x = NULL, *d_y = NULL, *d_z = NULL,
			 *d_vx = NULL, *d_vy = NULL, *d_vz = NULL;
static size_t oldcount = 0;

__host__ void process_particles(float* x, float* y, float* z,
	float* vx, float* vy, float*vz, size_t count, float tau)
{
	int size = count * sizeof(float);
	if(!d_x || oldcount != count)
	{
		cudaFree(d_x);
		cudaMalloc(&d_x, size);
		cudaFree(d_y);
		cudaMalloc(&d_y, size);
		cudaFree(d_z);
		cudaMalloc(&d_z, size);
		cudaFree(d_vx);
		cudaMalloc(&d_vx, size);
		cudaFree(d_vy);
		cudaMalloc(&d_vy, size);
		cudaFree(d_vz);
		cudaMalloc(&d_vz, size);
		oldcount = count;
	}
	cudaMemcpy(d_x, x, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_y, y, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_z, z, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_vx, vx, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_vy, vy, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_vz, vz, size, cudaMemcpyHostToDevice);

	kernel<<<count / 256 + 1, 256>>>(d_x, d_y, d_z, d_vx, d_vy, d_vz, count, tau);

	cudaMemcpy(x, d_x, size, cudaMemcpyDeviceToHost);
	cudaMemcpy(y, d_y, size, cudaMemcpyDeviceToHost);
	cudaMemcpy(z, d_z, size, cudaMemcpyDeviceToHost);
	cudaMemcpy(vx, d_vx, size, cudaMemcpyDeviceToHost);
	cudaMemcpy(vy, d_vy, size, cudaMemcpyDeviceToHost);
	cudaMemcpy(vz, d_vz, size, cudaMemcpyDeviceToHost);
}

