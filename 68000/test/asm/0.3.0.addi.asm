	mc68000
	opt o-

; =========================================================
; ADDI.b
; =========================================================

; ADDI.b, Dn - Mode 000 (Data register)
	ADDI.b #$12,D0
	ADDI.b #$12,D1
	ADDI.b #$12,D2
	ADDI.b #$12,D3
	ADDI.b #$12,D4
	ADDI.b #$12,D5
	ADDI.b #$12,D6
	ADDI.b #$12,D7

; ADDI.b, An - Mode 001 (Address)
;	Not supported

; ADDI.b, (An) - Mode 010
	ADDI.b #$12,(A0)
	ADDI.b #$12,(A1)
	ADDI.b #$12,(A2)
	ADDI.b #$12,(A3)
	ADDI.b #$12,(A4)
	ADDI.b #$12,(A5)
	ADDI.b #$12,(A6)
	ADDI.b #$12,(A7)

; ADDI.b (An)+ - Mode 011 (Address with postincrement)
	ADDI.b #$12,(A0)+
	ADDI.b #$12,(A1)+
	ADDI.b #$12,(A2)+
	ADDI.b #$12,(A3)+
	ADDI.b #$12,(A4)+
	ADDI.b #$12,(A5)+
	ADDI.b #$12,(A6)+
	ADDI.b #$12,(A7)+

; ADDI.b -(An) - Mode 100 (Address with predecrement)
	ADDI.b #$12,-(A0)
	ADDI.b #$12,-(A1)
	ADDI.b #$12,-(A2)
	ADDI.b #$12,-(A3)
	ADDI.b #$12,-(A4)
	ADDI.b #$12,-(A5)
	ADDI.b #$12,-(A6)
	ADDI.b #$12,-(A7)

; ADDI.b, d16(An) - Mode 101 (Address with displacement)
	ADDI.b #$12,-32768(A0)
	ADDI.b #$12,-32768(A1)
	ADDI.b #$12,-32768(A2)
	ADDI.b #$12,-32768(A3)
	ADDI.b #$12,32767(A4)
	ADDI.b #$12,32767(A5)
	ADDI.b #$12,32767(A6)
	ADDI.b #$12,32767(A7)

; ADDI.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ADDI.b #$12,-128(A0,D0.w)
	ADDI.b #$12,-128(A1,D1.w)
	ADDI.b #$12,-128(A2,D2.w)
	ADDI.b #$12,-128(A3,D3.w)
	ADDI.b #$12,127(A4,D4.w)
	ADDI.b #$12,127(A5,D5.w)
	ADDI.b #$12,127(A6,D6.w)
	ADDI.b #$12,127(A7,D7.w)

	ADDI.b #$12,-128(A0,A0.w)
	ADDI.b #$12,-128(A1,A1.w)
	ADDI.b #$12,-128(A2,A2.w)
	ADDI.b #$12,-128(A3,A3.w)
	ADDI.b #$12,127(A4,A4.w)
	ADDI.b #$12,127(A5,A5.w)
	ADDI.b #$12,127(A6,A6.w)
	ADDI.b #$12,127(A7,A7.w)

	ADDI.b #$12,-128(A0,D0.l)
	ADDI.b #$12,-128(A1,D1.l)
	ADDI.b #$12,-128(A2,D2.l)
	ADDI.b #$12,-128(A3,D3.l)
	ADDI.b #$12,127(A4,D4.l)
	ADDI.b #$12,127(A5,D5.l)
	ADDI.b #$12,127(A6,D6.l)
	ADDI.b #$12,127(A7,D7.l)

	ADDI.b #$12,-128(A0,A0.l)
	ADDI.b #$12,-128(A1,A1.l)
	ADDI.b #$12,-128(A2,A2.l)
	ADDI.b #$12,-128(A3,A3.l)
	ADDI.b #$12,127(A4,A4.l)
	ADDI.b #$12,127(A5,A5.l)
	ADDI.b #$12,127(A6,A6.l)
	ADDI.b #$12,127(A7,A7.l)

; ADDI.b, (xxx).w - Mode 111/000 (Absolute Short)
	ADDI.b #$12,($1234).w

; ADDI.b, (xxx).l - Mode 111/001 (Absolute Long)
	ADDI.b #$12,($12345678).l

; ADDI.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ADDI.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; ADDI.b, Imm - Mode 111/100 (Immediate)

	; Not supported

; =========================================================
; ADDI.w
; =========================================================

