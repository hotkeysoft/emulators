	mc68000
	opt o-

; =========================================================
; EORI.b
; =========================================================

; EORI.b, Dn - Mode 000 (Data register)
	EORI.b #$12,D0
	EORI.b #$12,D1
	EORI.b #$12,D2
	EORI.b #$12,D3
	EORI.b #$12,D4
	EORI.b #$12,D5
	EORI.b #$12,D6
	EORI.b #$12,D7

; EORI.b, An - Mode 001 (Address)
;	Not supported

; EORI.b, (An) - Mode 010
	EORI.b #$12,(A0)
	EORI.b #$12,(A1)
	EORI.b #$12,(A2)
	EORI.b #$12,(A3)
	EORI.b #$12,(A4)
	EORI.b #$12,(A5)
	EORI.b #$12,(A6)
	EORI.b #$12,(A7)

; EORI.b (An)+ - Mode 011 (Address with postincrement)
	EORI.b #$12,(A0)+
	EORI.b #$12,(A1)+
	EORI.b #$12,(A2)+
	EORI.b #$12,(A3)+
	EORI.b #$12,(A4)+
	EORI.b #$12,(A5)+
	EORI.b #$12,(A6)+
	EORI.b #$12,(A7)+

; EORI.b -(An) - Mode 100 (Address with predecrement)
	EORI.b #$12,-(A0)
	EORI.b #$12,-(A1)
	EORI.b #$12,-(A2)
	EORI.b #$12,-(A3)
	EORI.b #$12,-(A4)
	EORI.b #$12,-(A5)
	EORI.b #$12,-(A6)
	EORI.b #$12,-(A7)

; EORI.b, d16(An) - Mode 101 (Address with displacement)
	EORI.b #$12,-32768(A0)
	EORI.b #$12,-32768(A1)
	EORI.b #$12,-32768(A2)
	EORI.b #$12,-32768(A3)
	EORI.b #$12,32767(A4)
	EORI.b #$12,32767(A5)
	EORI.b #$12,32767(A6)
	EORI.b #$12,32767(A7)

; EORI.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	EORI.b #$12,-128(A0,D0.w)
	EORI.b #$12,-128(A1,D1.w)
	EORI.b #$12,-128(A2,D2.w)
	EORI.b #$12,-128(A3,D3.w)
	EORI.b #$12,127(A4,D4.w)
	EORI.b #$12,127(A5,D5.w)
	EORI.b #$12,127(A6,D6.w)
	EORI.b #$12,127(A7,D7.w)

	EORI.b #$12,-128(A0,A0.w)
	EORI.b #$12,-128(A1,A1.w)
	EORI.b #$12,-128(A2,A2.w)
	EORI.b #$12,-128(A3,A3.w)
	EORI.b #$12,127(A4,A4.w)
	EORI.b #$12,127(A5,A5.w)
	EORI.b #$12,127(A6,A6.w)
	EORI.b #$12,127(A7,A7.w)

	EORI.b #$12,-128(A0,D0.l)
	EORI.b #$12,-128(A1,D1.l)
	EORI.b #$12,-128(A2,D2.l)
	EORI.b #$12,-128(A3,D3.l)
	EORI.b #$12,127(A4,D4.l)
	EORI.b #$12,127(A5,D5.l)
	EORI.b #$12,127(A6,D6.l)
	EORI.b #$12,127(A7,D7.l)

	EORI.b #$12,-128(A0,A0.l)
	EORI.b #$12,-128(A1,A1.l)
	EORI.b #$12,-128(A2,A2.l)
	EORI.b #$12,-128(A3,A3.l)
	EORI.b #$12,127(A4,A4.l)
	EORI.b #$12,127(A5,A5.l)
	EORI.b #$12,127(A6,A6.l)
	EORI.b #$12,127(A7,A7.l)

; EORI.b, (xxx).w - Mode 111/000 (Absolute Short)
	EORI.b #$12,($1234).w

; EORI.b, (xxx).l - Mode 111/001 (Absolute Long)
	EORI.b #$12,($12345678).l

; EORI.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; EORI.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; EORI.b, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "EORI to CCR/SR"

; =========================================================
; EORI.w
; =========================================================

; EORI.w, Dn - Mode 000 (Data register)
	EORI.w #$1234,D0
	EORI.w #$1234,D1
	EORI.w #$1234,D2
	EORI.w #$1234,D3
	EORI.w #$1234,D4
	EORI.w #$1234,D5
	EORI.w #$1234,D6
	EORI.w #$1234,D7

