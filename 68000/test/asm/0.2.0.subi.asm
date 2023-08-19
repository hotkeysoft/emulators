	mc68000
	opt o-
	
; =========================================================
; SUBI.b
; =========================================================

; SUBI.b, Dn - Mode 000 (Data register)
	SUBI.b #$12,D0
	SUBI.b #$12,D1
	SUBI.b #$12,D2
	SUBI.b #$12,D3
	SUBI.b #$12,D4
	SUBI.b #$12,D5
	SUBI.b #$12,D6
	SUBI.b #$12,D7

; SUBI.b, An - Mode 001 (Address)
;	Not supported

; SUBI.b, (An) - Mode 010
	SUBI.b #$12,(A0)
	SUBI.b #$12,(A1)
	SUBI.b #$12,(A2)
	SUBI.b #$12,(A3)
	SUBI.b #$12,(A4)
	SUBI.b #$12,(A5)
	SUBI.b #$12,(A6)
	SUBI.b #$12,(A7)
	
; SUBI.b (An)+ - Mode 011 (Address with postincrement)
	SUBI.b #$12,(A0)+
	SUBI.b #$12,(A1)+
	SUBI.b #$12,(A2)+
	SUBI.b #$12,(A3)+
	SUBI.b #$12,(A4)+
	SUBI.b #$12,(A5)+
	SUBI.b #$12,(A6)+
	SUBI.b #$12,(A7)+

; SUBI.b -(An) - Mode 100 (Address with predecrement)
	SUBI.b #$12,-(A0)
	SUBI.b #$12,-(A1)
	SUBI.b #$12,-(A2)
	SUBI.b #$12,-(A3)
	SUBI.b #$12,-(A4)
	SUBI.b #$12,-(A5)
	SUBI.b #$12,-(A6)
	SUBI.b #$12,-(A7)

; SUBI.b, d16(An) - Mode 101 (Address with displacement)
	SUBI.b #$12,$5678(A0)
	SUBI.b #$12,$5678(A1)
	SUBI.b #$12,$5678(A2)
	SUBI.b #$12,$5678(A3)
	SUBI.b #$12,$5678(A4)
	SUBI.b #$12,$5678(A5)
	SUBI.b #$12,$5678(A6)
	SUBI.b #$12,$5678(A7)

; SUBI.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	SUBI.b #$12,$56(A0,D0.w)
	SUBI.b #$12,$56(A1,D1.w)
	SUBI.b #$12,$56(A2,D2.w)
	SUBI.b #$12,$56(A3,D3.w)
	SUBI.b #$12,$56(A4,D4.w)
	SUBI.b #$12,$56(A5,D5.w)
	SUBI.b #$12,$56(A6,D6.w)
	SUBI.b #$12,$56(A7,D7.w)

	SUBI.b #$12,$56(A0,A0.w)
	SUBI.b #$12,$56(A1,A1.w)
	SUBI.b #$12,$56(A2,A2.w)
	SUBI.b #$12,$56(A3,A3.w)
	SUBI.b #$12,$56(A4,A4.w)
	SUBI.b #$12,$56(A5,A5.w)
	SUBI.b #$12,$56(A6,A6.w)
	SUBI.b #$12,$56(A7,A7.w)

	SUBI.b #$12,$56(A0,D0.l)
	SUBI.b #$12,$56(A1,D1.l)
	SUBI.b #$12,$56(A2,D2.l)
	SUBI.b #$12,$56(A3,D3.l)
	SUBI.b #$12,$56(A4,D4.l)
	SUBI.b #$12,$56(A5,D5.l)
	SUBI.b #$12,$56(A6,D6.l)
	SUBI.b #$12,$56(A7,D7.l)

	SUBI.b #$12,$56(A0,A0.l)
	SUBI.b #$12,$56(A1,A1.l)
	SUBI.b #$12,$56(A2,A2.l)
	SUBI.b #$12,$56(A3,A3.l)
	SUBI.b #$12,$56(A4,A4.l)
	SUBI.b #$12,$56(A5,A5.l)
	SUBI.b #$12,$56(A6,A6.l)
	SUBI.b #$12,$56(A7,A7.l)

; SUBI.b, (xxx).w - Mode 111/000 (Absolute Short)
	SUBI.b #$12,($1234).w

; SUBI.b, (xxx).l - Mode 111/001 (Absolute Long)
	SUBI.b #$12,($12345678).l

; SUBI.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; SUBI.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; SUBI.b, Imm - Mode 111/100 (Immediate)

	; Not supported

; =========================================================
; SUBI.w
; =========================================================

