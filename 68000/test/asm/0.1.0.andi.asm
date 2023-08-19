	mc68000
	opt o-

; =========================================================
; ANDI.b
; =========================================================

; ANDI.b, Dn - Mode 000 (Data register)
	ANDI.b #$12,D0
	ANDI.b #$12,D1
	ANDI.b #$12,D2
	ANDI.b #$12,D3
	ANDI.b #$12,D4
	ANDI.b #$12,D5
	ANDI.b #$12,D6
	ANDI.b #$12,D7

; ANDI.b, An - Mode 001 (Address)
;	Not supported

; ANDI.b, (An) - Mode 010
	ANDI.b #$12,(A0)
	ANDI.b #$12,(A1)
	ANDI.b #$12,(A2)
	ANDI.b #$12,(A3)
	ANDI.b #$12,(A4)
	ANDI.b #$12,(A5)
	ANDI.b #$12,(A6)
	ANDI.b #$12,(A7)

; ANDI.b (An)+ - Mode 011 (Address with postincrement)
	ANDI.b #$12,(A0)+
	ANDI.b #$12,(A1)+
	ANDI.b #$12,(A2)+
	ANDI.b #$12,(A3)+
	ANDI.b #$12,(A4)+
	ANDI.b #$12,(A5)+
	ANDI.b #$12,(A6)+
	ANDI.b #$12,(A7)+

; ANDI.b -(An) - Mode 100 (Address with predecrement)
	ANDI.b #$12,-(A0)
	ANDI.b #$12,-(A1)
	ANDI.b #$12,-(A2)
	ANDI.b #$12,-(A3)
	ANDI.b #$12,-(A4)
	ANDI.b #$12,-(A5)
	ANDI.b #$12,-(A6)
	ANDI.b #$12,-(A7)

; ANDI.b, d16(An) - Mode 101 (Address with displacement)
	ANDI.b #$12,-32768(A0)
	ANDI.b #$12,-32768(A1)
	ANDI.b #$12,-32768(A2)
	ANDI.b #$12,-32768(A3)
	ANDI.b #$12,32767(A4)
	ANDI.b #$12,32767(A5)
	ANDI.b #$12,32767(A6)
	ANDI.b #$12,32767(A7)

; ANDI.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ANDI.b #$12,-128(A0,D0.w)
	ANDI.b #$12,-128(A1,D1.w)
	ANDI.b #$12,-128(A2,D2.w)
	ANDI.b #$12,-128(A3,D3.w)
	ANDI.b #$12,127(A4,D4.w)
	ANDI.b #$12,127(A5,D5.w)
	ANDI.b #$12,127(A6,D6.w)
	ANDI.b #$12,127(A7,D7.w)

	ANDI.b #$12,-128(A0,A0.w)
	ANDI.b #$12,-128(A1,A1.w)
	ANDI.b #$12,-128(A2,A2.w)
	ANDI.b #$12,-128(A3,A3.w)
	ANDI.b #$12,127(A4,A4.w)
	ANDI.b #$12,127(A5,A5.w)
	ANDI.b #$12,127(A6,A6.w)
	ANDI.b #$12,127(A7,A7.w)

	ANDI.b #$12,-128(A0,D0.l)
	ANDI.b #$12,-128(A1,D1.l)
	ANDI.b #$12,-128(A2,D2.l)
	ANDI.b #$12,-128(A3,D3.l)
	ANDI.b #$12,127(A4,D4.l)
	ANDI.b #$12,127(A5,D5.l)
	ANDI.b #$12,127(A6,D6.l)
	ANDI.b #$12,127(A7,D7.l)

	ANDI.b #$12,-128(A0,A0.l)
	ANDI.b #$12,-128(A1,A1.l)
	ANDI.b #$12,-128(A2,A2.l)
	ANDI.b #$12,-128(A3,A3.l)
	ANDI.b #$12,127(A4,A4.l)
	ANDI.b #$12,127(A5,A5.l)
	ANDI.b #$12,127(A6,A6.l)
	ANDI.b #$12,127(A7,A7.l)

; ANDI.b, (xxx).w - Mode 111/000 (Absolute Short)
	ANDI.b #$12,($1234).w

; ANDI.b, (xxx).l - Mode 111/001 (Absolute Long)
	ANDI.b #$12,($12345678).l

; ANDI.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ANDI.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; ANDI.b, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "ANDI to CCR/SR"

; =========================================================
; ANDI.w
; =========================================================

