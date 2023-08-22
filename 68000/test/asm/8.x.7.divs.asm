	mc68000
	opt o-

; =========================================================
; DIVS.w <ea>,Dn
; =========================================================

; DIVS.w, Dn,Dn
	DIVS.w D7,D0
	DIVS.w D6,D1
	DIVS.w D5,D2
	DIVS.w D4,D3
	DIVS.w D3,D4
	DIVS.w D2,D5
	DIVS.w D1,D6
	DIVS.w D0,D7

; DIVS.w, An,Dn (not allowed)

; DIVS.w, (An),Dn
	DIVS.w (A5),D0

; DIVS.w, (An)+,Dn
	DIVS.w (A6)+,D1

; DIVS.w, -(An),Dn
	DIVS.w -(A7),D2

; DIVS.w, d16(An),Dn
	DIVS.w -32768(A2),D3
	DIVS.w 0(A3),D4
	DIVS.w 1234(A4),D5
	DIVS.w 32767(A4),D6

; DIVS.w, d8(An, Xn.L|W),Dn
	DIVS.w 123(A2,A3.w),D7
	DIVS.w -1(A3,D2.l),D0
	DIVS.w -128(A4,D3.w),D1

; DIVS.w, (xxx).w,Dn
	DIVS.w ($0000).w,D2
	DIVS.w ($1234).w,D3
	DIVS.w ($7FFF).w,D4

; DIVS.w, (xxx).l,Dn
	DIVS.w ($00000000).l,D5
	DIVS.w ($12345678).l,D6
	DIVS.w ($FFFFFFFF).l,D7

; DIVS.w, d16(PC),Dn
	DIVS.w -32768(PC),D0
	DIVS.w 0(PC),D1
	DIVS.w 1234(PC),D2
	DIVS.w 32767(PC),D3

; DIVS.w, d8(PC, Xn),Dn
	DIVS.w 123(PC,A3.w),D4
	DIVS.w -1(PC,D2.l),D5
	DIVS.w -128(PC,D3.w),D6

; DIVS.w, Imm,Dn
	DIVS.w #$0000,D0
	DIVS.w #$8000,D1
	DIVS.w #$FFFF,D2
	DIVS.w #$FF00,D3
	DIVS.w #$00FF,D4
	DIVS.w #$F0F0,D5
	DIVS.w #$AA55,D6
	DIVS.w #$1234,D7
