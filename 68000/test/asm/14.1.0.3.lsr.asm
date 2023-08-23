	mc68000
	opt o-

; =========================================================
; LSR.w <ea>
; =========================================================

; LSR.w, Dn (not allowed)
; LSR.w, An (not allowed)

; LSR.w, (An)
	LSR.w (A5)

; LSR.w, (An)+
	LSR.w (A6)+

; LSR.w, -(An)
	LSR.w -(A7)

; LSR.w, d16(An)
	LSR.w -32768(A2)
	LSR.w 0(A3)
	LSR.w 1234(A4)
	LSR.w 32767(A4)

; LSR.w, d8(An, Xn.L|W)
	LSR.w 123(A2,A3.w)
	LSR.w -1(A3,D2.l)
	LSR.w -128(A4,D3.w)

; LSR.w, (xxx).w
	LSR.w ($0000).w
	LSR.w ($1234).w
	LSR.w ($7FFF).w

; LSR.w, (xxx).l
	LSR.w ($00000000).l
	LSR.w ($12345678).l
	LSR.w ($FFFFFFFF).l

; Not supported
; LSR.w, d16(PC)
; LSR.w, d8(PC, Xn)
; LSR.w, Imm
