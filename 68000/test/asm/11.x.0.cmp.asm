	mc68000
	opt o-

; =========================================================
; CMP.b <ea>,Dn
; =========================================================

; CMP.b, Dn,Dn
	CMP.b D7,D0
	CMP.b D6,D1
	CMP.b D5,D2
	CMP.b D4,D3
	CMP.b D3,D4
	CMP.b D2,D5
	CMP.b D1,D6
	CMP.b D0,D7

; CMP.b, An,(not allowed)

; CMP.b, (An),Dn,
	CMP.b (A5),D0

; CMP.b, (An)+,Dn,
	CMP.b (A6)+,D1

; CMP.b, -(An),Dn
	CMP.b -(A7),D2

; CMP.b, d16(An),Dn
	CMP.b -32768(A2),D3
	CMP.b 0(A3),D4
	CMP.b 1234(A4),D5
	CMP.b 32767(A4),D6

; CMP.b, d8(An, Xn.L|W),Dn
	CMP.b 123(A2,A3.w),D7
	CMP.b -1(A3,D2.l),D0
	CMP.b -128(A4,D3.w),D1

; CMP.b, (xxx).w,Dn
	CMP.b ($0000).w,D2
	CMP.b ($1234).w,D3
	CMP.b ($7FFF).w,D4

; CMP.b, (xxx).l,Dn
	CMP.b ($00000000).l,D5
	CMP.b ($12345678).l,D6
	CMP.b ($FFFFFFFF).l,D7

; CMP.b, d16(PC),Dn
	CMP.b -32768(PC),D0
	CMP.b 0(PC),D1
	CMP.b 1234(PC),D2
	CMP.b 32767(PC),D3

; CMP.b, d8(PC, Xn),Dn
	CMP.b 123(PC,A3.w),D4
	CMP.b -1(PC,D2.l),D5
	CMP.b -128(PC,D3.w),D6

; CMP.b, Dn,Imm
	CMP.b #$00,D0
	CMP.b #$12,D1
	CMP.b #$AB,D2
	CMP.b #$80,D3
	CMP.b #$F0,D4
	CMP.b #$AA,D5
	CMP.b #$0F,D6
	CMP.b #$FF,D7

; =========================================================
; CMP.w <ea>, Dn
; =========================================================

; CMP.w, Dn,Dn
	CMP.w D7,D0
	CMP.w D6,D1
	CMP.w D5,D2
	CMP.w D4,D3
	CMP.w D3,D4
	CMP.w D2,D5
	CMP.w D1,D6
	CMP.w D0,D7

; CMP.w, An, Dn
	CMP.w A7,D0
	CMP.w A6,D1
	CMP.w A5,D2
	CMP.w A4,D3
	CMP.w A3,D4
	CMP.w A2,D5
	CMP.w A1,D6
	CMP.w A0,D7

; CMP.w, (An),Dn
	CMP.w (A5),D0

; CMP.w, (An)+,Dn
	CMP.w (A6)+,D1

; CMP.w, -(An),Dn
	CMP.w -(A7),D2

; CMP.w, d16(An),Dn
	CMP.w -32768(A2),D3
	CMP.w 0(A3),D4
	CMP.w 1234(A4),D5
	CMP.w 32767(A4),D6

; CMP.w, d8(An, Xn.L|W),Dn
	CMP.w 123(A2,A3.w),D7
	CMP.w -1(A3,D2.l),D0
	CMP.w -128(A4,D3.w),D1

; CMP.w, (xxx).w,Dn
	CMP.w ($0000).w,D2
	CMP.w ($1234).w,D3
	CMP.w ($7FFF).w,D4

; CMP.w, (xxx).l,Dn
	CMP.w ($00000000).l,D5
	CMP.w ($12345678).l,D6
	CMP.w ($FFFFFFFF).l,D7

; CMP.w, d16(PC),Dn
	CMP.w -32768(PC),D0
	CMP.w 0(PC),D1
	CMP.w 1234(PC),D2
	CMP.w 32767(PC),D3

; CMP.w, d8(PC, Xn),Dn
	CMP.w 123(PC,A3.w),D4
	CMP.w -1(PC,D2.l),D5
	CMP.w -128(PC,D3.w),D6

; CMP.w, Imm,Dn
	CMP.w #$0000,D0
	CMP.w #$1234,D1
	CMP.w #$ABCD,D2
	CMP.w #$8000,D3
	CMP.w #$F0F0,D4
	CMP.w #$AA55,D5
	CMP.w #$0F0F,D6
	CMP.w #$FFFF,D7

; =========================================================
; CMP.l <ea>, Dn
; =========================================================

; CMP.l, Dn
	CMP.l D7,D0
	CMP.l D6,D1
	CMP.l D5,D2
	CMP.l D4,D3
	CMP.l D3,D4
	CMP.l D2,D5
	CMP.l D1,D6
	CMP.l D0,D7

; CMP.l, An,Dn
	CMP.l A7,D0
	CMP.l A6,D1
	CMP.l A5,D2
	CMP.l A4,D3
	CMP.l A3,D4
	CMP.l A2,D5
	CMP.l A1,D6
	CMP.l A0,D7

; CMP.l, (An),Dn
	CMP.l (A5),D0

; CMP.l, (An)+,Dn
	CMP.l (A6)+,D1

; CMP.l, -(An),Dn
	CMP.l -(A7),D2

; CMP.l, d16(An),Dn
	CMP.l -32768(A2),D3
	CMP.l 0(A3),D4
	CMP.l 1234(A4),D5
	CMP.l 32767(A4),D6

; CMP.l, d8(An, Xn.L|W),Dn
	CMP.l 123(A2,A3.w),D7
	CMP.l -1(A3,D2.l),D0
	CMP.l -128(A4,D3.w),D1

; CMP.l, (xxx).w,Dn
	CMP.l ($0000).w,D2
	CMP.l ($1234).w,D3
	CMP.l ($7FFF).w,D4

; CMP.l, (xxx).l,Dn
	CMP.l ($00000000).l,D5
	CMP.l ($12345678).l,D6
	CMP.l ($FFFFFFFF).l,D7

; CMP.l, d16(PC),Dn
	CMP.l -32768(PC),D0
	CMP.l 0(PC),D1
	CMP.l 1234(PC),D2
	CMP.l 32767(PC),D3

; CMP.l, d8(PC, Xn),Dn
	CMP.l 123(PC,A3.w),D4
	CMP.l -1(PC,D2.l),D5
	CMP.l -128(PC,D3.w),D6

; CMP.l, Imm,Dn
	CMP.l #$00000000,D0
	CMP.l #$12341234,D1
	CMP.l #$ABCDABCD,D2
	CMP.l #$80008000,D3
	CMP.l #$F0F0F0F0,D4
	CMP.l #$AA55AA55,D5
	CMP.l #$0F0F0F0F,D6
	CMP.l #$FFFFFFFF,D7