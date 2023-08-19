	mc68000
	opt o-
	
; =========================================================
; ORI.b
; =========================================================

; ORI.b, Dn - Mode 000 (Data register)
	ORI.b #$12,D0
	ORI.b #$12,D1
	ORI.b #$12,D2
	ORI.b #$12,D3
	ORI.b #$12,D4
	ORI.b #$12,D5
	ORI.b #$12,D6
	ORI.b #$12,D7

; ORI.b, An - Mode 001 (Address)
;	Not supported

; ORI.b, (An) - Mode 010
	ORI.b #$12,(A0)
	ORI.b #$12,(A1)
	ORI.b #$12,(A2)
	ORI.b #$12,(A3)
	ORI.b #$12,(A4)
	ORI.b #$12,(A5)
	ORI.b #$12,(A6)
	ORI.b #$12,(A7)
	
; ORI.b (An)+ - Mode 011 (Address with postincrement)
	ORI.b #$12,(A0)+
	ORI.b #$12,(A1)+
	ORI.b #$12,(A2)+
	ORI.b #$12,(A3)+
	ORI.b #$12,(A4)+
	ORI.b #$12,(A5)+
	ORI.b #$12,(A6)+
	ORI.b #$12,(A7)+

; ORI.b -(An) - Mode 100 (Address with predecrement)
	ORI.b #$12,-(A0)
	ORI.b #$12,-(A1)
	ORI.b #$12,-(A2)
	ORI.b #$12,-(A3)
	ORI.b #$12,-(A4)
	ORI.b #$12,-(A5)
	ORI.b #$12,-(A6)
	ORI.b #$12,-(A7)

; ORI.b, d16(An) - Mode 101 (Address with displacement)
	ORI.b #$12,$5678(A0)
	ORI.b #$12,$5678(A1)
	ORI.b #$12,$5678(A2)
	ORI.b #$12,$5678(A3)
	ORI.b #$12,$5678(A4)
	ORI.b #$12,$5678(A5)
	ORI.b #$12,$5678(A6)
	ORI.b #$12,$5678(A7)

; ORI.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ORI.b #$12,$56(A0,D0.w)
	ORI.b #$12,$56(A1,D1.w)
	ORI.b #$12,$56(A2,D2.w)
	ORI.b #$12,$56(A3,D3.w)
	ORI.b #$12,$56(A4,D4.w)
	ORI.b #$12,$56(A5,D5.w)
	ORI.b #$12,$56(A6,D6.w)
	ORI.b #$12,$56(A7,D7.w)

	ORI.b #$12,$56(A0,A0.w)
	ORI.b #$12,$56(A1,A1.w)
	ORI.b #$12,$56(A2,A2.w)
	ORI.b #$12,$56(A3,A3.w)
	ORI.b #$12,$56(A4,A4.w)
	ORI.b #$12,$56(A5,A5.w)
	ORI.b #$12,$56(A6,A6.w)
	ORI.b #$12,$56(A7,A7.w)

	ORI.b #$12,$56(A0,D0.l)
	ORI.b #$12,$56(A1,D1.l)
	ORI.b #$12,$56(A2,D2.l)
	ORI.b #$12,$56(A3,D3.l)
	ORI.b #$12,$56(A4,D4.l)
	ORI.b #$12,$56(A5,D5.l)
	ORI.b #$12,$56(A6,D6.l)
	ORI.b #$12,$56(A7,D7.l)

	ORI.b #$12,$56(A0,A0.l)
	ORI.b #$12,$56(A1,A1.l)
	ORI.b #$12,$56(A2,A2.l)
	ORI.b #$12,$56(A3,A3.l)
	ORI.b #$12,$56(A4,A4.l)
	ORI.b #$12,$56(A5,A5.l)
	ORI.b #$12,$56(A6,A6.l)
	ORI.b #$12,$56(A7,A7.l)

; ORI.b, (xxx).w - Mode 111/000 (Absolute Short)
	ORI.b #$12,($1234).w

; ORI.b, (xxx).l - Mode 111/001 (Absolute Long)
	ORI.b #$12,($12345678).l

; ORI.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ORI.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; ORI.b, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "ORI to CCR/SR"

; =========================================================
; ORI.w
; =========================================================

; ORI.w, Dn - Mode 000 (Data register)
	ORI.w #$1234,D0
	ORI.w #$1234,D1
	ORI.w #$1234,D2
	ORI.w #$1234,D3
	ORI.w #$1234,D4
	ORI.w #$1234,D5
	ORI.w #$1234,D6
	ORI.w #$1234,D7

