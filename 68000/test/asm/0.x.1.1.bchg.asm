	mc68000
	opt o-,w-

; =========================================================
; BCHG (bit number dynamic)
; =========================================================

; BCHG.l, Dn - Mode 000 (Data register, long only)
	BCHG.l D0,D0
	BCHG.l D1,D1
	BCHG.l D2,D2
	BCHG.l D3,D3
	BCHG.l D4,D4
	BCHG.l D5,D5
	BCHG.l D6,D6
	BCHG.l D7,D7

; BCHG.b, An - Mode 001 (Address)
;	Not supported

; BCHG.b, (An) - Mode 010
	BCHG.b D0,(A0)
	BCHG.b D1,(A1)
	BCHG.b D2,(A2)
	BCHG.b D3,(A3)
	BCHG.b D4,(A4)
	BCHG.b D5,(A5)
	BCHG.b D6,(A6)
	BCHG.b D7,(A7)
	
; BCHG.b (An)+ - Mode 011 (Address with postincrement)
	BCHG.b D0,(A0)+
	BCHG.b D1,(A1)+
	BCHG.b D2,(A2)+
	BCHG.b D3,(A3)+
	BCHG.b D4,(A4)+
	BCHG.b D5,(A5)+
	BCHG.b D6,(A6)+
	BCHG.b D7,(A7)+

; BCHG.b -(An) - Mode 100 (Address with predecrement)
	BCHG.b D0,-(A0)
	BCHG.b D1,-(A1)
	BCHG.b D2,-(A2)
	BCHG.b D3,-(A3)
	BCHG.b D4,-(A4)
	BCHG.b D5,-(A5)
	BCHG.b D6,-(A6)
	BCHG.b D7,-(A7)

; BCHG.b, d16(An) - Mode 101 (Address with displacement)
	BCHG.b D0,$5678(A0)
	BCHG.b D1,$5678(A1)
	BCHG.b D2,$5678(A2)
	BCHG.b D3,$5678(A3)
	BCHG.b D4,$5678(A4)
	BCHG.b D5,$5678(A5)
	BCHG.b D6,$5678(A6)
	BCHG.b D7,$5678(A7)

; BCHG.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	BCHG.b D0,$56(A0,D0.w)
	BCHG.b D1,$56(A1,D1.w)
	BCHG.b D2,$56(A2,D2.w)
	BCHG.b D3,$56(A3,D3.w)
	BCHG.b D4,$56(A4,D4.w)
	BCHG.b D5,$56(A5,D5.w)
	BCHG.b D6,$56(A6,D6.w)
	BCHG.b D7,$56(A7,D7.w)

	BCHG.b D0,$56(A0,A0.w)
	BCHG.b D1,$56(A1,A1.w)
	BCHG.b D2,$56(A2,A2.w)
	BCHG.b D3,$56(A3,A3.w)
	BCHG.b D4,$56(A4,A4.w)
	BCHG.b D5,$56(A5,A5.w)
	BCHG.b D6,$56(A6,A6.w)
	BCHG.b D7,$56(A7,A7.w)

	BCHG.b D0,$56(A0,D0.l)
	BCHG.b D1,$56(A1,D1.l)
	BCHG.b D2,$56(A2,D2.l)
	BCHG.b D3,$56(A3,D3.l)
	BCHG.b D4,$56(A4,D4.l)
	BCHG.b D5,$56(A5,D5.l)
	BCHG.b D6,$56(A6,D6.l)
	BCHG.b D7,$56(A7,D7.l)

	BCHG.b D0,$56(A0,A0.l)
	BCHG.b D1,$56(A1,A1.l)
	BCHG.b D2,$56(A2,A2.l)
	BCHG.b D3,$56(A3,A3.l)
	BCHG.b D4,$56(A4,A4.l)
	BCHG.b D5,$56(A5,A5.l)
	BCHG.b D6,$56(A6,A6.l)
	BCHG.b D7,$56(A7,A7.l)

; BCHG.b, (xxx).w - Mode 111/000 (Absolute Short)
	BCHG.b D0,($1234).w

; BCHG.b, (xxx).l - Mode 111/001 (Absolute Long)
	BCHG.b D0,($12345678).l

; BCHG.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
; BCHG.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
; BCHG.b, Imm - Mode 111/100 (Immediate)
	; Not supported
