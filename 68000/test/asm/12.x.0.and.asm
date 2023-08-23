	mc68000
	opt o-

; =========================================================
; AND.b Dn,<ea>
; =========================================================

; AND.b, Dn,Dn (not allowed)
; AND.b, Dn,An (not allowed)

; AND.b, Dn,(An)
	AND.b D0,(A5)

; AND.b, Dn,(An)+
	AND.b D1,(A6)+

; AND.b, Dn,-(An)
	AND.b D2,-(A7)

; AND.b, Dn,d16(An)
	AND.b D3,-32768(A2)
	AND.b D4,0(A3)
	AND.b D5,1234(A4)
	AND.b D6,32767(A4)

; AND.b, Dn,d8(An,Xn.L|W)
	AND.b D7,123(A2,A3.w)
	AND.b D0,-1(A3,D2.l)
	AND.b D1,-128(A4,D3.w)

; AND.b, Dn,(xxx).w
	AND.b D2,($0000).w
	AND.b D3,($1234).w
	AND.b D4,($7FFF).w

; AND.b, Dn,(xxx).l
	AND.b D5,($00000000).l
	AND.b D6,($12345678).l
	AND.b D7,($FFFFFFFF).l

; Not allowed
; AND.b, Dn,d16(PC)
; AND.b, Dn,d8(PC,Xn)
; AND.b, Dn,Imm

; =========================================================
; AND.w Dn,<ea>
; =========================================================

; AND.w, Dn,Dn (not allowed)
; AND.w, Dn,An (not allowed)

; AND.w, Dn,(An)
	AND.w D0,(A5)

; AND.w, Dn,(An)+
	AND.w D1,(A6)+

; AND.w, Dn,-(An)
	AND.w D2,-(A7)

; AND.w, Dn,d16(An)
	AND.w D3,-32768(A2)
	AND.w D4,0(A3)
	AND.w D5,1234(A4)
	AND.w D6,32767(A4)

; AND.w, Dn,d8(An,Xn.L|W)
	AND.w D7,123(A2,A3.w)
	AND.w D0,-1(A3,D2.l)
	AND.w D1,-128(A4,D3.w)

; AND.w, Dn,(xxx).w
	AND.w D2,($0000).w
	AND.w D3,($1234).w
	AND.w D4,($7FFF).w

; AND.w, Dn,(xxx).l
	AND.w D5,($00000000).l
	AND.w D6,($12345678).l
	AND.w D7,($FFFFFFFF).l

; Not allowed
; AND.w, Dn,d16(PC)
; AND.w, Dn,d8(PC,Xn)
; AND.w, Dn,Imm

; =========================================================
; AND.l Dn,<ea>
; =========================================================

; AND.l, Dn,Dn
; AND.l, Dn,An (not allowed)

; AND.l, Dn,(An)
	AND.l D0,(A5)

; AND.l, Dn,(An)+
	AND.l D1,(A6)+

; AND.l, Dn,-(An)
	AND.l D2,-(A7)

; AND.l, Dn,d16(An)
	AND.l D3,-32768(A2)
	AND.l D4,0(A3)
	AND.l D5,1234(A4)
	AND.l D6,32767(A4)

; AND.l, Dn,d8(An,Xn.L|W)
	AND.l D7,123(A2,A3.w)
	AND.l D0,-1(A3,D2.l)
	AND.l D1,-128(A4,D3.w)

; AND.l, Dn,(xxx).w
	AND.l D2,($0000).w
	AND.l D3,($1234).w
	AND.l D4,($7FFF).w

; AND.l, Dn,(xxx).l
	AND.l D5,($00000000).l
	AND.l D6,($12345678).l
	AND.l D7,($FFFFFFFF).l

; AND.l, Dn,d16(PC)
; AND.l, Dn,d8(PC,Xn)
; AND.l, Dn,Imm
