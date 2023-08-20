	mc68000
	opt o-

; =========================================================
; MOVEA.l <ea>, An
; =========================================================

; MOVEA.l, Dn, An - Mode 000 (Data register)
	MOVEA.l D0,A0
	MOVEA.l D1,A1
	MOVEA.l D2,A2
	MOVEA.l D3,A3
	MOVEA.l D4,A4
	MOVEA.l D5,A5
	MOVEA.l D6,A6
	MOVEA.l D7,A7

; MOVEA.l, An, An - Mode 001 (Address)
	MOVEA.l A0,A7
	MOVEA.l A1,A6
	MOVEA.l A2,A5
	MOVEA.l A3,A4
	MOVEA.l A4,A3
	MOVEA.l A5,A2
	MOVEA.l A6,A1
	MOVEA.l A7,A0
	MOVEA.l A0,A0

; MOVEA.l, (An), An - Mode 010
	MOVEA.l (A0),A7
	MOVEA.l (A1),A6
	MOVEA.l (A2),A5
	MOVEA.l (A3),A4
	MOVEA.l (A4),A3
	MOVEA.l (A5),A2
	MOVEA.l (A6),A1
	MOVEA.l (A7),A0
	MOVEA.l (A0),A0

; MOVEA.l (An)+, An - Mode 011 (Address with postincrement)
	MOVEA.l (A0)+,A7
	MOVEA.l (A1)+,A6
	MOVEA.l (A2)+,A5
	MOVEA.l (A3)+,A4
	MOVEA.l (A4)+,A3
	MOVEA.l (A5)+,A2
	MOVEA.l (A6)+,A1
	MOVEA.l (A7)+,A0
	MOVEA.l (A0)+,A0

; MOVEA.l -(An), An - Mode 100 (Address with predecrement)
	MOVEA.l -(A0),A7
	MOVEA.l -(A1),A6
	MOVEA.l -(A2),A5
	MOVEA.l -(A3),A4
	MOVEA.l -(A4),A3
	MOVEA.l -(A5),A2
	MOVEA.l -(A6),A1
	MOVEA.l -(A7),A0
	MOVEA.l -(A0),A0

; MOVEA.l, d16(An), An - Mode 101 (Address with displacement)
	MOVEA.l -32768(A0),A7
	MOVEA.l -32768(A1),A6
	MOVEA.l -32768(A2),A5
	MOVEA.l -32768(A3),A4
	MOVEA.l 32767(A4),A3
	MOVEA.l 32767(A5),A2
	MOVEA.l 32767(A6),A1
	MOVEA.l 32767(A7),A0
	MOVEA.l 32767(A0),A0

; MOVEA.l, d8(An, Xn.L|W), An - Mode 110 (Address with index)
	MOVEA.l -128(A0,D0.w),A7
	MOVEA.l -128(A1,D1.w),A7
	MOVEA.l -128(A2,D2.w),A7
	MOVEA.l -128(A3,D3.w),A7
	MOVEA.l 127(A4,D4.w),A7
	MOVEA.l 127(A5,D5.w),A7
	MOVEA.l 127(A6,D6.w),A7
	MOVEA.l 127(A7,D7.w),A7

	MOVEA.l -128(A0,A0.w),A6
	MOVEA.l -128(A1,A1.w),A6
	MOVEA.l -128(A2,A2.w),A6
	MOVEA.l -128(A3,A3.w),A6
	MOVEA.l 127(A4,A4.w),A6
	MOVEA.l 127(A5,A5.w),A6
	MOVEA.l 127(A6,A6.w),A6
	MOVEA.l 127(A7,A7.w),A6

	MOVEA.l -128(A0,D0.l),A5
	MOVEA.l -128(A1,D1.l),A5
	MOVEA.l -128(A2,D2.l),A5
	MOVEA.l -128(A3,D3.l),A5
	MOVEA.l 127(A4,D4.l),A5
	MOVEA.l 127(A5,D5.l),A5
	MOVEA.l 127(A6,D6.l),A5
	MOVEA.l 127(A7,D7.l),A5

	MOVEA.l -128(A0,A0.l),A4
	MOVEA.l -128(A1,A1.l),A4
	MOVEA.l -128(A2,A2.l),A4
	MOVEA.l -128(A3,A3.l),A4
	MOVEA.l 127(A4,A4.l),A4
	MOVEA.l 127(A5,A5.l),A4
	MOVEA.l 127(A6,A6.l),A4
	MOVEA.l 127(A7,A7.l),A4

; MOVEA.l, (xxx).w, An - Mode 111/000 (Absolute Short)
	MOVEA.l ($0000).w,A0
	MOVEA.l ($1234).w,A1
	MOVEA.l ($7FFF).w,A2

; MOVEA.l, (xxx).l, An - Mode 111/001 (Absolute Long)
	MOVEA.l ($00000000).l,A3
	MOVEA.l ($12345678).l,A3
	MOVEA.l ($FFFFFFFF).l,A3

; MOVEA.l, d16(PC), An - Mode 111/010 (Program Counter with displacement)
	MOVEA.l -32768(PC),A0
	MOVEA.l 0(PC),A1
	MOVEA.l 32767(PC),A2

; MOVEA.l, d8(PC, Xn), An - Mode 111/011 (Program Counter with displacement)
	MOVEA.l -128(PC,D0.w),A7
	MOVEA.l -128(PC,D1.w),A7
	MOVEA.l -128(PC,D2.w),A7
	MOVEA.l -128(PC,D3.w),A7
	MOVEA.l 127(PC,D4.w),A7
	MOVEA.l 127(PC,D5.w),A7
	MOVEA.l 127(PC,D6.w),A7
	MOVEA.l 127(PC,D7.w),A7

	MOVEA.l -128(PC,A0.w),A6
	MOVEA.l -128(PC,A1.w),A6
	MOVEA.l -128(PC,A2.w),A6
	MOVEA.l -128(PC,A3.w),A6
	MOVEA.l 127(PC,A4.w),A6
	MOVEA.l 127(PC,A5.w),A6
	MOVEA.l 127(PC,A6.w),A6
	MOVEA.l 127(PC,A7.w),A6

	MOVEA.l -128(PC,D0.l),A5
	MOVEA.l -128(PC,D1.l),A5
	MOVEA.l -128(PC,D2.l),A5
	MOVEA.l -128(PC,D3.l),A5
	MOVEA.l 127(PC,D4.l),A5
	MOVEA.l 127(PC,D5.l),A5
	MOVEA.l 127(PC,D6.l),A5
	MOVEA.l 127(PC,D7.l),A5

	MOVEA.l -128(PC,A0.l),A4
	MOVEA.l -128(PC,A1.l),A4
	MOVEA.l -128(PC,A2.l),A4
	MOVEA.l -128(PC,A3.l),A4
	MOVEA.l 127(PC,A4.l),A4
	MOVEA.l 127(PC,A5.l),A4
	MOVEA.l 127(PC,A6.l),A4
	MOVEA.l 127(PC,A7.l),A4

; MOVEA.l, Imm, An - Mode 111/100 (Immediate)
	MOVEA.l #$00000000,A1
	MOVEA.l #$12345678,A2
	MOVEA.l #$FFFFFFFF,A3
