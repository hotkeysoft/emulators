	mc68000
	opt o-

; =========================================================
; MOVE.w SR, <ea>
; =========================================================

; MOVE.w, SR, Dn
	MOVE.w SR,D0

; MOVE.w, SR, An (not allowed for dest)

; MOVE.w, SR, (An)
	MOVE.w SR,(A5)

; MOVE.w, SR, (An)+
	MOVE.w SR,(A6)+

; MOVE.w, SR, -(An)
	MOVE.w SR,-(A7)

; MOVE.w, SR, d16(An)
	MOVE.w SR,-32768(A2)
	MOVE.w SR,0(A3)
	MOVE.w SR,1234(A4)
	MOVE.w SR,32767(A4)

; MOVE.w, SR, d8(An, Xn.L|W)
	MOVE.w SR,123(A2,A3.w)
	MOVE.w SR,-1(A3,D2.l)
	MOVE.w SR,-128(A4,D3.w)

; MOVE.w, SR, (xxx).w
	MOVE.w SR,($0000).w
	MOVE.w SR,($1234).w
	MOVE.w SR,($7FFF).w

; MOVE.w, SR, (xxx).l
	MOVE.w SR,($00000000).l
	MOVE.w SR,($12345678).l
	MOVE.w SR,($FFFFFFFF).l

; MOVE.w, SR, d16(PC) (not allowed for dest)
; MOVE.w, SR, d8(PC, Xn) (not allowed for dest)
; MOVE.w, SR, Imm (not allowed for dest)
