	mc68000
	opt o-

; =========================================================
; ADDA.w <ea>, Dn
; =========================================================

; ADDA.w, Dn,An
	ADDA.w D7,A0
	ADDA.w D6,A1
	ADDA.w D5,A2
	ADDA.w D4,A3
	ADDA.w D3,A4
	ADDA.w D2,A5
	ADDA.w D1,A6
	ADDA.w D0,A7

; ADDA.w, An,An
	ADDA.w A7,A0
	ADDA.w A6,A1
	ADDA.w A5,A2
	ADDA.w A4,A3
	ADDA.w A3,A4
	ADDA.w A2,A5
	ADDA.w A1,A6
	ADDA.w A0,A7

; ADDA.w, (An),An
	ADDA.w (A5),A0

; ADDA.w, (An)+,An
	ADDA.w (A6)+,A1

; ADDA.w, -(An),An
	ADDA.w -(A7),A2

; ADDA.w, d16(An),An
	ADDA.w -32768(A2),A3
	ADDA.w 0(A3),A4
	ADDA.w 1234(A4),A5
	ADDA.w 32767(A4),A6

; ADDA.w, d8(An, Xn.L|W),An
	ADDA.w 123(A2,A3.w),A7
	ADDA.w -1(A3,D2.l),A0
	ADDA.w -128(A4,D3.w),A1

; ADDA.w, (xxx).w,An
	ADDA.w ($0000).w,A2
	ADDA.w ($1234).w,A3
	ADDA.w ($7FFF).w,A4

; ADDA.w, (xxx).l,An
	ADDA.w ($00000000).l,A5
	ADDA.w ($12345678).l,A6
	ADDA.w ($FFFFFFFF).l,A7

; ADDA.w, d16(PC),An
	ADDA.w -32768(PC),A0
	ADDA.w 0(PC),A1
	ADDA.w 1234(PC),A2
	ADDA.w 32767(PC),A3

; ADDA.w, d8(PC, Xn),An
	ADDA.w 123(PC,A3.w),A4
	ADDA.w -1(PC,D2.l),A5
	ADDA.w -128(PC,D3.w),A6

; ADDA.w, Imm,An
	ADDA.w #$0000,A0
	ADDA.w #$1234,A1
	ADDA.w #$ABCD,A2
	ADDA.w #$8000,A3
	ADDA.w #$F0F0,A4
	ADDA.w #$AA55,A5
	ADDA.w #$0F0F,A6
	ADDA.w #$FFFF,A7

; =========================================================
; ADDA.l <ea>, An
; =========================================================

; ADDA.l, An,Dn
	ADDA.l D7,A0
	ADDA.l D6,A1
	ADDA.l D5,A2
	ADDA.l D4,A3
	ADDA.l D3,A4
	ADDA.l D2,A5
	ADDA.l D1,A6
	ADDA.l D0,A7

; ADDA.l, An,An
	ADDA.l A7,A0
	ADDA.l A6,A1
	ADDA.l A5,A2
	ADDA.l A4,A3
	ADDA.l A3,A4
	ADDA.l A2,A5
	ADDA.l A1,A6
	ADDA.l A0,A7

; ADDA.l, (An),An
	ADDA.l (A5),A0

; ADDA.l, (An)+,An
	ADDA.l (A6)+,A1

; ADDA.l, -(An),An
	ADDA.l -(A7),A2

; ADDA.l, d16(An),An
	ADDA.l -32768(A2),A3
	ADDA.l 0(A3),A4
	ADDA.l 1234(A4),A5
	ADDA.l 32767(A4),A6

; ADDA.l, d8(An, Xn.L|W),An
	ADDA.l 123(A2,A3.w),A7
	ADDA.l -1(A3,D2.l),A0
	ADDA.l -128(A4,D3.w),A1

; ADDA.l, (xxx).w,An
	ADDA.l ($0000).w,A2
	ADDA.l ($1234).w,A3
	ADDA.l ($7FFF).w,A4

; ADDA.l, (xxx).l,An
	ADDA.l ($00000000).l,A5
	ADDA.l ($12345678).l,A6
	ADDA.l ($FFFFFFFF).l,A7

; ADDA.l, d16(PC),An
	ADDA.l -32768(PC),A0
	ADDA.l 0(PC),A1
	ADDA.l 1234(PC),A2
	ADDA.l 32767(PC),A3

; ADDA.l, d8(PC, Xn),An
	ADDA.l 123(PC,D3.w),A4
	ADDA.l -1(PC,D2.l),A5
	ADDA.l -128(PC,A3.w),A6

; ADDA.l, Imm,An
	ADDA.l #$00000000,A0
	ADDA.l #$12341234,A1
	ADDA.l #$ABCDABCD,A2
	ADDA.l #$80008000,A3
	ADDA.l #$F0F0F0F0,A4
	ADDA.l #$AA55AA55,A5
	ADDA.l #$0F0F0F0F,A6
	ADDA.l #$FFFFFFFF,A7