; ANDI.w, Dn - Mode 000 (Data register)
	ANDI.w #$1234,D0
	ANDI.w #$1234,D1
	ANDI.w #$1234,D2
	ANDI.w #$1234,D3
	ANDI.w #$1234,D4
	ANDI.w #$1234,D5
	ANDI.w #$1234,D6
	ANDI.w #$1234,D7

; ANDI.w, An - Mode 001 (Address)
;	Not supported

; ANDI.w, (An) - Mode 010
	ANDI.w #$1234,(A0)
	ANDI.w #$1234,(A1)
	ANDI.w #$1234,(A2)
	ANDI.w #$1234,(A3)
	ANDI.w #$1234,(A4)
	ANDI.w #$1234,(A5)
	ANDI.w #$1234,(A6)
	ANDI.w #$1234,(A7)

; ANDI.w (An)+ - Mode 011 (Address with postincrement)
	ANDI.w #$1234,(A0)+
	ANDI.w #$1234,(A1)+
	ANDI.w #$1234,(A2)+
	ANDI.w #$1234,(A3)+
	ANDI.w #$1234,(A4)+
	ANDI.w #$1234,(A5)+
	ANDI.w #$1234,(A6)+
	ANDI.w #$1234,(A7)+

; ANDI.w -(An) - Mode 100 (Address with predecrement)
	ANDI.w #$1234,-(A0)
	ANDI.w #$1234,-(A1)
	ANDI.w #$1234,-(A2)
	ANDI.w #$1234,-(A3)
	ANDI.w #$1234,-(A4)
	ANDI.w #$1234,-(A5)
	ANDI.w #$1234,-(A6)
	ANDI.w #$1234,-(A7)

; ANDI.w, d16(An) - Mode 101 (Address with displacement)
	ANDI.w #$1234,-32768(A0)
	ANDI.w #$1234,-32768(A1)
	ANDI.w #$1234,-32768(A2)
	ANDI.w #$1234,-32768(A3)
	ANDI.w #$1234,32767(A4)
	ANDI.w #$1234,32767(A5)
	ANDI.w #$1234,32767(A6)
	ANDI.w #$1234,32767(A7)

; ANDI.w, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ANDI.w #$1234,-128(A0,D0.w)
	ANDI.w #$1234,-128(A1,D1.w)
	ANDI.w #$1234,-128(A2,D2.w)
	ANDI.w #$1234,-128(A3,D3.w)
	ANDI.w #$1234,127(A4,D4.w)
	ANDI.w #$1234,127(A5,D5.w)
	ANDI.w #$1234,127(A6,D6.w)
	ANDI.w #$1234,127(A7,D7.w)

	ANDI.w #$1234,-128(A0,A0.w)
	ANDI.w #$1234,-128(A1,A1.w)
	ANDI.w #$1234,-128(A2,A2.w)
	ANDI.w #$1234,-128(A3,A3.w)
	ANDI.w #$1234,127(A4,A4.w)
	ANDI.w #$1234,127(A5,A5.w)
	ANDI.w #$1234,127(A6,A6.w)
	ANDI.w #$1234,127(A7,A7.w)

	ANDI.w #$1234,-128(A0,D0.l)
	ANDI.w #$1234,-128(A1,D1.l)
	ANDI.w #$1234,-128(A2,D2.l)
	ANDI.w #$1234,-128(A3,D3.l)
	ANDI.w #$1234,127(A4,D4.l)
	ANDI.w #$1234,127(A5,D5.l)
	ANDI.w #$1234,127(A6,D6.l)
	ANDI.w #$1234,127(A7,D7.l)

	ANDI.w #$1234,-128(A0,A0.l)
	ANDI.w #$1234,-128(A1,A1.l)
	ANDI.w #$1234,-128(A2,A2.l)
	ANDI.w #$1234,-128(A3,A3.l)
	ANDI.w #$1234,127(A4,A4.l)
	ANDI.w #$1234,127(A5,A5.l)
	ANDI.w #$1234,127(A6,A6.l)
	ANDI.w #$1234,127(A7,A7.l)

; ANDI.w, (xxx).w - Mode 111/000 (Absolute Short)
	ANDI.w #$1234,($1234).w

; ANDI.w, (xxx).l - Mode 111/001 (Absolute Long)
	ANDI.w #$1234,($12345678).l

; ANDI.w, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ANDI.w, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; ANDI.w, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "ANDI to CCR/SR"

; =========================================================
; ANDI.l
; =========================================================

; ANDI.l, Dn - Mode 000 (Data register)
	ANDI.l #$12345678,D0
	ANDI.l #$12345678,D1
	ANDI.l #$12345678,D2
	ANDI.l #$12345678,D3
	ANDI.l #$12345678,D4
	ANDI.l #$12345678,D5
	ANDI.l #$12345678,D6
	ANDI.l #$12345678,D7

