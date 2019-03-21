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
#define MIN_DIST 1e-5
#define iFracScale 1.8f
#define iFracAng1 -0.12f
#define iFracAng2 0.5f
#define sin1 -0.1197f
#define cos1 0.9928f
#define sin2 0.4794f
#define cos2 0.8775f
#define iFracShift (float4)(-2.12f, -2.75f, 0.49f,0.f)
#define iFracIter 16
/*



float4 col_fractal(float4 p)
{
	float4 orbit = (float4)(0.0);
	float w = 1;
	for (int i = 0; i < iFracIter; ++i) {
		p = fabs(p);
		rotZ(p, iFracAng1);
		mengerFold(p);
		rotX(p, iFracAng2);
		p *= iFracScale;
		w *= iFracScale;
		p += iFracShift;
		orbit = max(orbit, p*iFracCol);
	}
	return orbit;
}


float4 CF(float4 p)
{
	p = p - (float4)(9.f, 0, 0);
	//rotX(p, 3.14159f*0.08f);
	//rotY(p, 3.14159f*0.2f);
	return clamp((float4)(1.f, 0.5f, 0.1f, 0.f), 0.f, 1.f);
}

void GetCamera(vec2 pos, vec2 cres, float4& ray, float4 &p, float &pix_angle)
{
	vec2 posn = vec2(pos.x - 0.5f, pos.y - 0.5f)*cres / min(cres.x, cres.y);

	//should be tan(FOV), but who cares?
	float FOVp = FOV*PI / 180.f;

	ray = normalize(transpose(CDM)*(float4)(1, -posn.y*FOVp, posn.x*FOVp));
	p = iCameraPos + transpose(CDM)*(float4)(0, -posn.y*CAMERA_SIZE, posn.x*CAMERA_SIZE);
	pix_angle = FOV*PI / (180.f*cres.x);
}


void SampleRay(float4 ray, float4 p, float min_angle, float pix_angle, float4 &march_data)
{
	cone_march(ray, p, march_data, (float4)(MAX_DIST, 0, MAX_MARCHES), min_angle, pix_angle);
}

float4 render_point(vec2 pos, vec2 mres, vec2 cres, float4 data)
{
	float4 ray, p;
	float pix_angle;
	GetCamera(pos, cres, ray, p, pix_angle);

	float4 march_data = data;
	float dres = mres.x / cres.x;
	SampleRay(ray, p, pix_angle / dres, pix_angle, march_data);

	return march_data;
}*/

float sdPlane(float4 p, float4 n)
{
	// n must be normalized
	return dot((float4)(p.x, p.y, p.z, 1.f), n);
}

void planeFold(float4 *z, float4 n, float d) {
	*z -= 2.0f * min((float4)(0.0f, 0.0f, 0.0f, 0.0f), dot(*z, n) - d) * n;
}

void sierpinskiFold(float4 *z) {
	float a = min(z->x + z->y, 0.0f);
	z->x -= a; z->y -= a;
	a = min(z->x + z->z, 0.0f);
	z->x -= a; z->z -= a;
	a = min(z->y + z->z, 0.0f);
	z->y -= a; z->z -= a;
}

void mengerFold(float4 *z) {
	float a = min(z->x - z->y, 0.f);
	z->x -= a; z->y += a;
	a = min(z->x - z->z, 0.f);
	z->x -= a; z->z += a;
	a = min(z->y - z->z, 0.f);
	z->y -= a; z->z += a;
}

void boxFold(float4 *z, float4 r) {
	*z = clamp(*z, -r, r) * 2.f - *z;
}

void rotXa(float4 *z, float s, float c) {
	float2 rot = (float2)(c*z->y + s*z->z, c*z->z - s*z->y);
	z->y = rot.x;
	z->z = rot.y;
}

void rotYa(float4 *z, float s, float c) {
	float2 rot = (float2)(c*z->x - s*z->z, c*z->z + s*z->x);
	z->x = rot.x;
	z->z = rot.y;
}

void rotZa(float4 *z, float s, float c) {
	float2 rot = (float2)(c*z->x + s*z->y, c*z->y - s*z->x);
	z->x = rot.x;
	z->y = rot.y;
}

void rotX(float4 *z, float a) {
	rotXa(z, sin(a), cos(a));
}
void rotY(float4 *z, float a) {
	rotYa(z, sin(a), cos(a));
}
void rotZ(float4 *z, float a) {
	rotZa(z, sin(a), cos(a));
}

