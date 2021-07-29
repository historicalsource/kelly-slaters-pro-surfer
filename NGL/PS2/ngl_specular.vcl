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
	--endenter

nglSpecularMapAddr:
	LQ ViewDir, 0(gDataPtr)				
	LQ LightDir, 1(gDataPtr)			
    LQ Specular, 2(gDataPtr)			// specular power is in x, strength (0-255) is in y, 'magic' is in w.
	IADDI gDataPtr, gDataPtr, 3

    XITOP Temp                          
	IADDIU DestPtr, Temp, gVertBasePtr
    ILW.x VertCnt, gGIFTagPtr+0(Temp)x
    IAND VertCnt, VertCnt, gGIFTagMask
    IADD SrcPtr, Temp, gSrcBufPtr
    MULx Zero, VF00, VF00                   

SpecularVertLoop:
	--LoopCS	3,3		

    LQ SrcNorm, 1(SrcPtr)

	// dot ViewDir with SrcNorm
	MUL.xyz Dot, ViewDir, SrcNorm
    ADDAx.w ACC, Zero, Dot 
    MADDAy.w ACC, VF00, Dot
	MADDz.w Dot, VF00, Dot

	// reflect ViewDir across the plane made by SrcNorm, stored into Ref.
	// Ref = ( ViewDir dot SrcNorm ) * SrcNorm * 2.0f;
	MULw.xyz RefVec, SrcNorm, Dot

	LOI 2.0
	MULi.xyz RefVec2, RefVec, I

	ADD.xyz Ref, ViewDir, RefVec2

	// cos(a) = Ref dot LightDir;
	MUL.xyz Cos, Ref, LightDir
    ADDAx.w ACC, Zero, Cos
    MADDAy.w ACC, VF00, Cos
	MADDz.w Cos, VF00, Cos

	// highlight = pow(cos,pow)*strength
//	ITOF0.w Cos, Cos
//	MULAx.w ACC, Cos, Specular
//	MADDw.w Cos, Specular, VF00
//	FTOI0.w Cos, Cos

	// 32nd power
	MUL.w Cos, Cos, Cos
	MUL.w Cos, Cos, Cos
	MUL.w Cos, Cos, Cos
	MUL.w Cos, Cos, Cos
	MUL.w Cos, Cos, Cos
	
	// scale to range
	MULy.w Cos, Cos, Specular

	LOI 255.0	
	ADDw.xyz Color, VF00, Cos
	ADDi.w Color, Zero, I
	FTOI0 Color, Color
		
	SQ Color, 1(DestPtr)

	IADDIU SrcPtr, SrcPtr, 2
    IADDIU DestPtr, DestPtr, 3
	IADDI VertCnt, VertCnt, -1

	IBGTZ VertCnt, SpecularVertLoop

	--exit
	out_vi gDataPtr(VI15)
	--endexit