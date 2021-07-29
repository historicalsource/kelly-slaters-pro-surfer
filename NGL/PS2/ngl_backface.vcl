;
; This is the source VU code for backface culling.  To be run through VCL.
;

	.init_vf_all
	.init_vi_all
	.syntax new

	--enter
	in_vi gSrcBufPtr(VI13)
	in_vi gGIFTagMask(VI14)
	in_vi gDataPtr(VI15)
	--endenter

nglBackfaceCullScreenAddr:
	LQ ViewPos, 0(gDataPtr)
	IADDI gDataPtr, gDataPtr, 1

    XITOP Temp                          // get base ptr
	IADDIU DestPtr, Temp, gVertBasePtr
    ILW.x VertCnt, gGIFTagPtr+0(Temp)x
    IAND VertCnt, VertCnt, gGIFTagMask
    IADD SrcPtr, Temp, gSrcBufPtr

    MULx Zero, VF00, VF00                   

    IADDIU HardClip, VI00, 0x7fff
    IADDIU HardClip, HardClip, 1
    IADDIU SoftClip, HardClip, 1
	IADDIU CWFlag, VI00, 2

	LQ SrcXYZ2, 0(SrcPtr)
	LQ SrcXYZ1, 2(SrcPtr)
    IADDI VertCnt, VertCnt, -2
    IADDI DestPtr, DestPtr, 6
	IADDI SrcPtr, SrcPtr, 4

BackfaceScreenVertLoop:
	--LoopCS	3,3		
    ADD SrcXYZ3, SrcXYZ2, Zero              
    ADD SrcXYZ2, SrcXYZ1, Zero              

	LQ SrcXYZ1, 0(SrcPtr) 
    ADDw.w Sign, Zero, VF00					

	ILW.w Temp, 2(DestPtr)
    IAND Temp, CWFlag, Temp
    IBEQ Temp, CWFlag, BackfaceScreenSkipReset
	SUBw.w Sign, Zero, VF00                 

BackfaceScreenSkipReset:
    SUB Delta1, SrcXYZ1, SrcXYZ3            
    SUB Delta2, SrcXYZ2, SrcXYZ3            

    OPMULA.xyz ACC, Delta1, Delta2          
    OPMSUB.xyz Cross, Delta2, Delta1        
    SUB.xyz ViewVec, ViewPos, SrcXYZ1		

    MUL.xyz Dot, ViewVec, Cross				

    ADDAx.w ACC, Zero, Dot                  
    MADDAy.w ACC, VF00, Dot                 
    MADDz.w Dot, VF00, Dot                  

	IADDI VertCnt, VertCnt, -1
	IADDI SrcPtr, SrcPtr, 2   
	IADDI DestPtr, DestPtr, 3 

    MUL.w Dot, Dot, Sign					
    
	IADDIU VI01, VI00, 0x10				// W field sign bit.
    FMAND VI01, VI01

    IBNE VI01, VI00, BackfaceScreenClip

BackfaceScreenNextVert:
    IBGTZ VertCnt, BackfaceScreenVertLoop

	B CommandListReturn

BackfaceScreenClip:
    IBGTZ VertCnt, BackfaceScreenVertLoop
    ISW.w HardClip, -1(DestPtr)          // write out the XYZ3 bit.

	B CommandListReturn

CommandListReturn:
	--exit
	out_vi gDataPtr(VI15)
	--endexit