/*---------------------------------------------------------------------------------------------------------
- Input registers used by the vertex shaders:

  v0 = Position
  v1 = Color (RGBA)
  v2 = Normal
  v3 = UV set 0 (map)
  v4 = UV set 1 (light map)
  v5 = Bones indices
  v6 = Bones weights

-----------------------------------------------------------------------------------------------------------
NOTE: DON'T MODIFY THESE REGISTERS:

- r12 is reserved to read the oPos register.
- r11 is reserved to store the vertex color (RGBA [0,1]) after tint and before lighting.
- r10 is reserved to store the normal (skinned/unskinned) for enviro map and lighting.
- r9 is reserved for the halved vertex color (KellySlater only)

---------------------------------------------------------------------------------------------------------*/

#ifdef PROJECT_KELLYSLATER
#define NGL_OUTPUT_VERTEX_COLOR "r9"
#else
#define NGL_OUTPUT_VERTEX_COLOR "oD0"
#endif

// These constant are initialized in the nglInitShaders() function.
char NGL_VSC_ZERO[16];
char NGL_VSC_HALF[16];
char NGL_VSC_ONE[16];
char NGL_VSC_TWO[16];

char NGL_VSC_BONES_SCALE[16];
char NGL_VSC_BONES_OFS[16];
char NGL_VSC_SHADOW_FADING[16];

// Store the final VS source code.
char nglVSSrc[4096];
// Store each VS fragment source code.
char nglVSBuf[4096];

uint32 nglVSCurLight;