; EORI.w, An - Mode 001 (Address)
;	Not supported

; EORI.w, (An) - Mode 010
	EORI.w #$1234,(A0)
	EORI.w #$1234,(A1)
	EORI.w #$1234,(A2)
	EORI.w #$1234,(A3)
	EORI.w #$1234,(A4)
	EORI.w #$1234,(A5)
	EORI.w #$1234,(A6)
	EORI.w #$1234,(A7)

; EORI.w (An)+ - Mode 011 (Address with postincrement)
	EORI.w #$1234,(A0)+
	EORI.w #$1234,(A1)+
	EORI.w #$1234,(A2)+
	EORI.w #$1234,(A3)+
	EORI.w #$1234,(A4)+
	EORI.w #$1234,(A5)+
	EORI.w #$1234,(A6)+
	EORI.w #$1234,(A7)+

; EORI.w -(An) - Mode 100 (Address with predecrement)
	EORI.w #$1234,-(A0)
	EORI.w #$1234,-(A1)
	EORI.w #$1234,-(A2)
	EORI.w #$1234,-(A3)
	EORI.w #$1234,-(A4)
	EORI.w #$1234,-(A5)
	EORI.w #$1234,-(A6)
	EORI.w #$1234,-(A7)

; EORI.w, d16(An) - Mode 101 (Address with displacement)
	EORI.w #$1234,-32768(A0)
	EORI.w #$1234,-32768(A1)
	EORI.w #$1234,-32768(A2)
	EORI.w #$1234,-32768(A3)
	EORI.w #$1234,32767(A4)
	EORI.w #$1234,32767(A5)
	EORI.w #$1234,32767(A6)
	EORI.w #$1234,32767(A7)

; EORI.w, d8(An, Xn.L|W) - Mode 110 (Address with index)
	EORI.w #$1234,-128(A0,D0.w)
	EORI.w #$1234,-128(A1,D1.w)
	EORI.w #$1234,-128(A2,D2.w)
	EORI.w #$1234,-128(A3,D3.w)
	EORI.w #$1234,127(A4,D4.w)
	EORI.w #$1234,127(A5,D5.w)
	EORI.w #$1234,127(A6,D6.w)
	EORI.w #$1234,127(A7,D7.w)

	EORI.w #$1234,-128(A0,A0.w)
	EORI.w #$1234,-128(A1,A1.w)
	EORI.w #$1234,-128(A2,A2.w)
	EORI.w #$1234,-128(A3,A3.w)
	EORI.w #$1234,127(A4,A4.w)
	EORI.w #$1234,127(A5,A5.w)
	EORI.w #$1234,127(A6,A6.w)
	EORI.w #$1234,127(A7,A7.w)

	EORI.w #$1234,-128(A0,D0.l)
	EORI.w #$1234,-128(A1,D1.l)
	EORI.w #$1234,-128(A2,D2.l)
	EORI.w #$1234,-128(A3,D3.l)
	EORI.w #$1234,127(A4,D4.l)
	EORI.w #$1234,127(A5,D5.l)
	EORI.w #$1234,127(A6,D6.l)
	EORI.w #$1234,127(A7,D7.l)

	EORI.w #$1234,-128(A0,A0.l)
	EORI.w #$1234,-128(A1,A1.l)
	EORI.w #$1234,-128(A2,A2.l)
	EORI.w #$1234,-128(A3,A3.l)
	EORI.w #$1234,127(A4,A4.l)
	EORI.w #$1234,127(A5,A5.l)
	EORI.w #$1234,127(A6,A6.l)
	EORI.w #$1234,127(A7,A7.l)

; EORI.w, (xxx).w - Mode 111/000 (Absolute Short)
	EORI.w #$1234,($1234).w

; EORI.w, (xxx).l - Mode 111/001 (Absolute Long)
	EORI.w #$1234,($12345678).l

; EORI.w, d16(PC) - Mode 111/010 (Program Counter with displacement)
; EORI.w, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; EORI.w, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "EORI to CCR/SR"

; =========================================================
; EORI.l
; =========================================================

; EORI.l, Dn - Mode 000 (Data register)
	EORI.l #$12345678,D0
	EORI.l #$12345678,D1
	EORI.l #$12345678,D2
	EORI.l #$12345678,D3
	EORI.l #$12345678,D4
	EORI.l #$12345678,D5
	EORI.l #$12345678,D6
	EORI.l #$12345678,D7

