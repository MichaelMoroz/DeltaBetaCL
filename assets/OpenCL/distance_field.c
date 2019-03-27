#define PI 3.14159265f
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
	(*z) -= 2.0f * min((float4)(0.0f, 0.0f, 0.0f, 0.0f), dot((*z), n) - d) * n;
}

void sierpinskiFold(float4 *z) {
	float a = min((*z).x + (*z).y, 0.0f);
	(*z).x -= a; (*z).y -= a;
	a = min((*z).x + (*z).z, 0.0f);
	(*z).x -= a; (*z).z -= a;
	a = min((*z).y + (*z).z, 0.0f);
	(*z).y -= a; (*z).z -= a;
}

void mengerFold(float4 *z) {
	float a = min((*z).x - (*z).y, 0.f);
	(*z).x -= a; (*z).y += a;
	a = min((*z).x - (*z).z, 0.f);
	(*z).x -= a; (*z).z += a;
	a = min((*z).y - (*z).z, 0.f);
	(*z).y -= a; (*z).z += a;
}

void boxFold(float4 *z, float4 r) {
	*z = clamp(*z, -r, r) * 2.f - *z;
}

void rotXa(float4 *z, float s, float c) {
	float2 rot = (float2)(c*(*z).y + s*(*z).z, c*(*z).z - s*(*z).y);
	(*z).y = rot.x;
	(*z).z = rot.y;
}

void rotYa(float4 *z, float s, float c) {
	float2 rot = (float2)(c*(*z).x - s*(*z).z, c*(*z).z + s*(*z).x);
	(*z).x = rot.x;
	(*z).z = rot.y;
}

void rotZa(float4 *z, float s, float c) {
	float2 rot = (float2)(c*(*z).x + s*(*z).y, c*(*z).y - s*(*z).x);
	(*z).x = rot.x;
	(*z).y = rot.y;
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

//Main

float SDF(float4 p)
{
	p = p - (float4)(4.f, -3.5, 1.f,0.f);
	//rotX(p, 3.14159f*0.08f);
	//rotY(p, 3.14159f*0.2f);
	float DE = de_fractal(p);
	return DE;
}

//A faster formula to find the gradient/normal direction of the DE(the w component is the average DE)
//credit to http://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
float4 calcNormal(float4 p, float dx) {
	const float4 k = (float4)(1.f, -1.f, 0.f, 0.f);
	return (k.xyyx*SDF(p + k.xyyz*dx) +
		    k.yyxx*SDF(p + k.yyxz*dx) +
		    k.yxyx*SDF(p + k.yxyz*dx) +
		    k.xxxx*SDF(p + k.xxxz*dx))/(float4)(4.f*dx, 4.f*dx, 4.f*dx, 4.f);
}
