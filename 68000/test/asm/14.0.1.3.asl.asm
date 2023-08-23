	mc68000
	opt o-

; =========================================================
; ASL.w <ea>
; =========================================================

; ASL.w, Dn (not allowed)
; ASL.w, An (not allowed)

; ASL.w, (An)
	ASL.w (A5)

; ASL.w, (An)+
	ASL.w (A6)+

; ASL.w, -(An)
	ASL.w -(A7)

; ASL.w, d16(An)
	ASL.w -32768(A2)
	ASL.w 0(A3)
	ASL.w 1234(A4)
	ASL.w 32767(A4)

; ASL.w, d8(An, Xn.L|W)
	ASL.w 123(A2,A3.w)
	ASL.w -1(A3,D2.l)
	ASL.w -128(A4,D3.w)

; ASL.w, (xxx).w
	ASL.w ($0000).w
	ASL.w ($1234).w
	ASL.w ($7FFF).w

; ASL.w, (xxx).l
	ASL.w ($00000000).l
	ASL.w ($12345678).l
	ASL.w ($FFFFFFFF).l

; Not supported
; ASL.w, d16(PC)
; ASL.w, d8(PC, Xn)
; ASL.w, Imm
