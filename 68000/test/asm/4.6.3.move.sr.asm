	mc68000
	opt o-

; =========================================================
; MOVE.w <ea>, SR
; =========================================================

; MOVE.w, Dn, SR
	MOVE.w D0,SR

; MOVE.w, An, SR (not allowed for dest)

; MOVE.w, (An), SR
	MOVE.w (A5),SR

; MOVE.w, (An)+, SR
	MOVE.w (A6)+,SR

; MOVE.w, -(An), SR
	MOVE.w -(A7),SR

; MOVE.w, d16(An), SR
	MOVE.w -32768(A2),SR
	MOVE.w 0(A3),SR
	MOVE.w 1234(A4),SR
	MOVE.w 32767(A4),SR

; MOVE.w, d8(An, Xn.L|W), SR
	MOVE.w 123(A2,A3.w),SR
	MOVE.w -1(A3,D2.l),SR
	MOVE.w -128(A4,D3.w),SR

; MOVE.w, (xxx).w, SR
	MOVE.w ($0000).w,SR
	MOVE.w ($1234).w,SR
	MOVE.w ($7FFF).w,SR

; MOVE.w, (xxx).l, SR
	MOVE.w ($00000000).l,SR
	MOVE.w ($12345678).l,SR
	MOVE.w ($FFFFFFFF).l,SR

; MOVE.w, d16(PC), SR
	MOVE.w -32768(PC),SR
	MOVE.w 0(PC),SR
	MOVE.w 1234(PC),SR
	MOVE.w 32767(PC),SR

; MOVE.w, d8(PC, Xn), SR
	MOVE.w 123(PC,A3.w),SR
	MOVE.w -1(PC,D2.l),SR
	MOVE.w -128(PC,D3.w),SR

; MOVE.w, Imm, SR
	MOVE.w #$0000,SR
	MOVE.w #$FFFF,SR
	MOVE.w #$7FFF,SR
	MOVE.w #$1234,SR
