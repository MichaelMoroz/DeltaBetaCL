#version 330 core

//Maximal size of the IPN array for computing the SDF functions
//Used to store the ray marching models
#define MAX_SDF_ARRAY_SIZE 4096
#define PI 3.14159265f
#define MAX_MARCHES 256
#define AMBIENT_MARCHES 3
#define ENABLE_SHADOWS false
#define ENABLE_REFLECTION true
#define ENABLE_REFRACTION true
#define ENABLE_VOLUMETRIC true
#define SHADOW_SMOOTHNESS 13
#define MAX_DIST 250
#define MAX_EVAL_DIST 30

#define MIN_DIST 1e-5
#define AMBIENT_BRIGHTNESS 0.25
#define AMBIENT_COEF 0.3
#define CAMERA_SIZE 0
#define BACKGROUND_COLOR vec3(0, 0, 0)
#define AMBIENT_COLOR 1.6*vec3(0.3, 0.4, 0.7)
#define FOG_COLOR vec3(0,50.f/255.f,1)

uniform int iFracIter;
uniform float iFracScale;
uniform float iFracAng1;
uniform float iFracAng2;
uniform vec3 iFracShift;
uniform vec3 iFracCol;

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 additional0;
layout (location = 2) out vec4 additional1;
layout (location = 3) out vec4 additional2;
layout (location = 4) out vec4 additional3;
layout (location = 5) out vec4 additional4;
layout (location = 6) out vec4 additional5;

//input textures

uniform sampler2D texture_0_0;
uniform sampler2D texture_0_1;
uniform sampler2D texture_0_2;

uniform sampler2D texture_1_0;
uniform sampler2D texture_1_1;
uniform sampler2D texture_1_2;

uniform sampler2D texture_2_0;
uniform sampler2D texture_2_1;
uniform sampler2D texture_2_2;

uniform sampler2D texture_3_0;
uniform sampler2D texture_3_1;
uniform sampler2D texture_3_2;

uniform sampler2D texture_4_0;
uniform sampler2D texture_4_1;

uniform sampler2D texture_5_0;
uniform sampler2D texture_5_1;

uniform sampler2D texture_6_0;
uniform sampler2D texture_6_1;

uniform vec2 RayMarchResolution;
uniform vec2 MainResolution;
uniform vec3 CameraPosition;
uniform mat3 CameraDirMatrix;
uniform float time;
uniform float FOV;
uniform float SuperResCond;
uniform int RenderMode;
uniform int RenderPipelineIter;
uniform float Exposure;
uniform float metallic;
uniform float roughness;
uniform vec3 LIGHT_POSITION;
uniform float LIGHT_RADIUS;
uniform vec3 LIGHT_COLOR;

vec4 A,B,C,D, BB, CC;
vec2 pos;

vec3 ToneMapFilmicALU(vec3 _color)
{
	_color = max(vec3(0), _color - vec3(0.004));
	_color = (_color * (6.2*_color + vec3(0.5))) / (_color * (6.2 * _color + vec3(1.7)) + vec3(0.06));
	return _color;
}


struct RayMarchResult
{
	vec3 pos;
	vec3 normal;
	vec3 color;
	float passed_dist, DistEstim;
	float iter;
};

float de_sphere(vec4 p, float r) {
	return (length(p.xyz) - r) / p.w;
}
float de_box(vec4 p, vec3 s) {
	vec3 a = abs(p.xyz) - s;
	return (min(max(max(a.x, a.y), a.z), 0.0) + length(max(a, 0.0))) / p.w;
}

float sdPlane( vec3 p, vec4 n )
{
  // n must be normalized
  return dot(p,n.xyz) + n.w;
}

