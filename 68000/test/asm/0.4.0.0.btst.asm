	mc68000
	opt o-,w-

; =========================================================
; BTST
; =========================================================

; BTST.l, Dn - Mode 000 (Data register, long only)
	BTST.l #7,D0
	BTST.l #7,D1
	BTST.l #7,D2
	BTST.l #7,D3
	BTST.l #7,D4
	BTST.l #7,D5
	BTST.l #7,D6
	BTST.l #7,D7

; BTST.b, An - Mode 001 (Address)
;	Not supported

; BTST.b, (An) - Mode 010
	BTST.b #7,(A0)
	BTST.b #7,(A1)
	BTST.b #7,(A2)
	BTST.b #7,(A3)
	BTST.b #7,(A4)
	BTST.b #7,(A5)
	BTST.b #7,(A6)
	BTST.b #7,(A7)

; BTST.b (An)+ - Mode 011 (Address with postincrement)
	BTST.b #7,(A0)+
	BTST.b #7,(A1)+
	BTST.b #7,(A2)+
	BTST.b #7,(A3)+
	BTST.b #7,(A4)+
	BTST.b #7,(A5)+
	BTST.b #7,(A6)+
	BTST.b #7,(A7)+

; BTST.b -(An) - Mode 100 (Address with predecrement)
	BTST.b #7,-(A0)
	BTST.b #7,-(A1)
	BTST.b #7,-(A2)
	BTST.b #7,-(A3)
	BTST.b #7,-(A4)
	BTST.b #7,-(A5)
	BTST.b #7,-(A6)
	BTST.b #7,-(A7)

; BTST.b, d16(An) - Mode 101 (Address with displacement)
	BTST.b #7,-32768(A0)
	BTST.b #7,-32768(A1)
	BTST.b #7,-32768(A2)
	BTST.b #7,-32768(A3)
	BTST.b #7,32767(A4)
	BTST.b #7,32767(A5)
	BTST.b #7,32767(A6)
	BTST.b #7,32767(A7)

; BTST.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	BTST.b #7,-128(A0,D0.w)
	BTST.b #7,-128(A1,D1.w)
	BTST.b #7,-128(A2,D2.w)
	BTST.b #7,-128(A3,D3.w)
	BTST.b #7,127(A4,D4.w)
	BTST.b #7,127(A5,D5.w)
	BTST.b #7,127(A6,D6.w)
	BTST.b #7,127(A7,D7.w)

	BTST.b #7,-128(A0,A0.w)
	BTST.b #7,-128(A1,A1.w)
	BTST.b #7,-128(A2,A2.w)
	BTST.b #7,-128(A3,A3.w)
	BTST.b #7,127(A4,A4.w)
	BTST.b #7,127(A5,A5.w)
	BTST.b #7,127(A6,A6.w)
	BTST.b #7,127(A7,A7.w)

	BTST.b #7,-128(A0,D0.l)
	BTST.b #7,-128(A1,D1.l)
	BTST.b #7,-128(A2,D2.l)
	BTST.b #7,-128(A3,D3.l)
	BTST.b #7,127(A4,D4.l)
	BTST.b #7,127(A5,D5.l)
	BTST.b #7,127(A6,D6.l)
	BTST.b #7,127(A7,D7.l)

	BTST.b #7,-128(A0,A0.l)
	BTST.b #7,-128(A1,A1.l)
	BTST.b #7,-128(A2,A2.l)
	BTST.b #7,-128(A3,A3.l)
	BTST.b #7,127(A4,A4.l)
	BTST.b #7,127(A5,A5.l)
	BTST.b #7,127(A6,A6.l)
	BTST.b #7,127(A7,A7.l)

; BTST.b, (xxx).w - Mode 111/000 (Absolute Short)
	BTST.b #7,($1234).w

; BTST.b, (xxx).l - Mode 111/001 (Absolute Long)
	BTST.b #7,($12345678).l

; BTST.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
	BTST.b #7,$5678(PC)

; BTST.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	BTST.b #7,-128(PC,D0)
	BTST.b #7,-128(PC,D1)
	BTST.b #7,-128(PC,D2)
	BTST.b #7,-128(PC,D3)
	BTST.b #7,127(PC,D4)
	BTST.b #7,127(PC,D5)
	BTST.b #7,127(PC,D6)
	BTST.b #7,127(PC,D7)

; BTST.b, Imm - Mode 111/100 (Immediate)
	; Not supported
