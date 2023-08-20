	mc68000
	opt o-

; =========================================================
; PEA.l <ea>
; =========================================================

; PEA.l, Dn (not allowed for dest)
; PEA.l, An (not allowed for dest)

; PEA.l, (An)
	PEA.l (A5)

; PEA.l, (An)+  (not allowed for dest)
; PEA.l, -(An)  (not allowed for dest)

; PEA.l, d16(An)
	PEA.l -32768(A2)
	PEA.l 0(A3)
	PEA.l 1234(A4)
	PEA.l 32767(A4)

; PEA.l, d8(An, Xn.L|W)
	PEA.l 123(A2,A3.w)
	PEA.l -1(A3,D2.l)
	PEA.l -128(A4,D3.w)

; PEA.l, (xxx).w
	PEA.l ($0000).w
	PEA.l ($1234).w
	PEA.l ($7FFF).w

; PEA.l, (xxx).l
	PEA.l ($00000000).l
	PEA.l ($12345678).l
	PEA.l ($FFFFFFFF).l

; PEA.l, d16(PC)
	PEA.l -32768(PC)
	PEA.l 0(PC)
	PEA.l 1234(PC)
	PEA.l 32767(PC)

; PEA.l, d8(PC, Xn.L|W)
	PEA.l 123(PC,A3.w)
	PEA.l -1(PC,D2.l)
	PEA.l -128(PC,D3.w)

; PEA.l, Imm  (not allowed for dest)
