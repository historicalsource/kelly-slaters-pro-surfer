#ifndef NGL_VERSION_H
#define NGL_VERSION_H

// Numeric and string version numbers.
#define NGL_RELEASE_VERSION 1
#define NGL_MAJOR_VERISON 7
#define NGL_MINOR_VERSION 6

#define MAKE_VERSION(a,b,c) "v" #a "." #b "." #c "P Working"
#define NGL_VERSION MAKE_VERSION(NGL_RELEASE_VERSION,NGL_MAJOR_VERSION,NGL_MINOR_VERSION)

/*
--------------------------------------------------------------------------------
 NGL PS2 Release Notes
--------------------------------------------------------------------------------

6/06/2002 - v1.7.6P
- Changes for integration with latest KS. (dc 06/06/02)
- Updated custom VU code to build under new assembler.
- Removed a lot of KS-specific code.
- A few bug fixes.

6/03/2002 - v1.7.5P
- Fixed NGL_EMULATE_480_LINES allocating 480 line frame buffers (!).
- Added nglSetSectionRenderer API as an alternative to writing complete custom nodes.
- Added UserFlags to nglMaterial so custom material renderers don't have to hijack our flags.
- Merged Flo's nglGetStringDimensions bugfix (KS bug #636).
- Added nglGetFont function to match mesh and texture APIs.
- Added System flag for nglSysFont to protect from nglReleaseAllFonts.
- Fixed bug where nglLoadFont would not properly initialize all members.
- Renamed nglSetFontBlendMode to nglSetFontBlend, to match the Quad API.
- Renamed nglSetLockBufferSize to nglSetLockBufferSizePS2, as it's platform specific.
- Renamed NGLPS2.mak to ngl_ps2.mk, now loads w/o Visual Studio thinking it's a project file.

5/14/2002 - v1.7.4P
- Added NGL_USE_SN_TUNER define to support using SN Systems' PS2 analyzer.

5/13/2002 - v1.7.3P
- Changed ngl_vu1.dsm to use .equr for macro definitions, ';' for comments.
- Removed m4 and .pp? from the VU code build process.
- Changed NGLMAP_XXX flags to a new (and hopefully final) format.
- Changed internal texture handling code to use NGLMAP_XXX flags directly.
- Fixed internal handling of ArchMat bilinear, trilinear and point sampling flags.
- Added NGL_FORCE_FILTER define to override filtering flags (which only now are 
  processed correctly).
Warnings:
- You MUST get the latest alpha Ps2DvpAs.exe from SN Systems to use this version,
  as it fixes a bug in the handling of ".equr".
- If your project has bad filtering (bilinear, trilinear) flags in its mesh data,
  you may define NGL_FORCE_FILTER on the command line to a numerical NGLMAP_XXX
	value, and thus override filtering settings for all materials.

5/10/2002 - v1.7.2P
- Removed legacy scratch mesh, quad and lighting API's.

5/10/2002 - v1.7.1P
- Fixed a bug in axis independent font scaling.
- Added ngl_version.h to VC7 workspace (still not in VC6 workspace).
- Added NStrips to nglMeshAddSection.
- Using ngl_version.h for initialization printout.
- Improved nglDebug.ShowPerfInfo display (version info).

5/10/2002
- Converted ngl_version.txt to ngl_version.h, added appropriate defines.
- Added feature defines to ngl_ps2.h to improve conditional compiling:
  NGL_HAS_VECTOR_CLASS, NGL_HAS_NEW_FONT_API, NGL_HAS_MORPH_API


5/10/2002 
- Branched to v1.6.0P Stable from v1.3.128P Working.
- Advanced to v1.7.0P Working.

5/9/2002 - v1.3.128P
- Reworked makefile to call ee-gcc2953 directly, versus going through ps2cc.  This 
  required adding include paths to NGLPS2.mak.
- Added NGLMAP_XXXX flags for quads and fonts, nglSetQuadMapFlags, nglSetFontMapFlags.
- Added nglSetFontBlendMode, settable per font.
- Added axis independent font scaling (\3[X,Y]).

4/30/2002 - v1.3.127P
- Put code in nglLoadTextureTM2() to limit the smallest mipmap level to at least
  8 width and 8 high.  I was encountering a 128x32 texture with 7 levels,
  causing some levels to have height 0, resulting in nglTim2CalcBufSize returning
  an erroneous size.
- Fixed bug in CreateScratchMesh().  Needed to set material.map to nglWhiteTex.
  Was crashing with a null pointer for scratch meshes that didn't set a material.
  
4/26/2002 - v1.3.126P
- Moved NGL_ASYNC_LIST_SEND define to ngl_ps2_internal.h, modified to behave similar
  to other project settable options.
- Renamed nglDefaultLightContext to nglGlobalLightContext, which is more accurate.
- Changed Light Context API to deal w/a pointer to a structure rather than a u_int.
- Fixed a bug in the Light Context API - it wouldn't always clear the right context
  when rendering a new node.
- Renamed nglVif1DetermineLocalLights to nglDetermineLocalLights, reworked parameters
  to make it more custom node friendly (no more MeshNode).  More work to do in this area.
- Added TW, TH members to nglTexture, added calculation code to nglCalcTextureGSRegs.
- Changed nglTexture Width and Height members to shorts to make room.
- Added nglNextPow2 to ngl_ps2_internal.h, replaced some for() loops with it.
- Added NGLTF_RENDER_TARGET for nglCreateTexture.  Need to discuss implementation
  with Paul further.
- Removed nglBakeTextureTM2 and moved the code into nglLoadTextureTM2.  Reworked a
  few things to reduce dependency on the TM2 picture header.
- Moved Tex0 and Tex1 field calculation into nglUploadTexture as an optimization.
- Added internal nglHostOpen/nglHostClose/nglHostPrintf functions, modified Scene Dump
  code to call them instead of the ArchEngine specific functions.
    
4/25/2002 - v1.3.125P
- More revisions to uploading only referenced mipmaps.  Identified bug in v1.3.123P
  and put back changes removed in v1.3.124P which worked around the original problem
  but introduced some other potential problems.  Also put back ability to enable
  and disable from debugger, to allow performance comparisons.  Also added some
  additional minor performance improvements.
- If NGL_DISABLE_ASYNC_LIST_SEND is defined with compiler switches,
  NGL_ASYNC_LIST_SEND is not defined and NGL uses single buffering.
  (Used by NHL2K3).
- Fixed incorrect rounding code in align2k(), nglLockTexturePS2, and elsewhere...
  was using (x+1)&~31 instead of (x+31)&~31.
- Rewrote nglTim2CalcBufSize to accurately return texture sizes as minimum necessary
  contiguous blocks, rather than effectively in units of pages.
Remarks:
- Currently, VRAMOnly for TM2 can allocate space for a texture image
  but doesn't then set its address.  This is correct if the texture
  is designed to upload a palette but reference the frame buffer or zbuffer,
  but is incorrect if it is also allocating space for a render target image.
  Additional flags are needed to signal this case (and changes will also
  be needed in nglBakeTexture to reserve space in gsImage[].Offset/Size
  without generating dma packet to upload texture).
  If a palette is not needed, but a VRAMOnly streamed texture is needed,
  this can currently be done with a TGA texture, which *does* set its address.
  (Obviously all this needs to be cleaned up with a better interface.)
  (See nglUploadTexture and nglBakeTexture.)
- We should really have a flag to indicate that a texture
  is legal as a render target, and only perform page alignment
  if it is set.  SetRenderTarget would then reject textures which
  hadn't been thus set.  SetRenderTarget could still set the current
  render target bit in the texture to indicate whether it is actually
  being used as a render target (but this is of questionable utility,
  since hopefully the render target legal bit would only be set for
  textures which actually were going to be used as render targets).
  (See nglBakeTexture.)

4/25/2002 - v1.3.124P
- Uploading only referenced mipmaps cleaned up and fixed, appears to be working.
- Added maximum recorded usage meters on the various buffers.

4/24/2002 - v1.3.123P
- *Work in progress* on only uploading referenced mipmap levels.
  Feature is enabled by setting nglMipUploadOptimization to true (currently defaults
  to false).  Code is implemented, but textures do not always appear correctly
  when feature is enabled.
- Modified Tex Upload display and frame log to report texture upload in terms
  of gsSize (video ram size) instead of data size (source data size) (this was
  necessary to determine amount uploaded when not all mipmap levels are loaded).
  Also modified frame log to display starting lod level loaded.

4/24/2002 - Released v1.3.122P to the stable directory.

4/24/2002 - v1.3.122P
- Changed nglSysFont to Courier 10pt.  May have to move to 12pt as it's hard to read.
- Increased ScratchPad DMA building threshold to prevent some rare overflows.
- Added support for not flipping via nglListSend when double buffering is off.
- Fixed nglUnlockAllTextures to not affect System textures (like the framebuffer).
- Fixed two pretty serious bugs with NGLP_BONES_WORLD.
- Switched some instances of Sony's transform_t class to matrix_t.
- Switched nglVif1UnpackBones to use the aligned unpack function, which is faster.
- Changed baked section renderers to call nglDmaAddLargeRef to avoid some asserts.
- Mike has added preliminary Doxygen tags to ngl_ps2.h.

4/24/2002 - Released v1.3.121P to the stable directory.

4/23/2002 - v1.3.121P
- Mike modified nglCreateMesh to ensure scratch buffer pointer is aligned 128,
  and that nglDestroyMesh first does an nglVif1SafeWait().
- Modified nglMeshAddSection() to no longer set NGLMAT_TEXTURE_MAP if it is not set.
  Now the diffuse map is only set (and to nglWhiteTex) if no maps are specified.
  NOTE: Even automatically doing that is somewhat questionable, as it then requires the
  caller to set uv's in vertices.

4/19/2002 - v1.3.120P
- Made corrections to NGLTF_VRAM_ONLY loading of paletted textures
  in nglUploadTexture.

4/15/2002 - v1.3.119P
- The ngl interrupt service was damaging floating point registers!
  Changed nglGifSafeWait (which is called by nglVif1Interrupt) to
  not use floating point.  Ultimately, it would probably be wiser to
  have nglVif1Interrupt save and restore floating point state.
- Added new 'void nglBakeTexture( nglTexture* Tex )' to ngl_ps2_internal.h
  and reworked nglBakeTextureTM2 to call it.
  Added ClutColors and MipMapTextures fields to nglTexture.
  Changed internal functions nglUploadTextureTM2 and nglUploadTextureChunkTM2 to
  nglUploadTexture and nglUploadTextureChunk.
  These changes are on the path to removing the dependancies on the tim2 header in nglTexture
  so that it can eventually be removed.  They were also needed to be able to create
  a paletted texture with a vram only image (see v1.3.117P).

4/15/2002 - v1.3.118P
- Made nglRebaseMesh available from ngl_ps2_internal.h.  Currently needed for NHL2K3
  morph implementation.

4/15/2002 - v1.3.117P
- Modified meaning of NGLTF_VRAM_ONLY slightly, to make it more useful.
  If a texture is marked NGLTF_VRAM_ONLY but has a palette, then the palette is
  still uploaded.  This allows rendering to a hi-8 paletted texture, where the palette
  comes from main memory.  It allows allows the hi-8 bits of the frame buffer or z buffer
  to be used as a source texture with different supplied palettes.  (This is being used
  by the NHL2K3 shadow system.)  Only change was to nglUploadTextureTM2().

4/12/2002 - v1.3.116P
- Fixed NGL_FBWRITE_xxx masks, they had R,G,B,A reversed.
- Changed NGL_SPR_NODE_THRESHOLD from 8K to 7K to leave more room between
  flush threshold and limit.  Was necessary for shadow instancing code.

4/10/2002 - v1.3.115P
- Fixed bug introduced accidently in v1.3.111P causing mip mapping to always be disabled.
- Added per scene 'void nglSetZWriteEnable(bool enable)'.
- Added NGLBM_DESTALPHA_ADDITIVE, which adds the texel to the background, modulated by
  destination alpha instead of source alpha.  This is needed for the NHL2K3 PS2 shadow implementation.
Known Problems:
- For now, nglSetZWriteEnable only applies to Quads and Strings.
  This is because material baking of meshes would freeze ztest enable to whatever
  ZTestEnable was set to for the current scene when the material was baked.
  For now Quads and Strings are sufficient, as this feature is initially intended to facilitate
  rendering of scenes with quads and strings in front of a previously rendered 3d scenes.
  Created an alternate version of nglVif1AddSetBlendMode called nglVif1AddSetBlendModeCheckZTest
  which tests nglCurScene->ZTestEnable and disables z testing if false, which is now used
  for quads and strings.
- NGLMAT_FORCE_Z_WRITE is not implemented correctly for punchthrough blend modes.
  It disables the punchthrough test, making it appear to always be true.

4/9/2002 - v1.3.114P
Changes/additions:
- Added texture locking support for TM2 textures.
- Fixed a bug in the implementation of nglSetFBWriteMask.
- Reworked nglUnpackBones to take different parameters.
- Added VU code to support instanced shadow rendering.

4/9/2002 - v1.3.113P
Changes/additions:
- Fixed nglVif1RenderString to use NGLBM_BLEND instead of NGLBM_OPAQUE.
Known Problems:
- If a texture for a material is changed, material baking currently causes the
  material info hash key to not be updated.  This can cause incorrect sorting
  of nodes by material and excessive texture uploading.
  (See also Known Problems under v.1.3.109P.)
  The new function nglBindMeshTextures does not currently address this issue.
  Its purpose is apparently to remap material texture names to texture pointers.

4/8/2002 - v1.3.112P
Changes/additions:
- Added NGL_EMULATE_480_LINES mode, off by default.  This can emulate the way the XBox and GC 
  map Quads and Strings onto the TV, and will cause nglGetScreenHeight to return 480.  
  Does not affect 3D rendering.
- Added nglSetFBWriteMask API.  This allows clients to control writing to only certain channels
  (or bits - PS2 only) of the render target.
- Added nglCreateMeshCopy API, to allocate and create a deep copy of a loaded or created mesh.
- Added Morph Target API, for creating and applying morph targets to loaded or created meshes.
  See header file for documentation and instructions.
- Added new Font API.  The new API requires a FDF file in addition to the texture.  Also added 
  the nglSysFont embedded font and FDF file, which replaces ngl_font8x12.
- Added a Font compatibility API to ease transition to the new API. Also useful until the GameCube 
  supports the new Font API.  It isn't 100% compatible, but close enough to make things easy.
- Added NGL_ALIGN macro to ngl_ps2_internal.h.  Useful for moving pointers forward to a given power 
  of 2 boundary.
- Added nglGifStartTag/nglGifEndTag/nglGifCalcNLoop, plus nglVif1StartDirect/nglVif1EndDirect.
  These functions provide a fast/simple/flexible way to write GS registers via Path2.
- Font code optimized to 32bytes/char, using new streamlined Path2 functions.
- Changed nglListAddMesh to use the matrix_t type as opposed to transform_t, which has bugs.
- Changed nglFatal to call asm( "break 1" ); instead of an infinite loop.
- Changed NGL_ASSERT to call asm( "break 1" ); directly instead of calling nglFatal.  This
  brings the callstack directly to the assertion, which is more convenient.
- Added new Data member to nglTexGsChunk, points to actual pixel data.  This can be used to
  procedurally modify loaded TM2 files, including editing of Mip levels and the Palette,
  without going through the TM2 header structure.
- Reworked and cleaned up rebasing and processing of loaded meshes.
- Added nglBindMeshTextures, which can reevaluate those textures pointed to by a loaded mesh.
  Use this if you've reloaded some textures with the same name, and want to update meshes
  that use them.  Can also optionally try to load textures that it can't find.
- Added a ScratchPad streaming library to ngl_dma.h, may need some generalization but should
  be useful for simple data processing tasks.
- Added nglScreenXToGS and nglScreenYToGS to ngl_ps2_internal.h, useful for rendering sprites
  and quad like things directly to the GS.
- Added Visual Studio .NET Project and Workspace files for ngl_ps2 and Midnight.
- Temporarily disabled detail textures, they were causing rare crashes on Minority Report
  and weren't working correctly anyway.  The disable happens in nglProcessMesh, where we
  automatically remove the flag.
- Rewrote confusing comments and added new ones in ngl_ps2.h.
- Removed obsolete APIs - nglLoadTextureLockPS2, nglGetTexel.
Known problems:
- The makefile NGLPS2.mak relies on ps2cc, which isn't always set up correctly on everyone's
  machines, plus it relies on having an SN license installed.  In the future, the makefile
  will likely be converted to call ee-gcc2953 directly, however this will require some include
  and library path defines to be set up, which may impose library version dependencies.  Need
  to work out a good project independent yet convenient solution here.

4/8/2002 - v1.3.111P
Changes/additions:
- When mipratio is specified as 0.0f in a material map, mip mapping is supposed to be disabled.
  Instead it was only partially enabled, incorrectly calculating a mip map lod and using garbage
  for the mip map levels.  Fixed nglCalcTextureGSRegs to initialize GsTex1 to not mipmap even
  if the texture includes mipmaps.  This is later overriden if mip maps are used.
- Added void nglSetRequiredTexturesPS2( nglTexture** TexTbl, u_int NTex).
  Used to ensure a streamed RenderTarget will not be flushed from video ram
  while needed, due to references to any of the specified textures.
  
4/8/2002 - v1.3.110P
Changes/additions:
- Fixes to nglVif1AddTextureFixup: Added code to do mipmap lod computation and setting
  of TEX1 register (otherwise mipmap lod was coming out wrong when camera changed field of view).
  Moved conditional TexFlush to end of nglVif1AddSetTexture, so that nglVif1AddSetTexture and
  nglVif1AddTextureFixup wouldn't get out of sync when Tex==nglCurScene->RenderTarget.
  
4/5/2002 - v1.3.109P
Changes/additions:
- Fixed a bug in scene clearing left in 108P, related to reworking the rendering
  code not to depend upon nglCurScene but instead upon scene arguments.
- Fixed reporting of texture addresses in Dump Frame Log.
Known problems:
- Baked materials are freezing some things that they shouldn't.
  Examples:
   -Mip map lod mapping parameters are set based upon the nglCurScene at the time
    the material is baked (at mesh creation time for scratch meshes).  If later
	the camera field of view is changed to zoom in or out, the wrong mip map levels
	will be choosen.
   -Changing settings such as texture clamping modes after a material is baked will be ignored.
   -If a material is baked with null for one of its textures (say the light map), then it assumes
    that map is not present.  If later the map is set, when it is rendered only some of the registers
	for that texture will be uploaded (the ones normally handled by baking won't be sent).
  One solution is not to bake things like mip map lod calculations.
  Another is to flag when things the baked material depends on have been modified,
  and to rebuild the baked material when anything it depends upon have changed.
  Note that this would mean that for permanent scratch meshes, the baked materials dma
  (usually just one) would need to be in separate memory objects that could be freed
  and reallocated... currently they are allocated interleaved as part of the permanent
  scratch mesh memory object.
  
4/4/2002 - v1.3.108P
Changes/additions:
- Fixed a bug in sub scenes that specified sortinfo.  nglCurScene was being altered
  to subscene and left that way when rendering resumed in parent.  Reworked code where possible
  to use local Scene arguments rather than global nglCurScene.  Made nglVif1RenderScene preserve
  nglCurScene (it still sets it to scene being rendering during rendering, so that
  custom node functions can access the current scene being rendered... this is also
  necessary because many node functions call nglVif1AddTextureStreaming,
  which implicitly references nglCurScene->RenderTarget).
- Added scene nglSetZWriteEnable(bool enable).  Allows z writes to be disabled for a scene.
  This can be especially useful when rendering to a texture.
- Added blend mode NGLBM_PUNCHTHROUGH_NO_ZTEST.  Similar to NGLBM_PUNCHTHROUGH except
  that it writes when alpha is nonzero instead of when alpha>=64, and performs no
  zbuffer testing.  Again, useful for rendering to a texture.
Known problems:
- Streamed render targets are getting flushed and reloaded when we need them preserved.
  Fix may be to reset texture stream vram before loading them, to ensure maximal
  retention, but this reset should only occur at appropriate times.  Fix might also
  be to temporarily lock texture at start of vram until it is not referenced as either
  a render target or a source texture.
Recommendation:
- Split nglCurScene into nglCurSelectedScene and nglCurRenderScene,
  one for tracking scene additions and parameter setting, the other for tracking
  which scene is currently being rendered.
  
4/4/2002 - v1.3.107P
Changes/additions:
- Correction to nglListBeginScene handling of sortinfo

4/3/2002 - v1.3.106P
Changes/additions:
- Corrected code for streamed render target textures, so that texture is guaranteed
  to be present whenever it is needed as a target, and the frame buffer pointer
  is pointing to it.  Also reworked code so that any texture used as a streamed render
  target, not just vram only textures, will be correctly aligned on a 2k boundary.

4/2/2002 - v1.3.105P
Changes/additions:
- Changes to Scene API:
  nglListBeginScene now takes an optional nglSortInfo parameter and returns the new scene.
  The nglSortInfo parameter if supplied, allows the scene to be sorted and rendered
  intermixed with node rendering, rather than before all nodes.  This is a PS2 only feature.
  nglListSelectScene is new, and allows the current scene to be selected, returning the
  previously selected scene.
- Added support for NGLTF_VRAM_ONLY textures (render targets) that are not locked
  (locked vram only textures were already supported).

4/1/2002 - v1.3.104P
Bug fixes:
- nglRebaseMesh was not relocating Section->Material->Info->BakedDMA.
- nglVif1UnpackBones was generating packet data with wrong alignment
- In nglListAddMesh, use of 3x4 transform_t was leaving last column garbage.
  Modified to fill in last column with 0,0,0,1.
- Rendering of clipped skinned scratch meshes is being corrupted.
  The problem goes away when rendering is stepped with breakpoints,
  so it appears to be a synchronization issue, with data being uploaded
  too early on top of data being rendered.  For now, nglCreateMeshDMA
  was modified to add the following at the end of each batch:
    *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );
    nglVif1AddCommandListExec( Packet, 0, NGLCMD_VERTEX_LIGHT );
    *(Packet++) = SCE_VIF1_SET_FLUSH( 0 );

3/11/2002 - v1.3.103P
Changes/additions:
- Added new baked material code.  Big EE performance improvement to DMA building.
- Moved the sorting of translucent and opaque models onto the scratchpad.

Known problems:
- Custom nodes will likely need updating.
- Tinting code can cause Vif1 timeouts (nonfatal).
- Some features may remain unimplemented.

2/20/2002 - v1.3.102P:
Changes/additions:
- Added a stall to wait for rendering to finish before releasing texture resources.
- Switched to bilinear filtering with nearest mipmaps by default.
- Moved the palette AFTER the texture image data in video memory.
- Added code to normalize the rotation component of the LocalToWorld matrix in nglListAddMesh.

2/18/2002 - v1.3.101P:
Changes/additions:
- New baked texture DMA code, big optimization to texture streaming time and gif packet workspace size.
- Cleaned up the multi-format TGA/TM2 texture code somewhat.
- Readded Montague's Ortho matrix fix.

Known problems:
- Experimental texture streaming code, needs extensive testing.
- Had to drop support for nglLoadTextureSplitTM2.
- Slightly higher memory usage for textures (baked DMA).

2/13/2002 - v1.3.100P:
Changes/additions:
- Started ngl_version.txt for the PS2.
*/
#endif