; ADDI.w, Dn - Mode 000 (Data register)
	ADDI.w #$1234,D0
	ADDI.w #$1234,D1
	ADDI.w #$1234,D2
	ADDI.w #$1234,D3
	ADDI.w #$1234,D4
	ADDI.w #$1234,D5
	ADDI.w #$1234,D6
	ADDI.w #$1234,D7

; ADDI.w, An - Mode 001 (Address)
;	Not supported

; ADDI.w, (An) - Mode 010
	ADDI.w #$1234,(A0)
	ADDI.w #$1234,(A1)
	ADDI.w #$1234,(A2)
	ADDI.w #$1234,(A3)
	ADDI.w #$1234,(A4)
	ADDI.w #$1234,(A5)
	ADDI.w #$1234,(A6)
	ADDI.w #$1234,(A7)

; ADDI.w (An)+ - Mode 011 (Address with postincrement)
	ADDI.w #$1234,(A0)+
	ADDI.w #$1234,(A1)+
	ADDI.w #$1234,(A2)+
	ADDI.w #$1234,(A3)+
	ADDI.w #$1234,(A4)+
	ADDI.w #$1234,(A5)+
	ADDI.w #$1234,(A6)+
	ADDI.w #$1234,(A7)+

; ADDI.w -(An) - Mode 100 (Address with predecrement)
	ADDI.w #$1234,-(A0)
	ADDI.w #$1234,-(A1)
	ADDI.w #$1234,-(A2)
	ADDI.w #$1234,-(A3)
	ADDI.w #$1234,-(A4)
	ADDI.w #$1234,-(A5)
	ADDI.w #$1234,-(A6)
	ADDI.w #$1234,-(A7)

; ADDI.w, d16(An) - Mode 101 (Address with displacement)
	ADDI.w #$1234,-32768(A0)
	ADDI.w #$1234,-32768(A1)
	ADDI.w #$1234,-32768(A2)
	ADDI.w #$1234,-32768(A3)
	ADDI.w #$1234,32767(A4)
	ADDI.w #$1234,32767(A5)
	ADDI.w #$1234,32767(A6)
	ADDI.w #$1234,32767(A7)

; ADDI.w, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ADDI.w #$1234,-128(A0,D0.w)
	ADDI.w #$1234,-128(A1,D1.w)
	ADDI.w #$1234,-128(A2,D2.w)
	ADDI.w #$1234,-128(A3,D3.w)
	ADDI.w #$1234,127(A4,D4.w)
	ADDI.w #$1234,127(A5,D5.w)
	ADDI.w #$1234,127(A6,D6.w)
	ADDI.w #$1234,127(A7,D7.w)

	ADDI.w #$1234,-128(A0,A0.w)
	ADDI.w #$1234,-128(A1,A1.w)
	ADDI.w #$1234,-128(A2,A2.w)
	ADDI.w #$1234,-128(A3,A3.w)
	ADDI.w #$1234,127(A4,A4.w)
	ADDI.w #$1234,127(A5,A5.w)
	ADDI.w #$1234,127(A6,A6.w)
	ADDI.w #$1234,127(A7,A7.w)

	ADDI.w #$1234,-128(A0,D0.l)
	ADDI.w #$1234,-128(A1,D1.l)
	ADDI.w #$1234,-128(A2,D2.l)
	ADDI.w #$1234,-128(A3,D3.l)
	ADDI.w #$1234,127(A4,D4.l)
	ADDI.w #$1234,127(A5,D5.l)
	ADDI.w #$1234,127(A6,D6.l)
	ADDI.w #$1234,127(A7,D7.l)

	ADDI.w #$1234,-128(A0,A0.l)
	ADDI.w #$1234,-128(A1,A1.l)
	ADDI.w #$1234,-128(A2,A2.l)
	ADDI.w #$1234,-128(A3,A3.l)
	ADDI.w #$1234,127(A4,A4.l)
	ADDI.w #$1234,127(A5,A5.l)
	ADDI.w #$1234,127(A6,A6.l)
	ADDI.w #$1234,127(A7,A7.l)

; ADDI.w, (xxx).w - Mode 111/000 (Absolute Short)
	ADDI.w #$1234,($1234).w

; ADDI.w, (xxx).l - Mode 111/001 (Absolute Long)
	ADDI.w #$1234,($12345678).l

; ADDI.w, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ADDI.w, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; ADDI.w, Imm - Mode 111/100 (Immediate)
	; Not supported

; =========================================================
; ADDI.l
; =========================================================

