	mc68000
	opt o-

; =========================================================
; MOVEA.w <ea>, An
; =========================================================

; MOVEA.w, Dn, An - Mode 000 (Data register)
	MOVEA.w D0,A0
	MOVEA.w D1,A1
	MOVEA.w D2,A2
	MOVEA.w D3,A3
	MOVEA.w D4,A4
	MOVEA.w D5,A5
	MOVEA.w D6,A6
	MOVEA.w D7,A7

; MOVEA.w, An, An - Mode 001 (Address)
	MOVEA.w A0,A7
	MOVEA.w A1,A6
	MOVEA.w A2,A5
	MOVEA.w A3,A4
	MOVEA.w A4,A3
	MOVEA.w A5,A2
	MOVEA.w A6,A1
	MOVEA.w A7,A0
	MOVEA.w A0,A0

; MOVEA.w, (An), An - Mode 010
	MOVEA.w (A0),A7
	MOVEA.w (A1),A6
	MOVEA.w (A2),A5
	MOVEA.w (A3),A4
	MOVEA.w (A4),A3
	MOVEA.w (A5),A2
	MOVEA.w (A6),A1
	MOVEA.w (A7),A0
	MOVEA.w (A0),A0

; MOVEA.w (An)+, An - Mode 011 (Address with postincrement)
	MOVEA.w (A0)+,A7
	MOVEA.w (A1)+,A6
	MOVEA.w (A2)+,A5
	MOVEA.w (A3)+,A4
	MOVEA.w (A4)+,A3
	MOVEA.w (A5)+,A2
	MOVEA.w (A6)+,A1
	MOVEA.w (A7)+,A0
	MOVEA.w (A0)+,A0

; MOVEA.w -(An), An - Mode 100 (Address with predecrement)
	MOVEA.w -(A0),A7
	MOVEA.w -(A1),A6
	MOVEA.w -(A2),A5
	MOVEA.w -(A3),A4
	MOVEA.w -(A4),A3
	MOVEA.w -(A5),A2
	MOVEA.w -(A6),A1
	MOVEA.w -(A7),A0
	MOVEA.w -(A0),A0

; MOVEA.w, d16(An), An - Mode 101 (Address with displacement)
	MOVEA.w -32768(A0),A7
	MOVEA.w -32768(A1),A6
	MOVEA.w -32768(A2),A5
	MOVEA.w -32768(A3),A4
	MOVEA.w 32767(A4),A3
	MOVEA.w 32767(A5),A2
	MOVEA.w 32767(A6),A1
	MOVEA.w 32767(A7),A0
	MOVEA.w 32767(A0),A0

; MOVEA.w, d8(An, Xn.L|W), An - Mode 110 (Address with index)
	MOVEA.w -128(A0,D0.w),A7
	MOVEA.w -128(A1,D1.w),A7
	MOVEA.w -128(A2,D2.w),A7
	MOVEA.w -128(A3,D3.w),A7
	MOVEA.w 127(A4,D4.w),A7
	MOVEA.w 127(A5,D5.w),A7
	MOVEA.w 127(A6,D6.w),A7
	MOVEA.w 127(A7,D7.w),A7

	MOVEA.w -128(A0,A0.w),A6
	MOVEA.w -128(A1,A1.w),A6
	MOVEA.w -128(A2,A2.w),A6
	MOVEA.w -128(A3,A3.w),A6
	MOVEA.w 127(A4,A4.w),A6
	MOVEA.w 127(A5,A5.w),A6
	MOVEA.w 127(A6,A6.w),A6
	MOVEA.w 127(A7,A7.w),A6

	MOVEA.w -128(A0,D0.l),A5
	MOVEA.w -128(A1,D1.l),A5
	MOVEA.w -128(A2,D2.l),A5
	MOVEA.w -128(A3,D3.l),A5
	MOVEA.w 127(A4,D4.l),A5
	MOVEA.w 127(A5,D5.l),A5
	MOVEA.w 127(A6,D6.l),A5
	MOVEA.w 127(A7,D7.l),A5

	MOVEA.w -128(A0,A0.l),A4
	MOVEA.w -128(A1,A1.l),A4
	MOVEA.w -128(A2,A2.l),A4
	MOVEA.w -128(A3,A3.l),A4
	MOVEA.w 127(A4,A4.l),A4
	MOVEA.w 127(A5,A5.l),A4
	MOVEA.w 127(A6,A6.l),A4
	MOVEA.w 127(A7,A7.l),A4

; MOVEA.w, (xxx).w, An - Mode 111/000 (Absolute Short)
	MOVEA.w ($0000).w,A0
	MOVEA.w ($1234).w,A1
	MOVEA.w ($7FFF).w,A2

; MOVEA.w, (xxx).l, An - Mode 111/001 (Absolute Long)
	MOVEA.w ($00000000).l,A3
	MOVEA.w ($12345678).l,A3
	MOVEA.w ($FFFFFFFF).l,A3

; MOVEA.w, d16(PC), An - Mode 111/010 (Program Counter with displacement)
	MOVEA.w -32768(PC),A0
	MOVEA.w 0(PC),A1
	MOVEA.w 32767(PC),A2

; MOVEA.w, d8(PC, Xn), An - Mode 111/011 (Program Counter with displacement)
	MOVEA.w -128(PC,D0.w),A7
	MOVEA.w -128(PC,D1.w),A7
	MOVEA.w -128(PC,D2.w),A7
	MOVEA.w -128(PC,D3.w),A7
	MOVEA.w 127(PC,D4.w),A7
	MOVEA.w 127(PC,D5.w),A7
	MOVEA.w 127(PC,D6.w),A7
	MOVEA.w 127(PC,D7.w),A7

	MOVEA.w -128(PC,A0.w),A6
	MOVEA.w -128(PC,A1.w),A6
	MOVEA.w -128(PC,A2.w),A6
	MOVEA.w -128(PC,A3.w),A6
	MOVEA.w 127(PC,A4.w),A6
	MOVEA.w 127(PC,A5.w),A6
	MOVEA.w 127(PC,A6.w),A6
	MOVEA.w 127(PC,A7.w),A6

	MOVEA.w -128(PC,D0.l),A5
	MOVEA.w -128(PC,D1.l),A5
	MOVEA.w -128(PC,D2.l),A5
	MOVEA.w -128(PC,D3.l),A5
	MOVEA.w 127(PC,D4.l),A5
	MOVEA.w 127(PC,D5.l),A5
	MOVEA.w 127(PC,D6.l),A5
	MOVEA.w 127(PC,D7.l),A5

	MOVEA.w -128(PC,A0.l),A4
	MOVEA.w -128(PC,A1.l),A4
	MOVEA.w -128(PC,A2.l),A4
	MOVEA.w -128(PC,A3.l),A4
	MOVEA.w 127(PC,A4.l),A4
	MOVEA.w 127(PC,A5.l),A4
	MOVEA.w 127(PC,A6.l),A4
	MOVEA.w 127(PC,A7.l),A4

; MOVEA.w, Imm, An - Mode 111/100 (Immediate)
	MOVEA.w #$0000,A0
	MOVEA.w #$1234,A1
	MOVEA.w #$FFFF,A2
	MOVEA.w #$55AA,A3
	MOVEA.w #$1234,A4
	MOVEA.w #$ABCD,A5
	MOVEA.w #$0000,A6
	MOVEA.w #$9876,A7