void planeFold(inout vec4 z, vec3 n, float d) {
	z.xyz -= 2.0 * min(0.0, dot(z.xyz, n) - d) * n;
}
void sierpinskiFold(inout vec4 z) {
	z.xy -= min(z.x + z.y, 0.0);
	z.xz -= min(z.x + z.z, 0.0);
	z.yz -= min(z.y + z.z, 0.0);
}
void mengerFold(inout vec4 z) {
	float a = min(z.x - z.y, 0.0);
	z.x -= a;
	z.y += a;
	a = min(z.x - z.z, 0.0);
	z.x -= a;
	z.z += a;
	a = min(z.y - z.z, 0.0);
	z.y -= a;
	z.z += a;
}

void boxFold(inout vec4 z, vec3 r) {
	z.xyz = clamp(z.xyz, -r, r) * 2.0 - z.xyz;
}
void rotX(inout vec4 z, float s, float c) {
	z.yz = vec2(c*z.y + s*z.z, c*z.z - s*z.y);
}
void rotY(inout vec4 z, float s, float c) {
	z.xz = vec2(c*z.x - s*z.z, c*z.z + s*z.x);
}
void rotZ(inout vec4 z, float s, float c) {
	z.xy = vec2(c*z.x + s*z.y, c*z.y - s*z.x);
}
void rotX(inout vec4 z, float a) {
	rotX(z, sin(a), cos(a));
}
void rotY(inout vec4 z, float a) {
	rotY(z, sin(a), cos(a));
}
void rotZ(inout vec4 z, float a) {
	rotZ(z, sin(a), cos(a));
}

float de_fractal(vec4 p) {
  for (int i = 0; i < iFracIter; ++i) 
  {
    p.xyz = abs(p.xyz);
    rotZ(p, iFracAng1);
    mengerFold(p);
    rotX(p, iFracAng2);
    p *= iFracScale;
    p.xyz += iFracShift;
  }
  return de_box(p, vec3(6.0));
}

vec4 col_fractal(vec4 p) 
{
  vec3 orbit = vec3(0.0);
  for (int i = 0; i < iFracIter; ++i) {
    p.xyz = abs(p.xyz);
    rotZ(p, iFracAng1);
    mengerFold(p);
    rotX(p, iFracAng2);
    p *= iFracScale;
    p.xyz += iFracShift;
    orbit = max(orbit, p.xyz*iFracCol);
  }
  return vec4(orbit, de_box(p, vec3(6.0)));
}


float SDF(vec3 p)
{
	float DE = de_fractal(vec4((p-vec3(6.f,0.5,0)),1));
	DE = min(DE, 0.95*sdPlane(p, vec4(0,-1,0,1.2)));
	DE = min(DE, de_sphere(vec4(p-LIGHT_POSITION,1), LIGHT_RADIUS));

	return DE;
}

float FogDensity(vec3 p)
{
	return 0;//min(exp(p.y/50 - 7),1);
}

vec3 CF(vec3 p)
{
	if(length(LIGHT_POSITION-p)<=LIGHT_RADIUS*1.01)
	{
		return vec3(10);
	}
	if(p.y < 0.5 && abs(p.x) < 10 && abs(p.z) < 10)
	{
		return clamp(col_fractal(vec4(p-vec3(6.f,0.5,0),1)).xyz,0,1);
	}
	if( mod((floor(p.x/2)+floor(p.z/2)),2) == 0 || p.y <-1.35)
	{
		return vec3(1.f);
	} 
	else
	{
		return vec3(0.5f);
	}
}

vec3 calcNormal(vec3 p, float dx) 
{
    const vec2 k = vec2(1,-1);
    return normalize(k.xyy*SDF( p + k.xyy*dx ) + 
				 	k.yyx*SDF( p + k.yyx*dx ) + 
					k.yxy*SDF( p + k.yxy*dx ) + 
					k.xxx*SDF( p + k.xxx*dx ));
    
}

/*
float WORLD_SDF(vec3 p, out int object_type)
{

}

void WORLD_MATERIAL(vec3 p, out vec3 normal, out vec3 color, out vec3 material)
{

}*/

//when usual marching the cone angle(radians) is the pixel angle
//when shadow marching the cone angle should be equal to the angle of the light source

