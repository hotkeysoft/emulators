	mc68000
	opt o-

; =========================================================
; BSET (bit number static)
; =========================================================

; BSET.l, Dn - Mode 000 (Data register, long only)
	BSET.l #7,D0
	BSET.l #7,D1
	BSET.l #7,D2
	BSET.l #7,D3
	BSET.l #7,D4
	BSET.l #7,D5
	BSET.l #7,D6
	BSET.l #7,D7

; BSET.b, An - Mode 001 (Address)
;	Not supported

; BSET.b, (An) - Mode 010
	BSET.b #7,(A0)
	BSET.b #7,(A1)
	BSET.b #7,(A2)
	BSET.b #7,(A3)
	BSET.b #7,(A4)
	BSET.b #7,(A5)
	BSET.b #7,(A6)
	BSET.b #7,(A7)
	
; BSET.b (An)+ - Mode 011 (Address with postincrement)
	BSET.b #7,(A0)+
	BSET.b #7,(A1)+
	BSET.b #7,(A2)+
	BSET.b #7,(A3)+
	BSET.b #7,(A4)+
	BSET.b #7,(A5)+
	BSET.b #7,(A6)+
	BSET.b #7,(A7)+

; BSET.b -(An) - Mode 100 (Address with predecrement)
	BSET.b #7,-(A0)
	BSET.b #7,-(A1)
	BSET.b #7,-(A2)
	BSET.b #7,-(A3)
	BSET.b #7,-(A4)
	BSET.b #7,-(A5)
	BSET.b #7,-(A6)
	BSET.b #7,-(A7)

; BSET.b, d16(An) - Mode 101 (Address with displacement)
	BSET.b #7,$5678(A0)
	BSET.b #7,$5678(A1)
	BSET.b #7,$5678(A2)
	BSET.b #7,$5678(A3)
	BSET.b #7,$5678(A4)
	BSET.b #7,$5678(A5)
	BSET.b #7,$5678(A6)
	BSET.b #7,$5678(A7)

; BSET.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	BSET.b #7,$56(A0,D0.w)
	BSET.b #7,$56(A1,D1.w)
	BSET.b #7,$56(A2,D2.w)
	BSET.b #7,$56(A3,D3.w)
	BSET.b #7,$56(A4,D4.w)
	BSET.b #7,$56(A5,D5.w)
	BSET.b #7,$56(A6,D6.w)
	BSET.b #7,$56(A7,D7.w)

	BSET.b #7,$56(A0,A0.w)
	BSET.b #7,$56(A1,A1.w)
	BSET.b #7,$56(A2,A2.w)
	BSET.b #7,$56(A3,A3.w)
	BSET.b #7,$56(A4,A4.w)
	BSET.b #7,$56(A5,A5.w)
	BSET.b #7,$56(A6,A6.w)
	BSET.b #7,$56(A7,A7.w)

	BSET.b #7,$56(A0,D0.l)
	BSET.b #7,$56(A1,D1.l)
	BSET.b #7,$56(A2,D2.l)
	BSET.b #7,$56(A3,D3.l)
	BSET.b #7,$56(A4,D4.l)
	BSET.b #7,$56(A5,D5.l)
	BSET.b #7,$56(A6,D6.l)
	BSET.b #7,$56(A7,D7.l)

	BSET.b #7,$56(A0,A0.l)
	BSET.b #7,$56(A1,A1.l)
	BSET.b #7,$56(A2,A2.l)
	BSET.b #7,$56(A3,A3.l)
	BSET.b #7,$56(A4,A4.l)
	BSET.b #7,$56(A5,A5.l)
	BSET.b #7,$56(A6,A6.l)
	BSET.b #7,$56(A7,A7.l)

; BSET.b, (xxx).w - Mode 111/000 (Absolute Short)
	BSET.b #7,($1234).w

; BSET.b, (xxx).l - Mode 111/001 (Absolute Long)
	BSET.b #7,($12345678).l

; BSET.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; BSET.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; BSET.b, Imm - Mode 111/100 (Immediate)
	; Not supported