; ORI.w, An - Mode 001 (Address)
;	Not supported

; ORI.w, (An) - Mode 010
	ORI.w #$1234,(A0)
	ORI.w #$1234,(A1)
	ORI.w #$1234,(A2)
	ORI.w #$1234,(A3)
	ORI.w #$1234,(A4)
	ORI.w #$1234,(A5)
	ORI.w #$1234,(A6)
	ORI.w #$1234,(A7)
	
; ORI.w (An)+ - Mode 011 (Address with postincrement)
	ORI.w #$1234,(A0)+
	ORI.w #$1234,(A1)+
	ORI.w #$1234,(A2)+
	ORI.w #$1234,(A3)+
	ORI.w #$1234,(A4)+
	ORI.w #$1234,(A5)+
	ORI.w #$1234,(A6)+
	ORI.w #$1234,(A7)+

; ORI.w -(An) - Mode 100 (Address with predecrement)
	ORI.w #$1234,-(A0)
	ORI.w #$1234,-(A1)
	ORI.w #$1234,-(A2)
	ORI.w #$1234,-(A3)
	ORI.w #$1234,-(A4)
	ORI.w #$1234,-(A5)
	ORI.w #$1234,-(A6)
	ORI.w #$1234,-(A7)

; ORI.w, d16(An) - Mode 101 (Address with displacement)
	ORI.w #$1234,$5678(A0)
	ORI.w #$1234,$5678(A1)
	ORI.w #$1234,$5678(A2)
	ORI.w #$1234,$5678(A3)
	ORI.w #$1234,$5678(A4)
	ORI.w #$1234,$5678(A5)
	ORI.w #$1234,$5678(A6)
	ORI.w #$1234,$5678(A7)

; ORI.w, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ORI.w #$1234,$56(A0,D0.w)
	ORI.w #$1234,$56(A1,D1.w)
	ORI.w #$1234,$56(A2,D2.w)
	ORI.w #$1234,$56(A3,D3.w)
	ORI.w #$1234,$56(A4,D4.w)
	ORI.w #$1234,$56(A5,D5.w)
	ORI.w #$1234,$56(A6,D6.w)
	ORI.w #$1234,$56(A7,D7.w)

	ORI.w #$1234,$56(A0,A0.w)
	ORI.w #$1234,$56(A1,A1.w)
	ORI.w #$1234,$56(A2,A2.w)
	ORI.w #$1234,$56(A3,A3.w)
	ORI.w #$1234,$56(A4,A4.w)
	ORI.w #$1234,$56(A5,A5.w)
	ORI.w #$1234,$56(A6,A6.w)
	ORI.w #$1234,$56(A7,A7.w)

	ORI.w #$1234,$56(A0,D0.l)
	ORI.w #$1234,$56(A1,D1.l)
	ORI.w #$1234,$56(A2,D2.l)
	ORI.w #$1234,$56(A3,D3.l)
	ORI.w #$1234,$56(A4,D4.l)
	ORI.w #$1234,$56(A5,D5.l)
	ORI.w #$1234,$56(A6,D6.l)
	ORI.w #$1234,$56(A7,D7.l)

	ORI.w #$1234,$56(A0,A0.l)
	ORI.w #$1234,$56(A1,A1.l)
	ORI.w #$1234,$56(A2,A2.l)
	ORI.w #$1234,$56(A3,A3.l)
	ORI.w #$1234,$56(A4,A4.l)
	ORI.w #$1234,$56(A5,A5.l)
	ORI.w #$1234,$56(A6,A6.l)
	ORI.w #$1234,$56(A7,A7.l)

; ORI.w, (xxx).w - Mode 111/000 (Absolute Short)
	ORI.w #$1234,($1234).w

; ORI.w, (xxx).l - Mode 111/001 (Absolute Long)
	ORI.w #$1234,($12345678).l

; ORI.w, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ORI.w, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; ORI.w, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "ORI to CCR/SR"

; =========================================================
; ORI.l
; =========================================================

; ORI.l, Dn - Mode 000 (Data register)
	ORI.l #$12345678,D0
	ORI.l #$12345678,D1
	ORI.l #$12345678,D2
	ORI.l #$12345678,D3
	ORI.l #$12345678,D4
	ORI.l #$12345678,D5
	ORI.l #$12345678,D6
	ORI.l #$12345678,D7

; ORI.l, An - Mode 001 (Address)
;	Not supported

