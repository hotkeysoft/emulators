	mc68000
	opt o-

; =========================================================
; ROXL.w <ea>
; =========================================================

; ROXL.w, Dn (not allowed)
; ROXL.w, An (not allowed)

; ROXL.w, (An)
	ROXL.w (A5)

; ROXL.w, (An)+
	ROXL.w (A6)+

; ROXL.w, -(An)
	ROXL.w -(A7)

; ROXL.w, d16(An)
	ROXL.w -32768(A2)
	ROXL.w 0(A3)
	ROXL.w 1234(A4)
	ROXL.w 32767(A4)

; ROXL.w, d8(An, Xn.L|W)
	ROXL.w 123(A2,A3.w)
	ROXL.w -1(A3,D2.l)
	ROXL.w -128(A4,D3.w)

; ROXL.w, (xxx).w
	ROXL.w ($0000).w
	ROXL.w ($1234).w
	ROXL.w ($7FFF).w

; ROXL.w, (xxx).l
	ROXL.w ($00000000).l
	ROXL.w ($12345678).l
	ROXL.w ($FFFFFFFF).l

; Not supported
; ROXL.w, d16(PC)
; ROXL.w, d8(PC, Xn)
; ROXL.w, Imm
