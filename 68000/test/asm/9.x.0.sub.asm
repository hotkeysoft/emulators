	mc68000
	opt o-

; =========================================================
; SUB.b <ea>,Dn
; =========================================================

; SUB.b, Dn,Dn
	SUB.b D7,D0
	SUB.b D6,D1
	SUB.b D5,D2
	SUB.b D4,D3
	SUB.b D3,D4
	SUB.b D2,D5
	SUB.b D1,D6
	SUB.b D0,D7

; SUB.b, An,(not allowed)

; SUB.b, (An),Dn,
	SUB.b (A5),D0

; SUB.b, (An)+,Dn,
	SUB.b (A6)+,D1

; SUB.b, -(An),Dn
	SUB.b -(A7),D2

; SUB.b, d16(An),Dn
	SUB.b -32768(A2),D3
	SUB.b 0(A3),D4
	SUB.b 1234(A4),D5
	SUB.b 32767(A4),D6

; SUB.b, d8(An, Xn.L|W),Dn
	SUB.b 123(A2,A3.w),D7
	SUB.b -1(A3,D2.l),D0
	SUB.b -128(A4,D3.w),D1

; SUB.b, (xxx).w,Dn
	SUB.b ($0000).w,D2
	SUB.b ($1234).w,D3
	SUB.b ($7FFF).w,D4

; SUB.b, (xxx).l,Dn
	SUB.b ($00000000).l,D5
	SUB.b ($12345678).l,D6
	SUB.b ($FFFFFFFF).l,D7

; SUB.b, d16(PC),Dn
	SUB.b -32768(PC),D0
	SUB.b 0(PC),D1
	SUB.b 1234(PC),D2
	SUB.b 32767(PC),D3

; SUB.b, d8(PC, Xn),Dn
	SUB.b 123(PC,A3.w),D4
	SUB.b -1(PC,D2.l),D5
	SUB.b -128(PC,D3.w),D6

; SUB.b, Dn,Imm
	SUB.b #$00,D0
	SUB.b #$12,D1
	SUB.b #$AB,D2
	SUB.b #$80,D3
	SUB.b #$F0,D4
	SUB.b #$AA,D5
	SUB.b #$0F,D6
	SUB.b #$FF,D7

; =========================================================
; SUB.w <ea>, Dn
; =========================================================

; SUB.w, Dn,Dn
	SUB.w D7,D0
	SUB.w D6,D1
	SUB.w D5,D2
	SUB.w D4,D3
	SUB.w D3,D4
	SUB.w D2,D5
	SUB.w D1,D6
	SUB.w D0,D7

; SUB.w, An, Dn
	SUB.w A7,D0
	SUB.w A6,D1
	SUB.w A5,D2
	SUB.w A4,D3
	SUB.w A3,D4
	SUB.w A2,D5
	SUB.w A1,D6
	SUB.w A0,D7

; SUB.w, (An),Dn
	SUB.w (A5),D0

; SUB.w, (An)+,Dn
	SUB.w (A6)+,D1

; SUB.w, -(An),Dn
	SUB.w -(A7),D2

; SUB.w, d16(An),Dn
	SUB.w -32768(A2),D3
	SUB.w 0(A3),D4
	SUB.w 1234(A4),D5
	SUB.w 32767(A4),D6

; SUB.w, d8(An, Xn.L|W),Dn
	SUB.w 123(A2,A3.w),D7
	SUB.w -1(A3,D2.l),D0
	SUB.w -128(A4,D3.w),D1

; SUB.w, (xxx).w,Dn
	SUB.w ($0000).w,D2
	SUB.w ($1234).w,D3
	SUB.w ($7FFF).w,D4

; SUB.w, (xxx).l,Dn
	SUB.w ($00000000).l,D5
	SUB.w ($12345678).l,D6
	SUB.w ($FFFFFFFF).l,D7

; SUB.w, d16(PC),Dn
	SUB.w -32768(PC),D0
	SUB.w 0(PC),D1
	SUB.w 1234(PC),D2
	SUB.w 32767(PC),D3

; SUB.w, d8(PC, Xn),Dn
	SUB.w 123(PC,A3.w),D4
	SUB.w -1(PC,D2.l),D5
	SUB.w -128(PC,D3.w),D6

; SUB.w, Imm,Dn
	SUB.w #$0000,D0
	SUB.w #$1234,D1
	SUB.w #$ABCD,D2
	SUB.w #$8000,D3
	SUB.w #$F0F0,D4
	SUB.w #$AA55,D5
	SUB.w #$0F0F,D6
	SUB.w #$FFFF,D7

; =========================================================
; SUB.l <ea>, Dn
; =========================================================

; SUB.l, Dn
	SUB.l D7,D0
	SUB.l D6,D1
	SUB.l D5,D2
	SUB.l D4,D3
	SUB.l D3,D4
	SUB.l D2,D5
	SUB.l D1,D6
	SUB.l D0,D7

; SUB.l, An,Dn
	SUB.l A7,D0
	SUB.l A6,D1
	SUB.l A5,D2
	SUB.l A4,D3
	SUB.l A3,D4
	SUB.l A2,D5
	SUB.l A1,D6
	SUB.l A0,D7

; SUB.l, (An),Dn
	SUB.l (A5),D0

; SUB.l, (An)+,Dn
	SUB.l (A6)+,D1

; SUB.l, -(An),Dn
	SUB.l -(A7),D2

; SUB.l, d16(An),Dn
	SUB.l -32768(A2),D3
	SUB.l 0(A3),D4
	SUB.l 1234(A4),D5
	SUB.l 32767(A4),D6

; SUB.l, d8(An, Xn.L|W),Dn
	SUB.l 123(A2,A3.w),D7
	SUB.l -1(A3,D2.l),D0
	SUB.l -128(A4,D3.w),D1

; SUB.l, (xxx).w,Dn
	SUB.l ($0000).w,D2
	SUB.l ($1234).w,D3
	SUB.l ($7FFF).w,D4

; SUB.l, (xxx).l,Dn
	SUB.l ($00000000).l,D5
	SUB.l ($12345678).l,D6
	SUB.l ($FFFFFFFF).l,D7

; SUB.l, d16(PC),Dn
	SUB.l -32768(PC),D0
	SUB.l 0(PC),D1
	SUB.l 1234(PC),D2
	SUB.l 32767(PC),D3

; SUB.l, d8(PC, Xn),Dn
	SUB.l 123(PC,A3.w),D4
	SUB.l -1(PC,D2.l),D5
	SUB.l -128(PC,D3.w),D6

; SUB.l, Imm,Dn
	SUB.l #$00000000,D0
	SUB.l #$12341234,D1
	SUB.l #$ABCDABCD,D2
	SUB.l #$80008000,D3
	SUB.l #$F0F0F0F0,D4
	SUB.l #$AA55AA55,D5
	SUB.l #$0F0F0F0F,D6
	SUB.l #$FFFFFFFF,D7