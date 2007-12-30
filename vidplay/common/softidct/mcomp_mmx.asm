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
;* $Id: mcomp_mmx.asm 131 2004-12-04 20:36:04Z picard $
;*
;* The Core Pocket Media Player
;* Copyright (c) 2004-2005 Gabor Kovacs
;*
;*****************************************************************************

BITS 32
SECTION .text

%macro cglobal 2
%define %1 _%1@%2
global %1
%endmacro

cglobal EMMS,0
cglobal MMXAddBlock,12
cglobal MMXAddBlockHor,12
cglobal MMXAddBlockVer,12
cglobal MMXAddBlockHorVer,12
cglobal MMXCopyBlock16x16,16
cglobal MMXCopyBlock,16
cglobal MMXCopyBlockHor,16
cglobal MMXCopyBlockVer,16
cglobal MMXCopyBlockHorVer,16 
cglobal MMXCopyBlockHorRound,16
cglobal MMXCopyBlockVerRound,16 
cglobal MMXCopyBlockHorVerRound,16

ALIGN 16
EMMS:
	emms
	ret 0

%macro loadparam 1
	mov		esi,[esp+12]		;src
	mov		edi,[esp+12+4]		;dst
	mov		eax,[esp+12+8]		;src pitch
%if %1>0
	mov		edx,8				;dst pitch (fixed for MMXAddBlock)
%else	
	mov		edx,[esp+12+12]		;dst pitch
%endif
%endmacro

%macro loadmask1 0
	mov		ecx,0x01010101
	movd	mm6,ecx
	pcmpeqb mm7,mm7
	punpckldq mm6,mm6
	pxor	mm7,mm6
%endmacro

%macro loadmask4 0
	mov		ecx,0x03030303
	movd	mm6,ecx
	pcmpeqb mm7,mm7
	punpckldq mm6,mm6
	pxor	mm7,mm6
%endmacro

%macro load1 2
	movq	mm0,[esi+%1]
%if %2>0
	add		esi,eax
%endif
	movq	mm1,mm0
	pand	mm1,mm7
	psrlq	mm1,1
%endmacro

%macro load2 2
	movq	mm2,[esi+%1]
%if %2>0
	add		esi,eax
%endif
	movq	mm3,mm2
	pand	mm3,mm7
	psrlq	mm3,1
%endmacro

%macro load1hv 0
	movq	mm0,[esi]
	movq	mm4,[esi+1]
	add		esi,eax
	movq	mm1,mm0
	movq	mm5,mm4
	pand	mm0,mm6
	pand	mm4,mm6
	pand	mm1,mm7
	pand	mm5,mm7
	psrlq	mm1,2
	psrlq	mm5,2
	paddb	mm0,mm4
	paddb	mm1,mm5
%endmacro

%macro load2hv 0
	movq	mm2,[esi]
	movq	mm4,[esi+1]
	add		esi,eax
	movq	mm3,mm2
	movq	mm5,mm4
	pand	mm2,mm6
	pand	mm4,mm6
	pand	mm3,mm7
	pand	mm5,mm7
	psrlq	mm3,2
	psrlq	mm5,2
	paddb	mm2,mm4
	paddb	mm3,mm5
%endmacro

%macro avg1 0
	por		mm0,mm2
	pand	mm0,mm6
	paddb	mm0,mm1
	paddb	mm0,mm3
%endmacro

%macro avg2 0
	por		mm2,mm0
	pand	mm2,mm6
	paddb	mm2,mm3
	paddb	mm2,mm1
%endmacro

%macro avground1 0
	pand	mm0,mm2
	pand	mm0,mm6
	paddb	mm0,mm1
	paddb	mm0,mm3
%endmacro

%macro avground2 0
	pand	mm2,mm0
	pand	mm2,mm6
	paddb	mm2,mm3
	paddb	mm2,mm1
%endmacro

%macro save1 0
	movq	[edi],mm0
	add		edi,edx
%endmacro

%macro save2 0
	movq	[edi],mm2
	add		edi,edx
%endmacro

%macro saveadd1 0
	movq	mm4,[edi]
	movq	mm1,mm0
	pand	mm0,mm7
	por		mm1,mm4
	pand	mm4,mm7
	pand	mm1,mm6
	psrlq	mm0,1
	psrlq	mm4,1
	paddb	mm1,mm0
	paddb	mm1,mm4
	movq	[edi],mm1
	add		edi,edx
%endmacro

%macro saveadd2 0
	movq	mm4,[edi]
	movq	mm3,mm2
	pand	mm2,mm7
	por		mm3,mm4
	pand	mm4,mm7
	pand	mm3,mm6
	psrlq	mm2,1
	psrlq	mm4,1
	paddb	mm3,mm2
	paddb	mm3,mm4
	movq	[edi],mm3
	add		edi,edx
%endmacro

ALIGN 16
MMXCopyBlock:
	push	esi
	push	edi
	loadparam 0

%rep 4
	movq	mm0,[esi]
	movq	mm1,[esi+eax]
	lea		esi,[esi+eax*2]
	movq	[edi],mm0
	movq	[edi+edx],mm1
	lea		edi,[edi+edx*2]
%endrep

	pop		edi
	pop		esi 
	ret 16

ALIGN 16
MMXCopyBlock16x16:
	push	esi
	push	edi
	loadparam 0

