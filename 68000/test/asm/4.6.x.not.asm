	mc68000
	opt o-

; =========================================================
; NOT.b <ea>
; =========================================================

; NOT.b, Dn
	NOT.b D0

; NOT.b, An (not allowed for dest)

; NOT.b, (An)
	NOT.b (A5)

; NOT.b, (An)+
	NOT.b (A6)+

; NOT.b, -(An)
	NOT.b -(A7)

; NOT.b, d16(An)
	NOT.b -32768(A2)
	NOT.b 0(A3)
	NOT.b 1234(A4)
	NOT.b 32767(A4)

; NOT.b, d8(An, Xn.L|W)
	NOT.b 123(A2,A3.w)
	NOT.b -1(A3,D2.l)
	NOT.b -128(A4,D3.w)

; NOT.b, (xxx).w
	NOT.b ($0000).w
	NOT.b ($1234).w
	NOT.b ($7FFF).w

; NOT.b, (xxx).l
	NOT.b ($00000000).l
	NOT.b ($12345678).l
	NOT.b ($FFFFFFFF).l

; Not supported
; NOT.b, d16(PC)
; NOT.b, d8(PC, Xn)
; NOT.b, Imm

; =========================================================
; NOT.w <ea>
; =========================================================

; NOT.w, Dn
	NOT.w D0

; NOT.w, An (not allowed for dest)

; NOT.w, (An)
	NOT.w (A5)

; NOT.w, (An)+
	NOT.w (A6)+

; NOT.w, -(An)
	NOT.w -(A7)

; NOT.w, d16(An)
	NOT.w -32768(A2)
	NOT.w 0(A3)
	NOT.w 1234(A4)
	NOT.w 32767(A4)

; NOT.w, d8(An, Xn.L|W)
	NOT.w 123(A2,A3.w)
	NOT.w -1(A3,D2.l)
	NOT.w -128(A4,D3.w)

; NOT.w, (xxx).w
	NOT.w ($0000).w
	NOT.w ($1234).w
	NOT.w ($7FFF).w

; NOT.w, (xxx).l
	NOT.w ($00000000).l
	NOT.w ($12345678).l
	NOT.w ($FFFFFFFF).l

; Not supported
; NOT.w, d16(PC)
; NOT.w, d8(PC, Xn)
; NOT.w, Imm

; =========================================================
; NOT.l <ea>
; =========================================================

; NOT.l, Dn
	NOT.l D0

; NOT.l, An (not allowed for dest)

; NOT.l, (An)
	NOT.l (A5)

; NOT.l, (An)+
	NOT.l (A6)+

; NOT.l, -(An)
	NOT.l -(A7)

; NOT.l, d16(An)
	NOT.l -32768(A2)
	NOT.l 0(A3)
	NOT.l 1234(A4)
	NOT.l 32767(A4)

; NOT.l, d8(An, Xn.L|W)
	NOT.l 123(A2,A3.w)
	NOT.l -1(A3,D2.l)
	NOT.l -128(A4,D3.w)

; NOT.l, (xxx).w
	NOT.l ($0000).w
	NOT.l ($1234).w
	NOT.l ($7FFF).w

; NOT.l, (xxx).l
	NOT.l ($00000000).l
	NOT.l ($12345678).l
	NOT.l ($FFFFFFFF).l

; Not supported
; NOT.l, d16(PC)
; NOT.l, d8(PC, Xn)
; NOT.l, Imm
