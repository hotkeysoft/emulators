	mc68000
	opt o-

; =========================================================
; ROR.w <ea>
; =========================================================

; ROR.w, Dn (not allowed)
; ROR.w, An (not allowed)

; ROR.w, (An)
	ROR.w (A5)

; ROR.w, (An)+
	ROR.w (A6)+

; ROR.w, -(An)
	ROR.w -(A7)

; ROR.w, d16(An)
	ROR.w -32768(A2)
	ROR.w 0(A3)
	ROR.w 1234(A4)
	ROR.w 32767(A4)

; ROR.w, d8(An, Xn.L|W)
	ROR.w 123(A2,A3.w)
	ROR.w -1(A3,D2.l)
	ROR.w -128(A4,D3.w)

; ROR.w, (xxx).w
	ROR.w ($0000).w
	ROR.w ($1234).w
	ROR.w ($7FFF).w

; ROR.w, (xxx).l
	ROR.w ($00000000).l
	ROR.w ($12345678).l
	ROR.w ($FFFFFFFF).l

; Not supported
; ROR.w, d16(PC)
; ROR.w, d8(PC, Xn)
; ROR.w, Imm
