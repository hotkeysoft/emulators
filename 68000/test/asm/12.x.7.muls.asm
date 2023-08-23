	mc68000
	opt o-

; =========================================================
; MULS.w <ea>,Dn
; =========================================================

; MULS.w, Dn,Dn
	MULS.w D7,D0
	MULS.w D6,D1
	MULS.w D5,D2
	MULS.w D4,D3
	MULS.w D3,D4
	MULS.w D2,D5
	MULS.w D1,D6
	MULS.w D0,D7

; MULS.w, An,Dn (not allowed)

; MULS.w, (An),Dn
	MULS.w (A5),D0

; MULS.w, (An)+,Dn
	MULS.w (A6)+,D1

; MULS.w, -(An),Dn
	MULS.w -(A7),D2

; MULS.w, d16(An),Dn
	MULS.w -32768(A2),D3
	MULS.w 0(A3),D4
	MULS.w 1234(A4),D5
	MULS.w 32767(A4),D6

; MULS.w, d8(An, Xn.L|W),Dn
	MULS.w 123(A2,A3.w),D7
	MULS.w -1(A3,D2.l),D0
	MULS.w -128(A4,D3.w),D1

; MULS.w, (xxx).w,Dn
	MULS.w ($0000).w,D2
	MULS.w ($1234).w,D3
	MULS.w ($7FFF).w,D4

; MULS.w, (xxx).l,Dn
	MULS.w ($00000000).l,D5
	MULS.w ($12345678).l,D6
	MULS.w ($FFFFFFFF).l,D7

; MULS.w, d16(PC),Dn
	MULS.w -32768(PC),D0
	MULS.w 0(PC),D1
	MULS.w 1234(PC),D2
	MULS.w 32767(PC),D3

; MULS.w, d8(PC, Xn),Dn
	MULS.w 123(PC,A3.w),D4
	MULS.w -1(PC,D2.l),D5
	MULS.w -128(PC,D3.w),D6

; MULS.w, Imm,Dn
	MULS.w #$0000,D0
	MULS.w #$8000,D1
	MULS.w #$FFFF,D2
	MULS.w #$FF00,D3
	MULS.w #$00FF,D4
	MULS.w #$F0F0,D5
	MULS.w #$AA55,D6
	MULS.w #$1234,D7
