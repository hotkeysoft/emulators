	mc68000
	opt o-

; =========================================================
; ROXR.w <ea>
; =========================================================

; ROXR.w, Dn (not allowed)
; ROXR.w, An (not allowed)

; ROXR.w, (An)
	ROXR.w (A5)

; ROXR.w, (An)+
	ROXR.w (A6)+

; ROXR.w, -(An)
	ROXR.w -(A7)

; ROXR.w, d16(An)
	ROXR.w -32768(A2)
	ROXR.w 0(A3)
	ROXR.w 1234(A4)
	ROXR.w 32767(A4)

; ROXR.w, d8(An, Xn.L|W)
	ROXR.w 123(A2,A3.w)
	ROXR.w -1(A3,D2.l)
	ROXR.w -128(A4,D3.w)

; ROXR.w, (xxx).w
	ROXR.w ($0000).w
	ROXR.w ($1234).w
	ROXR.w ($7FFF).w

; ROXR.w, (xxx).l
	ROXR.w ($00000000).l
	ROXR.w ($12345678).l
	ROXR.w ($FFFFFFFF).l

; Not supported
; ROXR.w, d16(PC)
; ROXR.w, d8(PC, Xn)
; ROXR.w, Imm
