/*****************************************************************************
 *
 * This code has been developed by Project Mayo. This software is an
 * implementation of a part of one or more MPEG-4 Video tools as
 * specified in ISO/IEC 14496-2 standard.  Those intending to use this
 * software module in hardware or software products are advised that its
 * use may infringe existing patents or copyrights, and any such use
 * would be at such party's own risk.  The original developer of this
 * software module and his/her company, and subsequent editors and their
 * companies (including Project Mayo), will have no liability for use of
 * this software or modifications or derivatives thereof.
 *
 *****************************************************************************
 *																				*	
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: idct_mips.c 5 2004-07-15 13:20:46Z picard $
 *
 *****************************************************************************
 *
 * Authors:
 *
 *	Pedro	Mateu     (Pm)
 *
 ****************************************************************************/

#include "../common.h"
#include "softidct.h"

#if defined(MIPS64) || defined(MIPS32)

#if defined(MIPSVR41XX)
#define MACC(out,ina,inb)				\
		"macc	$0," #ina "," #inb ";"	\
		"mflo	" #out ";"
//		"macc	" #out "," #ina "," #inb ";"
#else
#define MACC(out,ina,inb)				\
		".set noat;"					\
		"mflo	$1;"					\
		"mult	" #ina "," #inb ";"		\
		"mflo	" #out ";"				\
		"addu	" #out "," #out ",$1;"	
#endif

