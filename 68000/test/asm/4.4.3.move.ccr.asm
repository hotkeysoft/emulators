	mc68000
	opt o-

; =========================================================
; MOVE.w <ea>, CCR
; =========================================================

; MOVE.w, Dn, CCR
	MOVE.w D0,CCR

; MOVE.w, An, CCR (not allowed for dest)

; MOVE.w, (An), CCR
	MOVE.w (A5),CCR

; MOVE.w, (An)+, CCR
	MOVE.w (A6)+,CCR

; MOVE.w, -(An), CCR
	MOVE.w -(A7),CCR

; MOVE.w, d16(An), CCR
	MOVE.w -32768(A2),CCR
	MOVE.w 0(A3),CCR
	MOVE.w 1234(A4),CCR
	MOVE.w 32767(A4),CCR

; MOVE.w, d8(An, Xn.L|W), CCR
	MOVE.w 123(A2,A3.w),CCR
	MOVE.w -1(A3,D2.l),CCR
	MOVE.w -128(A4,D3.w),CCR

; MOVE.w, (xxx).w, CCR
	MOVE.w ($0000).w,CCR
	MOVE.w ($1234).w,CCR
	MOVE.w ($7FFF).w,CCR

; MOVE.w, (xxx).l, CCR
	MOVE.w ($00000000).l,CCR
	MOVE.w ($12345678).l,CCR
	MOVE.w ($FFFFFFFF).l,CCR

; MOVE.w, d16(PC), CCR
	MOVE.w -32768(PC),CCR
	MOVE.w 0(PC),CCR
	MOVE.w 1234(PC),CCR
	MOVE.w 32767(PC),CCR

; MOVE.w, d8(PC, Xn), CCR
	MOVE.w 123(PC,A3.w),CCR
	MOVE.w -1(PC,D2.l),CCR
	MOVE.w -128(PC,D3.w),CCR

; MOVE.w, Imm, CCR
	MOVE.w #$0000,CCR
	MOVE.w #$FFFF,CCR
	MOVE.w #$7FFF,CCR
	MOVE.w #$1234,CCR
