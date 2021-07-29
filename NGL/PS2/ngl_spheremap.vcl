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

nglEnvironmentMapAddr:
	LQ CameraPos, 0(gDataPtr)
    IADDI gDataPtr, gDataPtr, 1

    XITOP Temp                          // get base ptr
    MULx Zero, VF00, VF00                   
	IADDIU DestPtr, Temp, gVertBasePtr
    ILW.x VertCnt, gGIFTagPtr+0(Temp)x
    IAND VertCnt, VertCnt, gGIFTagMask
    IADD SrcPtr, Temp, gSrcBufPtr

EnvSphereVertLoop:
	--LoopCS	3,3		

	LQ SrcColor, 1(DestPtr)
    LQ SrcXYZ, 0(SrcPtr)
    LQ SrcNorm, 1(SrcPtr)
    LQ SrcST, 0(DestPtr)

    SUB.w SrcColor, VF00, SrcColor          
	IADDI VertCnt, VertCnt, -1
	IADDIU SrcPtr, SrcPtr, 2
    IADDIU DestPtr, DestPtr, 3

	// calculate a normalized vector from the vertex to the camera stored in Vec0.
    SUB.xyz Vec0, SrcXYZ, CameraPos         
	ERLENG P, Vec0
	MFP.w InvLength, P
	MULw.xyz Vec0, Vec0, InvLength	    

	// reflect the camera -> vertex vector across the normal (ends in Norm)
	MUL.xyz Dot, Vec0, SrcNorm              
	ADDz.x Dot, Dot, Dot				    
	ADDy.x Dot, Dot, Dot			    

	LOI -2.0
	MULi.xyz Norm, SrcNorm, I			    
	MULx.xyz Norm, Norm, Dot		    
	ADD.xyz Norm, Vec0, Norm				

	LOI 0.0
	MINIi.x Dot, Dot, I				        
	ADDw.y Norm, Norm, VF00			

	LOI 1.0
	ADDi.x Dot, Dot, I				        

	LOI 0.25
	MAXi.x Dot, Dot, I				        

	MUL.xyz Vec0, Norm, Norm				
	ADDy.x Vec0, Vec0, Vec0			
	ADDz.x Vec0, Vec0, Vec0			

	LOI 0.5
	MULi.xz Norm, Norm, I					

	RSQRT Q, VF00[w], Vec0[x]
	ADDAi.xz ACC, VF00, I					
	MADDq.xz SrcXYZ, Norm, Q				

	LOI 128.0
	ADDi.xyz SrcColor, VF00, I				

	LOI 80.0
	MULi.w SrcColor, SrcColor, I			
	MULx.w SrcColor, SrcColor, Dot
	FTOI0 SrcColor, SrcColor                

	ADDz.y SrcXYZ, VF00, SrcXYZ		    
	MULz.xy SrcXYZ, SrcXYZ, SrcST           

//	SQ SrcColor, -2(DestPtr)
	SQ.xy SrcXYZ, -3(DestPtr)

	IBGTZ VertCnt, EnvSphereVertLoop

	--exit
	out_vi gDataPtr(VI15)
	--endexit