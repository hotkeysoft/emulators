	mc68000
	opt o-

; =========================================================
; SUB.b Dn,<ea>
; =========================================================

; SUB.b, Dn,Dn (not allowed)
; SUB.b, Dn,An (not allowed)

; SUB.b, Dn,(An)
	SUB.b D0,(A5)

; SUB.b, Dn,(An)+
	SUB.b D1,(A6)+

; SUB.b, Dn,-(An)
	SUB.b D2,-(A7)

; SUB.b, Dn,d16(An)
	SUB.b D3,-32768(A2)
	SUB.b D4,0(A3)
	SUB.b D5,1234(A4)
	SUB.b D6,32767(A4)

; SUB.b, Dn,d8(An,Xn.L|W)
	SUB.b D7,123(A2,A3.w)
	SUB.b D0,-1(A3,D2.l)
	SUB.b D1,-128(A4,D3.w)

; SUB.b, Dn,(xxx).w
	SUB.b D2,($0000).w
	SUB.b D3,($1234).w
	SUB.b D4,($7FFF).w

; SUB.b, Dn,(xxx).l
	SUB.b D5,($00000000).l
	SUB.b D6,($12345678).l
	SUB.b D7,($FFFFFFFF).l

; Not allowed
; SUB.b, Dn,d16(PC)
; SUB.b, Dn,d8(PC,Xn)
; SUB.b, Dn,Imm

; =========================================================
; SUB.w Dn,<ea>
; =========================================================

; SUB.w, Dn,Dn (not allowed)
; SUB.w, Dn,An (not allowed)

; SUB.w, Dn,(An)
	SUB.w D0,(A5)

; SUB.w, Dn,(An)+
	SUB.w D1,(A6)+

; SUB.w, Dn,-(An)
	SUB.w D2,-(A7)

; SUB.w, Dn,d16(An)
	SUB.w D3,-32768(A2)
	SUB.w D4,0(A3)
	SUB.w D5,1234(A4)
	SUB.w D6,32767(A4)

; SUB.w, Dn,d8(An,Xn.L|W)
	SUB.w D7,123(A2,A3.w)
	SUB.w D0,-1(A3,D2.l)
	SUB.w D1,-128(A4,D3.w)

; SUB.w, Dn,(xxx).w
	SUB.w D2,($0000).w
	SUB.w D3,($1234).w
	SUB.w D4,($7FFF).w

; SUB.w, Dn,(xxx).l
	SUB.w D5,($00000000).l
	SUB.w D6,($12345678).l
	SUB.w D7,($FFFFFFFF).l

; Not allowed
; SUB.w, Dn,d16(PC)
; SUB.w, Dn,d8(PC,Xn)
; SUB.w, Dn,Imm

; =========================================================
; SUB.l Dn,<ea>
; =========================================================

; SUB.l, Dn,Dn
; SUB.l, Dn,An (not allowed)

; SUB.l, Dn,(An)
	SUB.l D0,(A5)

; SUB.l, Dn,(An)+
	SUB.l D1,(A6)+

; SUB.l, Dn,-(An)
	SUB.l D2,-(A7)

; SUB.l, Dn,d16(An)
	SUB.l D3,-32768(A2)
	SUB.l D4,0(A3)
	SUB.l D5,1234(A4)
	SUB.l D6,32767(A4)

; SUB.l, Dn,d8(An,Xn.L|W)
	SUB.l D7,123(A2,A3.w)
	SUB.l D0,-1(A3,D2.l)
	SUB.l D1,-128(A4,D3.w)

; SUB.l, Dn,(xxx).w
	SUB.l D2,($0000).w
	SUB.l D3,($1234).w
	SUB.l D4,($7FFF).w

; SUB.l, Dn,(xxx).l
	SUB.l D5,($00000000).l
	SUB.l D6,($12345678).l
	SUB.l D7,($FFFFFFFF).l

; SUB.l, Dn,d16(PC)
; SUB.l, Dn,d8(PC,Xn)
; SUB.l, Dn,Imm