%rep 8
	movq	mm0,[esi]
	movq	mm1,[esi+8]
	movq	mm2,[esi+eax]
	movq	mm3,[esi+eax+8]
	lea		esi,[esi+eax*2]
	movq	[edi],mm0
	movq	[edi+8],mm1
	movq	[edi+edx],mm2
	movq	[edi+edx+8],mm3
	lea		edi,[edi+edx*2]
%endrep

	pop		edi
	pop		esi 
	ret 16

ALIGN 16
MMXCopyBlockHor:
	push	esi
	push	edi

	loadparam 0
	loadmask1
	
%rep 8
	load1	0,0
	load2	1,1
	avg1
	save1
%endrep

	pop		edi
	pop		esi 
	ret 16

ALIGN 16
MMXCopyBlockVer:
	push	esi
	push	edi

	loadparam 0
	loadmask1
	load1	0,1
%rep 4 
	load2	0,1
	avg1
	save1
	load1	0,1
	avg2
	save2
%endrep

	pop		edi
	pop		esi 
	ret 16

ALIGN 16
MMXCopyBlockHorVer:
	push	esi
	push	edi

	loadparam 0
	loadmask4
	
	load1hv
%rep 4
	load2hv

	pcmpeqb mm4,mm4		;-1
	paddb	mm0,mm2
	paddb	mm4,mm4		;-2
	paddb	mm1,mm3
	psubb	mm0,mm4		;+2
	pand	mm0,mm7
	psrlq	mm0,2
	paddb	mm0,mm1

	save1

	load1hv

	pcmpeqb mm4,mm4		;-1
	paddb	mm2,mm0
	paddb	mm4,mm4		;-2
	paddb	mm3,mm1
	psubb	mm2,mm4		;+2
	pand	mm2,mm7
	psrlq	mm2,2
	paddb	mm2,mm3

	save2
%endrep

	pop		edi
	pop		esi 
	ret 16

ALIGN 16
MMXCopyBlockHorRound:
	push	esi
	push	edi

	loadparam 0
	loadmask1
	
%rep 8
	load1	0,0
	load2	1,1
	avground1
	save1
%endrep

	pop		edi
	pop		esi 
	ret 16

ALIGN 16
MMXCopyBlockVerRound:
	push	esi
	push	edi

	loadparam 0
	loadmask1
	load1	0,1
%rep 4 
	load2	0,1
	avground1
	save1
	load1	0,1
	avground2
	save2
%endrep

	pop		edi
	pop		esi 
	ret 16

ALIGN 16
MMXCopyBlockHorVerRound:
	push	esi
	push	edi

	loadparam 0
	loadmask4
	
	load1hv
%rep 4
	load2hv

	pcmpeqb mm4,mm4		;-1
	paddb	mm0,mm2
	paddb	mm1,mm3
	psubb	mm0,mm4		;+1
	pand	mm0,mm7
	psrlq	mm0,2
	paddb	mm0,mm1

	save1

	load1hv

	pcmpeqb mm4,mm4		;-1
	paddb	mm2,mm0
	paddb	mm3,mm1
	psubb	mm2,mm4		;+1
	pand	mm2,mm7
	psrlq	mm2,2
	paddb	mm2,mm3

	save2
%endrep

	pop		edi
	pop		esi 
	ret 16

ALIGN 16
MMXAddBlock:
	push	esi
	push	edi

	loadparam 1
	loadmask1

%rep 8
	movq	mm0,[esi]
	add		esi,eax
	saveadd1
%endrep

	pop		edi
	pop		esi 
	ret 12

ALIGN 16
MMXAddBlockHor:
	push	esi
	push	edi

	loadparam 1
	loadmask1
	
%rep 8
	load1	0,0
	load2	1,1
	avg1
	saveadd1
%endrep

	pop		edi
	pop		esi 
	ret 12

ALIGN 16
MMXAddBlockVer:
	push	esi
	push	edi

	loadparam 1
	loadmask1
	load1	0,1
%rep 4 
	load2	0,1
	avg1
	saveadd1
	load1	0,1
	avg2
	saveadd2
%endrep

	pop		edi
	pop		esi 
	ret 12

ALIGN 16
MMXAddBlockHorVer:
	push	esi
	push	edi

	loadparam 1
	loadmask4
	
	load1hv
%rep 4
	load2hv

	pcmpeqb mm5,mm5		;-1
	paddb	mm0,mm2
	paddb	mm5,mm5		;-2
	paddb	mm1,mm3
	psubb	mm0,mm5		;+=2
	pand	mm0,mm7
	psrlq	mm0,2
	paddb	mm0,mm1

	paddb	mm6,mm5		;0x03-2=0x01
	psubb	mm7,mm5		;0xFD+2=0xFF
	saveadd1
	psubb	mm6,mm5		;restore mask
	paddb	mm7,mm5		;restore mask

	load1hv

	pcmpeqb mm5,mm5		;-1
	paddb	mm2,mm0
	paddb	mm5,mm5		;-2
	paddb	mm3,mm1
	psubb	mm2,mm5		;+=2
	pand	mm2,mm7
	psrlq	mm2,2
	paddb	mm2,mm3

	paddb	mm6,mm5		;0x03-2=0x01
	psubb	mm7,mm5		;0xFD+2=0xFF
	saveadd2
	psubb	mm6,mm5		;restore mask
	paddb	mm7,mm5		;restore mask

%endrep

	pop		edi
	pop		esi 
	ret 12