//march-data x - td, y - max_occ, z - iter  
void cone_march(inout vec3 ray, inout vec3 p, inout vec3 march_data, vec3 limits, float cone_angle, bool shadow)
{
	float h, n, cone_radius, td = march_data.x;
	int object_type = 0; //nothing
	float subsurf_td = 0; // for subsurface scattering and/or soft shadows

	for(; ((march_data.z < limits.z)) ; march_data.z+=1)
	{
		cone_radius = max(MIN_DIST, td*cone_angle);
		h = SDF(p+td*ray);
		
		if(h < 0)
		{
			subsurf_td -= h;
		}
		
		if(shadow)
		{
			//if h = 0, then the light is half occluded
			if(td>cone_radius)
			{
				march_data.y = max(march_data.y, 0.5*(1 - 2*((h >= 0)?(h):(subsurf_td/2))/cone_radius));
				//if completely occluded
				if(march_data.y >= 1)
				{
					break;
				}
			}
		}
		else if(h<cone_radius)
		{
			break;
		} 
		
		if(td < limits.x)
		{
			march_data.y = 1;
		}
		
		h = max(h, cone_radius);
		td += h;
	}
	//whiout +h we are losing 1 SDF calcualtion each ray, aka slight optimization
	march_data.x = td + h;
	p = p+march_data.x*ray;
	march_data.z += h/cone_radius;
}


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


void SampleRay(vec3 ray, vec3 p, float pix_angle, inout vec3 march_data)
{
	cone_march(ray, p, march_data, vec3(MAX_DIST, 0, MAX_MARCHES), pix_angle, false);
}

void SampleShadow(vec3 p, float pix_angle, inout vec3 shadow_data, inout vec3 ambient_data)
{
	//normal = calcNormal(p, march_data.x*pix_angle*0.25);
	for(int i = 0; i<1 && ENABLE_SHADOWS; i++) // loop through all lights
	{
		float distance = length(LIGHT_POSITION - p);
		vec3 L = (LIGHT_POSITION - p)/distance;
		vec3 shad_point = p;
		cone_march(L, shad_point, shadow_data, vec3(distance, 0, MAX_MARCHES), max(float(LIGHT_RADIUS)/distance, pix_angle), false);
	}
	//cone_march(normal, p, ambient_data, vec3(MAX_DIST, 0, 1), 0.f, false);
}

vec3 SampleColor0(vec3 V, vec3 WorldPos, vec3 march_data, vec3 N, vec4 ambient_data, vec4 shadows)
{	
	if(march_data.y >= MAX_DIST)
	{
		return BACKGROUND_COLOR;
	}	
	
	vec3 albedo = CF(WorldPos);
	
	///Physically Based Rendering
	//Physically Based Rendering
	
    // reflectance equation
    vec3 Lo = vec3(0.0);
	V = -V;
	
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	
	for(int i = 0; i < 1; i++)
	{
	// calculate per-light radiance
		float distance    = length(LIGHT_POSITION - WorldPos);
		float attenuation = 1.0 / (distance * distance);
		
		vec3 L = (LIGHT_POSITION - WorldPos)/distance;
		vec3 H = normalize(V + L);
		
		attenuation *= clamp(1 - shadows[i],0,1);

		vec3 radiance = LIGHT_COLOR * attenuation;        
		
		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);        
		float G   = GeometrySmith(N, V, L, roughness);      
		vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;	  
		
		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular     = numerator / max(denominator, 0.001);  
			
		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);                
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	//Advanced ambient occlusion 
	float ao0 = 1/(AMBIENT_COEF*(ambient_data.x) + 1);
    vec3 ambient = AMBIENT_BRIGHTNESS*albedo*(AMBIENT_COLOR * ao0);
	
    return ambient + Lo ;
}