void IDCT_Row8(idct_block_t *Blk, uint8_t *Dest, const uint8_t *Src)
{
  __asm("move	$24,$5;"
		"move	$25,$6;"
		"lh		$5,0($4);"		//x0 
		"lh		$9,2($4);"		//x4 
		"lh		$8,4($4);"		//x3 
		"lh		$12,6($4);"		//x7 
		"lh		$6,8($4);"		//x1 
		"lh		$11,10($4);"	//x6 
		"lh		$7,12($4);"		//x2 
		"lh		$10,14($4);"	//x5 

		"sll	$6,$6,8;"		// x1<< 

		"or		$2,$7,$6;" 
		"or		$2,$2,$8;" 
		"or		$2,$2,$9;" 
		"or		$2,$2,$10;" 
		"or		$2,$2,$11;" 
		"or		$2,$2,$12;"		//shortcut 
		 
		"bne $2,$0,idct_row_a;"	//if some values aren´t 0, we can´t take the shortcut 
		
			"addi	$5,$5,32;"
			"sra	$11,$5,6;"
			"bne	$25,$0,add_eq;"
			
			"srl	$15,$11,16;"	//if less than 0, then 0
			"srlv	$11,$11,$15;"

			"sll	$15,$11,23;"	//if bigger than 255:255
			"sra	$15,$15,31;"
			"or		$11,$11,$15;"
			
			"sll	$14,$11,8;"
			"or		$11,$11,$14;"
			"sll	$14,$11,16;"
			"or		$11,$11,$14;"
			
			"sw		$11,0($24);"	//guardamos x0 
			"sw		$11,4($24);" 

#ifdef MIPSVR41XX		
			"andi	$7,$24,0x8;"
			"beq	$7,$0, fin;"
			".set noreorder;"
			"cache	25,-8($24);"	// hit writeback
			".set reorder;"
#endif
			"jr		$31;");
						
__asm(		"add_eq:"
	  
			"lbu	$15,0($25);"	//we load older values
			"lbu	$6,1($25);"		
			"lbu	$7,2($25);"			
			"lbu	$9,3($25);"		
			"lbu	$13,4($25);"	
			"lbu	$5,5($25);"
			"lbu	$8,6($25);"
			"lbu	$12,7($25);"

			"add	$15,$11,$15;"
			"add	$6,$11,$6;"
			"add	$7,$11,$7;"		
			"add	$9,$11,$9;"		
			"add	$13,$11,$13;"		
			"add	$5,$11,$5;"		
			"add	$8,$11,$8;"		
			"add	$12,$11,$12;"		
		
			// Check if there is an overflow and store

			"b		check_overflow;"); 
	
__asm  ("idct_row_a:add	$13,$9,$10;" //x8=x4+x5
		"li		$14,565;"		//W7
		"mult	$14,$13;"		//W7*x8
		"li		$15,2276;"		//W1_minus_W7 
		MACC($9,$15,$9)			//x4=x(W1-W7)*x4+W7*8

		"sll	$5,$5,8;"		// x0<<8)
		
		"mult	$14,$13;"		//W7*x8
		"li		$15,-3406;"		//-W1_plus_W7
		MACC($10,$15,$10)		//-((W1+W7)*x5)+W7*x8

		"li		$14,2408;"		//W3
		"add	$13,$11,$12;"	// x6+x7
		"mult	$14,$13;"		//W3*(x6+x7)

		"li		$15,-799;"		//-W3_minus_W5
		MACC($11,$15,$11)		//-((W3-W5)*x6)+W3*(x6+x7)

		"add	$5,$5,8192;"	//x0+=128

		"mult	$14,$13;"
		"li		$14,-4017;"
		MACC($12,$12,$14));	

__asm (	"add	$13,$5,$6;" 
		"sub	$5,$5,$6;"		//x0 -=x1 
		
		"add	$6,$7,$8;"		//x1=(x2+x3)
		"li		$15,1108;"		//W6 
		"mult	$15,$6;"		//W6*(x3+x2) 
		
		"li		$14,-3784;"		//-W2_plus_W6 
		MACC($7,$14,$7)			//x1+(-W2_plus_W6*x2) 
		"mult	$15,$6;"		//W6*(x3+x2)
		"li		$14,1568;"		//W2_minus_W6 
		MACC($8,$14,$8)			//W2_minus_W6*x3 

		"addi	$9,$9,4;"
		"addi	$11,$11,4;"
		"addi	$10,$10,4;"
		"addi	$12,$12,4;"

		"sra	$9,$9,3;"
		"sra	$10,$10,3;"
		"sra	$11,$11,3;"
		"sra	$12,$12,3;"
		
		"add	$6,$9,$11;"		//x1=x4+x6 
		"sub	$9,$9,$11;"		//x4=x4-x6 
		"add	$11,$10,$12;"	//x6=x5+x7 
		"sub	$10,$10,$12;"	//x5=x5-x7 
		
); 

__asm (	"addi	$7,$7,4;"
		"addi	$8,$8,4;"
		"sra	$7,$7,3;"
		"sra	$8,$8,3;"
		"add	$12,$13,$8;"	//x7=x8+x3 
		"sub	$13,$13,$8;"	//x8-=x3 
		"add	$8,$5,$7;"		//x3=x0+x2 
		"sub	$5,$5,$7;"		//x0-=x2 

		"li		$14,181;" 
		"add	$7,$9,$10;"		//x4+x5 
		"mult	$14,$7;"		// 
		"mflo	$7;" 
		"addi	$7,$7,128;" 
		"sra	$7,$7,8;" 

		"sub	$9,$9,$10;" 
		"mult	$14,$9;" 
		"mflo	$9;" 
		"addi	$9,$9,128;" 
		"sra	$9,$9,8;"); 

//Fourth Stage 


__asm (	//x7+x1 

		"add	$15,$12,$6;"	
		"sra	$15,$15,14;" 
		
		//x7-x1

		"sub	$12,$12,$6;"		
		"sra	$12,$12,14;" 
		
		//x3+x2 
		
		"add	$6,$8,$7;"			
		"sra	$6,$6,14;" 
		
		//x3-x2 
		
		"sub	$8,$8,$7;" 
		"sra	$8,$8,14;"
		
		//x0+x4 

		"add	$7,$5,$9;"		
		"sra	$7,$7,14;"
		
		//x0-x4 
		
		"sub	$5,$5,$9;" 
		"sra	$5,$5,14;"
				
		//x8+x6
		
		"add	$9,$13,$11;"	 
		"sra	$9,$9,14;"
		
		//x8-x6

		"sub	$13,$13,$11;"
		"sra	$13,$13,14;"
		
		// If we don´t have to add older values, check if there is an overflow and store

		"beq	$25,$0,check_overflow;"); 


__asm (	"lbu	$2,0($25);"
		"lbu	$3,1($25);"
		
		"add	$15,$15,$2;"
		"add	$6,$6,$3;"

		"lbu	$2,2($25);"
		"lbu	$3,3($25);"
		
		"add	$7,$7,$2;"
		"add	$9,$9,$3;"
		
		"lbu	$2,4($25);"
		"lbu	$3,5($25);"
		
		"add	$13,$13,$2;"
		"add	$5,$5,$3;"
		
		"lbu	$2,6($25);"
		"lbu	$3,7($25);"
		
		"add	$8,$8,$2;"
		"add	$12,$12,$3;");

		
__asm(	// Check for overflow

		"check_overflow:"

		"or		$14,$15,$12;"
		"or		$14,$14,$6;"
		"or		$14,$14,$8;"
		"or		$14,$14,$7;"
		"or		$14,$14,$5;"
		"or		$14,$14,$9;"
		"or		$14,$14,$13;"
		"srl	$14,$14,8;"

		// If there is an overflow, we must saturate all the values

		"beq	$14,$0,no_saturar;"
		); 

__asm (	"srl	$14,$15,16;"	//	if less than 0 -> 0
		"srlv	$15,$15,$14;"	//
		"sll	$14,$15,23;"	//	if bigger than 255 -> 255
		"sra	$14,$14,31;"	//
		"or		$15,$15,$14;"	//

		"srl	$14,$12,16;"	//	if less than 0 -> 0
		"srlv	$12,$12,$14;"	//
		"sll	$14,$12,23;"	//	if bigger than 255 -> 255
		"sra	$14,$14,31;"	//
		"or		$12,$12,$14;"	//

		"srl	$14,$6,16;"		//	if less than 0 -> 0
		"srlv	$6,$6,$14;"		//
		"sll	$14,$6,23;"		//	if bigger than 255 -> 255
		"sra	$14,$14,31;"	//
		"or		$6,$6,$14;"		//

		"srl	$14,$8,16;"		//	if less than 0 -> 0
		"srlv	$8,$8,$14;"		//
		"sll	$14,$8,23;"		//	if bigger than 255 -> 255
		"sra	$14,$14,31;"	//
		"or		$8,$8,$14;"		//

		"srl	$14,$7,16;"		//	if less than 0 -> 0
		"srlv	$7,$7,$14;"		//
		"sll	$14,$7,23;"		//	if bigger than 255 -> 255
		"sra	$14,$14,31;"	//
		"or		$7,$7,$14;"		//

		"srl	$14,$5,16;"		//	if less than 0 -> 0
		"srlv	$5,$5,$14;"		//
		"sll	$14,$5,23;"		//	if bigger than 255 -> 255
		"sra	$14,$14,31;"	//
		"or		$5,$5,$14;"		//

		"srl	$14,$9,16;"		//	if less than 0 -> 0
		"srlv	$9,$9,$14;"		//
		"sll	$14,$9,23;"		//	if bigger than 255 -> 255
		"sra	$14,$14,31;"	//
		"or		$9,$9,$14;"		//

		"srl	$14,$13,16;"	//	if less than 0 -> 0
		"srlv	$13,$13,$14;"	//
		"sll	$14,$13,23;"	//	if bigger than 255 -> 255
		"sra	$14,$14,31;"	//
		"or		$13,$13,$14;"	//

"no_saturar:"
	 
		"sb		$15,0($24);"
		"sb		$6,1($24);"
		"sb		$7,2($24);"
		"sb		$9,3($24);"
		"sb		$13,4($24);"
		"sb		$5,5($24);"
		"sb		$8,6($24);"
		"sb		$12,7($24);"

#ifdef MIPSVR41XX		
		"andi	$7,$24,0x8;"
		"beq	$7,$0, fin;"
		".set noreorder;"
		"cache	25,-8($24);"	// hit writeback
		".set reorder;"
		"fin:" 
#endif
		);

}

