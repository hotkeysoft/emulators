	mc68000
	opt o-

; =========================================================
; LEA.l <ea>, An
; =========================================================

; LEA.l, Dn, An (not allowed)
; LEA.l, An, An (not allowed)

; LEA.l, (An), An
	LEA.l (A7),A0
	LEA.l (A6),A1
	LEA.l (A5),A2
	LEA.l (A4),A3
	LEA.l (A3),A4
	LEA.l (A2),A5
	LEA.l (A1),A6
	LEA.l (A0),A7
	LEA.l (A0),A0
	LEA.l (A7),A7

; LEA.l, (An)+, An (not allowed)
; LEA.l, -(An), An (not allowed)

; LEA.l, d16(An), An
	LEA.l -32768(A2),A0
	LEA.l 0(A3),A1
	LEA.l 1234(A4),A2
	LEA.l 32767(A4),A3

; LEA.l, d8(An, Xn.L|W), An
	LEA.l 123(A2,A3.w),A4
	LEA.l -1(A3,D2.l),A5
	LEA.l -128(A4,D3.w),A6

; LEA.l, (xxx).w, An
	LEA.l ($0000).w,A0
	LEA.l ($1234).w,A1
	LEA.l ($7FFF).w,A2

; LEA.l, (xxx).l, An
	LEA.l ($00000000).l,A3
	LEA.l ($12345678).l,A4
	LEA.l ($FFFFFFFF).l,A5

; LEA.l, d16(PC), An
	LEA.l -32768(PC),A7
	LEA.l 0(PC),A6
	LEA.l 1234(PC),A5
	LEA.l 32767(PC),A4

; LEA.l, d8(PC, Xn), An
	LEA.l 123(PC,A3.w),A3
	LEA.l -1(PC,D2.l),A2
	LEA.l -128(PC,D3.w),A1

; LEA.l, Imm, An (not allowed)
