	mc68000
	opt o-

; =========================================================
; ROL.w <ea>
; =========================================================

; ROL.w, Dn (not allowed)
; ROL.w, An (not allowed)

; ROL.w, (An)
	ROL.w (A5)

; ROL.w, (An)+
	ROL.w (A6)+

; ROL.w, -(An)
	ROL.w -(A7)

; ROL.w, d16(An)
	ROL.w -32768(A2)
	ROL.w 0(A3)
	ROL.w 1234(A4)
	ROL.w 32767(A4)

; ROL.w, d8(An, Xn.L|W)
	ROL.w 123(A2,A3.w)
	ROL.w -1(A3,D2.l)
	ROL.w -128(A4,D3.w)

; ROL.w, (xxx).w
	ROL.w ($0000).w
	ROL.w ($1234).w
	ROL.w ($7FFF).w

; ROL.w, (xxx).l
	ROL.w ($00000000).l
	ROL.w ($12345678).l
	ROL.w ($FFFFFFFF).l

; Not supported
; ROL.w, d16(PC)
; ROL.w, d8(PC, Xn)
; ROL.w, Imm
