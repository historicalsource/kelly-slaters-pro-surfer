;
; This is the source VU code for the environment mapping code.  To be run through VCL.
;

	.init_vf_all
	.init_vi_all
	.syntax new

	--enter
	in_vi gSrcBufPtr(VI13)
	in_vi gGIFTagMask(VI14)
	in_vi gDataPtr(VI15)
	out_vi gDataPtr(VI15)
	--endenter

nglEnvironmentMapCylinderAddr:
	LQ CamPos, 0(gDataPtr)
    LQ.xy CylParms, 1(gDataPtr)
    IADDI gDataPtr, gDataPtr, 2

    XITOP Temp                          // get base ptr
    MULx Zero, VF00, VF00                   
	IADDIU DestPtr, Temp, gVertBasePtr
    ILW.x VertCnt, gGIFTagPtr+0(Temp)x
    IAND VertCnt, VertCnt, gGIFTagMask
    IADD SrcPtr, Temp, gSrcBufPtr

	// set up sign flag registers
	IADDIU XSignFlag, VI00, 0x0080
	IADDIU ZSignFlag, VI00, 0x0020

	// set up our PI constants
	LOI 1.5707963267948966192313216916398
	ADDi.x PI_OVER_2, VF00, I				

	LOI 3.1415926535897932384626433832795
	ADDi.x PI, VF00, I						

	LOI 4.7123889803846898576939650749193
	ADDi.x _3PI_OVER_2, VF00, I				

	LOI 6.283185307179586476925286766559
	ADDi.x _2PI, VF00, I					

EnvCylinderVertLoop:

	--LoopCS	3,3		
	--LoopExtra 3

	LQ SrcXYZ, 0(SrcPtr)
	LQ SrcNorm, 1(SrcPtr)
	LQ SrcColor, 1(DestPtr)
    LQ SrcST, 0(DestPtr)


	; compute the vector from vertex to camera
    IADDI VertCnt, VertCnt, -1
	IADDI SrcPtr, SrcPtr, 2
	IADDI DestPtr, DestPtr, 3
	
	SUB.xyzw CamNorm, CamPos, SrcXYZ		

	; normalize the vector from vertex to camera.
	ERLENG P, CamNorm
	MFP.w InvLength, P
	MULw.xyz CamNorm, CamNorm, InvLength	

	; dot the vertex normal with the normalized vector to the camera
	MUL.xyz Dot, CamNorm, SrcNorm
	ADDz.x Dot, Dot, Dot
	ADDy.x Dot, Dot, Dot				    

	; compute the reflection vector
	LOI 2.0
	MULi.xyz Refl, SrcNorm, I			   
	MULx.xyz Refl, Refl, Dot				
	SUB.xyz Refl, Refl, CamNorm				

	; use this to turn off the reflection mapping to see how normals map into the map
;		ADD.xyz Refl, VF00, SrcNorm				

	// begin ST calculations
	ADD.xyz ST, VF00, Refl					
	ABS.xz ST, ST							
	SUBz.x Compare, ST, ST					
	FMAND Reflect, XSignFlag
	IBEQ VI00, Reflect, SwitchDone

	// switch x and z
	MULx.w ST, VF00, ST					
	ADDz.x ST, VF00, ST					
	ADDw.z ST, VF00, ST					

SwitchDone:

	; TODO: replace EATANxz with a less accurate approximation (say four terms instead of 8)
	; and done in the upper execution unit

	; now start the atan and square root
	EATANxz P, ST
	MUL.xz ST, ST, ST						
	ADDz.x ST, ST, ST					
	MUL.y ST, ST, CylParms					
	RSQRT Q, VF00[w], ST[x]
	MULq.y ST, ST, Q						
	MFP.x ST, P

	; figure out the octant from the original reflection vector and adjust the angle appropriately
	ADD.xyz Refl, VF00, Refl				
	FMAND Branch, ZSignFlag
	IBNE VI00, Branch, Quadrant3or4
	FMAND Branch, XSignFlag
	IBNE VI00, Branch, Quadrant2

; Quadrant 1/Octant 1
	IBEQ VI00, Reflect, AngleDone

; Octant 2
	SUB.x ST, PI_OVER_2, ST					
	B AngleDone

Quadrant2:
	IBEQ VI00, Reflect, Octant4

; Octant 3
	ADD.x ST, PI_OVER_2, ST					
	B AngleDone

Octant4:
	SUB.x ST, PI, ST						
	B AngleDone

Quadrant3or4:
	FMAND Branch, XSignFlag
	IBEQ VI00, Branch, Quadrant4

// Quadrant 3
	IBNE VI00, Reflect, Octant6

// Octant 5
	ADD.x ST, PI, ST						
	B AngleDone

Octant6:
	SUB.x ST, _3PI_OVER_2, ST				
	B AngleDone

Quadrant4:
	IBEQ VI00, Reflect, Octant8

// Octant 7
	ADD.x ST, _3PI_OVER_2, ST				
	B AngleDone

Octant8:
	SUB.x ST, _2PI, ST						

AngleDone:
	LOI 0.15915494309189533576888376337251
	MULi.x ST, ST, I						
	MULx.y ST, ST, CylParms				
	LOI 0.5
	ADDi.y ST, ST, I						
	MULz.xy ST, ST, SrcST					
	SQ SrcColor, -2(DestPtr)
	SQ.xy ST, -3(DestPtr)

	IBGTZ VertCnt, EnvCylinderVertLoop

	--exit
	--endexit
