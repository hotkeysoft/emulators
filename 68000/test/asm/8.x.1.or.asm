	mc68000
	opt o-

; =========================================================
; OR.b <ea>,Dn
; =========================================================

; OR.b, Dn,Dn
	OR.b D7,D0
	OR.b D6,D1
	OR.b D5,D2
	OR.b D4,D3
	OR.b D3,D4
	OR.b D2,D5
	OR.b D1,D6
	OR.b D0,D7

; OR.b, An,(not allowed)

; OR.b, (An),Dn,
	OR.b (A5),D0

; OR.b, (An)+,Dn,
	OR.b (A6)+,D1

; OR.b, -(An),Dn
	OR.b -(A7),D2

; OR.b, d16(An),Dn
	OR.b -32768(A2),D3
	OR.b 0(A3),D4
	OR.b 1234(A4),D5
	OR.b 32767(A4),D6

; OR.b, d8(An, Xn.L|W),Dn
	OR.b 123(A2,A3.w),D7
	OR.b -1(A3,D2.l),D0
	OR.b -128(A4,D3.w),D1

; OR.b, (xxx).w,Dn
	OR.b ($0000).w,D2
	OR.b ($1234).w,D3
	OR.b ($7FFF).w,D4

; OR.b, (xxx).l,Dn
	OR.b ($00000000).l,D5
	OR.b ($12345678).l,D6
	OR.b ($FFFFFFFF).l,D7

; OR.b, d16(PC),Dn
	OR.b -32768(PC),D0
	OR.b 0(PC),D1
	OR.b 1234(PC),D2
	OR.b 32767(PC),D3

; OR.b, d8(PC, Xn),Dn
	OR.b 123(PC,A3.w),D4
	OR.b -1(PC,D2.l),D5
	OR.b -128(PC,D3.w),D6

; OR.b, Dn,Imm
	OR.b #$00,D0
	OR.b #$12,D1
	OR.b #$AB,D2
	OR.b #$80,D3
	OR.b #$F0,D4
	OR.b #$AA,D5
	OR.b #$0F,D6
	OR.b #$FF,D7

; =========================================================
; OR.w <ea>, Dn
; =========================================================

; OR.w, Dn,Dn
	OR.w D7,D0
	OR.w D6,D1
	OR.w D5,D2
	OR.w D4,D3
	OR.w D3,D4
	OR.w D2,D5
	OR.w D1,D6
	OR.w D0,D7

; OR.w, An, Dn (not allowed)

; OR.w, (An),Dn
	OR.w (A5),D0

; OR.w, (An)+,Dn
	OR.w (A6)+,D1

; OR.w, -(An),Dn
	OR.w -(A7),D2

; OR.w, d16(An),Dn
	OR.w -32768(A2),D3
	OR.w 0(A3),D4
	OR.w 1234(A4),D5
	OR.w 32767(A4),D6

; OR.w, d8(An, Xn.L|W),Dn
	OR.w 123(A2,A3.w),D7
	OR.w -1(A3,D2.l),D0
	OR.w -128(A4,D3.w),D1

; OR.w, (xxx).w,Dn
	OR.w ($0000).w,D2
	OR.w ($1234).w,D3
	OR.w ($7FFF).w,D4

; OR.w, (xxx).l,Dn
	OR.w ($00000000).l,D5
	OR.w ($12345678).l,D6
	OR.w ($FFFFFFFF).l,D7

; OR.w, d16(PC),Dn
	OR.w -32768(PC),D0
	OR.w 0(PC),D1
	OR.w 1234(PC),D2
	OR.w 32767(PC),D3

; OR.w, d8(PC, Xn),Dn
	OR.w 123(PC,A3.w),D4
	OR.w -1(PC,D2.l),D5
	OR.w -128(PC,D3.w),D6

; OR.w, Imm,Dn
	OR.w #$0000,D0
	OR.w #$1234,D1
	OR.w #$ABCD,D2
	OR.w #$8000,D3
	OR.w #$F0F0,D4
	OR.w #$AA55,D5
	OR.w #$0F0F,D6
	OR.w #$FFFF,D7

; =========================================================
; OR.l <ea>, Dn
; =========================================================

; OR.l, Dn
	OR.l D7,D0
	OR.l D6,D1
	OR.l D5,D2
	OR.l D4,D3
	OR.l D3,D4
	OR.l D2,D5
	OR.l D1,D6
	OR.l D0,D7

; OR.l, An,Dn (not allowed)

; OR.l, (An),Dn
	OR.l (A5),D0

; OR.l, (An)+,Dn
	OR.l (A6)+,D1

; OR.l, -(An),Dn
	OR.l -(A7),D2

; OR.l, d16(An),Dn
	OR.l -32768(A2),D3
	OR.l 0(A3),D4
	OR.l 1234(A4),D5
	OR.l 32767(A4),D6

; OR.l, d8(An, Xn.L|W),Dn
	OR.l 123(A2,A3.w),D7
	OR.l -1(A3,D2.l),D0
	OR.l -128(A4,D3.w),D1

; OR.l, (xxx).w,Dn
	OR.l ($0000).w,D2
	OR.l ($1234).w,D3
	OR.l ($7FFF).w,D4

; OR.l, (xxx).l,Dn
	OR.l ($00000000).l,D5
	OR.l ($12345678).l,D6
	OR.l ($FFFFFFFF).l,D7

; OR.l, d16(PC),Dn
	OR.l -32768(PC),D0
	OR.l 0(PC),D1
	OR.l 1234(PC),D2
	OR.l 32767(PC),D3

; OR.l, d8(PC, Xn),Dn
	OR.l 123(PC,A3.w),D4
	OR.l -1(PC,D2.l),D5
	OR.l -128(PC,D3.w),D6

; OR.l, Imm,Dn
	OR.l #$00000000,D0
	OR.l #$12341234,D1
	OR.l #$ABCDABCD,D2
	OR.l #$80008000,D3
	OR.l #$F0F0F0F0,D4
	OR.l #$AA55AA55,D5
	OR.l #$0F0F0F0F,D6
	OR.l #$FFFFFFFF,D7