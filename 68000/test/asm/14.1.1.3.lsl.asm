	mc68000
	opt o-

; =========================================================
; LSL.w <ea>
; =========================================================

; LSL.w, Dn (not allowed)
; LSL.w, An (not allowed)

; LSL.w, (An)
	LSL.w (A5)

; LSL.w, (An)+
	LSL.w (A6)+

; LSL.w, -(An)
	LSL.w -(A7)

; LSL.w, d16(An)
	LSL.w -32768(A2)
	LSL.w 0(A3)
	LSL.w 1234(A4)
	LSL.w 32767(A4)

; LSL.w, d8(An, Xn.L|W)
	LSL.w 123(A2,A3.w)
	LSL.w -1(A3,D2.l)
	LSL.w -128(A4,D3.w)

; LSL.w, (xxx).w
	LSL.w ($0000).w
	LSL.w ($1234).w
	LSL.w ($7FFF).w

; LSL.w, (xxx).l
	LSL.w ($00000000).l
	LSL.w ($12345678).l
	LSL.w ($FFFFFFFF).l

; Not supported
; LSL.w, d16(PC)
; LSL.w, d8(PC, Xn)
; LSL.w, Imm
