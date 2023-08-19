	mc68000
	opt o-

; =========================================================
; BCHG (bit number static)
; =========================================================

; BCHG.l, Dn - Mode 000 (Data register, long only)
	BCHG.l #7,D0
	BCHG.l #7,D1
	BCHG.l #7,D2
	BCHG.l #7,D3
	BCHG.l #7,D4
	BCHG.l #7,D5
	BCHG.l #7,D6
	BCHG.l #7,D7

; BCHG.b, An - Mode 001 (Address)
;	Not supported

; BCHG.b, (An) - Mode 010
	BCHG.b #7,(A0)
	BCHG.b #7,(A1)
	BCHG.b #7,(A2)
	BCHG.b #7,(A3)
	BCHG.b #7,(A4)
	BCHG.b #7,(A5)
	BCHG.b #7,(A6)
	BCHG.b #7,(A7)
	
; BCHG.b (An)+ - Mode 011 (Address with postincrement)
	BCHG.b #7,(A0)+
	BCHG.b #7,(A1)+
	BCHG.b #7,(A2)+
	BCHG.b #7,(A3)+
	BCHG.b #7,(A4)+
	BCHG.b #7,(A5)+
	BCHG.b #7,(A6)+
	BCHG.b #7,(A7)+

; BCHG.b -(An) - Mode 100 (Address with predecrement)
	BCHG.b #7,-(A0)
	BCHG.b #7,-(A1)
	BCHG.b #7,-(A2)
	BCHG.b #7,-(A3)
	BCHG.b #7,-(A4)
	BCHG.b #7,-(A5)
	BCHG.b #7,-(A6)
	BCHG.b #7,-(A7)

; BCHG.b, d16(An) - Mode 101 (Address with displacement)
	BCHG.b #7,$5678(A0)
	BCHG.b #7,$5678(A1)
	BCHG.b #7,$5678(A2)
	BCHG.b #7,$5678(A3)
	BCHG.b #7,$5678(A4)
	BCHG.b #7,$5678(A5)
	BCHG.b #7,$5678(A6)
	BCHG.b #7,$5678(A7)

; BCHG.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	BCHG.b #7,$56(A0,D0.w)
	BCHG.b #7,$56(A1,D1.w)
	BCHG.b #7,$56(A2,D2.w)
	BCHG.b #7,$56(A3,D3.w)
	BCHG.b #7,$56(A4,D4.w)
	BCHG.b #7,$56(A5,D5.w)
	BCHG.b #7,$56(A6,D6.w)
	BCHG.b #7,$56(A7,D7.w)

	BCHG.b #7,$56(A0,A0.w)
	BCHG.b #7,$56(A1,A1.w)
	BCHG.b #7,$56(A2,A2.w)
	BCHG.b #7,$56(A3,A3.w)
	BCHG.b #7,$56(A4,A4.w)
	BCHG.b #7,$56(A5,A5.w)
	BCHG.b #7,$56(A6,A6.w)
	BCHG.b #7,$56(A7,A7.w)

	BCHG.b #7,$56(A0,D0.l)
	BCHG.b #7,$56(A1,D1.l)
	BCHG.b #7,$56(A2,D2.l)
	BCHG.b #7,$56(A3,D3.l)
	BCHG.b #7,$56(A4,D4.l)
	BCHG.b #7,$56(A5,D5.l)
	BCHG.b #7,$56(A6,D6.l)
	BCHG.b #7,$56(A7,D7.l)

	BCHG.b #7,$56(A0,A0.l)
	BCHG.b #7,$56(A1,A1.l)
	BCHG.b #7,$56(A2,A2.l)
	BCHG.b #7,$56(A3,A3.l)
	BCHG.b #7,$56(A4,A4.l)
	BCHG.b #7,$56(A5,A5.l)
	BCHG.b #7,$56(A6,A6.l)
	BCHG.b #7,$56(A7,A7.l)

; BCHG.b, (xxx).w - Mode 111/000 (Absolute Short)
	BCHG.b #7,($1234).w

; BCHG.b, (xxx).l - Mode 111/001 (Absolute Long)
	BCHG.b #7,($12345678).l

; BCHG.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; BCHG.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; BCHG.b, Imm - Mode 111/100 (Immediate)
	; Not supported