; ORI.l, (An) - Mode 010
	ORI.l #$12345678,(A0)
	ORI.l #$12345678,(A1)
	ORI.l #$12345678,(A2)
	ORI.l #$12345678,(A3)
	ORI.l #$12345678,(A4)
	ORI.l #$12345678,(A5)
	ORI.l #$12345678,(A6)
	ORI.l #$12345678,(A7)
	
; ORI.l (An)+ - Mode 011 (Address with postincrement)
	ORI.l #$12345678,(A0)+
	ORI.l #$12345678,(A1)+
	ORI.l #$12345678,(A2)+
	ORI.l #$12345678,(A3)+
	ORI.l #$12345678,(A4)+
	ORI.l #$12345678,(A5)+
	ORI.l #$12345678,(A6)+
	ORI.l #$12345678,(A7)+

; ORI.l -(An) - Mode 100 (Address with predecrement)
	ORI.l #$12345678,-(A0)
	ORI.l #$12345678,-(A1)
	ORI.l #$12345678,-(A2)
	ORI.l #$12345678,-(A3)
	ORI.l #$12345678,-(A4)
	ORI.l #$12345678,-(A5)
	ORI.l #$12345678,-(A6)
	ORI.l #$12345678,-(A7)

; ORI.l, d16(An) - Mode 101 (Address with displacement)
	ORI.l #$12345678,$5678(A0)
	ORI.l #$12345678,$5678(A1)
	ORI.l #$12345678,$5678(A2)
	ORI.l #$12345678,$5678(A3)
	ORI.l #$12345678,$5678(A4)
	ORI.l #$12345678,$5678(A5)
	ORI.l #$12345678,$5678(A6)
	ORI.l #$12345678,$5678(A7)

; ORI.l, d8(An, Xn.L|W) - Mode 110 (Address with index)
	ORI.l #$12345678,$56(A0,D0.w)
	ORI.l #$12345678,$56(A1,D1.w)
	ORI.l #$12345678,$56(A2,D2.w)
	ORI.l #$12345678,$56(A3,D3.w)
	ORI.l #$12345678,$56(A4,D4.w)
	ORI.l #$12345678,$56(A5,D5.w)
	ORI.l #$12345678,$56(A6,D6.w)
	ORI.l #$12345678,$56(A7,D7.w)

	ORI.l #$12345678,$56(A0,A0.w)
	ORI.l #$12345678,$56(A1,A1.w)
	ORI.l #$12345678,$56(A2,A2.w)
	ORI.l #$12345678,$56(A3,A3.w)
	ORI.l #$12345678,$56(A4,A4.w)
	ORI.l #$12345678,$56(A5,A5.w)
	ORI.l #$12345678,$56(A6,A6.w)
	ORI.l #$12345678,$56(A7,A7.w)

	ORI.l #$12345678,$56(A0,D0.l)
	ORI.l #$12345678,$56(A1,D1.l)
	ORI.l #$12345678,$56(A2,D2.l)
	ORI.l #$12345678,$56(A3,D3.l)
	ORI.l #$12345678,$56(A4,D4.l)
	ORI.l #$12345678,$56(A5,D5.l)
	ORI.l #$12345678,$56(A6,D6.l)
	ORI.l #$12345678,$56(A7,D7.l)

	ORI.l #$12345678,$56(A0,A0.l)
	ORI.l #$12345678,$56(A1,A1.l)
	ORI.l #$12345678,$56(A2,A2.l)
	ORI.l #$12345678,$56(A3,A3.l)
	ORI.l #$12345678,$56(A4,A4.l)
	ORI.l #$12345678,$56(A5,A5.l)
	ORI.l #$12345678,$56(A6,A6.l)
	ORI.l #$12345678,$56(A7,A7.l)

; ORI.l, (xxx).w - Mode 111/000 (Absolute Short)
	ORI.l #$12345678,($1234).w

; ORI.l, (xxx).l - Mode 111/001 (Absolute Long)
	ORI.l #$12345678,($12345678).l

; ORI.l, d16(PC) - Mode 111/010 (Program Counter with displacement)
; ORI.l, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	; Not supported

; ORI.l, Imm - Mode 111/100 (Immediate)
	; Not supported, used for "ORI to CCR/SR"

; =========================================================
; ORI(.b) to CCR
; =========================================================

; ORI #xxx, CCR - Mode 111/100
	ORI #$12,CCR

; =========================================================
; ORI(.w) to SR
; =========================================================

; ORI #xxx, SR - Mode 111/100
	ORI #$2345,SR
