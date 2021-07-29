/*--------------------------------------------------------------------------------
ngl_version.h  -  Release notes
XBox NGL
--------------------------------------------------------------------------------*/

#ifndef NGL_VERSION_H
#define NGL_VERSION_H

#define NGL_VERSION       "1.7.5x"
#define NGL_VERSION_DATE  "05/29/2002"

/*--------------------------------------------------------------------------------
New features / bug fixes
----------------------------------------------------------------------------------

[Version 1.7.5x - 05/29/02]
- Changes to support custom water node in KS.  Mostly #ifdef PROJECT_KELLYSLATER + 
  a few bug fixes.  (dc 05/29/02)

[Version 1.7.4x - 05/14/02]
- Added support for XPR texture format.

[Version 1.7.3x - 05/13/02]
- Temporary changes for NHL2K (E3).

[Version 1.7.2x - 05/11/02]
- Definitely fixed the scratch mesh issues (!), but need rewrite for speed.
- Removed the NGL_ASSERT when trying to load a texture with an empty filename
  (nglWarning instead).
- Point filtering is used when no filtering flag is specified (meshes only).
- Small changes to the quad/font's filtering API.

[Version 1.7.1x - 05/10/02]
- Added nglSetFontBlendMode.

[Version 1.7.0x - 05/10/02]
- New working branch.  No new features.

[Version 1.3.146x - 05/09/02]
- Fixed: nglEditMesh() works now, but it is pretty inefficient as it doesn't use
  any double buffering mechanism (CPU is waiting during the lock of the static
  scratch mesh vertex buffer).
- Fixed: scratch mesh code has been rewritten to fix the stripping bugs/indices
  bugs. This implementation should be slower than the previous one.

[Version 1.3.145x - 05/07/02]
- Added a special set of flags for quads and fonts. The client can now specify
  some filtering options per quad/font (see NGLMAP_xxx in ngl_xbox.h for more
  details). A few functions have been added for this purpose:
  * void nglSetQuadMapFlags(nglQuad *Quad, uint32 MapFlags);
  * uint32 nglGetQuadMapFlags(nglQuad *Quad);
  * void nglSetFontMapFlags(nglFont *Font, uint32 MapFlags);
  * uint32 nglGetFontMapFlags(nglFont *Font);
- Put back the version API (nglGetVersion, nglGetVersionDate).
- Made nglWarning, nglError, nglInfo and nglLog available to the client.
- Removed the old scratch mesh API, so removed the NGL_NEW_SCRATCH_API define.
- Added independent font scaling in the x/y axis using the \3[SCALEX,SCALEY] token.
  (see ngl_xbox.h for more details).
- Fixed a bug in the GPU counters: they needed to be resetted when timings couldn't
  be read successfully.
- Fixed a bug in nglSetShaderConfig() where the current texture stage config was not
  cleared (potential bug if the new TS config has less bits than the previous one).
- Fixed a bug in nglListAddMesh() where the nglStage.Scratch debug flag wasn't
  processed properly.
- Fixed a bug in the static scratch mesh API. It was only working for 1 (!) static
  scratch mesh and furthermore it had to be created first in the scratch mesh render
  list.
- Fixed a bug in the compatibility scratch mesh API: nglCreateScratchMesh() now
  should work fine.

[Version 1.3.144x - 05/02/02]
- Removed the version API (nglGetVersion(), nglGetVersionDate(); ngl_version.txt has
  been renamed into ngl_version.h which defines NGL_VERSION and NGL_VERSION_DATE, so
  this is the only place where to increment the version number/date.
- nglMatrix and nglVector are now defined as a class for more flexibility (the same
  as the PS2 and GC). In the future I'll move to Sean Palmer's math library.
- Added the XB suffix to some XBox specific function's names.
- Modified the nglMesh and nglMeshSection structure to store the VertexSize in the
  section instead of storing it in the mesh (required for the future scratch mesh's
  implementation using different vertex formats).
- Changed Light Context API to deal w/a pointer to a structure rather than a uint;
  Added nglSetLightContext and nglGetLightContext functions.
- Temporary texture filtering changes to satisfy the NHL team: if the project is
  Kelly Slater then the texture filtering is implemented as below:
  1. If NGLMAT_BILINEAR is set, then use a bilinear filter.
  2. If NGLMAT_TRILINEAR is set, then use an anisotropic filter.
  3. If nothing is set, then use the point filter.
  Otherwise:
  1. If NGLMAT_TRILINEAR is set, then use an anisotropic filter.
  2. Otherwise use bilinear filtering.
  Basically, in the first case if nothing is specified it doesn't filter the texture
  and in the 2nd case it uses bilinear filtering.
  This is just a temporary fix and the first version will be the default behaviour
  very soon ! Work in progress: way to set the filtering mode for quads & fonts.
- Clear the 2 backbuffers and the frontbuffer in nglInit() to avoid having
  some garbage displayed after the first nglListSend(true) call.
- Fixed: Changed the subtractives modes to match the PS2 implementation.
- Fixed a bug in the backface culling caching system.
- Fixed a bug in the nglListAddString() function.

[Version 1.3.143x - 04/30/02]
- Reworked nglMatrix declarations (what was there was a hack and was causing problems).
  nglMatrix is now an aligned type instead of a #define that included the align
  declaration.  nglMatrix arguments must be passed by reference, not by value.
  This would be necessary if nglMatrix were a class, and is also necessary when
  declaring its alignment.  Reworked all ngl function declarations to use nglMatrix &.

[Version 1.3.142x - 04/22/02]
- Added per scene 'void nglSetZTestEnable(bool Enable)'.

[Version 1.3.141x - 04/18/02]
- Scratch mesh extension (XBox specific for now): added a third parameter which
  specify the primitive type to the nglMeshAddSection function (see NGLPRIM_xxx
  in ngl_xbox.h to see which primitive types are supported). In the past, only
  triangle strip was supported (still true for GC and PS2).
  Use nglMeshWritePrimitive() instead of nglMeshWriteStrip() which only works
  with triangle strips (and will be removed in the future).
- Removed the texture filtering fields from nglTexture and set them at runtime
  according to the material flags.
- Fixed a bug in the nglListAddString(nglFont *Font, float x, float y, float z, const char *Text, ...)
  where the parameters after the string were ignored.
- Fixed a crash that was occuring in the pixel shader setup when 4 texture
  stages were used.
- Fixed a bug in nglSetPerspectiveMatrix() were the WorldToScreen matrix was not
  recomputed.

[Version 1.3.140x - 04/10/02]
- Completely removed the particle system stuff (some declarations were still
  remaining in the header file).

[Version 1.3.139x - 04/10/02]
1. Added a parameter to nglMemAlloc to gives the possibility to allocate write-
   combined contiguous memory (required for pushbuffer memory allocation).
2. Removed KS' particle system. Very easy to implement on the client side due to
   the new custom render API.
3. "Pushbufferized" the DrawIndexedVertices() call for faster rendering (strangely
   it doesn't seem to be faster [?]). Set NGL_USE_PUSHBUFFER to enable it.
1. Fix: fixed the FOV calculations and the aspect ratio.

[Version 1.3.138x - 04/09/02]
1. Added a Font compatibility API to ease transition to the new API. Also useful
   until the GameCube supports the new Font API.  It isn't 100% compatible, but
   close enough to make things easy.

[Version 1.3.137x - 04/08/02]
1. Fix: oops, forgot to initialize Scene->ZWriteEnable to true.

[Version 1.3.136x - 04/08/02]
1. Added nglSetZWriteEnable(bool Enable) which works as the following:
   ZWrites is enabled ONLY if nglScene.ZWriteEnable is true (so, it only affect
   opaque and punchthrough modes).
2. Added void nglListAddString(nglFont *Font, float x, float y, float z, uint32 Color, const char *Text, ...);
   To set the default text color via a new argument.
3. Fixed a bug (introduced in 1.3.135x) in the texturing when rendering quads.

[Version 1.3.135x - 04/08/02]
1. Added two parameters in nglTexture: mipmap and magnification filters. For
   optimization reasons, they are set at loading time (in nglProcessMesh). In
   the past, texture filtering was done at runtime (in nglBindTexture).
2. Much better pixel shader support (and slightly faster as well). Now, adding
   some new internal blending mode combo or per pixel effects is much easier.
3. Updated nglUnrebaseMesh() to restore the original texture flags.

[Version 1.3.134x - 04/08/02]
1. Converted the sources to the new coding standard (tabs with size = 4).

[Version 1.3.133x - 04/05/02]
1. Fixed nglMeshAddSection to set RenderSectionFunc.  Was causing a fault with call to
   NodeFn at location 0.
2. Fixed nglMeshWriteVertexPNUV2 to correctly set second uv into UV[1] instead of UV[0].

[Version 1.3.132x - 04/04/02]
1. Added new flags to allow the clearing of specific channels of the framebuffer
   (see NGLCLEAR_xxx in ngl_xbox.h).
2. Added a function, nglSetFBWriteMask(), which specify on which channel the
   geometry is rendered in the framebuffer (see NGL_FBWRITE_xxx in ngl_xbox.h).
3. Wrote a "Custom Render API" accessible through ngl_xbox_internal.h to allow
   users to override the default NGL internal rendering pipeline. It includes:
   * Functions to create their own pixel/vertex shaders directly in ASM:
     - void nglCreatePixelShader(nglShader* PS, const char* SrcCode, ...);
	 - void nglCreateVertexShader(const DWORD* Decl, nglShader* VS, bool WTransform, const char* SrcCode, ...);
     - void nglReleaseShader(nglShader* shader);
   * Functions to override the internal NGL section and quad rendering:
     - void nglSetSectionRenderer(nglMeshSection* Section, void (*RenderSectionFunc)(void* Data), nglShader* VS);
     - void nglSetQuadRenderer(nglQuad* Quad, void (*RenderQuadFunc)(void* Data), nglShader* VS);
   * Functions to set their pixel/vertex shaders (cached):
     - void nglSetPixelShader(DWORD PSHandle);
     - void nglSetVertexShader(DWORD VSHandle);
   * Functions to flush the internal NGL renderstates' cache:
    - void nglFlushBlendingMode();
    - void nglFlushBackFaceCulling();
    - void nglFlushVertexShader();
    - void nglFlushPixelShader();
   Look into ngl_xbox_internal.h, I put some documentation and I give a small
   example that show how to overrides the NGL internal section rendering.

[Version 1.3.131x - 04/03/02]
1. Added new scene api changes.  nglListBeginScene() now returns nglScene*.
   nglScene* nglListSelectScene(nglScene* scene) changes the currently selected
   scene to the specified scene, and returns the previously selected scene.
   Pointers to nglScene can be used by applications, but the internals are
   in ngl_xbox_internal.h.  class nglScene was previously called struct nglSceneInfo.
   The internal nglScene variable is now called nglCurScene.

[Version 1.3.130x - 04/01/02]
1. Fixed a bug in the scratch mesh strips creation.
2. Added automatic scaling of the UVs for quads when linear textures are used.

[Version 1.3.129x - 03/29/02]
1. Fixed a bug in nglListAddString() and nglGetStringDimensions().

[Version 1.3.128x - 03/29/02]
1. Renamed NGLASSERT to NGL_ASSERT and moved it from ngl_Assert.h into
   ngl_xbox_internal.h
2. Implemented the new font API (removed the old one). New functions:
   nglFont* nglLoadFont(const nglFixedString& FontName);
   nglFont* nglLoadFontInPlace(const nglFixedString& FontName, void* FDFdata);
   void nglReleaseFont(nglFont* Font);
   void nglReleaseAllFonts();
   void nglListAddString(nglFont* Font, float x, float y, float z, const char* Text, ...)
   void nglGetStringDimensions(nglFont* Font, uint32* Width, uint32* Height, const char* Text, ...)
   The NGL internal font (or system font) is accessible via nglSysFont (global).
   See ngl_xbox.h (cf. Font API) for more details.

[Version 1.3.127x - 03/27/02]
1. Implementation of the "new" scratch mesh API (static/temp scratch meshes).

[Version 1.3.126x - 03/26/02]
1. Fixed an alignment bug in the mesh loading code.

[Version 1.3.125x - 03/22/02]
1. Removed nglLockTexture/nglUnLockTexture, use the XBox specific function
   instead: nglLockTextureXB/nglUnlockTextureXB.
2. Added nglReleaseAllFonts(), nglGetStringDimensions().
3. Rewrote the font API rendering code.
1. Fix: nglLockParticlesVertexBuffer() was crashing.
2. Fix: was crashing if any scratch mesh was created before the first
   nglListSend call.

[Version 1.3.124x - 03/21/02]
1. User has access to nglMatrixTrans() and nglMatrixRot() [required by KS team].

[Version 1.3.123x - 03/21/02]
1. Removed PROJECT_SPIDERMAN, PROJECT_KELLYSLATER and PROJECT_MINORITYREPORT.
   Now, the project define must be defined in the project itself.
2. Integrate the particle system API from the KS team.
3. Made some changes required by the KS team (mainly undo of the rebase
   functions at mesh deallocation time).

[Version 1.3.122x - 03/20/02]
1. Added the NGL_USE_SPHERICAL_ENVMAP define. If defined, NGL will use spherical
   environment mapping instead of cubic one (requested by the spidey team...).
2. The new XBMESH 0x305 format is required.
3. Increased NGL_LIST_WORK_SIZE (1MB instead of 512KB).
4. Changed the NGL_SHADOW_FADING form 0.6f to 0.75f.
5. Improved the projected light code for more flexibility/extensibility.
6. Added a kind-of-crappy widescreen support (by changing the aspect ratio).
7. Made some modifications for the KS team (nglSetPerspectiveMatrix,
   nglRenderNodes)
1. Fix: Implemented new pixels shaders (tel3, tel3_mc) to support the "diffuse1
   (opaque) + enviro map (const blend) + diffuse2 (additive)" blending mode
   combination.
2. Fix: Updated the multitexture directional projected lights code to cast them
   correctly (in the past, only 1 direction was taking in account when casting
   several projector in the same pass). Updated the corresponding vertex shaders
   code as well.

[Version 1.3.121x - 03/08/02]
1. Removed the "int 0x3" in NGLASSERT, as it already calls nglFatal().
1. Fix: nglGetDebugFlagPtr() was not working for MatAlpha and MatLight.
2. Fix: nglSetCullingMode() was not working properly.
3. Fix: nglRenderSingleQuad() was not setting the backface culling mode. So,
   some quads couldn't have been rendered (kind of blinking artifacts).

[Version 1.3.120x - 03/06/02]
1. Removed the NGLTF_A8 texture format (cannot be used as a render target).

[Version 1.3.119x - 03/06/02]
1. Deep changes in the section's vertex shader key generation. Before rendering,
   the nodes are parsed to set the "dynamic" bits of the vshader key (lighting,
   NGLP_xxx, fog, etc.). So, the sorting is better compared to the previous
   version where only the partial key (no lighting nor NGLP_xxx, etc.) was used
   It implies that every dynamic rendering parameter (like lighting) has to be
   determined before sorting the nodes (because they are the hash function).
2. Added NGLP_REVERSE_BACKFACECULL to allow the client to reverse the backface
   culling (requested by the SPalmer team).
3. Updated nglMemAllocInternal/nglMemFreeInternal to match KS team requirement.
4. Updated nglCreateTexture to support several new texture formats (requested
   by NVL). See the NGLTF_xxx flags in ngl_xbox.h. When calling this function it
   is required to both specify if the tex is linear/swizzled AND the depth.
   By-the-way, it's better to use linear textures for shadows (UV are scaled
   automatically in the vshader).
5. Added some fields to the nglEnable (debug) structure: Fog, MatAlpha (to
   enable/disable materials that have NGLMAT_ALPHA flag and MatLight (the same
   for NGLMAT_LIGHT). It helps a lot for debugging lighting/shadows.

[Version 1.3.118x - 03/01/02]
1. Support the new XBMESH format 0x304 that is handling multiple vertex buffers
   per file.
2. Added a new optimization define: NGL_USE_NONSKIN_PUSHBUFFER. When set to 1,
   the render is creating a pushbuffer that store the DrawIndexVertices command
   at loading time. Not sure that the rendering is faster when using this flag.

[Version 1.3.117x - 02/27/02]
1. Fix: fixed a bug that appeared in version 1.3.116x where shadows were casted
   on materials that are translucent.

[Version 1.3.116x - 02/27/02]
1. Vertex shaders opcodes are stored in a static array instead of using dynamic
   memory (to prevent memory fragmentation).
2. Shadows change: remove the check to only cast shadows on materials that have
   the NGLMAT_LIGHT flag set (I think it's wrong but it fixes some spidey bugs).

[Version 1.3.115x - 02/26/02]
1. Removed the NGL_READWRITE_VS_FILE and added 2 client functions to avoid
   generating vshader at rendering time. Look at nglReadVShaderFile() and
   nglWriteVShaderFile() funtions in ngl_xbox.h for more details.
2. Optimization: implemented the "new" sorting code which is using the STL.

[Version 1.3.114x - 02/25/02]
1. Optimization: rewrote the vshader selection to directly access the right
   vshader (no more DB parsing/cache system - big speedup).
   Furthermore, the lights encoding has been changed to use less bits, so the
   vshader key size has been changed to 32-bits (instead of 64).
2. Optimization: added a define NGL_READWRITE_VS_FILE which tells the library to
   read a "vshader keys" file at initialization time and compile/create the
   corresponding vertex shaders. Moreover, when this flag is specified all the
   vshaders used by the game are appended to the "vshader keys" file. So, it
   allows the client to create a file that contains all the required vshaders and
   then compile them at initialization time (instead of doing it at runtime).

[Version 1.3.113x - 02/23/02]
1. Removed the NGL_SCRATCH_TEST code (experimental, was too slow).
2. Slightly modified the lock strategy for the scratch mesh VB.
3. Optimization: sections are now sorted by vertex shaders instead of beeing
   sorted by textures. Guess what ? That's a lot faster ! =:)

[Version 1.3.112x - 02/22/02]
1. Updated the nglSystemCallbackStruct struct to reflect the PS2 changes
   (added const to CriticalError and DebugPrint callbacks).
2. Fix: the nglProjectPoint() was not implemented correctly (z-value was wrong).
   I validated it in midnight and now it's correct.

[Version 1.3.111x - 02/21/02]
1. Hack: added a special pixel shader for Spidey style framed-windows rendering.
2. Added a detailed description (if nglDebug.InfoPrints is enabled) of each
   vshader that is compiled.
1. Fix: now when there is no base texture, the blending with the frame buffer is
   handled correctly (before it was always opaque).
2. Fix: the nglProjectPoint() must work now (implemented by the KS team).

[Version 1.3.110x - 02/20/02]
1. Fix: NGLBM_CONST_xxx are now implemented correctly for map (diffuse1).
2. Fix: Was using additive for lightmap instead of enviro when texture stage
   combination was map + enviro + lightmap.

[Version 1.3.109x - 02/20/02]
1. Created an ngl_xbox_internal.h include file to allow the client to implement
   custom nodes easily (by including this file).
2. Added a billboards vertex shader (similar to a quad except that it is in view
   space instead of screenspace) with its associated vertex format.
3. Removed the alpha test function nglSetAlphaTest().
4. Added a couple of functions to handle the VS database (clear, add entry).
5. When NGL_VS_POOL overflows, clear the VS database instead of asserting (crash
   proof but may introduce a slowdown).
6. Updated pixel shaders code for TEL/TL when using additive mode.
1. Fix: vertex/pixel shaders are now cached properly.
2. Fix: fixed a bug in the function ngl_PS_TEL() which was using the wrong pixel
   shader (bad index !).
3. Fix: nglCreateScratchMesh() overflow check.
4. Fix: IFL animation speed is finally synchronized with the VBlank.

[Version 1.3.108x - 02/16/02]
1. Updated some functions and added a constant to match the Kelly Slater team's
   requirements.
2. Get ride of the vertex shader dynamic register allocation (not really usefull
   afterall). VShader registers are fixed so it's now possible to precompile the
   most common vertex shaders in nglInit(), it should remove most of the jerks
   due to the compilation of vshaders at runtime. Also, create the most used
   (spidey) vertex shaders at NGL initialization.
3. Fix: Set the correct texture addressing modes: wrap for detail map and clamp
   for enviro map.
4. Fix: Shadows are now more realistic because their alpha component is
   multiplied by a factor < 1 (NGL_SHADOW_FADING) to prevent them beeing too
   dark/opaque.

[Version 1.3.107x - 02/15/02]
1. Fix: pixel shaders are processing correctly materials that have the 
   NGLMAT_MATERIAL_COLOR flag specified (only for non-skinned meshes).
2. Fix: nglFatal() for final build has now callback support.
3. Fix: Changed a bunch of nglFatal() to NGLASSERT or nglError() to minimize the
   risk of crashes.
4. Fix: nglCreateScratchMesh() now returns NGL_SCRATCH_MESH_INVALID when the
   scratch's list overflows (before it was asserting).
5. Fix: if the vshader database overflows, flush the cache entries and restart at
   index zero (introduce a slowdown but avoid crashing).

[Version 1.3.106x - 02/14/02]
1. Removed the recently added billboard API (I still have it somewhere on my
   harddisk).
2. Optimization: D3D device is created as "pure". It means that D3D dosen't keep
   some states internally, so it speedups a bit. But now we can't retrieve the
   status of renderstates, etc. (anyway, it wasn't used).
3. Removed the NGL_NOVEMBER_DEVKIT define, assuming that everybody has at least a
   november SDK.
4. Big projectors optimization: now they are rendered using multi-texturing, so
   we have up to 4 projectors per pass (instead of 1 proj/pass !). Also added
   support to use linear texture for projector (faster), the UV are scaled
   automatically at the VS level. Drawback: if several projectors are rendered in
   the same pass, I assume that their directions are similar, so the alpha
   modulation is done using the first projector's direction. Furthermore the
   #projectors has been increased to 12 (max of 3 passes). Added 4 new PS and VS
   to implement these optimizations.
5. Pixel shaders: material color should be correctly rendered (using specular
   register). Modified several pixel shaders to have better blending
   implementation.

[Version 1.3.105x - 02/12/02]
1. Re-introduced the NGLMAT_MATERIAL_COLOR flag and added pixel shaders to
   support it (by using the specular register for now). Added the color
   member to nglMaterial as well; this requires the new xbmesh 0x303 format.
2. Added a billboard API to improve the spiderman particle system's rendering
   speed. Basically, 2 "types" of billboards: single ones which are similar to
   quads but are sorted like meshes and billboard lists that are similar to
   scratch mesh but without the whole useless mesh-stuff-overhead.
3. Added the nglEnableFog(bool Enable) function, which enable/disable the fog on
   a per scene basis (turned off in the default scene).
4. Slightly modified the lighting code and restored the previous lighting model.

[Version 1.3.104x - 02/08/02]
1. Added a define PROJECT_SPIDERMAN, which enable some hacks specific to spidey.
2. Added some spiderman specific (hack) code to have the shadows casted on most
   of the materials. Removed the alpha component of the lights to make the thugs
   fading correctly.
3. Slightly changed the light implementation (VS). Old version:
   Final vertex_color = (Tint * vertex_color) + Tint + Light_1 + ... + Light_n
   New version:
   Final vertex_color = (Tint + Light_1 + ... + Light_n) * vertex_color
   (Tint is considered as ambiant light).
1. Fix: a bug in the lighting test (to know if a mesh is to be lit or not).

[Version 1.3.103x - 02/08/02]
1. Removed NGLMAT_MATERIAL_COLOR and NGLMESH_INACCURATE_XFORM from NGL header
   file.
2. Optimized the parsing of the lights/projectors list.

[Version 1.3.102x - 02/07/02]
1. Added a check which tests if the nglLoadDDS() function succeeded or not
   (typically when trying to load a DDS file that has an unsupported format).
2. Hack (flag change at mesh loading time - dirty) to have shadows casted to the
   appropriate materials.

[Version 1.3.101x - 02/06/02]
1. Added an NGL function which allows the user to setup it's own alpha-test:
   void nglSetAlphaTest(int32 Enable, uint32 Threshold).
   If enabled, only fragments with an alpha value > Threshold will be accepted
   (rendered). This state remains active until the user turn it off (by
   specifying Enable = 0 - default). The material alpha-test blending mode
   overrides the global alpha-test during material rendering. For example, this
   alpha-test could be used to speedup particle system rendering by eliminating
   particles with an alpha value close to zero (particles almost translucent are
   not rendered - fillrate optimization).

[Version 1.3.100x - 02/06/02]
1. Moved to the new version numbering.
2. Modified nglSetBlendingMode() function to be more flexible when used in
   custom nodes. Now the client can use its own renderstates as the following:
   // Render a quad with a special blending mode:
   nglQuad q;
   nglInitQuad(&q);
   nglSetQuadC(&q, 1.0f, 1.0f, 1.0f, 1.0f);
   nglSetQuadRect(&q, 0, 0, 32, 32);
   // Tell NGL that we use a custom blending mode.
   nglSetQuadBlend(&q, NGLBM_CUSTOM, 0);
   nglListInit();
   nglListAddQuad(&q);
   nglDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
   nglDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
   nglDev->SetRenderState(D3DRS_BLENDCOLOR, 0x00ffffff);
   nglDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_CONSTANTCOLOR);
   nglDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
   nglDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
   // Reset the cache system used for blending modes.
   nglPreviousBM = NGLBM_MAX_BLEND_MODES;
   nglListSend(true);

[Version 0.5.24x - 02/05/02]
1. Added a new way of rendering the scratch meshes (when NGL_SCRATCH_TEST is set)
   by copying directly the vertices to the pushbuffer instead of using a vertex
   buffer which needs to be locked. Unfortunately it seems to be slower.
2. Added two defines that control the global pushbuffer parameters :
   NGL_GLOBAL_PUSHBUFFER_SIZE and NGL_GLOBAL_KICKOFF_SIZE.
3. Modified the clear flags of the default scene to be similar to the PS2
   (NGLCLEAR_Z instead of (NGLCLEAR_COLOR | NGLCLEAR_Z | NGLCLEAR_STENCIL)).
1. Fix: there was still an overflow bug with the 64-bits vertex shader key.
2. Fix: the directional light had the y-axis inversed.

[Version 0.5.23x - 02/01/02]
1. Fix: now, meshes are correctly blended when tint alpha < 1.0.

[Version 0.5.22x - 02/01/02]
1. Fix: environment mapping was buggy (forgot to transpose the transform matrix
   before sending it to the VS).

[Version 0.5.21x - 01/31/02]
1. Reduced the number of projectors to 8.
2. Added GPU counters (nglPerfInfo) for more profiling infos.
   NOTE: now the client needs to link to the XbDm.lib library.
3. Updated the back/front buffer texture acquisition by saving some memory.
   NOTE: if you want to retrieve the back/front buffer, don't use the
   nglGetTexture("BACKBUFFER") call, it will not work. Use nglGetBackBufferTex()
   or nglGetFrontBufferTex() instead (and don't forget to scale the UVs).
4. Switched to triple buffering instead of two.
5. Added a test which verify if a texture stages combination for a given section
   is valid.
1. Fix: the 64-bits key was truncated to 32-bits in the VS key calculation for
   lighting and projected lights.

[Version 0.5.20x - 01/30/02]
1. Fix: now the release version compiles correctly.

[Version 0.5.19x - 01/29/02]
1. Moved to the new NGL revision system, defined the following functions:
   char* nglGetVersion(), char* nglGetVersionDate().
2. Updated nglMemAllocInternal() to be able to allocate aligned memory.

[Version 0.5.18x - 01/28/02]
1. Fix: pixel shaders with detail/enviro passes.

[Version 0.5.17x - 01/28/02]
1. Added an assert when the client tries to load a texture which hasn't a power
   of 2 dimension.
2. Changed the way the vertex shader opcodes are stored. Now, they are stored
   into a static array (see NGL_VS_POOL) to prevent memory fragmentation.

[Version 0.5.16x - 01/25/02]
1. Fix: when DrawMeshSpheres or DrawLightSpheres are set, NGL doesn't crash
   anymore.

[Version 0.5.15x - 01/24/02]
1. Updated the mesh loading API to be the same as the PS2 (only supported by
   the xbmesh version 0x302 or higher).
2. Updated the debug interface to be the same as the PS2
   (see nglSetDebugFlag/nglGetDebugFlag).
3. All the debug flags are implemented and are working (added some debug stuff).
4. Fix: updated the quad->Z value calculation to avoid a compiler optimization
   bug when using the /Og switch (release build with optimize for speed).

[Version 0.5.14x - 01/22/02]
1. Updated NGLASSERT to use nglFatal to allow the client to override the assert
   behaviour.
2. Got ride of the vblank interrupt handler (better D3D implementation).
3. Updated nglSetFrameLock. Only support 60 and 30 fps lock. Client can specify
   0 for unlimited framerate.
4. Added nglLoadTextureInPlace.
5. Added nglDestroyTexture.
6. Added nglSaveTexture.
7. Got ride of the pixel shaders used when NGLMESH_MATERIAL_COLOR was defined
   (wrong implementation).
8. Now, the processing of this flag is done in the meshcvt. When it is
   specified, meshcvt replaces the vertex color by the material color.

[Version 0.5.13x - 01/18/02]
1. New timings functions (IFL speed is independant of the fps) and ability to
   take a screenshot: nglSetFrameLock, nglSetIFLSpeed, nglSetAnimTime,
   nglScreenShot, nglGetMeshPath, nglGetTexturePath.
2. Renamed:
   nglTexture* nglGetFrontBufTexture();
   nglTexture* nglGetBackBufTexture();
   by:
   nglTexture* nglGetFrontBufferTex();
   nglTexture* nglGetBackBufferTex();

[Version 0.5.12x - 01/16/02]
1. Fix: bad "sorting" (actually a bug in the distance calculation) bug.

[Version 0.5.11x - 01/15/02]
1. Added a new texture format (YUY2) required by the NVL library.

[Version 0.5.10x - 01/15/02]
1. Restored NGL_FORCE_CREATE_VB for spidey's compatibility.
2. Fix: bug in nglSphereInFrustum() - thanx Brian :)

[Version 0.5.9x - 01/13/02]
1. Projected lights is in.

[Version 0.5.8x - 01/10/02]
1. Fix: fov is closer to the PS2 version.

[Version 0.5.7x - 01/09/02]
1. Better visual stability for timing display (average of several frames).

[Version 0.5.6x - 01/09/02]
1. Removed the XB at the end of the XBox specific functions (I was the only guy
   doing this...).
2. Primary pushbuffer optimization for skinned/unskinned meshes (only store the
   DrawIndexedVertices for now).
3. Updated nglLockTextureXB() to lock linear texture as well (added an example
   (as comments) how to write to a linear texture).
3. Removed NGL_FORCE_CREATE_VB flag, not used anymore (was for spidey reasons).

--------------------------------------------------------------------------------
NGL Tool Version
--------------------------------------------------------------------------------
 
DDS plugin    1.2.0x
GCT Plugin    2.2.0x
TGA Plugin    1.2.0
TM2 Plugin    2.2.0x
TextCvt       1.2.0
Envmap        1.2.0

--------------------------------------------------------------------------------
Known Problems
--------------------------------------------------------------------------------

-

*/

#endif