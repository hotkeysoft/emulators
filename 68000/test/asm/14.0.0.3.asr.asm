	mc68000
	opt o-

; =========================================================
; ASR.w <ea>
; =========================================================

; ASR.w, Dn (not allowed)
; ASR.w, An (not allowed)

; ASR.w, (An)
	ASR.w (A5)

; ASR.w, (An)+
	ASR.w (A6)+

; ASR.w, -(An)
	ASR.w -(A7)

; ASR.w, d16(An)
	ASR.w -32768(A2)
	ASR.w 0(A3)
	ASR.w 1234(A4)
	ASR.w 32767(A4)

; ASR.w, d8(An, Xn.L|W)
	ASR.w 123(A2,A3.w)
	ASR.w -1(A3,D2.l)
	ASR.w -128(A4,D3.w)

; ASR.w, (xxx).w
	ASR.w ($0000).w
	ASR.w ($1234).w
	ASR.w ($7FFF).w

; ASR.w, (xxx).l
	ASR.w ($00000000).l
	ASR.w ($12345678).l
	ASR.w ($FFFFFFFF).l

; Not supported
; ASR.w, d16(PC)
; ASR.w, d8(PC, Xn)
; ASR.w, Imm
