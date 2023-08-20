	mc68000
	opt o-

; =========================================================
; CHK.w <ea>, Dn
; =========================================================

; CHK.w, Dn, Dn
	CHK.w D7,D0
	CHK.w D6,D1
	CHK.w D5,D2
	CHK.w D4,D3
	CHK.w D3,D4
	CHK.w D2,D5
	CHK.w D1,D6
	CHK.w D0,D7
	CHK.w D0,D0
	CHK.w D7,D7

; CHK.w, An, Dn (not allowed)

; CHK.w, (An), Dn
	CHK.w (A7),D0
	CHK.w (A6),D1
	CHK.w (A0),D0
	CHK.w (A7),D7

; CHK.w, (An)+, Dn
	CHK.w (A7)+,D0
	CHK.w (A6)+,D1
	CHK.w (A0)+,D0
	CHK.w (A7)+,D7

; CHK.w, -(An), Dn
	CHK.w -(A7),D0
	CHK.w -(A6),D1
	CHK.w -(A0),D0
	CHK.w -(A7),D7

; CHK.w, d16(An), Dn
	CHK.w -32768(A2),D0
	CHK.w 0(A3),D1
	CHK.w 1234(A4),D2
	CHK.w 32767(A4),D3

; CHK.w, d8(An, Xn.L|W), Dn
	CHK.w 123(A2,D3.w),D4
	CHK.w -1(A3,D2.l),D5
	CHK.w -128(A4,D3.w),D6

; CHK.w, (xxx).w, Dn
	CHK.w ($0000).w,D0
	CHK.w ($1234).w,D1
	CHK.w ($7FFF).w,D2

; CHK.w, (xxx).l, Dn
	CHK.w ($00000000).l,D3
	CHK.w ($12345678).l,D4
	CHK.w ($FFFFFFFF).l,D5

; CHK.w, d16(PC), Dn
	CHK.w -32768(PC),D7
	CHK.w 0(PC),D6
	CHK.w 1234(PC),D5
	CHK.w 32767(PC),D4

; CHK.w, d8(PC, Xn), Dn
	CHK.w 123(PC,D3.w),D3
	CHK.w -1(PC,D2.l),D2
	CHK.w -128(PC,D3.w),D1

; CHK.w, Imm, Dn
	CHK.w #$FFFF,D0
	CHK.w #$0000,D1
	CHK.w #$1234,D2
	CHK.w #$FFFF,D3
	CHK.w #$0000,D4
	CHK.w #$FF00,D5
	CHK.w #$00FF,D6
	CHK.w #$0F0F,D7