void GetCamera(vec2 pos, float down_res, out vec3 ray, out vec3 p, out float pix_angle)
{
	//rescaling
	vec2 cres= MainResolution/down_res;
	vec2 posn = vec2(pos.x-0.5f,pos.y-0.5f)*cres/min(cres.x,cres.y) - vec2(0, (down_res-1)*0.0005);
	
	//should be tan(FOV), but who cares?
	float FOVp = FOV*PI/180.f;

	ray = normalize(transpose(CameraDirMatrix)*vec3(1,-posn.y*FOVp,posn.x*FOVp));
	p = CameraPosition+ transpose(CameraDirMatrix)*vec3(0,-posn.y*CAMERA_SIZE,posn.x*CAMERA_SIZE);
	pix_angle = (0.6+down_res*0.09)*FOV*PI/(180.f*RayMarchResolution.x);
}

vec4 HDR_map(vec3 hdrColor)
{
	// Exposure tone mapping
	vec3 mapped = vec3(1.0) - exp(-hdrColor * Exposure);
	
	// Gamma correction 
	return vec4(pow(mapped, vec3(0.48)),0);
}


vec4 billinear_texture(sampler2D render_texture, float down_res)
{
	vec2 DX = vec2(1.f)*down_res/MainResolution;
	pos -= DX*0.5;
	vec2 DD = vec2(0.f,1.f);
	vec2 posi = pos/DX;
	vec2 dpos0 = posi - floor(posi);
	vec2 dpos1 = 1 - dpos0;
	vec4 A = texture(render_texture, pos);
	vec4 B = texture(render_texture, pos+DX*DD.yx);
	vec4 C = texture(render_texture, pos+DX*DD.xy);
	vec4 D = texture(render_texture, pos+DX*DD.yy);
	
	return A*dpos1.x*dpos1.y+B*dpos0.x*dpos1.y+C*dpos1.x*dpos0.y+D*dpos0.x*dpos0.y;
}

//render depth map
void render_point_0(vec2 pos, float down_res, bool addit, vec3 init1, float inaccuracy)
{
	vec3 ray, p;
	float pix_angle;
	GetCamera(pos, down_res, ray, p, pix_angle);
	
	vec3 march_data = vec3(init1.x, 0, init1.y);
	SampleRay(ray, p, pix_angle*inaccuracy, march_data);
	
	color = vec4(march_data.x, march_data.z, 0, 1.f);
}

//render shadow map and ambient occlusion
void render_point_1(vec2 pos, float down_res, bool addit, vec3 init1, vec3 init2, float inaccuracy)
{
	vec3 ray, p;
	float pix_angle;
	GetCamera(pos, down_res, ray, p, pix_angle);
	
	vec3 shadow_data = vec3(init1.x, init1.y, init1.z);
	vec3 ambient_data = vec3(init2.x, 0, 0);
	SampleShadow(p, pix_angle*inaccuracy, shadow_data, ambient_data);
	
	color = vec4(shadow_data.x, shadow_data.y, shadow_data.z, 1.f);
}

vec3 findMAX(sampler2D tex, float down_res)
{
	vec3 DD = vec3(0.f,1.f,-1.f);
	vec2 DX = vec2(1.f)*down_res/MainResolution;
	vec4 res;
	res = max(texture(tex, pos+DX*DD.xy), texture(tex, pos+DX*DD.zx));
	res = max(max(res, texture(tex, pos+DX*DD.yy)), texture(tex, pos+DX*DD.yz));
	res = max(max(res, texture(tex, pos+DX*DD.zy)), texture(tex, pos+DX*DD.zz));
	res = max(max(res, texture(tex, pos+2*DX*DD.xy)), texture(tex, pos+2*DX*DD.zx));
	res = max(max(res, texture(tex, pos+2*DX*DD.yx)), texture(tex, pos+2*DX*DD.xz));
	res = max(max(res, texture(tex, pos + DX*DD.yx)), texture(tex, pos + DX*DD.xz));
	res = max(res, texture(tex, pos));
	return res.xyz;
}


