	.init_vf_all
	.init_vi_all
	.syntax new

	--enter
	in_vi gSrcBufPtr(VI13)
	in_vi gGIFTagMask(VI14)
	in_vi gDataPtr(VI15)
	--endenter

nglTransformAddr:
	XITOP Temp
    IADDIU DestPtr, Temp, gVertBasePtr
    IADD SrcPtr, Temp, gSrcBufPtr
	ILW.x VertCnt, gGIFTagPtr+0(Temp)x
	IAND VertCnt, VertCnt, gGIFTagMask

    LQ LToS1, gLocalToScreenPtr+0(VI00)     // load screen x transformation matrix
    LQ LToS2, gLocalToScreenPtr+1(VI00) 
    LQ LToS3, gLocalToScreenPtr+2(VI00) 
    LQ LToS4, gLocalToScreenPtr+3(VI00) 

    // --- calc transformed position and store it ---
VertLoop:
	--LoopCS 3,3
	LQ SrcXYZ, 0(SrcPtr)
	LQ SrcST, 0(DestPtr)

    MULAx ACC, LToS1, SrcXYZ             
    MADDAy ACC, LToS2, SrcXYZ            
    MADDAz ACC, LToS3, SrcXYZ            
    MADDw ProjXYZ, LToS4, VF00
	 
	DIV Q, VF00[w], ProjXYZ[w]

    MULq.xyz ProjST, SrcST, Q
	SQ.xyz ProjST, 0(DestPtr)

    MULq ProjXYZ2, ProjXYZ, Q               
    FTOI4.xyz FixedXYZ, ProjXYZ2			
	SQ.xyz FixedXYZ, 2(DestPtr)

	IADDI DestPtr, DestPtr, 3
	IADDI SrcPtr, SrcPtr, 2

	IADDI VertCnt, VertCnt, -1
	IBGTZ VertCnt, VertLoop

	--exit
	out_vi gDataPtr(VI15)
	--endexit