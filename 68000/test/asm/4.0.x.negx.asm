	mc68000
	opt o-

; =========================================================
; NEGX.b <ea>
; =========================================================

; NEGX.b, Dn
	NEGX.b D0

; NEGX.b, An (not allowed for dest)

; NEGX.b, (An)
	NEGX.b (A5)

; NEGX.b, (An)+
	NEGX.b (A6)+

; NEGX.b, -(An)
	NEGX.b -(A7)

; NEGX.b, d16(An)
	NEGX.b -32768(A2)
	NEGX.b 0(A3)
	NEGX.b 1234(A4)
	NEGX.b 32767(A4)

; NEGX.b, d8(An, Xn.L|W)
	NEGX.b 123(A2,A3.w)
	NEGX.b -1(A3,D2.l)
	NEGX.b -128(A4,D3.w)

; NEGX.b, (xxx).w
	NEGX.b ($0000).w
	NEGX.b ($1234).w
	NEGX.b ($7FFF).w

; NEGX.b, (xxx).l
	NEGX.b ($00000000).l
	NEGX.b ($12345678).l
	NEGX.b ($FFFFFFFF).l

; Not supported
; NEGX.b, d16(PC)
; NEGX.b, d8(PC, Xn)
; NEGX.b, Imm

; =========================================================
; NEGX.w <ea>
; =========================================================

; NEGX.w, Dn
	NEGX.w D0

; NEGX.w, An (not allowed for dest)

; NEGX.w, (An)
	NEGX.w (A5)

; NEGX.w, (An)+
	NEGX.w (A6)+

; NEGX.w, -(An)
	NEGX.w -(A7)

; NEGX.w, d16(An)
	NEGX.w -32768(A2)
	NEGX.w 0(A3)
	NEGX.w 1234(A4)
	NEGX.w 32767(A4)

; NEGX.w, d8(An, Xn.L|W)
	NEGX.w 123(A2,A3.w)
	NEGX.w -1(A3,D2.l)
	NEGX.w -128(A4,D3.w)

; NEGX.w, (xxx).w
	NEGX.w ($0000).w
	NEGX.w ($1234).w
	NEGX.w ($7FFF).w

; NEGX.w, (xxx).l
	NEGX.w ($00000000).l
	NEGX.w ($12345678).l
	NEGX.w ($FFFFFFFF).l

; Not supported
; NEGX.w, d16(PC)
; NEGX.w, d8(PC, Xn)
; NEGX.w, Imm

; =========================================================
; NEGX.l <ea>
; =========================================================

; NEGX.l, Dn
	NEGX.l D0

; NEGX.l, An (not allowed for dest)

; NEGX.l, (An)
	NEGX.l (A5)

; NEGX.l, (An)+
	NEGX.l (A6)+

; NEGX.l, -(An)
	NEGX.l -(A7)

; NEGX.l, d16(An)
	NEGX.l -32768(A2)
	NEGX.l 0(A3)
	NEGX.l 1234(A4)
	NEGX.l 32767(A4)

; NEGX.l, d8(An, Xn.L|W)
	NEGX.l 123(A2,A3.w)
	NEGX.l -1(A3,D2.l)
	NEGX.l -128(A4,D3.w)

; NEGX.l, (xxx).w
	NEGX.l ($0000).w
	NEGX.l ($1234).w
	NEGX.l ($7FFF).w

; NEGX.l, (xxx).l
	NEGX.l ($00000000).l
	NEGX.l ($12345678).l
	NEGX.l ($FFFFFFFF).l

; Not supported
; NEGX.l, d16(PC)
; NEGX.l, d8(PC, Xn)
; NEGX.l, Imm
