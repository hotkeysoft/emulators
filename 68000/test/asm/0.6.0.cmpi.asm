	mc68000
	opt o-
	
; =========================================================
; CMPI.b
; =========================================================

; CMPI.b, Dn - Mode 000 (Data register)
	CMPI.b #$12,D0
	CMPI.b #$12,D1
	CMPI.b #$12,D2
	CMPI.b #$12,D3
	CMPI.b #$12,D4
	CMPI.b #$12,D5
	CMPI.b #$12,D6
	CMPI.b #$12,D7

; CMPI.b, An - Mode 001 (Address)
;	Not supported

; CMPI.b, (An) - Mode 010
	CMPI.b #$12,(A0)
	CMPI.b #$12,(A1)
	CMPI.b #$12,(A2)
	CMPI.b #$12,(A3)
	CMPI.b #$12,(A4)
	CMPI.b #$12,(A5)
	CMPI.b #$12,(A6)
	CMPI.b #$12,(A7)
	
; CMPI.b (An)+ - Mode 011 (Address with postincrement)
	CMPI.b #$12,(A0)+
	CMPI.b #$12,(A1)+
	CMPI.b #$12,(A2)+
	CMPI.b #$12,(A3)+
	CMPI.b #$12,(A4)+
	CMPI.b #$12,(A5)+
	CMPI.b #$12,(A6)+
	CMPI.b #$12,(A7)+

; CMPI.b -(An) - Mode 100 (Address with predecrement)
	CMPI.b #$12,-(A0)
	CMPI.b #$12,-(A1)
	CMPI.b #$12,-(A2)
	CMPI.b #$12,-(A3)
	CMPI.b #$12,-(A4)
	CMPI.b #$12,-(A5)
	CMPI.b #$12,-(A6)
	CMPI.b #$12,-(A7)

; CMPI.b, d16(An) - Mode 101 (Address with displacement)
	CMPI.b #$12,$5678(A0)
	CMPI.b #$12,$5678(A1)
	CMPI.b #$12,$5678(A2)
	CMPI.b #$12,$5678(A3)
	CMPI.b #$12,$5678(A4)
	CMPI.b #$12,$5678(A5)
	CMPI.b #$12,$5678(A6)
	CMPI.b #$12,$5678(A7)

; CMPI.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	CMPI.b #$12,$56(A0,D0.w)
	CMPI.b #$12,$56(A1,D1.w)
	CMPI.b #$12,$56(A2,D2.w)
	CMPI.b #$12,$56(A3,D3.w)
	CMPI.b #$12,$56(A4,D4.w)
	CMPI.b #$12,$56(A5,D5.w)
	CMPI.b #$12,$56(A6,D6.w)
	CMPI.b #$12,$56(A7,D7.w)

	CMPI.b #$12,$56(A0,A0.w)
	CMPI.b #$12,$56(A1,A1.w)
	CMPI.b #$12,$56(A2,A2.w)
	CMPI.b #$12,$56(A3,A3.w)
	CMPI.b #$12,$56(A4,A4.w)
	CMPI.b #$12,$56(A5,A5.w)
	CMPI.b #$12,$56(A6,A6.w)
	CMPI.b #$12,$56(A7,A7.w)

	CMPI.b #$12,$56(A0,D0.l)
	CMPI.b #$12,$56(A1,D1.l)
	CMPI.b #$12,$56(A2,D2.l)
	CMPI.b #$12,$56(A3,D3.l)
	CMPI.b #$12,$56(A4,D4.l)
	CMPI.b #$12,$56(A5,D5.l)
	CMPI.b #$12,$56(A6,D6.l)
	CMPI.b #$12,$56(A7,D7.l)

	CMPI.b #$12,$56(A0,A0.l)
	CMPI.b #$12,$56(A1,A1.l)
	CMPI.b #$12,$56(A2,A2.l)
	CMPI.b #$12,$56(A3,A3.l)
	CMPI.b #$12,$56(A4,A4.l)
	CMPI.b #$12,$56(A5,A5.l)
	CMPI.b #$12,$56(A6,A6.l)
	CMPI.b #$12,$56(A7,A7.l)

; CMPI.b, (xxx).w - Mode 111/000 (Absolute Short)
	CMPI.b #$12,($1234).w

; CMPI.b, (xxx).l - Mode 111/001 (Absolute Long)
	CMPI.b #$12,($12345678).l

; CMPI.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; CMPI.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; CMPI.b, Imm - Mode 111/100 (Immediate)

	; Not supported

; =========================================================
; CMPI.w
; =========================================================

