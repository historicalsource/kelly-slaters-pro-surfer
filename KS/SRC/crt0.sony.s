/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 2.5
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.10
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libkernel - crt0.s
 *                        kernel libraly
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.10           Oct.12.1999     horikawa    renewal
 *       1.50           May.16.2000     horikawa
 *       2.50           Feb.18.2002     kumagae
 */

#ifdef __mips16
	.set nomips16	/* This file contains 32 bit assembly code. */
#endif

#define	ARG_SIZ     256 + 16*4 + 1*4

	.set noat
    	.set noreorder
	.global ENTRYPOINT
	.global _start
	.ent	_start
	.text				# 0x00200000
	nop
	nop
ENTRYPOINT:
_start:
/* Clear GPR , FPR */
        padduw  $1, $0, $0
        padduw  $2, $0, $0
        padduw  $3, $0, $0
        padduw  $4, $0, $0
        padduw  $5, $0, $0
        padduw  $6, $0, $0
        padduw  $7, $0, $0
        padduw  $8, $0, $0
        padduw  $9, $0, $0
        padduw  $10, $0, $0
        padduw  $11, $0, $0
        padduw  $12, $0, $0
        padduw  $13, $0, $0
        padduw  $14, $0, $0
        padduw  $15, $0, $0
        padduw  $16, $0, $0
        padduw  $17, $0, $0
        padduw  $18, $0, $0
        padduw  $19, $0, $0
        padduw  $20, $0, $0
        padduw  $21, $0, $0
        padduw  $22, $0, $0
        padduw  $23, $0, $0
        padduw  $24, $0, $0
        padduw  $25, $0, $0

        padduw  $28, $0, $0
        padduw  $29, $0, $0	
        padduw  $30, $0, $0
        padduw  $31, $0, $0

	mthi    $0
	mthi1   $0
	mtlo    $0
	mtlo1   $0
	mtsah   $0, 0
	
    	mtc1    $0, $f0
   	mtc1    $0, $f1
    	mtc1    $0, $f2
    	mtc1    $0, $f3
    	mtc1    $0, $f4
    	mtc1    $0, $f5
    	mtc1    $0, $f6
    	mtc1    $0, $f7
    	mtc1    $0, $f8
    	mtc1    $0, $f9
    	mtc1    $0, $f10
    	mtc1    $0, $f11
    	mtc1    $0, $f12
    	mtc1    $0, $f13
    	mtc1    $0, $f14
    	mtc1    $0, $f15
    	mtc1    $0, $f16
    	mtc1    $0, $f17
    	mtc1    $0, $f18
    	mtc1    $0, $f19
    	mtc1    $0, $f20
    	mtc1    $0, $f21
    	mtc1    $0, $f22
    	mtc1    $0, $f23
    	mtc1    $0, $f24
    	mtc1    $0, $f25
    	mtc1    $0, $f26
    	mtc1    $0, $f27
    	mtc1    $0, $f28
    	mtc1    $0, $f29
    	mtc1    $0, $f30
    	mtc1    $0, $f31
    	adda.s  $f0, $f1
    	sync.p
    	ctc1    $0, $31
	
/*
 * clear .bss
 */
zerobss:
	lui	$2, %hi(_fbss)
	lui	$3, %hi(_end)
	addiu	$2, $2, %lo(_fbss)
	addiu	$3, $3, %lo(_end)
1:
	sq	$0, ($2)
	nop
	sltu	$1, $2, $3
	nop
	nop
	bne	$1, $0, 1b
	addiu	$2, $2, 16

/*
 * initialize main thread
 */
	lui	$4, %hi(_gp)
	lui	$5, %hi(_stack)
	lui	$6, %hi(_stack_size)
	lui	$7, %hi(_args)
	lui	$8, %hi(_root)
	addiu	$4, $4, %lo(_gp)
	addiu	$5, $5, %lo(_stack)
	addiu	$6, $6, %lo(_stack_size)
	addiu	$7, $7, %lo(_args)
	addiu	$8, $8, %lo(_root)
	move	$28, $4
	addiu	$3, $0, 60
	syscall
	move	$29, $2

/*
 * initialize heap area
 */
	lui	$4, %hi(_end)
	lui	$5, %hi(_heap_size)
	addiu	$4, $4, %lo(_end)
	addiu	$5, $5, %lo(_heap_size)
	addiu	$3, $0, 61
	syscall

/*
 * initialize System
 */
	jal	_InitSys
	nop

/*
 * flush data cache
 */
	jal	FlushCache
	move	$4, $0

/*
 * call main program
 */
	ei
	lui	$2, %hi(_args)
	addiu	$2, $2, %lo(_args)
	lw	$4, ($2)
	jal	main
	addiu	$5, $2, 4

	j	Exit
	move	$4, $2
	.end	_start

/**************************************/
	.align	3
	.global	_exit
	.ent	_exit
_exit:
	j	Exit			# Exit(0);
	move	$4, $0
	.end	_exit
    
	.align	3
	.ent	_root
_root:
	addiu	$3, $0, 35		# ExitThread();
	syscall
	.end	_root

	.bss
	.align	6
_args: .space	ARG_SIZ

