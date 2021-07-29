#ifndef NGL_VERSION_H
#define NGL_VERSION_H

// Change these for each build
#define NGL_VERSION         0x010701
#define NGL_VERSION_MAJOR   1
#define NGL_VERSION_MINOR   7
#define NGL_VERSION_PATCH   1
#define NGL_VERSION_DATE    "06/18/2002"

// Helper macros to build other types of version info
#define _NGLSTR(x)  #x
#define _NGLXSTR(x) _NGLSTR(x)

#define NGL_VERSION_STR _NGLXSTR(NGL_VERSION_MAJOR) "." _NGLXSTR(NGL_VERSION_MINOR) "." _NGLXSTR(NGL_VERSION_PATCH) "g Working"

#undef NGL
#undef NGL_GC

#define NGL			NGL_VERSION
#define NGL_GC		NGL_VERSION

/*
------------------------------------------------------------------------------
  NGL GameCube Working Version Notes
------------------------------------------------------------------------------

------------------------------------------------------------------------------
  TODO List
------------------------------------------------------------------------------

-- Post NGL 1.8 Release
  * Add solution for GX parallelism (double or triple buffering)
  * add more paired single math functions
  * fix warning callback to weed out bogus errors
  * add API for adjusting GX warn level
  * add API for adjusting GX warning callback
  * convert to newer scene sorting
  * optimize quad rendering
  * optimize string rendering
  * reformat all code
  * nglWhiteTex is a pointer on PS2
  * nglPhongTex is a pointer on PS2
  * Use tex demo to fix nglSetTevBlend()

-- TODO NGL 1.8 Release

  * add nglGetVersion()

-- BUGS NGL 1.8 Release

------------------------------------------------------------------------------
  New features / bug fixes
------------------------------------------------------------------------------

v1.7.1g: In Progress

  - Removed legacy quad API
  - Removed legacy scratch mesh API
  - Removed ngl_gc_profile code
  - Changed nglGetStringDimensions to accept NULL params
  - Changed nglCreateMesh to take MeshFlags
  - Removed nglGetMeshFlags and nglSetMeshFlags
  - Changed nglMeshWriteStrip to nglMeshWritePrimitive
  - Added optional threaded list send switchable by a define
  - Added paired single function for converting NGL to Nintendo matrix
  - Removed legacy ngl_font.cpp
  - nglQuadSetBlend now defaults to a constant of 0
  - Removed asserts on nglListSendInProgress, now silently waits
  - nglSetFilterMode had mipmap logic backwards
  - Added NGLTF_SWIZZLED and NGLTF_LINEAR for xbox compatibility
  - Added NGLCLEAR_STENCIL for xbox compatibility
  - Added nglEnableFog(bool) for the scene API
  - Fixed bug in drawing a font (V was flipped)
  - Added a new nglSysFont
  - NGL_ASSERT was compiled out in debug, fixed
  - Fixed internals of nglQuadInit to be to spec
  - Fixed asserts in nglCreateTexture to be crossplatform compatible
  - nglCreateTexutre had bad texture format logic, fixed
  - Removed segregation of quads, all quads are now added as nodes
  - Removed legacy font API
  - Upgrade NGL internals to new font API
  - Changed nglDebug and nglStage from bitfields to u_chars
  - Clear color was not being set before to clear, fixed
  - Disabled the vertex color fix by default to fix lighting
  - Normal matrix is loaded for all models, fixes lighting bugs
  - Fonts were not defaulting to blend mode and bilinear filtering, fixed
  - nglSetMeshSection not correctly reseting the vertex index, fixed
  - nglCloseMesh now required for nglCreateMesh and nglEditMesh
  - Removed render target scene hack, clients now control this
  - Fixed setting of viewports and projection parameters (splitscreen works properly)
  - Fixed render target viewport and projection settings
  - added NGL_GC_LENS_FLARE to compile out lens flare code
  - Detail maps are forced to bilinear filtering like the PS2
  - Implemented lighting for all models, not just skinned meshes
  - Adding a light to a mesh ignored the vertex color, fixed
  - Moved to ngl_verion.h like PS2/xbox
  - NGL and NGL_GC are now defined to versions numbers if ngl_version.h is included
  - Implemented performance screen and cleaned up FPS counter
  - Adjusted ngl_gc_internals.h to be formated more like the ps2
  - Added public function nglGetVBlankCount() (crossplatform)
  - updated nglSetPerspectiveMatrix/nglSetOrthoMatrix/nglGetProjectionParams to
	  new declarations (removes cx, cy, zmin, zmax)
  - fixed scene clearing to use a full screen polygon (speeds up rendering a tad too)
  - VISetBlack is now set to false after to inagural flips in nglInit()
      (removes initial screen garbage)
  - Added performance debug screen that closely matches the PS2
  - Fixed where perf.lib was being started/stop to clean up perf display
  - Implemented nglSetBufferSize() API, defaults are low so check the docs
  - Moved aspect ratio to be a per scene parameter
  - Added colors to nglWarning and nglFatal (like PS2)
  - Added paired single version of nglCopyMatrix
  - Added ngl_sphere and ngl_radius meshes to debug builds
  - Implemented ambient lighting, removed alpha param from nglSetAmbient
  - Implemented nglGetDebug/nglSetDebug calls like PS2
  - Implemented nglMeshBindTextures, mesh processing is now compatible with PS2
  - Implemented nglScreenShot and nglDebug.ScreenShot, identical to the PS2
  - VIInit() is now called exactly once from nglInit()
  - Fixed several GX bugs in Projector Lights and removed usage of some globals
  - Streamlined the interchange between nglListInit/nglListSend/nglFlip
  - unimplemented nglStage/nglDebug variables now have NI suffix
  - added static stricmp for Metrowerks compiles
  - removed all hacks related to Spiderman
  - added defines for NGL_DEBUGGING and NGL_PROFILING to match PS2
  - Lightmaps are forced to bilinear
  - nglMeshAddSection now recalculates Mat -> Map flags (fixes Detail Map Demo)
  - Textures generated at nglInit were not getting flushed, fixed
  - nglDefaultScene now defaults to an fov of 60
  - Cleaned up quad rendering, still needs matrix optimizations
  - Implemented NGLP_REVERSE_BACKFACECULL render parameter
  - Implemented file i/o library to handle MW or SN debugger
  - Removed Kelly Slater ATE code
  - Removed Kelly Slater particle code
  - nglSystemCallbackStruct renamed to nglSystemCallbacks
  - Orthographic projection for non-quads being setup incorrectly, fixed
  - added GXSetCurrentGXThread to nglListInit(), allows rendering from threads
  - nglSetOrthoMatrix enhanced to take t,b,l,r
  - switched to a standard function to calculate hFov

v1.7.0g: May 30, 2002

  - Removed threaded list send, to be replaced by triple buffering
  - Fixed scratch mesh API to require strip count
  - Fixed nglAddListString from parsing the format string twice
  - Fixed bug where texture filters were being set backwards
  - Fixed clamp implementation for quads/fonts
  - Fixed scratch meshes not getting cache flushed with new API
  - Fixed nglSetFilter wasn't initializing min/max vars
  - Fixed nglGetStringDimensions returning incorrect values
  - Fixed made asserts call "trap"
  - Added Linear Fog is the default fog when using the generic API.
  - Added Font Scale to nglAddListString API
  - Added Makefile to compile into library
  - Added new NGLMAP flags for POINT and ANISOTROPIC
  - Added conversion of ArchMat flags to NGL Map Flags at mesh load
  - Added external declaration for nglWarning
  - Added default HFOV for a scene is now 90
  - Added NGL_ASSERT for all assert() calls

v1.5.0g: May 10, 2002

  - Linear Fog is the default fog when using the generic API.
  - Fixed scratch mesh API to require strip count
  - Added Font Scale to nglAddListString API
  - Fixed nglAddListString from parsing the format string twice
  - Added Makefile to compile into library
  - Added NGL_ASSERT code

v1.3.111G: 26-Apr-2002
  - Added call to OSInitFastCast() in nglInit().
    NGL gc uses this facility but wasn't ensuring it was initialized.

v1.3.110G: 25-Apr-2002
  - Implement subtractive blending.
  - Implement nglVector and nglMatrix classes.

v1.3.109G: 23-Apr-2002
  - Added missing definition of nglGetStringDimensions.
  - Added per scene nglSetZWriteEnable(bool enable) and nglSetZTestEnable(bool enable).
  - Added code to print version info at start of nglInit().

v1.3.108G:- 11-Apr-2002
  - Support viewport settings.
  - New Font API (and functionality).

v1.3.107G: 04-Apr-2002
  - Add RenderTest.DrawAsLines support.
  - LoadTextureInPlace implemented. KellySlater ATE (animated textures) support.

v1.3.106G: 3-Apr-2002
  - Added new Scene api.  nglListBeginScene now returns a nglScene*.
  - New nglScene* nglListSelectScene(nglScene* scene) selects a new
    current scene and returns the previously selected scene.

v1.3.105G: 31-Mar-2002.
  - Fix Detail textures.

v1.3.104G: 21-Mar-2002.
  - New MeshFile API is in.
  - New ScratchMesh API are in.
  - Ported Edit Mesh API from PS2 to GameCube.  API is functionally equivelant
  - nglRebaseMesh() can now rebase meshes currently in memory
  - nglStageEnable/nglDebugEnable renamed to nglStage/nglDebug per the PS2

v1.3.103G: (Spidey Branch Version) 19-Mar-2002
  - Full implementation of the threading system for parallelized AI, ListSend,
    Graphics, and ListPreparation to a large extent.
  - Implement LensFlares.
  - Implement motion Blur.
  - MipMap levels will now switch smoothly.
  - Fix a Quad postioning bug in Orthographic scenes.
  - Reduce number of TEV stages in some cases.
  - Support NGL_MAT_COLOR.
  - Implement TV Mode API.
  - Implement perspective correct color interpolations for non-skinned meshes
    to bypass a hardware color interpolation bug on some near plane clipped polygons.
  - Fix a bug where the BlendMode settings of non-textured objects were getting
    lost. Support glass' special blending (to be standardized after Spidey).
  - Implement nglProjectPoint.
  - Reduce NumTevStages for quads to 1.
  - Skip Perspective Correct Color Interpolations for meshes meeting certain
    distance or dimension criteria. Add some support for RenderTests (We now have
    nglGCPD for debug info and nglGCRT for turning things on / off and other
    tests). Support widescreen mode, and fix Aspect Ratio computation to be
    mathematically correct.

v1.3.101G:
  - Diffuse EnvMap for Skinned characters is in.
*/

#endif
