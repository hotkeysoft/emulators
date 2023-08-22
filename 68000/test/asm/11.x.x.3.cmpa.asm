	mc68000
	opt o-

; =========================================================
; CMPA.w <ea>, Dn
; =========================================================

; CMPA.w, Dn,An
	CMPA.w D7,A0
	CMPA.w D6,A1
	CMPA.w D5,A2
	CMPA.w D4,A3
	CMPA.w D3,A4
	CMPA.w D2,A5
	CMPA.w D1,A6
	CMPA.w D0,A7

; CMPA.w, An,An
	CMPA.w A7,A0
	CMPA.w A6,A1
	CMPA.w A5,A2
	CMPA.w A4,A3
	CMPA.w A3,A4
	CMPA.w A2,A5
	CMPA.w A1,A6
	CMPA.w A0,A7

; CMPA.w, (An),An
	CMPA.w (A5),A0

; CMPA.w, (An)+,An
	CMPA.w (A6)+,A1

; CMPA.w, -(An),An
	CMPA.w -(A7),A2

; CMPA.w, d16(An),An
	CMPA.w -32768(A2),A3
	CMPA.w 0(A3),A4
	CMPA.w 1234(A4),A5
	CMPA.w 32767(A4),A6

; CMPA.w, d8(An, Xn.L|W),An
	CMPA.w 123(A2,A3.w),A7
	CMPA.w -1(A3,D2.l),A0
	CMPA.w -128(A4,D3.w),A1

; CMPA.w, (xxx).w,An
	CMPA.w ($0000).w,A2
	CMPA.w ($1234).w,A3
	CMPA.w ($7FFF).w,A4

; CMPA.w, (xxx).l,An
	CMPA.w ($00000000).l,A5
	CMPA.w ($12345678).l,A6
	CMPA.w ($FFFFFFFF).l,A7

; CMPA.w, d16(PC),An
	CMPA.w -32768(PC),A0
	CMPA.w 0(PC),A1
	CMPA.w 1234(PC),A2
	CMPA.w 32767(PC),A3

; CMPA.w, d8(PC, Xn),An
	CMPA.w 123(PC,A3.w),A4
	CMPA.w -1(PC,D2.l),A5
	CMPA.w -128(PC,D3.w),A6

; CMPA.w, Imm,An
	CMPA.w #$0000,A0
	CMPA.w #$1234,A1
	CMPA.w #$ABCD,A2
	CMPA.w #$8000,A3
	CMPA.w #$F0F0,A4
	CMPA.w #$AA55,A5
	CMPA.w #$0F0F,A6
	CMPA.w #$FFFF,A7

; =========================================================
; CMPA.l <ea>, An
; =========================================================

; CMPA.l, An,Dn
	CMPA.l D7,A0
	CMPA.l D6,A1
	CMPA.l D5,A2
	CMPA.l D4,A3
	CMPA.l D3,A4
	CMPA.l D2,A5
	CMPA.l D1,A6
	CMPA.l D0,A7

; CMPA.l, An,An
	CMPA.l A7,A0
	CMPA.l A6,A1
	CMPA.l A5,A2
	CMPA.l A4,A3
	CMPA.l A3,A4
	CMPA.l A2,A5
	CMPA.l A1,A6
	CMPA.l A0,A7

; CMPA.l, (An),An
	CMPA.l (A5),A0

; CMPA.l, (An)+,An
	CMPA.l (A6)+,A1

; CMPA.l, -(An),An
	CMPA.l -(A7),A2

; CMPA.l, d16(An),An
	CMPA.l -32768(A2),A3
	CMPA.l 0(A3),A4
	CMPA.l 1234(A4),A5
	CMPA.l 32767(A4),A6

; CMPA.l, d8(An, Xn.L|W),An
	CMPA.l 123(A2,A3.w),A7
	CMPA.l -1(A3,D2.l),A0
	CMPA.l -128(A4,D3.w),A1

; CMPA.l, (xxx).w,An
	CMPA.l ($0000).w,A2
	CMPA.l ($1234).w,A3
	CMPA.l ($7FFF).w,A4

; CMPA.l, (xxx).l,An
	CMPA.l ($00000000).l,A5
	CMPA.l ($12345678).l,A6
	CMPA.l ($FFFFFFFF).l,A7

; CMPA.l, d16(PC),An
	CMPA.l -32768(PC),A0
	CMPA.l 0(PC),A1
	CMPA.l 1234(PC),A2
	CMPA.l 32767(PC),A3

; CMPA.l, d8(PC, Xn),An
	CMPA.l 123(PC,D3.w),A4
	CMPA.l -1(PC,D2.l),A5
	CMPA.l -128(PC,A3.w),A6

; CMPA.l, Imm,An
	CMPA.l #$00000000,A0
	CMPA.l #$12341234,A1
	CMPA.l #$ABCDABCD,A2
	CMPA.l #$80008000,A3
	CMPA.l #$F0F0F0F0,A4
	CMPA.l #$AA55AA55,A5
	CMPA.l #$0F0F0F0F,A6
	CMPA.l #$FFFFFFFF,A7