vec4 findMIN(sampler2D tex, float down_res)
{
	vec3 DD = vec3(0.f,1.f,-1.f);
	vec2 DX = vec2(down_res)/MainResolution;
	vec4 res = min(texture(tex, pos+DX*DD.xy), texture(tex, pos+DX*DD.zx));
	     res = min(min(res, texture(tex, pos+DX*DD.yx)), texture(tex, pos+DX*DD.xz));
	     res = min(res, texture(tex, pos));
		 //res = min(min(res, texture(tex, pos+DX*DD.yy)), texture(tex, pos+DX*DD.yz));
		 //res = min(min(res, texture(tex, pos+DX*DD.zy)), texture(tex, pos+DX*DD.zz));
	return res;
}

void BorderFind(sampler2D tex, float down_res)
{
	vec3 DD = vec3(0.f,1.f,-1.f);
	vec2 DX = vec2(down_res)/MainResolution;
	
	//border finding filter
	vec4 filt = vec4(0);
	
	filt -= texture(tex, pos+DX*DD.xy);
	filt -= texture(tex, pos+DX*DD.zx);
	filt -= texture(tex, pos+DX*DD.yx);
	filt -= texture(tex, pos+DX*DD.xz);
	
	filt = filt*0.25 + texture(tex, pos);
	
	color = vec4(abs(filt.x/10),0,0,1); 
}

//who the f calculates normals in this way? (well. it may be stupid, but its fast)
vec3 CalculateNormalFromDepthMap(sampler2D tex, float down_res)
{
	vec3 DD = 2*vec3(0.f,1.f,-1.f);
	vec2 DX = vec2(down_res)/MainResolution;
	vec2 DXuv = vec2(DX.x,-DX.y);
	
	//triangle points
	vec3 R1, R2, R3, P;
	float pa;
	
	GetCamera(UV+DXuv*DD.xy, down_res, R1, P, pa);
	vec3 P1 = R1*texture(tex,pos+DX*DD.xy).x;
	GetCamera(UV+DXuv*DD.zz, down_res, R2, P, pa);
	vec3 P2 = R2*texture(tex,pos+DX*DD.zz).x;
	GetCamera(UV+DXuv*DD.yz, down_res, R3, P, pa);
	vec3 P3 = R3*texture(tex,pos+DX*DD.yz).x;
	
	vec3 normal = normalize(cross(P1-P2,P1-P3));
	
	GetCamera(UV+DXuv*DD.xz, down_res, R1, P, pa);
	P1 = R1*texture(tex,pos+DX*DD.xz).x;
	GetCamera(UV+DXuv*DD.yy, down_res, R2, P, pa);
	P2 = R2*texture(tex,pos+DX*DD.yy).x;
	GetCamera(UV+DXuv*DD.zy, down_res, R3, P, pa);
	P3 = R3*texture(tex,pos+DX*DD.zy).x;
	
	normal += normalize(cross(P1-P2,P1-P3));
	
	return normal/2;
}

/*
vec4 Blur(sampler2D texture, bool HV, float down_res, float R)
{
	vec4 sum = vec4(0);
	float dsum = 0;
	vec2 DX = vec2(1.f)*down_res/MainResolution;
	float k = 0;
	for(int i =-int(R); i < R; i++)
	{
		k = exp(-0.5f*i/R);
		sum += texture(texture, pos+((HV)?(vec2(DX.x*i,0)):(vec2(0,DX.y*i))))*k;
		dsum += k;
	}
	return sum/dsum;
}




//render at full resolution
void FinalPass()
{	
	//Correcting the SFML texture direction of y
    pos = vec2(UV.x,1-UV.y);
	switch(RenderMode)
	{
	case 0:
		MultiGridRender(texture_2_0, texture_3_0, texture_2_1, texture_2_2, 1, 4);
		
		break;
	case 1:
	    A = texture(texture_2_1, pos);
		color = vec4(A.xxx/20,1); 
		break;
	case 2:
     	A = texture(texture_2_0, pos);
		color = vec4(A.zyx,1); 
		break;
	case 3:
	    C = texture(texture_0_1, pos);
		D = texture(texture_2_1, pos);
		float d = (D.g-C.g)/C.g;
		color = vec4(C.g/MAX_MARCHES, d, d,  1.f); 
		break;
	case 4:
	    B = texture(texture_0_1, pos);
		color = vec4(B.xxx/20,1); 
		break;
	case 5:
	    D = texture(texture_1_0, pos);
		C = texture(texture_3_0, pos);
		bool bord = D.x>SuperResCond/4;
		bool bord1 = C.x>SuperResCond;
		color = vec4((bord)?1.f:0.f, (bord1)?1.f:0.f, 0,1); 
		break;
	default:
		color = vec4(1.f);
		break;
	}
}*/

