	mc68000
	opt o-

; =========================================================
; OR.b Dn,<ea>
; =========================================================

; OR.b, Dn,Dn (not allowed)
; OR.b, Dn,An (not allowed)

; OR.b, Dn,(An)
	OR.b D0,(A5)

; OR.b, Dn,(An)+
	OR.b D1,(A6)+

; OR.b, Dn,-(An)
	OR.b D2,-(A7)

; OR.b, Dn,d16(An)
	OR.b D3,-32768(A2)
	OR.b D4,0(A3)
	OR.b D5,1234(A4)
	OR.b D6,32767(A4)

; OR.b, Dn,d8(An,Xn.L|W)
	OR.b D7,123(A2,A3.w)
	OR.b D0,-1(A3,D2.l)
	OR.b D1,-128(A4,D3.w)

; OR.b, Dn,(xxx).w
	OR.b D2,($0000).w
	OR.b D3,($1234).w
	OR.b D4,($7FFF).w

; OR.b, Dn,(xxx).l
	OR.b D5,($00000000).l
	OR.b D6,($12345678).l
	OR.b D7,($FFFFFFFF).l

; Not allowed
; OR.b, Dn,d16(PC)
; OR.b, Dn,d8(PC,Xn)
; OR.b, Dn,Imm

; =========================================================
; OR.w Dn,<ea>
; =========================================================

; OR.w, Dn,Dn (not allowed)
; OR.w, Dn,An (not allowed)

; OR.w, Dn,(An)
	OR.w D0,(A5)

; OR.w, Dn,(An)+
	OR.w D1,(A6)+

; OR.w, Dn,-(An)
	OR.w D2,-(A7)

; OR.w, Dn,d16(An)
	OR.w D3,-32768(A2)
	OR.w D4,0(A3)
	OR.w D5,1234(A4)
	OR.w D6,32767(A4)

; OR.w, Dn,d8(An,Xn.L|W)
	OR.w D7,123(A2,A3.w)
	OR.w D0,-1(A3,D2.l)
	OR.w D1,-128(A4,D3.w)

; OR.w, Dn,(xxx).w
	OR.w D2,($0000).w
	OR.w D3,($1234).w
	OR.w D4,($7FFF).w

; OR.w, Dn,(xxx).l
	OR.w D5,($00000000).l
	OR.w D6,($12345678).l
	OR.w D7,($FFFFFFFF).l

; Not allowed
; OR.w, Dn,d16(PC)
; OR.w, Dn,d8(PC,Xn)
; OR.w, Dn,Imm

; =========================================================
; OR.l Dn,<ea>
; =========================================================

; OR.l, Dn,Dn
; OR.l, Dn,An (not allowed)

; OR.l, Dn,(An)
	OR.l D0,(A5)

; OR.l, Dn,(An)+
	OR.l D1,(A6)+

; OR.l, Dn,-(An)
	OR.l D2,-(A7)

; OR.l, Dn,d16(An)
	OR.l D3,-32768(A2)
	OR.l D4,0(A3)
	OR.l D5,1234(A4)
	OR.l D6,32767(A4)

; OR.l, Dn,d8(An,Xn.L|W)
	OR.l D7,123(A2,A3.w)
	OR.l D0,-1(A3,D2.l)
	OR.l D1,-128(A4,D3.w)

; OR.l, Dn,(xxx).w
	OR.l D2,($0000).w
	OR.l D3,($1234).w
	OR.l D4,($7FFF).w

; OR.l, Dn,(xxx).l
	OR.l D5,($00000000).l
	OR.l D6,($12345678).l
	OR.l D7,($FFFFFFFF).l

; OR.l, Dn,d16(PC)
; OR.l, Dn,d8(PC,Xn)
; OR.l, Dn,Imm
