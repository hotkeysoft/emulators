	mc68000
	opt o-

; =========================================================
; TAS.b <ea>
; =========================================================

; TAS.b, Dn
	TAS.b D0

; TAS.b, An (not allowed for dest)

; TAS.b, (An)
	TAS.b (A5)

; TAS.b, (An)+
	TAS.b (A6)+

; TAS.b, -(An)
	TAS.b -(A7)

; TAS.b, d16(An)
	TAS.b -32768(A2)
	TAS.b 0(A3)
	TAS.b 1234(A4)
	TAS.b 32767(A4)

; TAS.b, d8(An, Xn.L|W)
	TAS.b 123(A2,A3.w)
	TAS.b -1(A3,D2.l)
	TAS.b -128(A4,D3.w)

; TAS.b, (xxx).w
	TAS.b ($0000).w
	TAS.b ($1234).w
	TAS.b ($7FFF).w

; TAS.b, (xxx).l
	TAS.b ($00000000).l
	TAS.b ($12345678).l
	TAS.b ($FFFFFFFF).l

; Not supported
; TAS.b, d16(PC)
; TAS.b, d8(PC, Xn)
; TAS.b, Imm