; SUBI.w, Dn - Mode 000 (Data register)
	SUBI.w #$1234,D0
	SUBI.w #$1234,D1
	SUBI.w #$1234,D2
	SUBI.w #$1234,D3
	SUBI.w #$1234,D4
	SUBI.w #$1234,D5
	SUBI.w #$1234,D6
	SUBI.w #$1234,D7

; SUBI.w, An - Mode 001 (Address)
;	Not supported

; SUBI.w, (An) - Mode 010
	SUBI.w #$1234,(A0)
	SUBI.w #$1234,(A1)
	SUBI.w #$1234,(A2)
	SUBI.w #$1234,(A3)
	SUBI.w #$1234,(A4)
	SUBI.w #$1234,(A5)
	SUBI.w #$1234,(A6)
	SUBI.w #$1234,(A7)
	
; SUBI.w (An)+ - Mode 011 (Address with postincrement)
	SUBI.w #$1234,(A0)+
	SUBI.w #$1234,(A1)+
	SUBI.w #$1234,(A2)+
	SUBI.w #$1234,(A3)+
	SUBI.w #$1234,(A4)+
	SUBI.w #$1234,(A5)+
	SUBI.w #$1234,(A6)+
	SUBI.w #$1234,(A7)+

; SUBI.w -(An) - Mode 100 (Address with predecrement)
	SUBI.w #$1234,-(A0)
	SUBI.w #$1234,-(A1)
	SUBI.w #$1234,-(A2)
	SUBI.w #$1234,-(A3)
	SUBI.w #$1234,-(A4)
	SUBI.w #$1234,-(A5)
	SUBI.w #$1234,-(A6)
	SUBI.w #$1234,-(A7)

; SUBI.w, d16(An) - Mode 101 (Address with displacement)
	SUBI.w #$1234,$5678(A0)
	SUBI.w #$1234,$5678(A1)
	SUBI.w #$1234,$5678(A2)
	SUBI.w #$1234,$5678(A3)
	SUBI.w #$1234,$5678(A4)
	SUBI.w #$1234,$5678(A5)
	SUBI.w #$1234,$5678(A6)
	SUBI.w #$1234,$5678(A7)

; SUBI.w, d8(An, Xn.L|W) - Mode 110 (Address with index)
	SUBI.w #$1234,$56(A0,D0.w)
	SUBI.w #$1234,$56(A1,D1.w)
	SUBI.w #$1234,$56(A2,D2.w)
	SUBI.w #$1234,$56(A3,D3.w)
	SUBI.w #$1234,$56(A4,D4.w)
	SUBI.w #$1234,$56(A5,D5.w)
	SUBI.w #$1234,$56(A6,D6.w)
	SUBI.w #$1234,$56(A7,D7.w)

	SUBI.w #$1234,$56(A0,A0.w)
	SUBI.w #$1234,$56(A1,A1.w)
	SUBI.w #$1234,$56(A2,A2.w)
	SUBI.w #$1234,$56(A3,A3.w)
	SUBI.w #$1234,$56(A4,A4.w)
	SUBI.w #$1234,$56(A5,A5.w)
	SUBI.w #$1234,$56(A6,A6.w)
	SUBI.w #$1234,$56(A7,A7.w)

	SUBI.w #$1234,$56(A0,D0.l)
	SUBI.w #$1234,$56(A1,D1.l)
	SUBI.w #$1234,$56(A2,D2.l)
	SUBI.w #$1234,$56(A3,D3.l)
	SUBI.w #$1234,$56(A4,D4.l)
	SUBI.w #$1234,$56(A5,D5.l)
	SUBI.w #$1234,$56(A6,D6.l)
	SUBI.w #$1234,$56(A7,D7.l)

	SUBI.w #$1234,$56(A0,A0.l)
	SUBI.w #$1234,$56(A1,A1.l)
	SUBI.w #$1234,$56(A2,A2.l)
	SUBI.w #$1234,$56(A3,A3.l)
	SUBI.w #$1234,$56(A4,A4.l)
	SUBI.w #$1234,$56(A5,A5.l)
	SUBI.w #$1234,$56(A6,A6.l)
	SUBI.w #$1234,$56(A7,A7.l)

; SUBI.w, (xxx).w - Mode 111/000 (Absolute Short)
	SUBI.w #$1234,($1234).w

; SUBI.w, (xxx).l - Mode 111/001 (Absolute Long)
	SUBI.w #$1234,($12345678).l

; SUBI.w, d16(PC) - Mode 111/010 (Program Counter with displacement)
; SUBI.w, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; SUBI.w, Imm - Mode 111/100 (Immediate)
	; Not supported

; =========================================================
; SUBI.l
; =========================================================

