	mc68000
	opt o-,w-

; =========================================================
; BSET (bit number dynamic)
; =========================================================

; BSET.l, Dn - Mode 000 (Data register, long only)
	BSET.l D0,D0
	BSET.l D1,D1
	BSET.l D2,D2
	BSET.l D3,D3
	BSET.l D4,D4
	BSET.l D5,D5
	BSET.l D6,D6
	BSET.l D7,D7

; BSET.b, An - Mode 001 (Address)
;	Not supported

; BSET.b, (An) - Mode 010
	BSET.b D0,(A0)
	BSET.b D1,(A1)
	BSET.b D2,(A2)
	BSET.b D3,(A3)
	BSET.b D4,(A4)
	BSET.b D5,(A5)
	BSET.b D6,(A6)
	BSET.b D7,(A7)

; BSET.b (An)+ - Mode 011 (Address with postincrement)
	BSET.b D0,(A0)+
	BSET.b D1,(A1)+
	BSET.b D2,(A2)+
	BSET.b D3,(A3)+
	BSET.b D4,(A4)+
	BSET.b D5,(A5)+
	BSET.b D6,(A6)+
	BSET.b D7,(A7)+

; BSET.b -(An) - Mode 100 (Address with predecrement)
	BSET.b D0,-(A0)
	BSET.b D1,-(A1)
	BSET.b D2,-(A2)
	BSET.b D3,-(A3)
	BSET.b D4,-(A4)
	BSET.b D5,-(A5)
	BSET.b D6,-(A6)
	BSET.b D7,-(A7)

; BSET.b, d16(An) - Mode 101 (Address with displacement)
	BSET.b D0,-32768(A0)
	BSET.b D1,-32768(A1)
	BSET.b D2,-32768(A2)
	BSET.b D3,-32768(A3)
	BSET.b D4,32767(A4)
	BSET.b D5,32767(A5)
	BSET.b D6,32767(A6)
	BSET.b D7,32767(A7)

; BSET.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	BSET.b D0,-128(A0,D0.w)
	BSET.b D1,-128(A1,D1.w)
	BSET.b D2,-128(A2,D2.w)
	BSET.b D3,-128(A3,D3.w)
	BSET.b D4,127(A4,D4.w)
	BSET.b D5,127(A5,D5.w)
	BSET.b D6,127(A6,D6.w)
	BSET.b D7,127(A7,D7.w)

	BSET.b D0,-128(A0,A0.w)
	BSET.b D1,-128(A1,A1.w)
	BSET.b D2,-128(A2,A2.w)
	BSET.b D3,-128(A3,A3.w)
	BSET.b D4,127(A4,A4.w)
	BSET.b D5,127(A5,A5.w)
	BSET.b D6,127(A6,A6.w)
	BSET.b D7,127(A7,A7.w)

	BSET.b D0,-128(A0,D0.l)
	BSET.b D1,-128(A1,D1.l)
	BSET.b D2,-128(A2,D2.l)
	BSET.b D3,-128(A3,D3.l)
	BSET.b D4,127(A4,D4.l)
	BSET.b D5,127(A5,D5.l)
	BSET.b D6,127(A6,D6.l)
	BSET.b D7,127(A7,D7.l)

	BSET.b D0,-128(A0,A0.l)
	BSET.b D1,-128(A1,A1.l)
	BSET.b D2,-128(A2,A2.l)
	BSET.b D3,-128(A3,A3.l)
	BSET.b D4,127(A4,A4.l)
	BSET.b D5,127(A5,A5.l)
	BSET.b D6,127(A6,A6.l)
	BSET.b D7,127(A7,A7.l)

; BSET.b, (xxx).w - Mode 111/000 (Absolute Short)
	BSET.b D0,($1234).w

; BSET.b, (xxx).l - Mode 111/001 (Absolute Long)
	BSET.b D0,($12345678).l

; BSET.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; BSET.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; BSET.b, Imm - Mode 111/100 (Immediate)
	; Not supported