/*-----------------------------------------------------------------------------
Description: Begin the vertex shader source code.
-----------------------------------------------------------------------------*/
void nglvsf_begin()
{
	nglVSSrc[0] = 0;

	nglVSCurLight = 0;

	sprintf(nglVSBuf, 		"xvs.1.1\n" "#pragma screenspace\n"
#ifdef PROJECT_KELLYSLATER	// For shaders which don't write to oD0
		"mov " NGL_OUTPUT_VERTEX_COLOR ", %s", 
		NGL_VSC_ONE
#endif
	);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: End the vertex shader source code (in other words it calculates
             the screen space scale and offset).
-----------------------------------------------------------------------------*/
void nglvsf_end()
{
	sprintf(nglVSBuf,
#ifdef PROJECT_KELLYSLATER
			"mul oD0.xyz, " NGL_OUTPUT_VERTEX_COLOR ".xyz, %s\n"
			"mov oD0.w, " NGL_OUTPUT_VERTEX_COLOR ".w\n"
#endif
			// Scale.
			";---- vsf_end\n" "mul oPos.xyz, r12, c[%d]\n"
			// Compute 1/w.
			"+ rcc r1.x, r12.w\n"
			// Scale by 1/w, add offset.
			"mad oPos.xyz, r12, r1.x, c[%d]\n",
#ifdef PROJECT_KELLYSLATER
			NGL_VSC_HALF, 
#endif
			nglVSC_Scale, nglVSC_Offset);

	strcat(nglVSSrc, nglVSBuf);

#if NGL_DEBUG
	if (nglDebug.ShowVSCode)
		nglPrintf(";----------------------------------\n%s----------------------------------\n", nglVSSrc);
#endif
}

/*-----------------------------------------------------------------------------
Description: Build the billboards VS.
-----------------------------------------------------------------------------*/
void nglvsf_billboards()
{
	sprintf(nglVSBuf,
			";---- vsf_billboard\n"
			"dp4 oPos.x, v0, c[%d]\n"
			"dp4 oPos.y, v0, c[%d]\n"
			"dp4 oPos.z, v0, c[%d]\n"
			"dp4 oPos.w, v0, c[%d]\n"
			"mov " NGL_OUTPUT_VERTEX_COLOR ",  v1\n"
			"mov oT0,  v3\n"
			"mov oFog, %s\n",
			nglVSC_LocalToScreen + 0, nglVSC_LocalToScreen + 1,
			nglVSC_LocalToScreen + 2, nglVSC_LocalToScreen + 3, NGL_VSC_ONE);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Transform a non skinned vertex.
-----------------------------------------------------------------------------*/
void nglvsf_transform()
{
	sprintf(nglVSBuf,
			";---- vsf_transform\n"
			"dp4 oPos.x, v0, c[%d]\n"
			"dp4 oPos.y, v0, c[%d]\n"
			"dp4 oPos.z, v0, c[%d]\n"
			"dp4 oPos.w, v0, c[%d]\n"
			"mov r10, v2\n",
			nglVSC_LocalToScreen + 0, nglVSC_LocalToScreen + 1,
			nglVSC_LocalToScreen + 2, nglVSC_LocalToScreen + 3);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Transform a skinned vertex.
-----------------------------------------------------------------------------*/
void nglvsf_skin_transform()
{
	sprintf(nglVSBuf,
			// Load 1st index.
			";---- vsf_skin_transform\n"
			"mad r4, v5, %s, %s\n" "mov a0.x, r4.x\n"
			// Transform position by 1st bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 1st bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n" "dp3 r3.z, v2, c[a0.x+2]\n"
			// Apply weights to position and normal.
			"mul r0.xyz, r2.xyz, v6.x\n" "mul r1.xyz, r3.xyz, v6.x\n"
			// Load 2nd index.
			"mov a0.x, r4.y\n"
			// Transform position by 2nd bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 2nd bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.y, r0.xyz\n"
			"mad r1.xyz, r3.xyz, v6.y, r1.xyz\n"
			// Load 3rd index.
			"mov a0.x, r4.z\n"
			// Transform position by 3rd bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 3rd bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.z, r0.xyz\n"
			"mad r1.xyz, r3.xyz, v6.z, r1.xyz\n"
			// Load 4th index.
			"mov a0.x, r4.w\n"
			// Transform position by 4th bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 4th bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.w, r0.xyz\n"
			"mad r10.xyz, r3.xyz, v6.w, r1.xyz\n"
			"mov r0.w, %s\n"
			"dp4 oPos.x, r0, c[%d]\n"
			"dp4 oPos.y, r0, c[%d]\n"
			"dp4 oPos.z, r0, c[%d]\n"
			"dp4 oPos.w, r0, c[%d]\n",
			NGL_VSC_BONES_SCALE, NGL_VSC_BONES_OFS,
			NGL_VSC_ONE,
			nglVSC_LocalToScreen + 0, nglVSC_LocalToScreen + 1,
			nglVSC_LocalToScreen + 2, nglVSC_LocalToScreen + 3);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Calculate per vertex fog.

NGL model:
Fog = nglFogMin + ( DistZ - nglFogNearZ ) / ( nglFogFarZ - nglFogNearZ ) * ( nglFogMax - nglFogMin )

With: c[nglVSC_FogParams].x = nglFogMin
      c[nglVSC_FogParams].y = nglFogNearZ
      c[nglVSC_FogParams].z = 1 / ((nglFogFarZ - nglFogNearZ) * (nglFogMax - nglFogMin))
-----------------------------------------------------------------------------*/
void nglvsf_fog()
{
	sprintf(nglVSBuf,
			";---- vsf_fog\n"
			"#define FogMin     c[%d].x\n"
			"#define FogNearZ   c[%d].y\n" "#define FogCoeff   c[%d].z\n"
			// oPos is mapped into r12 - thus it is possible to read the position :)
			"add r0, r12.z, -FogNearZ\n"
			"mov r1, FogMin\n"
			"mad r0, r0, FogCoeff, r1\n"
			"add oFog, %s, -r0\n",
			nglVSC_FogParams, nglVSC_FogParams, nglVSC_FogParams,
			NGL_VSC_ONE);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Set the fog register to 1 (aka no fog).
-----------------------------------------------------------------------------*/
void nglvsf_nofog()
{
	sprintf(nglVSBuf, ";---- vsf_nofog\n" "mov oFog, %s\n", NGL_VSC_ONE);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Setup the map UVs.
-----------------------------------------------------------------------------*/
void nglvsf_map(uint32 Stage)
{
	sprintf(nglVSBuf, ";---- vsf_map\n" "mov oT%d.xy, v3\n", Stage);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Setup and scroll the map UVs.
-----------------------------------------------------------------------------*/
void nglvsf_map_scrolluv(uint32 Stage)
{
	sprintf(nglVSBuf,
			";---- vsf_map_scrolluv\n"
			"add oT%d.xy, v3, c[%d].xy\n", Stage, nglVSC_UVParams);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Setup the detailmap UVs.
-----------------------------------------------------------------------------*/
void nglvsf_detailmap(uint32 Stage)
{
	sprintf(nglVSBuf,
			";---- vsf_detailmap\n"
			"mul oT%d.xy, v3, c[%d].zw\n", Stage, nglVSC_UVParams);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Calculate spherical enviro map UVs.
-----------------------------------------------------------------------------*/
#if NGL_USE_SPHERICAL_ENVMAP

void nglvsf_enviromap(uint32 Stage)
{
	sprintf(nglVSBuf,
			";---- vsf_sphere_enviromap\n"
			"#define R_REFLECTION r0\n"
			"#define R_WORLDSPACE_NORMAL r1\n"
			"#define R_EYE_VECTOR r2\n"
			"#define R_DOT2 r3\n" "#define R_TEMP r4\n"
			// Transform vertex into world space (in r0)
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n"
			"dp4 r0.z, v0, c[%d]\n" "dp4 r0.w, v0, c[%d]\n"
			// Transform normal to world space
			"dp3 R_WORLDSPACE_NORMAL.x, r10, c[%d]\n"
			"dp3 R_WORLDSPACE_NORMAL.y, r10, c[%d]\n"
			"dp3 R_WORLDSPACE_NORMAL.z, r10, c[%d]\n"
			// Need to re-normalize normal
			"dp3 R_WORLDSPACE_NORMAL.w, R_WORLDSPACE_NORMAL, R_WORLDSPACE_NORMAL\n"
			"rsq R_WORLDSPACE_NORMAL.w, R_WORLDSPACE_NORMAL.w\n"
			"mul R_WORLDSPACE_NORMAL, R_WORLDSPACE_NORMAL, R_WORLDSPACE_NORMAL.w\n"
			// Get vector from eye to vertex
			"sub R_TEMP, r0, c[%d]\n"
			"dp3 R_EYE_VECTOR.w, R_TEMP, R_TEMP\n"
			"rsq R_EYE_VECTOR.w, R_EYE_VECTOR.w\n"
			"mul R_EYE_VECTOR, R_TEMP, R_EYE_VECTOR.w\n"
			// Calculate reflection vector: E - 2 * (E dot N) * N 
			"dp3 R_DOT2, R_EYE_VECTOR, R_WORLDSPACE_NORMAL\n"
			"add R_DOT2, R_DOT2, R_DOT2\n"
			"mad R_REFLECTION.xyz, R_WORLDSPACE_NORMAL, -R_DOT2, R_EYE_VECTOR\n"
			// Calculate m = 1 / ( 2 * sqrt(REFL.x^2 + REFL.y^2 + (REFL.z + 1)^2) )
			"add R_REFLECTION.z, R_REFLECTION.z, %s\n"
			"dp3 r1, R_REFLECTION, R_REFLECTION\n"
			"rsq r2.x, r1.x\n" "mul r1.x, r2.x, %s\n"
			// U = REFL.x * m + 0.5
			// V = REFL.y * m + 0.5
			"mad r2.x, R_REFLECTION.x, r1.x, %s\n"
			"mad r2.y, R_REFLECTION.y, r1.x, %s\n"
			"sub r2.y, %s, r2.y\n"
			"mov oT%d.xy, r2.xy\n",
			nglVSC_LocalToWorld + 0, nglVSC_LocalToWorld + 1,
			nglVSC_LocalToWorld + 2, nglVSC_LocalToWorld + 3,
			nglVSC_LocalToWorld + 0, nglVSC_LocalToWorld + 1,
			nglVSC_LocalToWorld + 2, nglVSC_CameraPos, NGL_VSC_ONE,
			NGL_VSC_HALF, NGL_VSC_HALF, NGL_VSC_HALF, NGL_VSC_ONE, Stage);

	strcat(nglVSSrc, nglVSBuf);
}

#else

/*-----------------------------------------------------------------------------
Description: Calculate cubic enviro map UVs.
-----------------------------------------------------------------------------*/
void nglvsf_enviromap(uint32 Stage)
{
	sprintf(nglVSBuf,
			";---- vsf_sphere_enviromap\n"
			"#define R_REFLECTION r0\n"
			"#define R_WORLDSPACE_NORMAL r1\n"
			"#define R_EYE_VECTOR r2\n"
			"#define R_DOT2 r3\n" "#define R_TEMP r4\n"
			// Transform vertex into world space (in r0)
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n"
			"dp4 r0.z, v0, c[%d]\n" "dp4 r0.w, v0, c[%d]\n"
			// Transform normal to world space
			"dp3 R_WORLDSPACE_NORMAL.x, r10, c[%d]\n"
			"dp3 R_WORLDSPACE_NORMAL.y, r10, c[%d]\n"
			"dp3 R_WORLDSPACE_NORMAL.z, r10, c[%d]\n"
			// Need to re-normalize normal
			"dp3 R_WORLDSPACE_NORMAL.w, R_WORLDSPACE_NORMAL, R_WORLDSPACE_NORMAL\n"
			"rsq R_WORLDSPACE_NORMAL.w, R_WORLDSPACE_NORMAL.w\n"
			"mul R_WORLDSPACE_NORMAL, R_WORLDSPACE_NORMAL, R_WORLDSPACE_NORMAL.w\n"
			// Get vector from eye to vertex
			"sub R_TEMP, r0, c[%d]\n"
			"dp3 R_EYE_VECTOR.w, R_TEMP, R_TEMP\n"
			"rsq R_EYE_VECTOR.w, R_EYE_VECTOR.w\n"
			"mul R_EYE_VECTOR, R_TEMP, R_EYE_VECTOR.w\n"
			// Calculate reflection vector: E - 2 * (E dot N) * N 
			"dp3 R_DOT2, R_EYE_VECTOR, R_WORLDSPACE_NORMAL\n"
			"add R_DOT2, R_DOT2, R_DOT2\n"
			"mad R_REFLECTION.xyz, R_WORLDSPACE_NORMAL, -R_DOT2, R_EYE_VECTOR\n"
			// For some reasons (?), the x component must be reversed (grmblbl, waste 2 instructions!).
			"mov R_REFLECTION.x, -R_REFLECTION.x\n"
			"mov oT%d.xyz, R_REFLECTION.xyz\n",
			nglVSC_LocalToWorld + 0, nglVSC_LocalToWorld + 1,
			nglVSC_LocalToWorld + 2, nglVSC_LocalToWorld + 3,
			nglVSC_LocalToWorld + 0, nglVSC_LocalToWorld + 1,
			nglVSC_LocalToWorld + 2, nglVSC_CameraPos, Stage);

	strcat(nglVSSrc, nglVSBuf);
}

#endif

/*-----------------------------------------------------------------------------
Description: Setup lightmap UVs.
-----------------------------------------------------------------------------*/
void nglvsf_lightmap(uint32 Stage)
{
	sprintf(nglVSBuf, ";---- vsf_lightmap\n" "mov oT%d.xy, v4\n", Stage);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Setup the vertex color register and eventually multiply it by
             the TINT value.
-----------------------------------------------------------------------------*/
void nglvsf_color(bool Tint)
{
	if (Tint)
		sprintf(nglVSBuf,
				";---- vsf_color_tint\n" "mul " NGL_OUTPUT_VERTEX_COLOR ", v1, c[%d]\n", nglVSC_Tint);
	else
		sprintf(nglVSBuf,
				";---- vsf_color_notint\n"
				"mov " NGL_OUTPUT_VERTEX_COLOR ", v1\n" "mov oD1, c[%d]\n", nglVSC_MaterialColor);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Start the lighting code (with/without TINT).
-----------------------------------------------------------------------------*/
void nglvsf_light_begin(bool Tint)
{
	char Buf[] =
		// Normalize normal vector in r2
		// and stores 1/length(normal) in r1.
		// r2 and r1 are used by the vsf_light_add_xxx() routines.
		";---- vsf_light_begin\n"
		"dp3 r1, r10, r10\n" "rsq r1.x, r1.x\n" "mul r2, r1.xxx, r10.xyz\n";

	strcat(nglVSSrc, Buf);

	if (Tint)
		sprintf(nglVSBuf, "mul r11, v1, c[%d]\n", nglVSC_Tint);
	else
		sprintf(nglVSBuf, "mov r11, v1\n");

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Add a directional light to the light code.
-----------------------------------------------------------------------------*/
void nglvsf_light_add_directional()
{
	sprintf(nglVSBuf, ";---- vsf_light_add_dir\n"
			// Calculate cosine of the angle between L and N (aka -L dot N).
			// Normal is in r2 (computed in vsf_light_begin) and r1 = 1/length(normal)
			"dp3 r0, c[%d], -r2\n" "mul r0, r0, r1.xxxx\n"
			// Clamping from [-1,1] to [0,1].
			"max r0, r0, %s\n"
			// Color after tint (or previous light) is in r11.
			// VertexColor = VertexColor + (LightCol * Dot)
			"mad r11, c[%d], r0, r11\n",
			nglVSC_Light[nglVSCurLight].Dir,
			NGL_VSC_ZERO, nglVSC_Light[nglVSCurLight].Col);

	strcat(nglVSSrc, nglVSBuf);

	nglVSCurLight++;
}

/*-----------------------------------------------------------------------------
Description: Add a point light to the light code.
             -NOT IMPLEMENTED YET-
-----------------------------------------------------------------------------*/
void nglvsf_light_add_point()
{
}

/*-----------------------------------------------------------------------------
Description: Finish the lighting code (output the result to the color register).
-----------------------------------------------------------------------------*/
void nglvsf_light_end()
{
	sprintf(nglVSBuf, "mov " NGL_OUTPUT_VERTEX_COLOR ", r11\n");

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Directional projector light code for unskinned transform.
-----------------------------------------------------------------------------*/
void nglvsf_1projector_dir()
{
	sprintf(nglVSBuf, ";---- vsf_projector_dir\n"
			// Transform the vertex position.
			"dp4 oPos.x, v0, c[%d]\n"
			"dp4 oPos.y, v0, c[%d]\n"
			"dp4 oPos.z, v0, c[%d]\n" "dp4 oPos.w, v0, c[%d]\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT0.xy, r0.xy, c[%d].xx\n"
			// Normalize normal vector in r2.
			"dp3 r2.w, v2, v2\n" "rsq r2.w, r2.w\n" "mul r2, v2, r2.w\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r0.x, c[%d], -r2\n"
			"max r0.x, r0.x, %s\n"
			"mul " NGL_OUTPUT_VERTEX_COLOR ".a, r0.x, %s\n",
			nglVSC2_Projector_LocalToScreen + 0,
			nglVSC2_Projector_LocalToScreen + 1,
			nglVSC2_Projector_LocalToScreen + 2,
			nglVSC2_Projector_LocalToScreen + 3,
			nglVSC2_Projector[0].Local2UV + 0,
			nglVSC2_Projector[0].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[0].Dir, NGL_VSC_ZERO, NGL_VSC_SHADOW_FADING);

	strcat(nglVSSrc, nglVSBuf);
}

void nglvsf_2projector_dir()
{
	sprintf(nglVSBuf, ";---- vsf_projector_dir\n"
			// Transform the vertex position.
			"dp4 oPos.x, v0, c[%d]\n"
			"dp4 oPos.y, v0, c[%d]\n"
			"dp4 oPos.z, v0, c[%d]\n" "dp4 oPos.w, v0, c[%d]\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT0.xy, r0.xy, c[%d].xx\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT1.xy, r0.xy, c[%d].yy\n"
			// Normalize normal vector in r2.
			"dp3 r2.w, v2, v2\n" "rsq r2.w, r2.w\n" "mul r2, v2, r2.w\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.x, c[%d], -r2\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.y, c[%d], -r2\n"
			"max r0.xy, r1.xy, %s\n"
			"mul " NGL_OUTPUT_VERTEX_COLOR ".a, r0.x, %s\n"
			"mul " NGL_OUTPUT_VERTEX_COLOR ".b, r0.y, %s\n",
			nglVSC2_Projector_LocalToScreen + 0,
			nglVSC2_Projector_LocalToScreen + 1,
			nglVSC2_Projector_LocalToScreen + 2,
			nglVSC2_Projector_LocalToScreen + 3,
			nglVSC2_Projector[0].Local2UV + 0,
			nglVSC2_Projector[0].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[1].Local2UV + 0,
			nglVSC2_Projector[1].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[0].Dir, nglVSC2_Projector[1].Dir, NGL_VSC_ZERO,
			NGL_VSC_SHADOW_FADING, NGL_VSC_SHADOW_FADING);

	strcat(nglVSSrc, nglVSBuf);
}

void nglvsf_3projector_dir()
{
	sprintf(nglVSBuf, ";---- vsf_projector_dir\n"
			// Transform the vertex position.
			"dp4 oPos.x, v0, c[%d]\n"
			"dp4 oPos.y, v0, c[%d]\n"
			"dp4 oPos.z, v0, c[%d]\n" "dp4 oPos.w, v0, c[%d]\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT0.xy, r0.xy, c[%d].xx\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT1.xy, r0.xy, c[%d].yy\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT2.xy, r0.xy, c[%d].zz\n"
			// Normalize normal vector in r2.
			"dp3 r2.w, v2, v2\n" "rsq r2.w, r2.w\n" "mul r2, v2, r2.w\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.x, c[%d], -r2\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.y, c[%d], -r2\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.z, c[%d], -r2\n"
			"max r0.xyz, r1.xyz, %s\n"
			"mul " NGL_OUTPUT_VERTEX_COLOR ".a, r0.x, %s\n"
			"mul " NGL_OUTPUT_VERTEX_COLOR ".b, r0.y, %s\n"
			"mul oD1.a, r0.z, %s\n",
			nglVSC2_Projector_LocalToScreen + 0,
			nglVSC2_Projector_LocalToScreen + 1,
			nglVSC2_Projector_LocalToScreen + 2,
			nglVSC2_Projector_LocalToScreen + 3,
			nglVSC2_Projector[0].Local2UV + 0,
			nglVSC2_Projector[0].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[1].Local2UV + 0,
			nglVSC2_Projector[1].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[2].Local2UV + 0,
			nglVSC2_Projector[2].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[0].Dir, nglVSC2_Projector[1].Dir,
			nglVSC2_Projector[2].Dir, NGL_VSC_ZERO, NGL_VSC_SHADOW_FADING,
			NGL_VSC_SHADOW_FADING, NGL_VSC_SHADOW_FADING);

	strcat(nglVSSrc, nglVSBuf);
}

void nglvsf_4projector_dir()
{
	sprintf(nglVSBuf, ";---- vsf_projector_dir\n"
			// Transform the vertex position.
			"dp4 oPos.x, v0, c[%d]\n"
			"dp4 oPos.y, v0, c[%d]\n"
			"dp4 oPos.z, v0, c[%d]\n" "dp4 oPos.w, v0, c[%d]\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT0.xy, r0.xy, c[%d].xx\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT1.xy, r0.xy, c[%d].yy\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT2.xy, r0.xy, c[%d].zz\n"
			// Transform the UV coordinates.
			"dp4 r0.x, v0, c[%d]\n"
			"dp4 r0.y, v0, c[%d]\n" "mul oT3.xy, r0.xy, c[%d].ww\n"
			// Normalize normal vector in r2.
			"dp3 r2.w, v2, v2\n" "rsq r2.w, r2.w\n" "mul r2, v2, r2.w\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.x, c[%d], -r2\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.y, c[%d], -r2\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.z, c[%d], -r2\n"
			// Calculate dot(-LightDir, N). Clamping to [0,1].
			"dp3 r1.w, c[%d], -r2\n"
			"max r0.xyzw, r1.xyzw, %s\n"
			"mul " NGL_OUTPUT_VERTEX_COLOR ".a, r0.x, %s\n"
			"mul " NGL_OUTPUT_VERTEX_COLOR ".b, r0.y, %s\n"
			"mul oD1.a, r0.z, %s\n"
			"mul oD1.b, r0.w, %s\n",
			nglVSC2_Projector_LocalToScreen + 0,
			nglVSC2_Projector_LocalToScreen + 1,
			nglVSC2_Projector_LocalToScreen + 2,
			nglVSC2_Projector_LocalToScreen + 3,
			nglVSC2_Projector[0].Local2UV + 0,
			nglVSC2_Projector[0].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[1].Local2UV + 0,
			nglVSC2_Projector[1].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[2].Local2UV + 0,
			nglVSC2_Projector[2].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[3].Local2UV + 0,
			nglVSC2_Projector[3].Local2UV + 1, nglVSC2_Projector_TexScale,
			nglVSC2_Projector[0].Dir, nglVSC2_Projector[1].Dir,
			nglVSC2_Projector[2].Dir, nglVSC2_Projector[3].Dir, NGL_VSC_ZERO,
			NGL_VSC_SHADOW_FADING, NGL_VSC_SHADOW_FADING,
			NGL_VSC_SHADOW_FADING, NGL_VSC_SHADOW_FADING);

	strcat(nglVSSrc, nglVSBuf);
}

/*-----------------------------------------------------------------------------
Description: Directional projector light code for skinned transform.
WARNING:     This VS must be used on the 2nd pass thus it can't be "glued" to
             the other ones (except vsf_begin and vsf_end) !
-----------------------------------------------------------------------------*/
void nglvsf_projector_dir_skin()
{
	sprintf(nglVSBuf,
			";---- vsf_projector_dir_skin\n" ";---- transform vertex\n"
			// Transform the vertex position.
			"mad r4, v5, %s, %s\n" "mov a0.x, r4.x\n"
			// Transform position by 1st bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 1st bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n" "dp3 r3.z, v2, c[a0.x+2]\n"
			// Apply weights to position and normal.
			"mul r0.xyz, r2.xyz, v6.x\n" "mul r1.xyz, r3.xyz, v6.x\n"
			// Load 2nd index.
			"mov a0.x, r4.y\n"
			// Transform position by 2nd bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 2nd bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.y, r0.xyz\n"
			"mad r1.xyz, r3.xyz, v6.y, r1.xyz\n"
			// Load 3rd index.
			"mov a0.x, r4.z\n"
			// Transform position by 3rd bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 3rd bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.z, r0.xyz\n"
			"mad r1.xyz, r3.xyz, v6.z, r1.xyz\n"
			// Load 4th index.
			"mov a0.x, r4.w\n"
			// Transform position by 4th bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 4th bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.w, r0.xyz\n"
			"mov r0.w, %s\n"
			"dp4 oPos.x, r0, c[%d]\n"
			"dp4 oPos.y, r0, c[%d]\n"
			"dp4 oPos.z, r0, c[%d]\n"
			"dp4 oPos.w, r0, c[%d]\n" ";---- transform UV\n"
			// Transform the vertex position.
			"mad r4, v5, %s, %s\n" "mov a0.x, r4.x\n"
			// Transform position by 1st bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 1st bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n" "dp3 r3.z, v2, c[a0.x+2]\n"
			// Apply weights to position and normal.
			"mul r0.xyz, r2.xyz, v6.x\n" "mul r1.xyz, r3.xyz, v6.x\n"
			// Load 2nd index.
			"mov a0.x, r4.y\n"
			// Transform position by 2nd bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 2nd bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.y, r0.xyz\n"
			"mad r1.xyz, r3.xyz, v6.y, r1.xyz\n"
			// Load 3rd index.
			"mov a0.x, r4.z\n"
			// Transform position by 3rd bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 3rd bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.z, r0.xyz\n"
			"mad r1.xyz, r3.xyz, v6.z, r1.xyz\n"
			// Load 4th index.
			"mov a0.x, r4.w\n"
			// Transform position by 4th bone.
			"dp4 r2.x, v0, c[a0.x+0]\n"
			"dp4 r2.y, v0, c[a0.x+1]\n" "dp4 r2.z, v0, c[a0.x+2]\n"
			// Transform normal by 4th bone.
			"dp3 r3.x, v2, c[a0.x+0]\n"
			"dp3 r3.y, v2, c[a0.x+1]\n"
			"dp3 r3.z, v2, c[a0.x+2]\n"
			"mad r0.xyz, r2.xyz, v6.w, r0.xyz\n"
			"mov r0.w, %s\n"
			"dp4 r1.x, r0, c[%d]\n"
			"dp4 r1.y, r0, c[%d]\n" "mul oT0.xy, r1.xy, c[%d].xx\n"
			// Assuming normalized normals.
			// Calculate cosine of the angle between L and N (aka -L dot N).
			"dp3 r0, c[%d], -v2\n"
			// Clamping from [-1,1] to [0,1].
			"max r0, r0, %s\n"
			// VertexColor = VertexColor * Dot
			"mul " NGL_OUTPUT_VERTEX_COLOR ", v1, r0\n",
			NGL_VSC_BONES_SCALE, NGL_VSC_BONES_OFS,
			NGL_VSC_ONE,
			NGL_VS_FIRST_FREE_REG + 0, NGL_VS_FIRST_FREE_REG + 1,
			NGL_VS_FIRST_FREE_REG + 2, NGL_VS_FIRST_FREE_REG + 3,
			NGL_VSC_BONES_SCALE, NGL_VSC_BONES_OFS,
			NGL_VSC_ONE,
			NGL_VS_FIRST_FREE_REG + 4, NGL_VS_FIRST_FREE_REG + 5,
			NGL_VS_FIRST_FREE_REG + 6, NGL_VS_FIRST_FREE_REG + 7,
			NGL_VSC_ZERO);

	strcat(nglVSSrc, nglVSBuf);
}

/*==================================================================================================
  Usefull vertex shader functions.
----------------------------------------------------------------------------------------------------
1) r0 = |r1|
MAX r0, r1, -r1
 
2) r0.x = r1.x/r2.x
RCP r0.x, r2.x
MUL r0.x, r1.x, r0.x 

3) r0.x = sqrt(r1.x)
RSQ r0.x, r1.x		    ; using x/sqrt(x) = sqrt(x) is higher
MUL r0.x, r0.x, r1.x	; precision than 1/(1/sqrt(x))

4) r0 = (r1 <= r2) ? 1 : 0
SGE r0, -r1, -r2

5) r0 = (r1 r2) ? 1 : 0
SLT r0, -r1, -r2

6) r0 = (r1 == r2) ? 1 : 0
SGE r0, -r1, -r2
SGE r2,  r1,  r2
MUL r0,  r0,  r2

7) r0 = (r1 != r2) ? 1 : 0
SLT r0,  r1,  r2
SLT r2, -r1, -r2
ADD r0,  r0,  r2

8) r0 = (r0 < 0) ? 0 : (r0 1) ? 1 : r0
DEF c0, 0.0f, 1.0f, 0.0f, 0.0f
MAX r0, r0, c0.x
MIN r0, r0, c0.y

9) r0.y = floor(r1.y)
EXPP r0.y, r1.y
ADD  r0.y, r1.y, - r0.y

10) r0.y = ceiling(r1.y)
EXPP r0.y, -r1.y
ADD  r0.y, r1.y, r0.y	

11) r0 = r1 x r2 (3-vector cross-product)
MUL r0,  r1.yzxw, r2.zxyw
MAD r0, -r2.yzxw, r1.zxyw, r0

12) c0-c3 is matrix to transpose and multiply r1 with
MUL r0, c0, r1.x
MAD r0, c1, r1.y, r0
MAD r0, c2, r1.z, r0
MAD r0, c3, r1.w, r0

13) r0 = (r1 >= r2) ? r3 : r4
SGE r0, r1,  r2		  ; one if (r1 >= r2) holds, zero otherwise
ADD r1, r3, -r4		
MAD r0, r0, r1, r4	; r0 = r0*(r3-r4) + r4 = r0*r3 + (1-r0)*r4 

14)        1 if (r0  > 0)
     r0 =  0 if (r0 == 0)
          -1 if (r0  < 0)
DEF c0, 0.0f, 0.0f, 0.0f, 0.0f 
SLT r1,  r0,  c0
SLT r0, -r0,  c0
ADD r0,  r0, -r1

15) Normalize r0
DP3 r0.w, r0, r0
RSQ r0.w, r0.w
MUL r0, r0, r0.w

==================================================================================================*/