; ADDI.l, Dn - Mode 000 (Data register)
	ADDI.l #$12345678,D0
	ADDI.l #$12345678,D1
	ADDI.l #$12345678,D2
	ADDI.l #$12345678,D3
	ADDI.l #$12345678,D4
	ADDI.l #$12345678,D5
	ADDI.l #$12345678,D6
	ADDI.l #$12345678,D7

; ADDI.l, An - Mode 001 (Address)
;	Not supported

; ADDI.l, (An) - Mode 010
	ADDI.l #$12345678,(A0)
	ADDI.l #$12345678,(A1)
	ADDI.l #$12345678,(A2)
	ADDI.l #$12345678,(A3)
	ADDI.l #$12345678,(A4)
	ADDI.l #$12345678,(A5)
	ADDI.l #$12345678,(A6)
	ADDI.l #$12345678,(A7)

; ADDI.l (An)+ - Mode 011 (Address with postincrement)
	ADDI.l #$12345678,(A0)+
	ADDI.l #$12345678,(A1)+
	ADDI.l #$12345678,(A2)+
	ADDI.l #$12345678,(A3)+
	ADDI.l #$12345678,(A4)+
	ADDI.l #$12345678,(A5)+
	ADDI.l #$12345678,(A6)+
	ADDI.l #$12345678,(A7)+

; ADDI.l -(An) - Mode 100 (Address with predecrement)
	ADDI.l #$12345678,-(A0)
	ADDI.l #$12345678,-(A1)
	ADDI.l #$12345678,-(A2)
	ADDI.l #$12345678,-(A3)
	ADDI.l #$12345678,-(A4)
	ADDI.l #$12345678,-(A5)
	ADDI.l #$12345678,-(A6)
	ADDI.l #$12345678,-(A7)

; ADDI.l, d16(An) - Mode 101 (Address with displacement)
	ADDI.l #$12345678,-32768(A0)
	ADDI.l #$12345678,-32768(A1)
	ADDI.l #$12345678,-32768(A2)
	ADDI.l #$12345678,-32768(A3)
	ADDI.l #$12345678,32767(A4)
	ADDI.l #$12345678,32767(A5)
	ADDI.l #$12345678,32767(A6)
	ADDI.l #$12345678,32767(A7)

; ADDI.l, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ADDI.l #$12345678,-128(A0,D0.w)
	ADDI.l #$12345678,-128(A1,D1.w)
	ADDI.l #$12345678,-128(A2,D2.w)
	ADDI.l #$12345678,-128(A3,D3.w)
	ADDI.l #$12345678,127(A4,D4.w)
	ADDI.l #$12345678,127(A5,D5.w)
	ADDI.l #$12345678,127(A6,D6.w)
	ADDI.l #$12345678,127(A7,D7.w)

	ADDI.l #$12345678,-128(A0,A0.w)
	ADDI.l #$12345678,-128(A1,A1.w)
	ADDI.l #$12345678,-128(A2,A2.w)
	ADDI.l #$12345678,-128(A3,A3.w)
	ADDI.l #$12345678,127(A4,A4.w)
	ADDI.l #$12345678,127(A5,A5.w)
	ADDI.l #$12345678,127(A6,A6.w)
	ADDI.l #$12345678,127(A7,A7.w)

	ADDI.l #$12345678,-128(A0,D0.l)
	ADDI.l #$12345678,-128(A1,D1.l)
	ADDI.l #$12345678,-128(A2,D2.l)
	ADDI.l #$12345678,-128(A3,D3.l)
	ADDI.l #$12345678,127(A4,D4.l)
	ADDI.l #$12345678,127(A5,D5.l)
	ADDI.l #$12345678,127(A6,D6.l)
	ADDI.l #$12345678,127(A7,D7.l)

	ADDI.l #$12345678,-128(A0,A0.l)
	ADDI.l #$12345678,-128(A1,A1.l)
	ADDI.l #$12345678,-128(A2,A2.l)
	ADDI.l #$12345678,-128(A3,A3.l)
	ADDI.l #$12345678,127(A4,A4.l)
	ADDI.l #$12345678,127(A5,A5.l)
	ADDI.l #$12345678,127(A6,A6.l)
	ADDI.l #$12345678,127(A7,A7.l)

; ADDI.l, (xxx).w - Mode 111/000 (Absolute Short)
	ADDI.l #$12345678,($1234).w

; ADDI.l, (xxx).l - Mode 111/001 (Absolute Long)
	ADDI.l #$12345678,($12345678).l

; ADDI.l, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ADDI.l, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; ADDI.l, Imm - Mode 111/100 (Immediate)
	; Not supported
