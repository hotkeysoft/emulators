	mc68000
	opt o-,w-

; =========================================================
; BTST (bit number dynamic)
; =========================================================

; BTST.l, Dn - Mode 000 (Data register, long only)
	BTST.l D0,D0
	BTST.l D1,D1
	BTST.l D2,D2
	BTST.l D3,D3
	BTST.l D4,D4
	BTST.l D5,D5
	BTST.l D6,D6
	BTST.l D7,D7

; BTST.b, An - Mode 001 (Address)
;	Not supported

; BTST.b, (An) - Mode 010
	BTST.b D0,(A0)
	BTST.b D1,(A1)
	BTST.b D2,(A2)
	BTST.b D3,(A3)
	BTST.b D4,(A4)
	BTST.b D5,(A5)
	BTST.b D6,(A6)
	BTST.b D7,(A7)
	
; BTST.b (An)+ - Mode 011 (Address with postincrement)
	BTST.b D0,(A0)+
	BTST.b D1,(A1)+
	BTST.b D2,(A2)+
	BTST.b D3,(A3)+
	BTST.b D4,(A4)+
	BTST.b D5,(A5)+
	BTST.b D6,(A6)+
	BTST.b D7,(A7)+

; BTST.b -(An) - Mode 100 (Address with predecrement)
	BTST.b D0,-(A0)
	BTST.b D1,-(A1)
	BTST.b D2,-(A2)
	BTST.b D3,-(A3)
	BTST.b D4,-(A4)
	BTST.b D5,-(A5)
	BTST.b D6,-(A6)
	BTST.b D7,-(A7)

; BTST.b, d16(An) - Mode 101 (Address with displacement)
	BTST.b D0,$5678(A0)
	BTST.b D1,$5678(A1)
	BTST.b D2,$5678(A2)
	BTST.b D3,$5678(A3)
	BTST.b D4,$5678(A4)
	BTST.b D5,$5678(A5)
	BTST.b D6,$5678(A6)
	BTST.b D7,$5678(A7)

; BTST.b, d8(An, Xn.L|W) - Mode 110 (Address with index)
	BTST.b D0,$56(A0,D0.w)
	BTST.b D1,$56(A1,D1.w)
	BTST.b D2,$56(A2,D2.w)
	BTST.b D3,$56(A3,D3.w)
	BTST.b D4,$56(A4,D4.w)
	BTST.b D5,$56(A5,D5.w)
	BTST.b D6,$56(A6,D6.w)
	BTST.b D7,$56(A7,D7.w)

	BTST.b D0,$56(A0,A0.w)
	BTST.b D1,$56(A1,A1.w)
	BTST.b D2,$56(A2,A2.w)
	BTST.b D3,$56(A3,A3.w)
	BTST.b D4,$56(A4,A4.w)
	BTST.b D5,$56(A5,A5.w)
	BTST.b D6,$56(A6,A6.w)
	BTST.b D7,$56(A7,A7.w)

	BTST.b D0,$56(A0,D0.l)
	BTST.b D1,$56(A1,D1.l)
	BTST.b D2,$56(A2,D2.l)
	BTST.b D3,$56(A3,D3.l)
	BTST.b D4,$56(A4,D4.l)
	BTST.b D5,$56(A5,D5.l)
	BTST.b D6,$56(A6,D6.l)
	BTST.b D7,$56(A7,D7.l)

	BTST.b D0,$56(A0,A0.l)
	BTST.b D1,$56(A1,A1.l)
	BTST.b D2,$56(A2,A2.l)
	BTST.b D3,$56(A3,A3.l)
	BTST.b D4,$56(A4,A4.l)
	BTST.b D5,$56(A5,A5.l)
	BTST.b D6,$56(A6,A6.l)
	BTST.b D7,$56(A7,A7.l)

; BTST.b, (xxx).w - Mode 111/000 (Absolute Short)
	BTST.b D0,($1234).w

; BTST.b, (xxx).l - Mode 111/001 (Absolute Long)
	BTST.b D0,($12345678).l

; BTST.b, d16(PC) - Mode 111/010 (Program Counter with displacement)
	BTST.b D0,$5678(PC)

; BTST.b, d8(PC, Xn) - Mode 111/011 (Program Counter with displacement)
	BTST.b D0,$56(PC,D0)
	BTST.b D1,$56(PC,D1)
	BTST.b D2,$56(PC,D2)
	BTST.b D3,$56(PC,D3)
	BTST.b D4,$56(PC,D4)
	BTST.b D5,$56(PC,D5)
	BTST.b D6,$56(PC,D6)
	BTST.b D7,$56(PC,D7)
	
; BTST.b, Imm - Mode 111/100 (Immediate)
	BTST.b D0,$12345678
	BTST.b D1,$12345678
	BTST.b D2,$12345678
	BTST.b D3,$12345678
	BTST.b D4,$12345678
	BTST.b D5,$12345678
	BTST.b D6,$12345678
	BTST.b D7,$12345678