float de_box(float4 p, float4 s) {
	float4 a = fabs(p) - s;
	return (min(max(max(a.x, a.y), a.z), 0.0f) + length(max(a, 0.0f)));
}


float de_sphere(float4 p, float r) {
	return length(p) - r;
}

float de_fractal(float4 p) {
	float w = 1;
	for (int i = 0; i < iFracIter; ++i)
	{
		p = fabs(p);
		rotZa(&p, sin1, cos1);
		//rotZ(&p, iFracAng1);
		mengerFold(&p);
		//rotX(&p, iFracAng2);
		rotXa(&p, sin2, cos2);
		p *= iFracScale;
		w *= iFracScale;
		p += iFracShift;
	}
	return de_box(p, (float4)(6.0f, 6.0f, 6.0f, 0.f)) / w;
}



float SDF(float4 p)
{
	p = p - (float4)(4.f, -3.5, 1.f,0.f);
	//rotX(p, 3.14159f*0.08f);
	//rotY(p, 3.14159f*0.2f);
	float DE = de_fractal(p);
	return DE;
}


void cone_march(float4 ray, float4 p, float4 *march_data, float4 limits, float cone_angle, float cone_angle_max)
{
	float h, n, cone_radius, td = march_data->x;
	float NdotR = 0, prev_h = 1e10;
	int object_type = 0; //nothing
	float subsurf_td = 0; // for subsurface scattering and/or soft shadows

	for (; ((march_data->z < limits.z)); march_data->z += 1.f)
	{
		cone_radius = max(MIN_DIST, td*cone_angle);

		h = SDF(p + td*ray);

		NdotR = clamp(fabs((h - prev_h) / prev_h), 0.f, 1.f);

		if (h < 0)
		{
			subsurf_td -= h;
			march_data->z += 2*h/cone_radius;
		}

		if (h<max(cone_radius, (1 - NdotR)*td*cone_angle_max))
		{
			march_data->z += 2*h / cone_radius;
			break;
		}

		if (td < limits.x)
		{
			march_data->y = 1;
			march_data->z += 2*NdotR;
		}
		else
		{
			break;
		}

		h = ((h>0) ? (1) : (-3))*max(fabs(h), cone_radius);
		td += fabs(h);
		prev_h = h;
	}
	//whiout +h we are losing 1 SDF calcualtion each ray, aka slight optimization
	march_data->x = td + h;
	march_data->y = h;
	//p = p + march_data.x*ray;
	
}

///MAIN FUNCTIONS
const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_LINEAR;

//find distance to surface(+ h and num of iterations) for each pixel
__kernel void first_pass_render(__read_only image2d_t prev_render, __write_only image2d_t render,
	float4 cam_pos, float4 dirx, float4 diry, float4 dirz, float4 resolution, float4 camera, float4 camera2, int step)
{
	//pixel coordinate [0..W/H]
	int2 pixel = (int2)(get_global_id(0), get_global_id(1));
	///camera stuff
	float2 step_resolution = resolution.xy / pow(resolution.w, resolution.z - step - 1);
	//float pixel coordinate [0..1]
	float whratio = resolution.x / resolution.y;
	float2 UV = (float2)(pixel.x, pixel.y) / step_resolution;
	float2 pos = 2.f*UV - 1.f;
	float FOV = camera.x*DEG;
	float4 ray = normalize(dirx + FOV*pos.x*dirz*whratio + FOV*pos.y*diry);
	float4 position = cam_pos + camera2.x*pos.x*dirz + camera2.x*pos.y*diry;
	float4 limits = (float4)(20.f, 0.f, 256.f, 0.f);
	float cone_angle = 4 * FOV / step_resolution.x;
	float4 march_data = (float4)(0.f, 0.f, 0.f, 1.f);

	//load data from previous render pass
	if (step != 0)
	{
		march_data = read_imagef(prev_render, sampler, UV);
	}

	cone_march(ray, position, &march_data, limits, cone_angle, cone_angle);

	if (step == resolution.z - 1)
	{
		march_data.x = march_data.x / 10.f;
		march_data.y = march_data.y / 100.f;
		march_data.z = march_data.z / 100.f;
	}

	write_imagef(render, pixel, march_data);
}

//calculate shadow rays, reflection rays and refraction rays


//calculate volumetrics


//calculate texturing and lighting


//calculate bloom, blur, dof and speckles 
