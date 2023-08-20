	mc68000
	opt o-

; =========================================================
; NEG.b <ea>
; =========================================================

; NEG.b, Dn
	NEG.b D0

; NEG.b, An (not allowed for dest)

; NEG.b, (An)
	NEG.b (A5)

; NEG.b, (An)+
	NEG.b (A6)+

; NEG.b, -(An)
	NEG.b -(A7)

; NEG.b, d16(An)
	NEG.b -32768(A2)
	NEG.b 0(A3)
	NEG.b 1234(A4)
	NEG.b 32767(A4)

; NEG.b, d8(An, Xn.L|W)
	NEG.b 123(A2,A3.w)
	NEG.b -1(A3,D2.l)
	NEG.b -128(A4,D3.w)

; NEG.b, (xxx).w
	NEG.b ($0000).w
	NEG.b ($1234).w
	NEG.b ($7FFF).w

; NEG.b, (xxx).l
	NEG.b ($00000000).l
	NEG.b ($12345678).l
	NEG.b ($FFFFFFFF).l

; Not supported
; NEG.b, d16(PC)
; NEG.b, d8(PC, Xn)
; NEG.b, Imm

; =========================================================
; NEG.w <ea>
; =========================================================

; NEG.w, Dn
	NEG.w D0

; NEG.w, An (not allowed for dest)

; NEG.w, (An)
	NEG.w (A5)

; NEG.w, (An)+
	NEG.w (A6)+

; NEG.w, -(An)
	NEG.w -(A7)

; NEG.w, d16(An)
	NEG.w -32768(A2)
	NEG.w 0(A3)
	NEG.w 1234(A4)
	NEG.w 32767(A4)

; NEG.w, d8(An, Xn.L|W)
	NEG.w 123(A2,A3.w)
	NEG.w -1(A3,D2.l)
	NEG.w -128(A4,D3.w)

; NEG.w, (xxx).w
	NEG.w ($0000).w
	NEG.w ($1234).w
	NEG.w ($7FFF).w

; NEG.w, (xxx).l
	NEG.w ($00000000).l
	NEG.w ($12345678).l
	NEG.w ($FFFFFFFF).l

; Not supported
; NEG.w, d16(PC)
; NEG.w, d8(PC, Xn)
; NEG.w, Imm

; =========================================================
; NEG.l <ea>
; =========================================================

; NEG.l, Dn
	NEG.l D0

; NEG.l, An (not allowed for dest)

; NEG.l, (An)
	NEG.l (A5)

; NEG.l, (An)+
	NEG.l (A6)+

; NEG.l, -(An)
	NEG.l -(A7)

; NEG.l, d16(An)
	NEG.l -32768(A2)
	NEG.l 0(A3)
	NEG.l 1234(A4)
	NEG.l 32767(A4)

; NEG.l, d8(An, Xn.L|W)
	NEG.l 123(A2,A3.w)
	NEG.l -1(A3,D2.l)
	NEG.l -128(A4,D3.w)

; NEG.l, (xxx).w
	NEG.l ($0000).w
	NEG.l ($1234).w
	NEG.l ($7FFF).w

; NEG.l, (xxx).l
	NEG.l ($00000000).l
	NEG.l ($12345678).l
	NEG.l ($FFFFFFFF).l

; Not supported
; NEG.l, d16(PC)
; NEG.l, d8(PC, Xn)
; NEG.l, Imm
