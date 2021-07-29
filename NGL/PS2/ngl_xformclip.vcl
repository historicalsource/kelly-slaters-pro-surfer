	.init_vf_all
	.init_vi_all
	.syntax new

	--enter
	in_vi gSrcBufPtr(VI13)
	in_vi gGIFTagMask(VI14)
	in_vi gDataPtr(VI15)
	--endenter

nglTransformClipAddr:
    XITOP Temp                          ; get base ptr
    IADDIU DestPtr, Temp, gVertBasePtr
    IADD SrcPtr, Temp, gSrcBufPtr
    ILW.x VertCnt, gGIFTagPtr+0(Temp)x
    IAND VertCnt, VertCnt, gGIFTagMask   

    LQ LToS1, gLocalToScreenPtr+0(VI00)   ; load screen x transformation matrix
    LQ LToS2, gLocalToScreenPtr+1(VI00)
    LQ LToS3, gLocalToScreenPtr+2(VI00)
    LQ LToS4, gLocalToScreenPtr+3(VI00)
    LQ LToC1, gLocalToClipPtr+0(VI00)   ; load clip x transformation matrix
    LQ LToC2, gLocalToClipPtr+1(VI00)
    LQ LToC3, gLocalToClipPtr+2(VI00)
    LQ LToC4, gLocalToClipPtr+3(VI00)

    FCSET 0                             ; clear clip flags.
    IADDIU XYZ3Bit, VI00, 0x7fff        ; set 0x8000 mask
    IADDIU XYZ3Bit, XYZ3Bit, 1                            
												                       
ClipVertLoop:
	--LoopCS 3, 3

    LQ SrcXYZ, 0(SrcPtr)                
    LQ SrcST, 0(DestPtr)                

    MULAx ACC, LToS1, SrcXYZ             
    MADDAy ACC, LToS2, SrcXYZ            
    MADDAz ACC, LToS3, SrcXYZ            
    MADDw ProjXYZ, LToS4, VF00        

	DIV Q, VF00[w], ProjXYZ[w]           
    
	MULq.xyz ProjST, SrcST, Q               
	SQ.xyz ProjST, 0(DestPtr)          

    MULq.xyz ProjXYZ2, ProjXYZ, Q          
    FTOI4.xyz FixedXYZ, ProjXYZ2            
    SQ.xyz FixedXYZ, 2(DestPtr)        ; store XYZ

	; clip the vertex
    MULAx.xyzw ACC, LToC1, SrcXYZ        
    MADDAy.xyzw ACC, LToC2, SrcXYZ   
    MADDAz.xyzw ACC, LToC3, SrcXYZ       
    MADDw.xyzw ClipXYZ, LToC4, VF00        
    CLIPw.xyz ClipXYZ, ClipXYZ  

	FCAND VI01, 0x0003ffff
	IADD VI01, VI01, XYZ3Bit

	; dummy instruction to generate a dependency on VI01
    ISW.w VI01, 2(DestPtr)          ; write out the XYZ3 bit.

	IADDI VertCnt, VertCnt, -1 
	IADDI DestPtr, DestPtr, 3  
	IADDI SrcPtr, SrcPtr, 2    

    IBGTZ VertCnt, ClipVertLoop
		
	--exit
	out_vi gDataPtr(VI15)
	--endexit