; ANDI.l, An - Mode 001 (Address)
;	Not supported

; ANDI.l, (An) - Mode 010
	ANDI.l #$12345678,(A0)
	ANDI.l #$12345678,(A1)
	ANDI.l #$12345678,(A2)
	ANDI.l #$12345678,(A3)
	ANDI.l #$12345678,(A4)
	ANDI.l #$12345678,(A5)
	ANDI.l #$12345678,(A6)
	ANDI.l #$12345678,(A7)

; ANDI.l (An)+ - Mode 011 (Address with postincrement)
	ANDI.l #$12345678,(A0)+
	ANDI.l #$12345678,(A1)+
	ANDI.l #$12345678,(A2)+
	ANDI.l #$12345678,(A3)+
	ANDI.l #$12345678,(A4)+
	ANDI.l #$12345678,(A5)+
	ANDI.l #$12345678,(A6)+
	ANDI.l #$12345678,(A7)+

; ANDI.l -(An) - Mode 100 (Address with predecrement)
	ANDI.l #$12345678,-(A0)
	ANDI.l #$12345678,-(A1)
	ANDI.l #$12345678,-(A2)
	ANDI.l #$12345678,-(A3)
	ANDI.l #$12345678,-(A4)
	ANDI.l #$12345678,-(A5)
	ANDI.l #$12345678,-(A6)
	ANDI.l #$12345678,-(A7)

; ANDI.l, d16(An) - Mode 101 (Address with displacement)
	ANDI.l #$12345678,-32768(A0)
	ANDI.l #$12345678,-32768(A1)
	ANDI.l #$12345678,-32768(A2)
	ANDI.l #$12345678,-32768(A3)
	ANDI.l #$12345678,32767(A4)
	ANDI.l #$12345678,32767(A5)
	ANDI.l #$12345678,32767(A6)
	ANDI.l #$12345678,32767(A7)

; ANDI.l, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ANDI.l #$12345678,-128(A0,D0.w)
	ANDI.l #$12345678,-128(A1,D1.w)
	ANDI.l #$12345678,-128(A2,D2.w)
	ANDI.l #$12345678,-128(A3,D3.w)
	ANDI.l #$12345678,127(A4,D4.w)
	ANDI.l #$12345678,127(A5,D5.w)
	ANDI.l #$12345678,127(A6,D6.w)
	ANDI.l #$12345678,127(A7,D7.w)

	ANDI.l #$12345678,-128(A0,A0.w)
	ANDI.l #$12345678,-128(A1,A1.w)
	ANDI.l #$12345678,-128(A2,A2.w)
	ANDI.l #$12345678,-128(A3,A3.w)
	ANDI.l #$12345678,127(A4,A4.w)
	ANDI.l #$12345678,127(A5,A5.w)
	ANDI.l #$12345678,127(A6,A6.w)
	ANDI.l #$12345678,127(A7,A7.w)

	ANDI.l #$12345678,-128(A0,D0.l)
	ANDI.l #$12345678,-128(A1,D1.l)
	ANDI.l #$12345678,-128(A2,D2.l)
	ANDI.l #$12345678,-128(A3,D3.l)
	ANDI.l #$12345678,127(A4,D4.l)
	ANDI.l #$12345678,127(A5,D5.l)
	ANDI.l #$12345678,127(A6,D6.l)
	ANDI.l #$12345678,127(A7,D7.l)

	ANDI.l #$12345678,-128(A0,A0.l)
	ANDI.l #$12345678,-128(A1,A1.l)
	ANDI.l #$12345678,-128(A2,A2.l)
	ANDI.l #$12345678,-128(A3,A3.l)
	ANDI.l #$12345678,127(A4,A4.l)
	ANDI.l #$12345678,127(A5,A5.l)
	ANDI.l #$12345678,127(A6,A6.l)
	ANDI.l #$12345678,127(A7,A7.l)

; ANDI.l, (xxx).w - Mode 111/000 (Absolute Short)
	ANDI.l #$12345678,($1234).w

; ANDI.l, (xxx).l - Mode 111/001 (Absolute Long)
	ANDI.l #$12345678,($12345678).l

; ANDI.l, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ANDI.l, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; ANDI.l, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "ANDI to CCR/SR"

; =========================================================
; ANDI(.b) to CCR
; =========================================================

; ANDI #xxx, CCR - Mode 111/100
	ANDI #$12,CCR

; =========================================================
; ANDI(.w) to SR
; =========================================================

; ANDI #xxx, SR - Mode 111/100
	ANDI #$2345,SR