void IDCT_Col8(idct_block_t *Blk)
{ 

__asm (	"lh		$5,0($4);"//x0 
		"lh		$9,16($4);"//x4 
		"lh		$8,32($4);"//x3 
		"lh		$12,48($4);"//x7 
		"lh		$6,64($4);"//x1 
		"lh		$11,80($4);"//x6 
		"lh		$7,96($4);"//x2 
		"lh		$10,112($4);"//x5 

		"or		$2,$7,$6;" 
		"or		$2,$2,$8;" 
		"or		$2,$2,$9;" 
		"or		$2,$2,$10;" 
		"or		$2,$2,$11;" 
		"or		$2,$2,$12;"//shortcut 

		"sll	$6,$6,11;"// x1<<11 
		
		"bne $2,$0,idct_estandar;"//si no es 0 no podemos atajar 
			"sll	$5,$5,3;"//x0<<3 
			"beq	$5,$0,final;"
			
			"sh		$5,0($4);"//guardamos x0 
			"sh		$5,16($4);" 
			"sh		$5,32($4);"
			"sh		$5,48($4);"
			"sh		$5,64($4);" 
			"sh		$5,80($4);"
			"sh		$5,96($4);"
			"sh		$5,112($4);" 
			"final:jr		$31;"); 
	
__asm  ("idct_estandar:add	$13,$9,$10;"//x8=x4+x5
		"li		$14,565;"//W7
		"mult	$14,$13;"//W7*x8
		"li		$15,2276;"//W1_minus_W7 
		MACC($9,$15,$9)   //x4=x(W1-W7)*x4+W7*8

		"sll	$5,$5,11;"// x0<<11
		
		"mult	$14,$13;"//W7*x8
		"li		$15,-3406;"	//-W1_plus_W7
		MACC($10,$15,$10)	//-((W1+W7)*x5)+W7*x8
		
		"li		$14,2408;"//W3
		"add	$13,$11,$12;"// x6+x7
		"mult	$14,$13;"//W3*(x6+x7)

		"li		$15,-799;"//-W3_minus_W5
		MACC($11,$15,$11) //-((W3-W5)*x6)+W3*(x6+x7)

		"add	$5,$5,128;"//x0+=128

		"mult	$14,$13;"
		"li		$14,-4017;"
		MACC($12,$12,$14)
		);		

__asm (	"add	$13,$5,$6;" 
		"sub	$5,$5,$6;"//x0 -=x1 
		
		"add	$6,$7,$8;"//x1=(x2+x3)
		"li		$15,1108;"//W6 
		"mult	$15,$6;"//W6*(x3+x2) 
		
		"li		$14,-3784;" //-W2_plus_W6 
		MACC($7,$14,$7) //x1+(-W2_plus_W6*x2) 
	
		"mult	$15,$6;"//W6*(x3+x2)
		"li		$14,1568;"//W2_minus_W6 
		MACC($8,$14,$8) //W2_minus_W6*x3 

		"add	$6,$9,$11;"//x1=x4+x6 
		"sub	$9,$9,$11;"//x4=x4-x6 
		"add	$11,$10,$12;"//x6=x5+x7 
		"sub	$10,$10,$12;"//x5=x5-x7 
		
); 

__asm (	"add	$12,$13,$8;"//x7=x8+x3 
		"sub	$13,$13,$8;"//x8-=x3 
		"add	$8,$5,$7;"//x3=x0+x2 
		"sub	$5,$5,$7;"//x0-=x2 

		"li		$14,181;" 
		"add	$7,$9,$10;"//x4+x5 
		"mult	$14,$7;"// 
		"mflo	$7;" 
		"addi	$7,$7,128;" 
		"sra	$7,$7,8;" 

		"sub	$9,$9,$10;" 
		"mult	$14,$9;" 
		"mflo	$9;" 
		"addi	$9,$9,128;" 
		"sra	$9,$9,8;"); 

//Fourth Stage 

__asm (	"add	$24,$12,$6;"//x7+x1 
		"sra	$24,$24,8;" 
		"sh		$24,0($4);" 

		"add	$24,$7,$8;"//x3+x2 
		"sra	$24,$24,8;" 
		"sh		$24,16($4);" 

		"add	$24,$5,$9;"//x0+x4 
		"sra	$24,$24,8;" 
		"sh		$24,32($4);" 

		"add	$24,$13,$11;"//x8+x6 
		"sra	$24,$24,8;" 
		"sh		$24,48($4);" 

		"sub	$24,$13,$11;" 
		"sra	$24,$24,8;" 
		"sh		$24,64($4);" 

		"sub	$24,$5,$9;" 
		"sra	$24,$24,8;" 
		"sh		$24,80($4);" 

		"sub	$24,$8,$7;" 
		"sra	$24,$24,8;" 
		"sh		$24,96($4);" 

		"sub	$24,$12,$6;" 
		"sra	$24,$24,8;" 
		"sh		$24,112($4);"); 
} 

#endif
