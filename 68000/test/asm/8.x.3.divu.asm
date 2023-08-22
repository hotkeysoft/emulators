	mc68000
	opt o-

; =========================================================
; DIVU.w <ea>,Dn
; =========================================================

; DIVU.w, Dn,Dn
	DIVU.w D7,D0
	DIVU.w D6,D1
	DIVU.w D5,D2
	DIVU.w D4,D3
	DIVU.w D3,D4
	DIVU.w D2,D5
	DIVU.w D1,D6
	DIVU.w D0,D7

; DIVU.w, An,Dn (not allowed)

; DIVU.w, (An),Dn
	DIVU.w (A5),D0

; DIVU.w, (An)+,Dn
	DIVU.w (A6)+,D1

; DIVU.w, -(An),Dn
	DIVU.w -(A7),D2

; DIVU.w, d16(An),Dn
	DIVU.w -32768(A2),D3
	DIVU.w 0(A3),D4
	DIVU.w 1234(A4),D5
	DIVU.w 32767(A4),D6

; DIVU.w, d8(An, Xn.L|W),Dn
	DIVU.w 123(A2,A3.w),D7
	DIVU.w -1(A3,D2.l),D0
	DIVU.w -128(A4,D3.w),D1

; DIVU.w, (xxx).w,Dn
	DIVU.w ($0000).w,D2
	DIVU.w ($1234).w,D3
	DIVU.w ($7FFF).w,D4

; DIVU.w, (xxx).l,Dn
	DIVU.w ($00000000).l,D5
	DIVU.w ($12345678).l,D6
	DIVU.w ($FFFFFFFF).l,D7

; DIVU.w, d16(PC),Dn
	DIVU.w -32768(PC),D0
	DIVU.w 0(PC),D1
	DIVU.w 1234(PC),D2
	DIVU.w 32767(PC),D3

; DIVU.w, d8(PC, Xn),Dn
	DIVU.w 123(PC,A3.w),D4
	DIVU.w -1(PC,D2.l),D5
	DIVU.w -128(PC,D3.w),D6

; DIVU.w, Imm,Dn
	DIVU.w #$0000,D0
	DIVU.w #$8000,D1
	DIVU.w #$FFFF,D2
	DIVU.w #$FF00,D3
	DIVU.w #$00FF,D4
	DIVU.w #$F0F0,D5
	DIVU.w #$AA55,D6
	DIVU.w #$1234,D7