; CMPI.w, Dn - Mode 000 (Data register)
	CMPI.w #$1234,D0
	CMPI.w #$1234,D1
	CMPI.w #$1234,D2
	CMPI.w #$1234,D3
	CMPI.w #$1234,D4
	CMPI.w #$1234,D5
	CMPI.w #$1234,D6
	CMPI.w #$1234,D7

; CMPI.w, An - Mode 001 (Address)
;	Not supported

; CMPI.w, (An) - Mode 010
	CMPI.w #$1234,(A0)
	CMPI.w #$1234,(A1)
	CMPI.w #$1234,(A2)
	CMPI.w #$1234,(A3)
	CMPI.w #$1234,(A4)
	CMPI.w #$1234,(A5)
	CMPI.w #$1234,(A6)
	CMPI.w #$1234,(A7)
	
; CMPI.w (An)+ - Mode 011 (Address with postincrement)
	CMPI.w #$1234,(A0)+
	CMPI.w #$1234,(A1)+
	CMPI.w #$1234,(A2)+
	CMPI.w #$1234,(A3)+
	CMPI.w #$1234,(A4)+
	CMPI.w #$1234,(A5)+
	CMPI.w #$1234,(A6)+
	CMPI.w #$1234,(A7)+

; CMPI.w -(An) - Mode 100 (Address with predecrement)
	CMPI.w #$1234,-(A0)
	CMPI.w #$1234,-(A1)
	CMPI.w #$1234,-(A2)
	CMPI.w #$1234,-(A3)
	CMPI.w #$1234,-(A4)
	CMPI.w #$1234,-(A5)
	CMPI.w #$1234,-(A6)
	CMPI.w #$1234,-(A7)

; CMPI.w, d16(An) - Mode 101 (Address with displacement)
	CMPI.w #$1234,$5678(A0)
	CMPI.w #$1234,$5678(A1)
	CMPI.w #$1234,$5678(A2)
	CMPI.w #$1234,$5678(A3)
	CMPI.w #$1234,$5678(A4)
	CMPI.w #$1234,$5678(A5)
	CMPI.w #$1234,$5678(A6)
	CMPI.w #$1234,$5678(A7)

; CMPI.w, d8(An, Xn.L|W) - Mode 110 (Address with index)
	CMPI.w #$1234,$56(A0,D0.w)
	CMPI.w #$1234,$56(A1,D1.w)
	CMPI.w #$1234,$56(A2,D2.w)
	CMPI.w #$1234,$56(A3,D3.w)
	CMPI.w #$1234,$56(A4,D4.w)
	CMPI.w #$1234,$56(A5,D5.w)
	CMPI.w #$1234,$56(A6,D6.w)
	CMPI.w #$1234,$56(A7,D7.w)

	CMPI.w #$1234,$56(A0,A0.w)
	CMPI.w #$1234,$56(A1,A1.w)
	CMPI.w #$1234,$56(A2,A2.w)
	CMPI.w #$1234,$56(A3,A3.w)
	CMPI.w #$1234,$56(A4,A4.w)
	CMPI.w #$1234,$56(A5,A5.w)
	CMPI.w #$1234,$56(A6,A6.w)
	CMPI.w #$1234,$56(A7,A7.w)

	CMPI.w #$1234,$56(A0,D0.l)
	CMPI.w #$1234,$56(A1,D1.l)
	CMPI.w #$1234,$56(A2,D2.l)
	CMPI.w #$1234,$56(A3,D3.l)
	CMPI.w #$1234,$56(A4,D4.l)
	CMPI.w #$1234,$56(A5,D5.l)
	CMPI.w #$1234,$56(A6,D6.l)
	CMPI.w #$1234,$56(A7,D7.l)

	CMPI.w #$1234,$56(A0,A0.l)
	CMPI.w #$1234,$56(A1,A1.l)
	CMPI.w #$1234,$56(A2,A2.l)
	CMPI.w #$1234,$56(A3,A3.l)
	CMPI.w #$1234,$56(A4,A4.l)
	CMPI.w #$1234,$56(A5,A5.l)
	CMPI.w #$1234,$56(A6,A6.l)
	CMPI.w #$1234,$56(A7,A7.l)

; CMPI.w, (xxx).w - Mode 111/000 (Absolute Short)
	CMPI.w #$1234,($1234).w

; CMPI.w, (xxx).l - Mode 111/001 (Absolute Long)
	CMPI.w #$1234,($12345678).l

; CMPI.w, d16(PC) - Mode 111/010 (Program Counter with displacement)
; CMPI.w, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; CMPI.w, Imm - Mode 111/100 (Immediate)
	; Not supported

; =========================================================
; CMPI.l
; =========================================================

