	mc68000
	opt o-

; =========================================================
; MOVEM <ea>,<register list>
; =========================================================
; Register list bit mask :
; All modes:   A7 A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0

; =========================================================
; MOVEM.w <ea>,<register list>
; =========================================================

; MOVEM.w, Dn,<regs> (not allowed for src)
; MOVEM.w, An,<regs> (not allowed for src)

; MOVEM.w, (An),<regs>
	MOVEM.w (A0),A0
	MOVEM.w (A1),A7
	MOVEM.w (A2),D0
	MOVEM.w (A3),D7
	MOVEM.w (A4),A0-A1
	MOVEM.w (A5),D0-D1
	MOVEM.w (A6),A0
	MOVEM.w (A7),D0/A0
	MOVEM.w (A0),D0-D7
	MOVEM.w (A1),A0-A7
	MOVEM.w (A2),D0-D7/A0-A7
	MOVEM.w (A3),D0-D3/A0-A3
	MOVEM.w (A4),D0/A7

; MOVEM.w, (An)+,<regs>
	MOVEM.w (A5)+,A0/A5
	MOVEM.w (A5)+,A7
	MOVEM.w (A5)+,D0/A7
	MOVEM.w (A5)+,D1-D3

; MOVEM.w, -(An),<regs> (not allowed for src)

; MOVEM.w, d16(An),<regs>
	MOVEM.w -32768(A2),A2
	MOVEM.w 0(A3),A3/A5
	MOVEM.w 1234(A4),D4
	MOVEM.w 32767(A4),D5-D6

; MOVEM.w, d8(An, Xn.L|W),<regs>
	MOVEM.w 123(A2,A3.w),A6
	MOVEM.w -1(A3,D2.l),A7
	MOVEM.w -128(A4,D3.w),D0

; MOVEM.w, (xxx).w,<regs>
	MOVEM.w ($0000).w,D1
	MOVEM.w ($1234).w,D2
	MOVEM.w ($7FFF).w,D3-D7

; MOVEM.w, (xxx).l,<regs>,
	MOVEM.w ($00000000).l,D0-D1
	MOVEM.w ($12345678).l,D5
	MOVEM.w ($FFFFFFFF).l,D6-D7
	MOVEM.w ($80808080).l,D7

; MOVEM.w, d16(PC),<regs>
	MOVEM.w -32768(PC),A2
	MOVEM.w 0(PC),A3
	MOVEM.w 1234(PC),D4
	MOVEM.w 32767(PC),D5/A0/A7

; MOVEM.w, d8(PC, Xn),<regs>
	MOVEM.w 123(PC,A3.w),A6
	MOVEM.w -1(PC,D2.l),A7
	MOVEM.w -128(PC,D3.w),D0

; MOVEM.w, Imm,<regs> (not allowed for src)

; =========================================================
; MOVEM.l <ea>,<register list>
; =========================================================

; MOVEM.l, Dn,<regs> (not allowed for src)
; MOVEM.l, An,<regs> (not allowed for src)

; MOVEM.l, (An),<regs>
	MOVEM.l (A0),A0
	MOVEM.l (A1),A7
	MOVEM.l (A2),D0
	MOVEM.l (A3),D7
	MOVEM.l (A4),A0-A1
	MOVEM.l (A5),D0-D1
	MOVEM.l (A6),A0
	MOVEM.l (A7),D0/A0
	MOVEM.l (A0),D0-D7
	MOVEM.l (A1),A0-A7
	MOVEM.l (A2),D0-D7/A0-A7
	MOVEM.l (A3),D0-D3/A0-A3
	MOVEM.l (A4),D0/A7

; MOVEM.l, (An)+,<regs>
	MOVEM.l (A5)+,A0
	MOVEM.l (A5)+,A7
	MOVEM.l (A5)+,D0
	MOVEM.l (A5)+,D7

; MOVEM.l, -(An),<regs> (not allowed for src)

; MOVEM.l, d16(An),<regs>
	MOVEM.l -32768(A2),A2
	MOVEM.l 0(A3),A3
	MOVEM.l 1234(A4),D4
	MOVEM.l 32767(A4),D5

; MOVEM.l, d8(An, Xn.L|W),<regs>
	MOVEM.l 123(A2,A3.w),A6
	MOVEM.l -1(A3,D2.l),A7
	MOVEM.l -128(A4,D3.w),D0

; MOVEM.l, (xxx).w,<regs>
	MOVEM.l ($0000).w,D1
	MOVEM.l ($1234).w,D2
	MOVEM.l ($7FFF).w,D3

; MOVEM.l, (xxx).l,<regs>,
	MOVEM.l ($00000000).l,D4
	MOVEM.l ($12345678).l,D5
	MOVEM.l ($FFFFFFFF).l,D6
	MOVEM.l ($80808080).l,D7
	MOVEM.l ($12345678).l,D0/D3/D5/D7/A0/A3/A5/A7
	MOVEM.l ($12345678).l,D0-D1/D3-D4/D6-D7/A0-A1/A3-A4/A6-A7

; MOVEM.l, d16(PC),<regs>
	MOVEM.l -32768(PC),A2
	MOVEM.l 0(PC),A3
	MOVEM.l 1234(PC),D4
	MOVEM.l 32767(PC),D5

; MOVEM.l, d8(PC, Xn),<regs>
	MOVEM.l 123(PC,A3.w),A6
	MOVEM.l -1(PC,D2.l),A7
	MOVEM.l -128(PC,D3.w),D0

; MOVEM.l, Imm,<regs> (not allowed for src)
