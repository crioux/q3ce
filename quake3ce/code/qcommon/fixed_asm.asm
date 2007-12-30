;******************************************************************************	
;* Fixed-point routines for Q3CE
;******************************************************************************	

	AREA	|.text|, CODE

	EXPORT VEC3DOT32_11
	EXPORT VEC3DOT32_20
	
;******************************************************************************	
; void VEC3DOT32_11(INT32 *vec_a, INT32 *vec_b, INT32 * rep_out);

VEC3DOT32_11 PROC

	stmfd   sp!, {r4-r8}
	ldmia   r0, {r3-r5}
	ldmia   r1, {r6-r8}
	
	smull   r12, r6, r3, r6
	smlal   r12, r6, r4, r7
	smlal   r12, r6, r5, r8
	mov     r6, r6, lsl #21
	orr     r12, r6, r12, lsr #11
	str     r12, [r2]
	
	ldmfd   sp!, {r4-r8}
	mov		pc, lr
	
	ENDP


;******************************************************************************	
; void VEC3DOT32_20(INT32 *vec_a, INT32 *vec_b, INT32 * rep_out);

VEC3DOT32_20 PROC

	stmfd   sp!, {r4-r8}
	ldmia   r0, {r3-r5}
	ldmia   r1, {r6-r8}
	
	smull   r12, r6, r3, r6
	smlal   r12, r6, r4, r7
	smlal   r12, r6, r5, r8
	mov     r6, r6, lsl #12
	orr     r12, r6, r12, lsr #20
	str     r12, [r2]
	
	ldmfd   sp!, {r4-r8}
	mov		pc, lr
	
	ENDP


;******************************************************************************	

	END
