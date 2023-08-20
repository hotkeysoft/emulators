	mc68000
	opt o-

; =========================================================
; JMP <ea>
; =========================================================

; JMP, Dn (not allowed for dest)
; JMP, An (not allowed for dest)

; JMP, (An)
	JMP (A5)

; JMP, (An)+ (not allowed for dest)
; JMP, -(An) (not allowed for dest)

; JMP, d16(An)
	JMP -32768(A2)
	JMP 0(A3)
	JMP 1234(A4)
	JMP 32767(A4)

; JMP, d8(An, Xn.L|W)
	JMP 123(A2,A3.w)
	JMP -1(A3,D2.l)
	JMP -128(A4,D3.w)

; JMP, (xxx).w
	JMP ($0000).w
	JMP ($1234).w
	JMP ($7FFF).w

; JMP, (xxx).l
	JMP ($00000000).l
	JMP ($12345678).l
	JMP ($FFFFFFFF).l

; JMP, d16(PC)
	JMP -32768(PC)
	JMP 0(PC)
	JMP 1234(PC)
	JMP 32767(PC)

; JMP, d8(PC, Xn)
	JMP 123(PC,A3.w)
	JMP -1(PC,D2.l)
	JMP -128(PC,D3.w)

; JMP, Imm  (not allowed for dest)
