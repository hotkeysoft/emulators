	mc68000
	opt o-,w-

; =========================================================
; BCLR (bit number dynamic)
; =========================================================

; BCLR.l, Dn - Mode 000 (Data register, long only)
	BCLR.l D0,D0
	BCLR.l D1,D1
	BCLR.l D2,D2
	BCLR.l D3,D3
	BCLR.l D4,D4
	BCLR.l D5,D5
	BCLR.l D6,D6
	BCLR.l D7,D7

; BCLR.b, An - Mode 001 (Address)
;	Not supported

; BCLR.b, (An) - Mode 010
	BCLR.b D0,(A0)
	BCLR.b D1,(A1)
	BCLR.b D2,(A2)
	BCLR.b D3,(A3)
	BCLR.b D4,(A4)
	BCLR.b D5,(A5)
	BCLR.b D6,(A6)
	BCLR.b D7,(A7)
	
; BCLR.b (An)+ - Mode 011 (Address with postincrement)
	BCLR.b D0,(A0)+
	BCLR.b D1,(A1)+
	BCLR.b D2,(A2)+
	BCLR.b D3,(A3)+
	BCLR.b D4,(A4)+
	BCLR.b D5,(A5)+
	BCLR.b D6,(A6)+
	BCLR.b D7,(A7)+

; BCLR.b -(An) - Mode 100 (Address with predecrement)
	BCLR.b D0,-(A0)
	BCLR.b D1,-(A1)
	BCLR.b D2,-(A2)
	BCLR.b D3,-(A3)
	BCLR.b D4,-(A4)
	BCLR.b D5,-(A5)
	BCLR.b D6,-(A6)
	BCLR.b D7,-(A7)

; BCLR.b, d16(An) - Mode 101 (Address with displacement)
	BCLR.b D0,$5678(A0)
	BCLR.b D1,$5678(A1)
	BCLR.b D2,$5678(A2)
	BCLR.b D3,$5678(A3)
	BCLR.b D4,$5678(A4)
	BCLR.b D5,$5678(A5)
	BCLR.b D6,$5678(A6)
	BCLR.b D7,$5678(A7)

; BCLR.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	BCLR.b D0,$56(A0,D0.w)
	BCLR.b D1,$56(A1,D1.w)
	BCLR.b D2,$56(A2,D2.w)
	BCLR.b D3,$56(A3,D3.w)
	BCLR.b D4,$56(A4,D4.w)
	BCLR.b D5,$56(A5,D5.w)
	BCLR.b D6,$56(A6,D6.w)
	BCLR.b D7,$56(A7,D7.w)

	BCLR.b D0,$56(A0,A0.w)
	BCLR.b D1,$56(A1,A1.w)
	BCLR.b D2,$56(A2,A2.w)
	BCLR.b D3,$56(A3,A3.w)
	BCLR.b D4,$56(A4,A4.w)
	BCLR.b D5,$56(A5,A5.w)
	BCLR.b D6,$56(A6,A6.w)
	BCLR.b D7,$56(A7,A7.w)

	BCLR.b D0,$56(A0,D0.l)
	BCLR.b D1,$56(A1,D1.l)
	BCLR.b D2,$56(A2,D2.l)
	BCLR.b D3,$56(A3,D3.l)
	BCLR.b D4,$56(A4,D4.l)
	BCLR.b D5,$56(A5,D5.l)
	BCLR.b D6,$56(A6,D6.l)
	BCLR.b D7,$56(A7,D7.l)

	BCLR.b D0,$56(A0,A0.l)
	BCLR.b D1,$56(A1,A1.l)
	BCLR.b D2,$56(A2,A2.l)
	BCLR.b D3,$56(A3,A3.l)
	BCLR.b D4,$56(A4,A4.l)
	BCLR.b D5,$56(A5,A5.l)
	BCLR.b D6,$56(A6,A6.l)
	BCLR.b D7,$56(A7,A7.l)

; BCLR.b, (xxx).w - Mode 111/000 (Absolute Short)
	BCLR.b D0,($1234).w

; BCLR.b, (xxx).l - Mode 111/001 (Absolute Long)
	BCLR.b D0,($12345678).l

; BCLR.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; BCLR.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; BCLR.b, Imm - Mode 111/100 (Immediate)
	; Not supported
