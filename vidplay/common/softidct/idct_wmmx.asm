;*****************************************************************************
;*
;* This program is free software ; you can redistribute it and/or modify
;* it under the terms of the GNU General Public License as published by
;* the Free Software Foundation; either version 2 of the License, or
;* (at your option) any later version.
;*
;* This program is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;* GNU General Public License for more details.
;*
;* You should have received a copy of the GNU General Public License
;* along with this program; if not, write to the Free Software
;* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
;*
;* $Id: idct_wmmx.asm 3 2004-07-13 11:26:13Z picard $
;*
;* The Core Pocket Media Player
;* Copyright (c) 2004-2005 Gabor Kovacs
;*
;*****************************************************************************

	macro
	deftable $c1,$c2,$c3,$c4,$c5,$c6,$c7
	DCW  $c4,  $c2, -$c4, -$c2
	DCW  $c4,  $c6,  $c4,  $c6
	DCW  $c1,  $c3, -$c1, -$c5
	DCW  $c5,  $c7,  $c3, -$c7
	DCW  $c4, -$c6,  $c4, -$c6
	DCW -$c4,  $c2,  $c4, -$c2
	DCW  $c5, -$c1,  $c3, -$c1
	DCW  $c7,  $c3,  $c7, -$c5
	mend

	AREA	|.text|, CODE

	EXPORT WMMXIDCT_Block8x4
	EXPORT WMMXIDCT_Block8x8
	EXPORT WMMXIDCT_Const8x8

	ALIGN 16
table04:
	deftable 22725, 21407, 19266, 16384, 12873,  8867, 4520
table17:
	deftable 31521, 29692, 26722, 22725, 17855, 12299, 6270
table26:
	deftable 29692, 27969, 25172, 21407, 16819, 11585, 5906
table35:
	deftable 26722, 25172, 22654, 19266, 15137, 10426, 5315

	macro
	col $n



	mend

	macro
	row_head $n,$table
	mend

	macro
	row $table, $rounder
	mend

	macro
	row_mid $n,$m,$table
	mend

	macro
	row_tail $n
	mend

	ALIGN 16
;WMMXIDCT_Block8x4(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src)
WMMXIDCT_Block8x4 PROC
    row_head 0*8, table04
    row table04, rounder0
    row_mid 0*8, 4*8, table04
    row table04, rounder4
    row_mid 4*8, 1*8, table17
    row table17, rounder1
    row_mid 1*8, 7*8, table17
    row table17, rounder7
    row_mid 7*8, 2*8, table26
    row table26, rounder2
    row_mid 2*8, 6*8, table26
    row table26, rounder6
    row_mid 6*8, 3*8, table35
    row table35, rounder3
    row_mid 3*8, 5*8, table35
    row table35, rounder5
    row_tail 5*8

    col 0
    col 4

	mov pc,lr
	endp

	ALIGN 16
;WMMXIDCT_Block8x8(idct_block_t *Block, uint8_t *Dest, int DestStride, const uint8_t *Src)
WMMXIDCT_Block8x8 PROC
    row_head 0*8, table04
    row table04, rounder0
    row_mid 0*8, 4*8, table04
    row table04, rounder4
    row_mid 4*8, 1*8, table17
    row table17, rounder1
    row_mid 1*8, 7*8, table17
    row table17, rounder7
    row_mid 7*8, 2*8, table26
    row table26, rounder2
    row_mid 2*8, 6*8, table26
    row table26, rounder6
    row_mid 6*8, 3*8, table35
    row table35, rounder3
    row_mid 3*8, 5*8, table35
    row table35, rounder5
    row_tail 5*8

	col 0
	col 4

	mov pc,lr
	endp

	ALIGN 16
;WMMXIDCT_Const8x8(int v,uint8_t * Dst,int DstPitch, uint8_t * Src)
WMMXIDCT_Const8x8 PROC
	cmp r0,#0
	bgt const8x8add
	blt const8x8sub
	cmp r1,r3
	beq const8x8done

	macro
	const8x8copyrow
	wldrd   wr1,[r3]
	add		r3,r3,#8
	wldrd   wr2,[r3]
	add		r3,r3,#8
	wstrd   wr1,[r1]
	add		r1,r1,r2
	wstrd   wr2,[r1]
	add		r1,r1,r2
	mend
	
	const8x8copyrow
	const8x8copyrow
	const8x8copyrow
	const8x8copyrow

const8x8done
	mov pc,lr

const8x8add	
	macro
	const8x8addrow
	wldrd   wr1,[r3]
	add		r3,r3,#8
	wldrd   wr2,[r3]
	add		r3,r3,#8
	waddbus wr1,wr1,wr0
	waddbus wr2,wr2,wr0
	wstrd   wr1,[r1]
	add		r1,r1,r2
	wstrd   wr2,[r1]
	add		r1,r1,r2
	mend

	tbcstb  wr0,r0
	const8x8addrow
	const8x8addrow
	const8x8addrow
	const8x8addrow
	mov pc,lr

const8x8sub
	macro
	const8x8subrow
	wldrd   wr1,[r3]
	add		r3,r3,#8
	wldrd   wr2,[r3]
	add		r3,r3,#8
	wsubbus wr1,wr1,wr0
	wsubbus wr2,wr2,wr0
	wstrd   wr1,[r1]
	add		r1,r1,r2
	wstrd   wr2,[r1]
	add		r1,r1,r2
	mend

	rsb r0,r0,#0
	tbcstb  wr0,r0
	const8x8subrow
	const8x8subrow
	const8x8subrow
	const8x8subrow
	mov pc,lr

	endp

	end
