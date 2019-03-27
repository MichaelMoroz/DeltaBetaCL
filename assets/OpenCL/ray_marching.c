
void cone_march(float4 ray, float4 p, float4 *march_data, float4 limits, float cone_angle, float cone_angle_max)
{
	float h, n, cone_radius, td = (*march_data).x;
	float NdotR = 0, prev_h = 1e10;
	int object_type = 0; //nothing
	float subsurf_td = 0; // for subsurface scattering and/or soft shadows

	for (; (((*march_data).z < limits.z)); (*march_data).z += 1.f)
	{
		cone_radius = max(MIN_DIST, td*cone_angle);

		h = SDF(p + td*ray);

		//NdotR = clamp(fabs((h - prev_h) / prev_h), 0.f, 1.f);

		if (h < 0)
		{
			subsurf_td -= h;
			//(*march_data).z += 2 * h / cone_radius;
			if (td < MIN_DIST * 10) // if camera is inside something
			{
				break;
			}
		}

		if (h<cone_radius)
		{
			(*march_data).z += 2 * h / cone_radius;
			break;
		}

		if (td >= limits.x)
		{
			break;
		}

		//march towards the surface
		//h = ((h>0) ? (h) : (-max(cone_radius*1.5f, -h)));
		td += h;
		prev_h = h;
	}
	//whiout +h we are losing 1 SDF calcualtion each ray, aka slight optimization
	(*march_data).x = td + h;
	(*march_data).y = h;
	//p = p + march_data.x*ray;

}