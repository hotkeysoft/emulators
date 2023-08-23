	mc68000
	opt o-

; =========================================================
; AND.b <ea>,Dn
; =========================================================

; AND.b, Dn,Dn
	AND.b D7,D0
	AND.b D6,D1
	AND.b D5,D2
	AND.b D4,D3
	AND.b D3,D4
	AND.b D2,D5
	AND.b D1,D6
	AND.b D0,D7

; AND.b, An,(not allowed)

; AND.b, (An),Dn,
	AND.b (A5),D0

; AND.b, (An)+,Dn,
	AND.b (A6)+,D1

; AND.b, -(An),Dn
	AND.b -(A7),D2

; AND.b, d16(An),Dn
	AND.b -32768(A2),D3
	AND.b 0(A3),D4
	AND.b 1234(A4),D5
	AND.b 32767(A4),D6

; AND.b, d8(An, Xn.L|W),Dn
	AND.b 123(A2,A3.w),D7
	AND.b -1(A3,D2.l),D0
	AND.b -128(A4,D3.w),D1

; AND.b, (xxx).w,Dn
	AND.b ($0000).w,D2
	AND.b ($1234).w,D3
	AND.b ($7FFF).w,D4

; AND.b, (xxx).l,Dn
	AND.b ($00000000).l,D5
	AND.b ($12345678).l,D6
	AND.b ($FFFFFFFF).l,D7

; AND.b, d16(PC),Dn
	AND.b -32768(PC),D0
	AND.b 0(PC),D1
	AND.b 1234(PC),D2
	AND.b 32767(PC),D3

; AND.b, d8(PC, Xn),Dn
	AND.b 123(PC,A3.w),D4
	AND.b -1(PC,D2.l),D5
	AND.b -128(PC,D3.w),D6

; AND.b, Dn,Imm
	AND.b #$00,D0
	AND.b #$12,D1
	AND.b #$AB,D2
	AND.b #$80,D3
	AND.b #$F0,D4
	AND.b #$AA,D5
	AND.b #$0F,D6
	AND.b #$FF,D7

; =========================================================
; AND.w <ea>, Dn
; =========================================================

; AND.w, Dn,Dn
	AND.w D7,D0
	AND.w D6,D1
	AND.w D5,D2
	AND.w D4,D3
	AND.w D3,D4
	AND.w D2,D5
	AND.w D1,D6
	AND.w D0,D7

; AND.w, An, Dn (not allowed)

; AND.w, (An),Dn
	AND.w (A5),D0

; AND.w, (An)+,Dn
	AND.w (A6)+,D1

; AND.w, -(An),Dn
	AND.w -(A7),D2

; AND.w, d16(An),Dn
	AND.w -32768(A2),D3
	AND.w 0(A3),D4
	AND.w 1234(A4),D5
	AND.w 32767(A4),D6

; AND.w, d8(An, Xn.L|W),Dn
	AND.w 123(A2,A3.w),D7
	AND.w -1(A3,D2.l),D0
	AND.w -128(A4,D3.w),D1

; AND.w, (xxx).w,Dn
	AND.w ($0000).w,D2
	AND.w ($1234).w,D3
	AND.w ($7FFF).w,D4

; AND.w, (xxx).l,Dn
	AND.w ($00000000).l,D5
	AND.w ($12345678).l,D6
	AND.w ($FFFFFFFF).l,D7

; AND.w, d16(PC),Dn
	AND.w -32768(PC),D0
	AND.w 0(PC),D1
	AND.w 1234(PC),D2
	AND.w 32767(PC),D3

; AND.w, d8(PC, Xn),Dn
	AND.w 123(PC,A3.w),D4
	AND.w -1(PC,D2.l),D5
	AND.w -128(PC,D3.w),D6

; AND.w, Imm,Dn
	AND.w #$0000,D0
	AND.w #$1234,D1
	AND.w #$ABCD,D2
	AND.w #$8000,D3
	AND.w #$F0F0,D4
	AND.w #$AA55,D5
	AND.w #$0F0F,D6
	AND.w #$FFFF,D7

; =========================================================
; AND.l <ea>, Dn
; =========================================================

; AND.l, Dn
	AND.l D7,D0
	AND.l D6,D1
	AND.l D5,D2
	AND.l D4,D3
	AND.l D3,D4
	AND.l D2,D5
	AND.l D1,D6
	AND.l D0,D7

; AND.l, An,Dn (not allowed)

; AND.l, (An),Dn
	AND.l (A5),D0

; AND.l, (An)+,Dn
	AND.l (A6)+,D1

; AND.l, -(An),Dn
	AND.l -(A7),D2

; AND.l, d16(An),Dn
	AND.l -32768(A2),D3
	AND.l 0(A3),D4
	AND.l 1234(A4),D5
	AND.l 32767(A4),D6

; AND.l, d8(An, Xn.L|W),Dn
	AND.l 123(A2,A3.w),D7
	AND.l -1(A3,D2.l),D0
	AND.l -128(A4,D3.w),D1

; AND.l, (xxx).w,Dn
	AND.l ($0000).w,D2
	AND.l ($1234).w,D3
	AND.l ($7FFF).w,D4

; AND.l, (xxx).l,Dn
	AND.l ($00000000).l,D5
	AND.l ($12345678).l,D6
	AND.l ($FFFFFFFF).l,D7

; AND.l, d16(PC),Dn
	AND.l -32768(PC),D0
	AND.l 0(PC),D1
	AND.l 1234(PC),D2
	AND.l 32767(PC),D3

; AND.l, d8(PC, Xn),Dn
	AND.l 123(PC,A3.w),D4
	AND.l -1(PC,D2.l),D5
	AND.l -128(PC,D3.w),D6

; AND.l, Imm,Dn
	AND.l #$00000000,D0
	AND.l #$12341234,D1
	AND.l #$ABCDABCD,D2
	AND.l #$80008000,D3
	AND.l #$F0F0F0F0,D4
	AND.l #$AA55AA55,D5
	AND.l #$0F0F0F0F,D6
	AND.l #$FFFFFFFF,D7