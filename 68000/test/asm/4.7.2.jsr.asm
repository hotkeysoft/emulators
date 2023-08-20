	mc68000
	opt o-

; =========================================================
; JSR <ea>
; =========================================================

; JSR, Dn (not allowed for dest)
; JSR, An (not allowed for dest)

; JSR, (An)
	JSR (A5)

; JSR, (An)+ (not allowed for dest)
; JSR, -(An) (not allowed for dest)

; JSR, d16(An)
	JSR -32768(A2)
	JSR 0(A3)
	JSR 1234(A4)
	JSR 32767(A4)

; JSR, d8(An, Xn.L|W)
	JSR 123(A2,A3.w)
	JSR -1(A3,D2.l)
	JSR -128(A4,D3.w)

; JSR, (xxx).w
	JSR ($0000).w
	JSR ($1234).w
	JSR ($7FFF).w

; JSR, (xxx).l
	JSR ($00000000).l
	JSR ($12345678).l
	JSR ($FFFFFFFF).l

; JSR, d16(PC)
	JSR -32768(PC)
	JSR 0(PC)
	JSR 1234(PC)
	JSR 32767(PC)

; JSR, d8(PC, Xn)
	JSR 123(PC,A3.w)
	JSR -1(PC,D2.l)
	JSR -128(PC,D3.w)

; JSR, Imm  (not allowed for dest)