; CMPI.l, Dn - Mode 000 (Data register)
	CMPI.l #$12345678,D0
	CMPI.l #$12345678,D1
	CMPI.l #$12345678,D2
	CMPI.l #$12345678,D3
	CMPI.l #$12345678,D4
	CMPI.l #$12345678,D5
	CMPI.l #$12345678,D6
	CMPI.l #$12345678,D7

; CMPI.l, An - Mode 001 (Address)
;	Not supported

; CMPI.l, (An) - Mode 010
	CMPI.l #$12345678,(A0)
	CMPI.l #$12345678,(A1)
	CMPI.l #$12345678,(A2)
	CMPI.l #$12345678,(A3)
	CMPI.l #$12345678,(A4)
	CMPI.l #$12345678,(A5)
	CMPI.l #$12345678,(A6)
	CMPI.l #$12345678,(A7)
	
; CMPI.l (An)+ - Mode 011 (Address with postincrement)
	CMPI.l #$12345678,(A0)+
	CMPI.l #$12345678,(A1)+
	CMPI.l #$12345678,(A2)+
	CMPI.l #$12345678,(A3)+
	CMPI.l #$12345678,(A4)+
	CMPI.l #$12345678,(A5)+
	CMPI.l #$12345678,(A6)+
	CMPI.l #$12345678,(A7)+

; CMPI.l -(An) - Mode 100 (Address with predecrement)
	CMPI.l #$12345678,-(A0)
	CMPI.l #$12345678,-(A1)
	CMPI.l #$12345678,-(A2)
	CMPI.l #$12345678,-(A3)
	CMPI.l #$12345678,-(A4)
	CMPI.l #$12345678,-(A5)
	CMPI.l #$12345678,-(A6)
	CMPI.l #$12345678,-(A7)

; CMPI.l, d16(An) - Mode 101 (Address with displacement)
	CMPI.l #$12345678,$5678(A0)
	CMPI.l #$12345678,$5678(A1)
	CMPI.l #$12345678,$5678(A2)
	CMPI.l #$12345678,$5678(A3)
	CMPI.l #$12345678,$5678(A4)
	CMPI.l #$12345678,$5678(A5)
	CMPI.l #$12345678,$5678(A6)
	CMPI.l #$12345678,$5678(A7)

; CMPI.l, d8(An, Xn.L|W) - Mode 110 (Address with index)
	CMPI.l #$12345678,$56(A0,D0.w)
	CMPI.l #$12345678,$56(A1,D1.w)
	CMPI.l #$12345678,$56(A2,D2.w)
	CMPI.l #$12345678,$56(A3,D3.w)
	CMPI.l #$12345678,$56(A4,D4.w)
	CMPI.l #$12345678,$56(A5,D5.w)
	CMPI.l #$12345678,$56(A6,D6.w)
	CMPI.l #$12345678,$56(A7,D7.w)

	CMPI.l #$12345678,$56(A0,A0.w)
	CMPI.l #$12345678,$56(A1,A1.w)
	CMPI.l #$12345678,$56(A2,A2.w)
	CMPI.l #$12345678,$56(A3,A3.w)
	CMPI.l #$12345678,$56(A4,A4.w)
	CMPI.l #$12345678,$56(A5,A5.w)
	CMPI.l #$12345678,$56(A6,A6.w)
	CMPI.l #$12345678,$56(A7,A7.w)

	CMPI.l #$12345678,$56(A0,D0.l)
	CMPI.l #$12345678,$56(A1,D1.l)
	CMPI.l #$12345678,$56(A2,D2.l)
	CMPI.l #$12345678,$56(A3,D3.l)
	CMPI.l #$12345678,$56(A4,D4.l)
	CMPI.l #$12345678,$56(A5,D5.l)
	CMPI.l #$12345678,$56(A6,D6.l)
	CMPI.l #$12345678,$56(A7,D7.l)

	CMPI.l #$12345678,$56(A0,A0.l)
	CMPI.l #$12345678,$56(A1,A1.l)
	CMPI.l #$12345678,$56(A2,A2.l)
	CMPI.l #$12345678,$56(A3,A3.l)
	CMPI.l #$12345678,$56(A4,A4.l)
	CMPI.l #$12345678,$56(A5,A5.l)
	CMPI.l #$12345678,$56(A6,A6.l)
	CMPI.l #$12345678,$56(A7,A7.l)

; CMPI.l, (xxx).w - Mode 111/000 (Absolute Short)
	CMPI.l #$12345678,($1234).w

; CMPI.l, (xxx).l - Mode 111/001 (Absolute Long)
	CMPI.l #$12345678,($12345678).l

; CMPI.l, d16(PC) - Mode 111/010 (Program Counter with displacement)
; CMPI.l, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; CMPI.l, Imm - Mode 111/100 (Immediate)
	; Not supported
