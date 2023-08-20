	mc68000
	opt o-

; =========================================================
; NBCD.b <ea>
; =========================================================

; NBCD.b, Dn
	NBCD.b D0

; NBCD.b, An (not allowed for dest)

; NBCD.b, (An)
	NBCD.b (A5)

; NBCD.b, (An)+
	NBCD.b (A6)+

; NBCD.b, -(An)
	NBCD.b -(A7)

; NBCD.b, d16(An)
	NBCD.b -32768(A2)
	NBCD.b 0(A3)
	NBCD.b 1234(A4)
	NBCD.b 32767(A4)

; NBCD.b, d8(An, Xn.L|W)
	NBCD.b 123(A2,A3.w)
	NBCD.b -1(A3,D2.l)
	NBCD.b -128(A4,D3.w)

; NBCD.b, (xxx).w
	NBCD.b ($0000).w
	NBCD.b ($1234).w
	NBCD.b ($7FFF).w

; NBCD.b, (xxx).l
	NBCD.b ($00000000).l
	NBCD.b ($12345678).l
	NBCD.b ($FFFFFFFF).l

; Not supported
; NBCD.b, d16(PC)
; NBCD.b, d8(PC, Xn)
; NBCD.b, Imm