; EORI.l, An - Mode 001 (Address)
;	Not supported

; EORI.l, (An) - Mode 010
	EORI.l #$12345678,(A0)
	EORI.l #$12345678,(A1)
	EORI.l #$12345678,(A2)
	EORI.l #$12345678,(A3)
	EORI.l #$12345678,(A4)
	EORI.l #$12345678,(A5)
	EORI.l #$12345678,(A6)
	EORI.l #$12345678,(A7)

; EORI.l (An)+ - Mode 011 (Address with postincrement)
	EORI.l #$12345678,(A0)+
	EORI.l #$12345678,(A1)+
	EORI.l #$12345678,(A2)+
	EORI.l #$12345678,(A3)+
	EORI.l #$12345678,(A4)+
	EORI.l #$12345678,(A5)+
	EORI.l #$12345678,(A6)+
	EORI.l #$12345678,(A7)+

; EORI.l -(An) - Mode 100 (Address with predecrement)
	EORI.l #$12345678,-(A0)
	EORI.l #$12345678,-(A1)
	EORI.l #$12345678,-(A2)
	EORI.l #$12345678,-(A3)
	EORI.l #$12345678,-(A4)
	EORI.l #$12345678,-(A5)
	EORI.l #$12345678,-(A6)
	EORI.l #$12345678,-(A7)

; EORI.l, d16(An) - Mode 101 (Address with displacement)
	EORI.l #$12345678,-32768(A0)
	EORI.l #$12345678,-32768(A1)
	EORI.l #$12345678,-32768(A2)
	EORI.l #$12345678,-32768(A3)
	EORI.l #$12345678,32767(A4)
	EORI.l #$12345678,32767(A5)
	EORI.l #$12345678,32767(A6)
	EORI.l #$12345678,32767(A7)

; EORI.l, d8(An, Xn.L|W) - Mode 110 (Address with index)
	EORI.l #$12345678,-128(A0,D0.w)
	EORI.l #$12345678,-128(A1,D1.w)
	EORI.l #$12345678,-128(A2,D2.w)
	EORI.l #$12345678,-128(A3,D3.w)
	EORI.l #$12345678,127(A4,D4.w)
	EORI.l #$12345678,127(A5,D5.w)
	EORI.l #$12345678,127(A6,D6.w)
	EORI.l #$12345678,127(A7,D7.w)

	EORI.l #$12345678,-128(A0,A0.w)
	EORI.l #$12345678,-128(A1,A1.w)
	EORI.l #$12345678,-128(A2,A2.w)
	EORI.l #$12345678,-128(A3,A3.w)
	EORI.l #$12345678,127(A4,A4.w)
	EORI.l #$12345678,127(A5,A5.w)
	EORI.l #$12345678,127(A6,A6.w)
	EORI.l #$12345678,127(A7,A7.w)

	EORI.l #$12345678,-128(A0,D0.l)
	EORI.l #$12345678,-128(A1,D1.l)
	EORI.l #$12345678,-128(A2,D2.l)
	EORI.l #$12345678,-128(A3,D3.l)
	EORI.l #$12345678,127(A4,D4.l)
	EORI.l #$12345678,127(A5,D5.l)
	EORI.l #$12345678,127(A6,D6.l)
	EORI.l #$12345678,127(A7,D7.l)

	EORI.l #$12345678,-128(A0,A0.l)
	EORI.l #$12345678,-128(A1,A1.l)
	EORI.l #$12345678,-128(A2,A2.l)
	EORI.l #$12345678,-128(A3,A3.l)
	EORI.l #$12345678,127(A4,A4.l)
	EORI.l #$12345678,127(A5,A5.l)
	EORI.l #$12345678,127(A6,A6.l)
	EORI.l #$12345678,127(A7,A7.l)

; EORI.l, (xxx).w - Mode 111/000 (Absolute Short)
	EORI.l #$12345678,($1234).w

; EORI.l, (xxx).l - Mode 111/001 (Absolute Long)
	EORI.l #$12345678,($12345678).l

; EORI.l, d16(PC) - Mode 111/010 (Program Counter with displacement)
; EORI.l, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; EORI.l, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "EORI to CCR/SR"

; =========================================================
; EORI(.b) to CCR
; =========================================================

; EORI #xxx, CCR - Mode 111/100
	EORI #$12,CCR

; =========================================================
; EORI(.w) to SR
; =========================================================

; EORI #xxx, SR - Mode 111/100
	EORI #$2345,SR
