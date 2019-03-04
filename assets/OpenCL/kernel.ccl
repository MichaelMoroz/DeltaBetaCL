//random number generator
//http://www.iquilezles.org/www/articles/sfrand/sfrand.htm
float sfrand( int *seed )
{
    float res;

    seed[0] *= 16807;

    *((unsigned int *) &res) = ( ((unsigned int)seed[0])>>9 ) | 0x40000000;

    return( res-3.0f );
}

//find distance to surface(+ h and num of iterations) for each pixel
__kernel void first_pass_render(__write_only image2d_t render, __read_only image2d_t prev_render)
{
	
	
	
	write_imagef(output, (int2)(get_global_id(0), get_global_id(1)), (float4)(0,0,0,0));
}

//calculate shadow rays, reflection rays and refraction rays
__kernel void second_pass_render(__write_only image2d_t render_shadow, __read_only image2d_t prev_render)
{
	
	
	
	write_imagef(output, (int2)(get_global_id(0), get_global_id(1)), (float4)(0,0,0,0));
}

//calculate volumetrics
__kernel void volume_pass_render(__write_only image2d_t render, __read_only image2d_t prev_render)
{
	
	
	
	write_imagef(output, (int2)(get_global_id(0), get_global_id(1)), (float4)(0,0,0,0));
}

//calculate texturing and lighting
__kernel void third_pass_render(__write_only image2d_t render, __read_only image2d_t prev_render)
{
	
	
	
	write_imagef(output, (int2)(get_global_id(0), get_global_id(1)), (float4)(0,0,0,0));
}

//calculate bloom, blur and dof
__kernel void fourth_pass_render(__write_only image2d_t render, __read_only image2d_t prev_render)
{
	
	
	
	write_imagef(output, (int2)(get_global_id(0), get_global_id(1)), (float4)(0,0,0,0));
}