	.init_vf_all
	.init_vi_all
	.syntax new

	--enter
	in_vi gSrcBufPtr(VI13)
	in_vi gGIFTagMask(VI14)
	in_vi gDataPtr(VI15)
	--endenter

nglFrustumClipAddr:
		// NOTE: This is not all the prologue code needed for nglFrustumClipAddr.
		// Needs some modification in order to be properly added to ngl_vu1.dsm.
        XITOP Temp 
        IADDIU StackPtr, VI00, 1023              // set stack pointer

        MULx Zero, VF00, VF00                   
		
		IADDIU DestPtr, Temp, gVertBasePtr
        ILW.x VertCnt, gGIFTagPtr+0(Temp)x
        IAND VertCnt, VertCnt, gGIFTagMask
        IADD SrcPtr, Temp, gSrcBufPtr

        IADDI Clip1, VI00, -2                   
        IADDI Clip2, VI00, -2                                            
        IADDI Clip3, VI00, -2                                            

        IADDIU MaskFlg, VI00, 0x7fff            // set mask                      
        IADDI  MaskFlg, MaskFlg, 1              // set XYZ3 flag to MaskFlg
        IADDI FacingFlg, VI00, 0                // Init FacingFlg to CW (as opposed to CCW).
        IADDI  StripFlg, MaskFlg, 1             // set XYZ3 flag to MaskFlg

        LQ LToC1, gLocalToClipPtr+0(VI00)   // load clip x transformation matrix
        LQ LToC2, gLocalToClipPtr+1(VI00)
        LQ LToC3, gLocalToClipPtr+2(VI00)
        LQ LToC4, gLocalToClipPtr+3(VI00)

		FCSET 0

FrustumClipVertLoop:
		--LoopCS 3, 3
        LQ SrcVert1, 0(SrcPtr)                  
        IADDI VertCnt, VertCnt, -1
        IADDI SrcPtr, SrcPtr, 2 

        MULAx ACC, LToC1, SrcVert1              
        MADDAy ACC, LToC2, SrcVert1             
        MADDAz ACC, LToC3, SrcVert1             
        MADDw XYZ, LToC4, VF00					

        CLIPw.xyz XYZ, XYZ						
        FCAND VI01, 0x3FFFF                        
        ISW.w VI01, 0(DestPtr)
//		IBGTZ VI01, ScissorVert                       

        IADDI DestPtr, DestPtr, 3                                   
        IBGTZ  VertCnt, FrustumClipVertLoop                          

ScissorReturn:
ScissorVert:
		// Coming here implies that atleast one vert is to be clipped by atleast one plane.
		// If all 3 verts are clipped off by any one plane then the triangle needs simple rejection.
        FCOR VI01, 0xfdf7df //^0x20820         
        IBNE VI01, VI00, RejectVert            

        FCOR VI01, 0xfefbef // ^0x10410         
        IBNE VI01, VI00, RejectVert            

        FCOR VI01, 0xff7df7 // ^0x8208          
        IBNE VI01, VI00, RejectVert            

        FCOR VI01, 0xffbefb // ^0x4104          
        IBNE VI01, VI00, RejectVert            

        FCOR VI01, 0xffdf7d // ^0x2082          
        IBNE VI01, VI00, RejectVert            

        FCOR VI01, 0xffefbe // ^0x1041          
        IBNE VI01, VI00, RejectVert            

        IAND VI01, Clip2, VI01					
        IAND VI01, Clip3, VI01
        IBNE VI01, VI00, RejectVert            

		ILW.w Temp, 2(DestPtr)
		IAND Temp, StripFlg, Temp
		IBEQ Temp, MaskFlg, ScissorReturn // (0x8001 implies clipped by VU routines (backface cull or frustum clip). 0x8000 implies culled elsewhere)

		// Load up the values expected by the scissor function
		LQ SrcVert1, -6(SrcPtr)                  // load vertex          
        MULAx ACC, LToC1, SrcVert1              
        MADDAy ACC, LToC2, SrcVert1             
        MADDAz ACC, LToC3, SrcVert1             
        MADDw PrevXYZ1, LToC4, VF00			
		
		LQ SrcVert1, -4(SrcPtr)                  
        MULAx ACC, LToC1, SrcVert1              
        MADDAy ACC, LToC2, SrcVert1             
        MADDAz ACC, LToC3, SrcVert1             
        MADDw PrevXYZ2, LToC4, VF00			
		
		LQ SrcVert1, -2(SrcPtr)                 
        MULAx ACC, LToC1, SrcVert1              
        MADDAy ACC, LToC2, SrcVert1             
        MADDAz ACC, LToC3, SrcVert1             
        MADDw PrevXYZ3, LToC4, VF00			

//        BAL  ReturnPtr, PushStackIFRegs        
//        BAL  ReturnPtr, SCISSOR                
//        BAL  ReturnPtr, PopStackIFRegs         
        
		CLIPw.xyz PrevXYZ1, PrevXYZ1    
        CLIPw.xyz PrevXYZ2, PrevXYZ2    
        CLIPw.xyz PrevXYZ3, PrevXYZ3

RejectVert: 
        ISW.w StripFlg, 2(DestPtr)             // kick XYZ3 (0x8001 indicates clipped by VU routines (frustum clip or backface cull)  
        B ScissorReturn

	--exit
	out_vi gDataPtr(VI15)
	--endexit