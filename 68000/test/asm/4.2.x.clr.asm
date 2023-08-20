	mc68000
	opt o-

; =========================================================
; CLR.b <ea>
; =========================================================

; CLR.b, Dn
	CLR.b D0

; CLR.b, An (not allowed for dest)

; CLR.b, (An)
	CLR.b (A5)

; CLR.b, (An)+
	CLR.b (A6)+

; CLR.b, -(An)
	CLR.b -(A7)

; CLR.b, d16(An)
	CLR.b -32768(A2)
	CLR.b 0(A3)
	CLR.b 1234(A4)
	CLR.b 32767(A4)

; CLR.b, d8(An, Xn.L|W)
	CLR.b 123(A2,A3.w)
	CLR.b -1(A3,D2.l)
	CLR.b -128(A4,D3.w)

; CLR.b, (xxx).w
	CLR.b ($0000).w
	CLR.b ($1234).w
	CLR.b ($7FFF).w

; CLR.b, (xxx).l
	CLR.b ($00000000).l
	CLR.b ($12345678).l
	CLR.b ($FFFFFFFF).l

; Not supported
; CLR.b, d16(PC)
; CLR.b, d8(PC, Xn)
; CLR.b, Imm

; =========================================================
; CLR.w <ea>
; =========================================================

; CLR.w, Dn
	CLR.w D0

; CLR.w, An (not allowed for dest)

; CLR.w, (An)
	CLR.w (A5)

; CLR.w, (An)+
	CLR.w (A6)+

; CLR.w, -(An)
	CLR.w -(A7)

; CLR.w, d16(An)
	CLR.w -32768(A2)
	CLR.w 0(A3)
	CLR.w 1234(A4)
	CLR.w 32767(A4)

; CLR.w, d8(An, Xn.L|W)
	CLR.w 123(A2,A3.w)
	CLR.w -1(A3,D2.l)
	CLR.w -128(A4,D3.w)

; CLR.w, (xxx).w
	CLR.w ($0000).w
	CLR.w ($1234).w
	CLR.w ($7FFF).w

; CLR.w, (xxx).l
	CLR.w ($00000000).l
	CLR.w ($12345678).l
	CLR.w ($FFFFFFFF).l

; Not supported
; CLR.w, d16(PC)
; CLR.w, d8(PC, Xn)
; CLR.w, Imm

; =========================================================
; CLR.l <ea>
; =========================================================

; CLR.l, Dn
	CLR.l D0

; CLR.l, An (not allowed for dest)

; CLR.l, (An)
	CLR.l (A5)

; CLR.l, (An)+
	CLR.l (A6)+

; CLR.l, -(An)
	CLR.l -(A7)

; CLR.l, d16(An)
	CLR.l -32768(A2)
	CLR.l 0(A3)
	CLR.l 1234(A4)
	CLR.l 32767(A4)

; CLR.l, d8(An, Xn.L|W)
	CLR.l 123(A2,A3.w)
	CLR.l -1(A3,D2.l)
	CLR.l -128(A4,D3.w)

; CLR.l, (xxx).w
	CLR.l ($0000).w
	CLR.l ($1234).w
	CLR.l ($7FFF).w

; CLR.l, (xxx).l
	CLR.l ($00000000).l
	CLR.l ($12345678).l
	CLR.l ($FFFFFFFF).l

; Not supported
; CLR.l, d16(PC)
; CLR.l, d8(PC, Xn)
; CLR.l, Imm