; SUBI.l, Dn - Mode 000 (Data register)
	SUBI.l #$12345678,D0
	SUBI.l #$12345678,D1
	SUBI.l #$12345678,D2
	SUBI.l #$12345678,D3
	SUBI.l #$12345678,D4
	SUBI.l #$12345678,D5
	SUBI.l #$12345678,D6
	SUBI.l #$12345678,D7

; SUBI.l, An - Mode 001 (Address)
;	Not supported

; SUBI.l, (An) - Mode 010
	SUBI.l #$12345678,(A0)
	SUBI.l #$12345678,(A1)
	SUBI.l #$12345678,(A2)
	SUBI.l #$12345678,(A3)
	SUBI.l #$12345678,(A4)
	SUBI.l #$12345678,(A5)
	SUBI.l #$12345678,(A6)
	SUBI.l #$12345678,(A7)
	
; SUBI.l (An)+ - Mode 011 (Address with postincrement)
	SUBI.l #$12345678,(A0)+
	SUBI.l #$12345678,(A1)+
	SUBI.l #$12345678,(A2)+
	SUBI.l #$12345678,(A3)+
	SUBI.l #$12345678,(A4)+
	SUBI.l #$12345678,(A5)+
	SUBI.l #$12345678,(A6)+
	SUBI.l #$12345678,(A7)+

; SUBI.l -(An) - Mode 100 (Address with predecrement)
	SUBI.l #$12345678,-(A0)
	SUBI.l #$12345678,-(A1)
	SUBI.l #$12345678,-(A2)
	SUBI.l #$12345678,-(A3)
	SUBI.l #$12345678,-(A4)
	SUBI.l #$12345678,-(A5)
	SUBI.l #$12345678,-(A6)
	SUBI.l #$12345678,-(A7)

; SUBI.l, d16(An) - Mode 101 (Address with displacement)
	SUBI.l #$12345678,$5678(A0)
	SUBI.l #$12345678,$5678(A1)
	SUBI.l #$12345678,$5678(A2)
	SUBI.l #$12345678,$5678(A3)
	SUBI.l #$12345678,$5678(A4)
	SUBI.l #$12345678,$5678(A5)
	SUBI.l #$12345678,$5678(A6)
	SUBI.l #$12345678,$5678(A7)

; SUBI.l, d8(An, Xn.L|W) - Mode 110 (Address with index)
	SUBI.l #$12345678,$56(A0,D0.w)
	SUBI.l #$12345678,$56(A1,D1.w)
	SUBI.l #$12345678,$56(A2,D2.w)
	SUBI.l #$12345678,$56(A3,D3.w)
	SUBI.l #$12345678,$56(A4,D4.w)
	SUBI.l #$12345678,$56(A5,D5.w)
	SUBI.l #$12345678,$56(A6,D6.w)
	SUBI.l #$12345678,$56(A7,D7.w)

	SUBI.l #$12345678,$56(A0,A0.w)
	SUBI.l #$12345678,$56(A1,A1.w)
	SUBI.l #$12345678,$56(A2,A2.w)
	SUBI.l #$12345678,$56(A3,A3.w)
	SUBI.l #$12345678,$56(A4,A4.w)
	SUBI.l #$12345678,$56(A5,A5.w)
	SUBI.l #$12345678,$56(A6,A6.w)
	SUBI.l #$12345678,$56(A7,A7.w)

	SUBI.l #$12345678,$56(A0,D0.l)
	SUBI.l #$12345678,$56(A1,D1.l)
	SUBI.l #$12345678,$56(A2,D2.l)
	SUBI.l #$12345678,$56(A3,D3.l)
	SUBI.l #$12345678,$56(A4,D4.l)
	SUBI.l #$12345678,$56(A5,D5.l)
	SUBI.l #$12345678,$56(A6,D6.l)
	SUBI.l #$12345678,$56(A7,D7.l)

	SUBI.l #$12345678,$56(A0,A0.l)
	SUBI.l #$12345678,$56(A1,A1.l)
	SUBI.l #$12345678,$56(A2,A2.l)
	SUBI.l #$12345678,$56(A3,A3.l)
	SUBI.l #$12345678,$56(A4,A4.l)
	SUBI.l #$12345678,$56(A5,A5.l)
	SUBI.l #$12345678,$56(A6,A6.l)
	SUBI.l #$12345678,$56(A7,A7.l)

; SUBI.l, (xxx).w - Mode 111/000 (Absolute Short)
	SUBI.l #$12345678,($1234).w

; SUBI.l, (xxx).l - Mode 111/001 (Absolute Long)
	SUBI.l #$12345678,($12345678).l

; SUBI.l, d16(PC) - Mode 111/010 (Program Counter with displacement)
; SUBI.l, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; SUBI.l, Imm - Mode 111/100 (Immediate)
	; Not supported
