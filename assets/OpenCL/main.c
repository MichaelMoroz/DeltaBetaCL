//random number generator
//http://www.iquilezles.org/www/articles/sfrand/sfrand.htm
float sfrand( int *seed )
{
    float res;

    seed[0] *= 16807;

    *((unsigned int *) &res) = ( ((unsigned int)seed[0])>>9 ) | 0x40000000;

    return( res-3.0f );
}

#define PI 3.14159265f
#define DEG 0.0174533f
#define MIN_DIST 1e-5f

#define iFracScale 1.8f
#define iFracAng1 -0.12f
#define iFracAng2 0.5f
#define sin1 -0.1197f
#define cos1 0.9928f
#define sin2 0.4794f
#define cos2 0.8775f
#define iFracShift (float4)(-2.12f, -2.75f, 0.49f,0.f)
#define iFracIter 16

#include<OpenCL\distance_field.c> 
#include<OpenCL\ray_marching.c> 

#pragma pack(push, r1, 1)
typedef struct
{
	float4 position;
	float4 dirx;
	float4 diry;
	float4 dirz;
	float2 resolution;
	float2 step_resolution;
	float FOV;
	float focus;
	float bokeh;
	float exposure;
	float mblur;
	float speckle;
	float size;
	int stepN;
	int step;
} cl_camera;
#pragma pack(pop, r1)


void GetRay(int2 pixel, float4 *position, float4 *ray, cl_camera camera)
{
	float2 UV = (float2)(pixel.x, pixel.y) / camera.step_resolution;
	float2 pos = 2.f*UV - 1.f;
	float FOV = camera.FOV*DEG;
	float whratio = camera.resolution.x / camera.resolution.y;
	(*ray) = normalize(camera.dirx + FOV*pos.x*camera.dirz*whratio + FOV*pos.y*camera.diry);
	(*position) = camera.position + camera.size*pos.x*camera.dirz*whratio + camera.size*pos.y*camera.diry;
}


///MAIN FUNCTIONS
const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_LINEAR;

//find distance to surface(+ h and num of iterations) for each pixel
__kernel void first_pass_render(__read_only image2d_t prev_render, __write_only image2d_t render, cl_camera camera)
{
	//pixel coordinate [0..W/H]
	int2 pixel = (int2)(get_global_id(0), get_global_id(1));

	if (pixel.x < camera.step_resolution.x && pixel.y < camera.step_resolution.y) // if thread/pixel is inside the image
	{
		float whratio = camera.resolution.x / camera.resolution.y;
		//float pixel coordinate [0..1]
		float2 UV = (float2)(pixel.x, pixel.y) / camera.step_resolution;
		float2 pos = 2.f*UV - 1.f;
		float FOV = camera.FOV*DEG;

		float4 ray, position;
		GetRay(pixel, &position, &ray, camera);

		const float4 limits = (float4)(20.f, 0.f, 256.f, 0.f);

		float cone_angle = 8.f * FOV / camera.step_resolution.x;
		if (camera.step == camera.stepN - 1)
		{
			cone_angle = 4.f * FOV / camera.step_resolution.x;
		}
		
		//float cone_anglem = 4 * FOV / resolution.x;
		float4 march_data = (float4)(0.f, 0.f, 0.f, 1.f);

		//load interpolated data from previous low res render pass
		if (camera.step != 0)//if not the first step
		{
			march_data = read_imagef(prev_render, sampler, UV);
		}

		cone_march(ray, position, &march_data, limits, cone_angle, cone_angle);
		
		if (camera.step == camera.stepN - 1)//if last step
		{
			if (march_data.x >= limits.x)
			{
				march_data.z = 1000;
			}
			march_data.x = 1.f - march_data.z*0.02f;
			march_data.y = 1.f - march_data.z*0.02f;
			march_data.z = 1.f - march_data.z*0.02f;
		}
		
		write_imagef(render, pixel, march_data);
	}
}

//calculate shadow rays, reflection rays and refraction rays


//calculate volumetrics


//calculate texturing and lighting
__kernel void shading_pass(__read_only image2d_t prev_render, __write_only image2d_t render,
	float4 cam_pos, float4 dirx, float4 diry, float4 dirz, float4 resolution, float4 camera, float4 camera2, int step)
{
	//pixel coordinate [0..W/H]
	int2 pixel = (int2)(get_global_id(0), get_global_id(1));
	///camera stuff
	//minimal resolution is 128*128 for Nvidia compatibility
	float2 step_resolution = max(resolution.xy / pow(resolution.w, resolution.z - step - 1), (float2)(128.f, 128.f));
	if (pixel.x < step_resolution.x && pixel.y < step_resolution.y) // if thread/pixel is inside the image
	{
		float whratio = resolution.x / resolution.y;
		//float pixel coordinate [0..1]
		float2 UV = (float2)(pixel.x, pixel.y) / step_resolution;
		float2 pos = 2.f*UV - 1.f;
		float FOV = camera.x*DEG;
		float4 ray = normalize(dirx + FOV*pos.x*dirz*whratio + FOV*pos.y*diry);
		float4 position = cam_pos + camera2.x*pos.x*dirz*whratio + camera2.x*pos.y*diry;
		const float4 limits = (float4)(20.f, 0.f, 256.f, 0.f);
		float cone_angle = 8.f * FOV / step_resolution.x;
		if (step == resolution.z - 1)
		{
			cone_angle = 4.f * FOV / step_resolution.x;
		}

		//float cone_anglem = 4 * FOV / resolution.x;
		float4 march_data = (float4)(0.f, 0.f, 0.f, 1.f);
	}
}

//calculate bloom, blur, dof and speckles 
