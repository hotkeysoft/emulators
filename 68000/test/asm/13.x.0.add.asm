	mc68000
	opt o-

; =========================================================
; ADD.b <ea>,Dn
; =========================================================

; ADD.b, Dn,Dn
	ADD.b D7,D0
	ADD.b D6,D1
	ADD.b D5,D2
	ADD.b D4,D3
	ADD.b D3,D4
	ADD.b D2,D5
	ADD.b D1,D6
	ADD.b D0,D7

; ADD.b, An,(not allowed)

; ADD.b, (An),Dn,
	ADD.b (A5),D0

; ADD.b, (An)+,Dn,
	ADD.b (A6)+,D1

; ADD.b, -(An),Dn
	ADD.b -(A7),D2

; ADD.b, d16(An),Dn
	ADD.b -32768(A2),D3
	ADD.b 0(A3),D4
	ADD.b 1234(A4),D5
	ADD.b 32767(A4),D6

; ADD.b, d8(An, Xn.L|W),Dn
	ADD.b 123(A2,A3.w),D7
	ADD.b -1(A3,D2.l),D0
	ADD.b -128(A4,D3.w),D1

; ADD.b, (xxx).w,Dn
	ADD.b ($0000).w,D2
	ADD.b ($1234).w,D3
	ADD.b ($7FFF).w,D4

; ADD.b, (xxx).l,Dn
	ADD.b ($00000000).l,D5
	ADD.b ($12345678).l,D6
	ADD.b ($FFFFFFFF).l,D7

; ADD.b, d16(PC),Dn
	ADD.b -32768(PC),D0
	ADD.b 0(PC),D1
	ADD.b 1234(PC),D2
	ADD.b 32767(PC),D3

; ADD.b, d8(PC, Xn),Dn
	ADD.b 123(PC,A3.w),D4
	ADD.b -1(PC,D2.l),D5
	ADD.b -128(PC,D3.w),D6

; ADD.b, Dn,Imm
	ADD.b #$00,D0
	ADD.b #$12,D1
	ADD.b #$AB,D2
	ADD.b #$80,D3
	ADD.b #$F0,D4
	ADD.b #$AA,D5
	ADD.b #$0F,D6
	ADD.b #$FF,D7

; =========================================================
; ADD.w <ea>, Dn
; =========================================================

; ADD.w, Dn,Dn
	ADD.w D7,D0
	ADD.w D6,D1
	ADD.w D5,D2
	ADD.w D4,D3
	ADD.w D3,D4
	ADD.w D2,D5
	ADD.w D1,D6
	ADD.w D0,D7

; ADD.w, An, Dn
	ADD.w A7,D0
	ADD.w A6,D1
	ADD.w A5,D2
	ADD.w A4,D3
	ADD.w A3,D4
	ADD.w A2,D5
	ADD.w A1,D6
	ADD.w A0,D7

; ADD.w, (An),Dn
	ADD.w (A5),D0

; ADD.w, (An)+,Dn
	ADD.w (A6)+,D1

; ADD.w, -(An),Dn
	ADD.w -(A7),D2

; ADD.w, d16(An),Dn
	ADD.w -32768(A2),D3
	ADD.w 0(A3),D4
	ADD.w 1234(A4),D5
	ADD.w 32767(A4),D6

; ADD.w, d8(An, Xn.L|W),Dn
	ADD.w 123(A2,A3.w),D7
	ADD.w -1(A3,D2.l),D0
	ADD.w -128(A4,D3.w),D1

; ADD.w, (xxx).w,Dn
	ADD.w ($0000).w,D2
	ADD.w ($1234).w,D3
	ADD.w ($7FFF).w,D4

; ADD.w, (xxx).l,Dn
	ADD.w ($00000000).l,D5
	ADD.w ($12345678).l,D6
	ADD.w ($FFFFFFFF).l,D7

; ADD.w, d16(PC),Dn
	ADD.w -32768(PC),D0
	ADD.w 0(PC),D1
	ADD.w 1234(PC),D2
	ADD.w 32767(PC),D3

; ADD.w, d8(PC, Xn),Dn
	ADD.w 123(PC,A3.w),D4
	ADD.w -1(PC,D2.l),D5
	ADD.w -128(PC,D3.w),D6

; ADD.w, Imm,Dn
	ADD.w #$0000,D0
	ADD.w #$1234,D1
	ADD.w #$ABCD,D2
	ADD.w #$8000,D3
	ADD.w #$F0F0,D4
	ADD.w #$AA55,D5
	ADD.w #$0F0F,D6
	ADD.w #$FFFF,D7

; =========================================================
; ADD.l <ea>, Dn
; =========================================================

; ADD.l, Dn
	ADD.l D7,D0
	ADD.l D6,D1
	ADD.l D5,D2
	ADD.l D4,D3
	ADD.l D3,D4
	ADD.l D2,D5
	ADD.l D1,D6
	ADD.l D0,D7

; ADD.l, An,Dn
	ADD.l A7,D0
	ADD.l A6,D1
	ADD.l A5,D2
	ADD.l A4,D3
	ADD.l A3,D4
	ADD.l A2,D5
	ADD.l A1,D6
	ADD.l A0,D7

; ADD.l, (An),Dn
	ADD.l (A5),D0

; ADD.l, (An)+,Dn
	ADD.l (A6)+,D1

; ADD.l, -(An),Dn
	ADD.l -(A7),D2

; ADD.l, d16(An),Dn
	ADD.l -32768(A2),D3
	ADD.l 0(A3),D4
	ADD.l 1234(A4),D5
	ADD.l 32767(A4),D6

; ADD.l, d8(An, Xn.L|W),Dn
	ADD.l 123(A2,A3.w),D7
	ADD.l -1(A3,D2.l),D0
	ADD.l -128(A4,D3.w),D1

; ADD.l, (xxx).w,Dn
	ADD.l ($0000).w,D2
	ADD.l ($1234).w,D3
	ADD.l ($7FFF).w,D4

; ADD.l, (xxx).l,Dn
	ADD.l ($00000000).l,D5
	ADD.l ($12345678).l,D6
	ADD.l ($FFFFFFFF).l,D7

; ADD.l, d16(PC),Dn
	ADD.l -32768(PC),D0
	ADD.l 0(PC),D1
	ADD.l 1234(PC),D2
	ADD.l 32767(PC),D3

; ADD.l, d8(PC, Xn),Dn
	ADD.l 123(PC,A3.w),D4
	ADD.l -1(PC,D2.l),D5
	ADD.l -128(PC,D3.w),D6

; ADD.l, Imm,Dn
	ADD.l #$00000000,D0
	ADD.l #$12341234,D1
	ADD.l #$ABCDABCD,D2
	ADD.l #$80008000,D3
	ADD.l #$F0F0F0F0,D4
	ADD.l #$AA55AA55,D5
	ADD.l #$0F0F0F0F,D6
	ADD.l #$FFFFFFFF,D7