void MultiGridRender(sampler2D prev_data1, sampler2D prev_data2, float dres, float prev_dres)
{
	//render at higher resolution
	BB = findMIN(prev_data1,prev_dres);
	bool rend = (BB.x<MAX_DIST*0.95 && BB.y<MAX_MARCHES*0.95);
	if(rend)
	{
		CC = findMIN(prev_data2,prev_dres);
		render_point_0(UV, dres, true, vec3(BB.x, BB.y, BB.z), 1);
	}
	else
	{
		color = texture(prev_data1, pos);
		additional0 = texture(prev_data2, pos);
		//additional1 = texture(prev_data2, pos);
	}
	
}

void MultiGridRenderBorder(sampler2D border_tex, sampler2D prev_data1, sampler2D prev_data2, float dres, float prev_dres)
{
	//render at higher resolution
	float bord = texture(border_tex, pos).x;
	BB = findMIN(prev_data1,prev_dres);
	bool rend = (BB.x<MAX_DIST*0.95 && BB.y<MAX_MARCHES*0.95);
	if(rend)
	{
		CC = findMIN(prev_data2,prev_dres);
		render_point_0(UV, dres, true, vec3(BB.x, BB.y, BB.z), 1+SuperResCond/(bord+0.05));
	}
	else
	{
		color = texture(prev_data1, pos);
		additional0 = texture(prev_data2, pos);
		//additional1 = texture(prev_data2, pos);
	}
	
}

void main()
{
	switch(RenderPipelineIter)
	{
	case 0:
		render_point_1(UV, 256, true, vec3(0), vec3(0), 1);
		break;
	
	case 1:
		pos = vec2(UV.x,1-UV.y);
		MultiGridRender(texture_0_0, texture_0_1, 64, 256);
		break;
		
	case 2:
		pos = vec2(UV.x,1-UV.y);
		MultiGridRender(texture_1_0, texture_1_1, 16, 64);
		break;
	
	case 3:
	    pos = vec2(UV.x,1-UV.y);
		BorderFind(texture_2_0, 16);
		break;

	case 4:
	    pos = vec2(UV.x,1-UV.y);
		MultiGridRenderBorder(texture_3_0, texture_2_0, texture_2_1, 4, 16);
		break;
	
	case 5:
	    pos = vec2(UV.x,1-UV.y);
		BorderFind(texture_4_0, 4);
		break;

	case 6:
	    pos = vec2(UV.x,1-UV.y);
		MultiGridRenderBorder(texture_5_0, texture_4_0, texture_4_1, 1, 4);
		break;
	
	case 7: 
		pos = vec2(UV.x,1-UV.y);
		float bord = texture(texture_1_0, pos).x;
		float iter1 = texture(texture_2_0, pos).y;
		float iter0 = texture(texture_0_0, pos).y;
		bool border = bord>(SuperResCond/4);
		color = border?vec4(1.f):vec4(0.f);
		
		vec3 ray, p;
		float pix_angle;
		
		color = vec4((0.5f*CalculateNormalFromDepthMap(texture_6_0, 1)+0.5f),1.f);
		//color = vec4(P,P,P,1.f);
		//color = vec4(0,0, (iter1 - iter0)/10, 1.f);
		//BorderFind(texture_2_0, 4);
		break;
	/*
		
	case 4:
		FinalPass();
		break;*/
		
	default:
		color = vec4(1);
		break;
	}
	
}