	mc68000
	opt o-

; =========================================================
; SUBA.w <ea>, Dn
; =========================================================

; SUBA.w, Dn,An
	SUBA.w D7,A0
	SUBA.w D6,A1
	SUBA.w D5,A2
	SUBA.w D4,A3
	SUBA.w D3,A4
	SUBA.w D2,A5
	SUBA.w D1,A6
	SUBA.w D0,A7

; SUBA.w, An,An
	SUBA.w A7,A0
	SUBA.w A6,A1
	SUBA.w A5,A2
	SUBA.w A4,A3
	SUBA.w A3,A4
	SUBA.w A2,A5
	SUBA.w A1,A6
	SUBA.w A0,A7

; SUBA.w, (An),An
	SUBA.w (A5),A0

; SUBA.w, (An)+,An
	SUBA.w (A6)+,A1

; SUBA.w, -(An),An
	SUBA.w -(A7),A2

; SUBA.w, d16(An),An
	SUBA.w -32768(A2),A3
	SUBA.w 0(A3),A4
	SUBA.w 1234(A4),A5
	SUBA.w 32767(A4),A6

; SUBA.w, d8(An, Xn.L|W),An
	SUBA.w 123(A2,A3.w),A7
	SUBA.w -1(A3,D2.l),A0
	SUBA.w -128(A4,D3.w),A1

; SUBA.w, (xxx).w,An
	SUBA.w ($0000).w,A2
	SUBA.w ($1234).w,A3
	SUBA.w ($7FFF).w,A4

; SUBA.w, (xxx).l,An
	SUBA.w ($00000000).l,A5
	SUBA.w ($12345678).l,A6
	SUBA.w ($FFFFFFFF).l,A7

; SUBA.w, d16(PC),An
	SUBA.w -32768(PC),A0
	SUBA.w 0(PC),A1
	SUBA.w 1234(PC),A2
	SUBA.w 32767(PC),A3

; SUBA.w, d8(PC, Xn),An
	SUBA.w 123(PC,A3.w),A4
	SUBA.w -1(PC,D2.l),A5
	SUBA.w -128(PC,D3.w),A6

; SUBA.w, Imm,An
	SUBA.w #$0000,A0
	SUBA.w #$1234,A1
	SUBA.w #$ABCD,A2
	SUBA.w #$8000,A3
	SUBA.w #$F0F0,A4
	SUBA.w #$AA55,A5
	SUBA.w #$0F0F,A6
	SUBA.w #$FFFF,A7

; =========================================================
; SUBA.l <ea>, An
; =========================================================

; SUBA.l, An,Dn
	SUBA.l D7,A0
	SUBA.l D6,A1
	SUBA.l D5,A2
	SUBA.l D4,A3
	SUBA.l D3,A4
	SUBA.l D2,A5
	SUBA.l D1,A6
	SUBA.l D0,A7

; SUBA.l, An,An
	SUBA.l A7,A0
	SUBA.l A6,A1
	SUBA.l A5,A2
	SUBA.l A4,A3
	SUBA.l A3,A4
	SUBA.l A2,A5
	SUBA.l A1,A6
	SUBA.l A0,A7

; SUBA.l, (An),An
	SUBA.l (A5),A0

; SUBA.l, (An)+,An
	SUBA.l (A6)+,A1

; SUBA.l, -(An),An
	SUBA.l -(A7),A2

; SUBA.l, d16(An),An
	SUBA.l -32768(A2),A3
	SUBA.l 0(A3),A4
	SUBA.l 1234(A4),A5
	SUBA.l 32767(A4),A6

; SUBA.l, d8(An, Xn.L|W),An
	SUBA.l 123(A2,A3.w),A7
	SUBA.l -1(A3,D2.l),A0
	SUBA.l -128(A4,D3.w),A1

; SUBA.l, (xxx).w,An
	SUBA.l ($0000).w,A2
	SUBA.l ($1234).w,A3
	SUBA.l ($7FFF).w,A4

; SUBA.l, (xxx).l,An
	SUBA.l ($00000000).l,A5
	SUBA.l ($12345678).l,A6
	SUBA.l ($FFFFFFFF).l,A7

; SUBA.l, d16(PC),An
	SUBA.l -32768(PC),A0
	SUBA.l 0(PC),A1
	SUBA.l 1234(PC),A2
	SUBA.l 32767(PC),A3

; SUBA.l, d8(PC, Xn),An
	SUBA.l 123(PC,D3.w),A4
	SUBA.l -1(PC,D2.l),A5
	SUBA.l -128(PC,A3.w),A6

; SUBA.l, Imm,An
	SUBA.l #$00000000,A0
	SUBA.l #$12341234,A1
	SUBA.l #$ABCDABCD,A2
	SUBA.l #$80008000,A3
	SUBA.l #$F0F0F0F0,A4
	SUBA.l #$AA55AA55,A5
	SUBA.l #$0F0F0F0F,A6
	SUBA.l #$FFFFFFFF,A7