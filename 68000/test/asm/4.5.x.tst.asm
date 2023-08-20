	mc68000
	opt o-

; =========================================================
; TST.b <ea>
; =========================================================

; TST.b, Dn
	TST.b D0

; TST.b, An (not allowed for dest)

; TST.b, (An)
	TST.b (A5)

; TST.b, (An)+
	TST.b (A6)+

; TST.b, -(An)
	TST.b -(A7)

; TST.b, d16(An)
	TST.b -32768(A2)
	TST.b 0(A3)
	TST.b 1234(A4)
	TST.b 32767(A4)

; TST.b, d8(An, Xn.L|W)
	TST.b 123(A2,A3.w)
	TST.b -1(A3,D2.l)
	TST.b -128(A4,D3.w)

; TST.b, (xxx).w
	TST.b ($0000).w
	TST.b ($1234).w
	TST.b ($7FFF).w

; TST.b, (xxx).l
	TST.b ($00000000).l
	TST.b ($12345678).l
	TST.b ($FFFFFFFF).l

; Not supported
; TST.b, d16(PC)
; TST.b, d8(PC, Xn)
; TST.b, Imm

; =========================================================
; TST.w <ea>
; =========================================================

; TST.w, Dn
	TST.w D0

; TST.w, An (not allowed for dest)

; TST.w, (An)
	TST.w (A5)

; TST.w, (An)+
	TST.w (A6)+

; TST.w, -(An)
	TST.w -(A7)

; TST.w, d16(An)
	TST.w -32768(A2)
	TST.w 0(A3)
	TST.w 1234(A4)
	TST.w 32767(A4)

; TST.w, d8(An, Xn.L|W)
	TST.w 123(A2,A3.w)
	TST.w -1(A3,D2.l)
	TST.w -128(A4,D3.w)

; TST.w, (xxx).w
	TST.w ($0000).w
	TST.w ($1234).w
	TST.w ($7FFF).w

; TST.w, (xxx).l
	TST.w ($00000000).l
	TST.w ($12345678).l
	TST.w ($FFFFFFFF).l

; Not supported
; TST.w, d16(PC)
; TST.w, d8(PC, Xn)
; TST.w, Imm

; =========================================================
; TST.l <ea>
; =========================================================

; TST.l, Dn
	TST.l D0

; TST.l, An (not allowed for dest)

; TST.l, (An)
	TST.l (A5)

; TST.l, (An)+
	TST.l (A6)+

; TST.l, -(An)
	TST.l -(A7)

; TST.l, d16(An)
	TST.l -32768(A2)
	TST.l 0(A3)
	TST.l 1234(A4)
	TST.l 32767(A4)

; TST.l, d8(An, Xn.L|W)
	TST.l 123(A2,A3.w)
	TST.l -1(A3,D2.l)
	TST.l -128(A4,D3.w)

; TST.l, (xxx).w
	TST.l ($0000).w
	TST.l ($1234).w
	TST.l ($7FFF).w

; TST.l, (xxx).l
	TST.l ($00000000).l
	TST.l ($12345678).l
	TST.l ($FFFFFFFF).l

; Not supported
; TST.l, d16(PC)
; TST.l, d8(PC, Xn)
; TST.l, Imm
