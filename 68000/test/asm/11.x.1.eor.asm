	mc68000
	opt o-

; =========================================================
; EOR.b Dn,<ea>
; =========================================================

; EOR.b, Dn,Dn
	EOR.b D7,D0
	EOR.b D6,D1
	EOR.b D5,D2
	EOR.b D4,D3
	EOR.b D3,D4
	EOR.b D2,D5
	EOR.b D1,D6
	EOR.b D0,D7

; EOR.b, Dn,An (not allowed)

; EOR.b, Dn,(An)
	EOR.b D0,(A5)

; EOR.b, Dn,(An)+
	EOR.b D1,(A6)+

; EOR.b, Dn,-(An)
	EOR.b D2,-(A7)

; EOR.b, Dn,d16(An)
	EOR.b D3,-32768(A2)
	EOR.b D4,0(A3)
	EOR.b D5,1234(A4)
	EOR.b D6,32767(A4)

; EOR.b, Dn,d8(An,Xn.L|W)
	EOR.b D7,123(A2,A3.w)
	EOR.b D0,-1(A3,D2.l)
	EOR.b D1,-128(A4,D3.w)

; EOR.b, Dn,(xxx).w
	EOR.b D2,($0000).w
	EOR.b D3,($1234).w
	EOR.b D4,($7FFF).w

; EOR.b, Dn,(xxx).l
	EOR.b D5,($00000000).l
	EOR.b D6,($12345678).l
	EOR.b D7,($FFFFFFFF).l

; Not allowed
; EOR.b, Dn,d16(PC)
; EOR.b, Dn,d8(PC,Xn)
; EOR.b, Dn,Imm

; =========================================================
; EOR.w Dn,<ea>
; =========================================================

; EOR.w, Dn,Dn
	EOR.w D7,D0
	EOR.w D6,D1
	EOR.w D5,D2
	EOR.w D4,D3
	EOR.w D3,D4
	EOR.w D2,D5
	EOR.w D1,D6
	EOR.w D0,D7

; EOR.w, Dn,An (not allowed)

; EOR.w, Dn,(An)
	EOR.w D0,(A5)

; EOR.w, Dn,(An)+
	EOR.w D1,(A6)+

; EOR.w, Dn,-(An)
	EOR.w D2,-(A7)

; EOR.w, Dn,d16(An)
	EOR.w D3,-32768(A2)
	EOR.w D4,0(A3)
	EOR.w D5,1234(A4)
	EOR.w D6,32767(A4)

; EOR.w, Dn,d8(An,Xn.L|W)
	EOR.w D7,123(A2,A3.w)
	EOR.w D0,-1(A3,D2.l)
	EOR.w D1,-128(A4,D3.w)

; EOR.w, Dn,(xxx).w
	EOR.w D2,($0000).w
	EOR.w D3,($1234).w
	EOR.w D4,($7FFF).w

; EOR.w, Dn,(xxx).l
	EOR.w D5,($00000000).l
	EOR.w D6,($12345678).l
	EOR.w D7,($FFFFFFFF).l

; Not allowed
; EOR.w, Dn,d16(PC)
; EOR.w, Dn,d8(PC,Xn)
; EOR.w, Dn,Imm

; =========================================================
; EOR.l Dn,<ea>
; =========================================================

; EOR.l, Dn,Dn
	EOR.l D7,D0
	EOR.l D6,D1
	EOR.l D5,D2
	EOR.l D4,D3
	EOR.l D3,D4
	EOR.l D2,D5
	EOR.l D1,D6
	EOR.l D0,D7

; EOR.l, Dn,An (not allowed)

; EOR.l, Dn,(An)
	EOR.l D0,(A5)

; EOR.l, Dn,(An)+
	EOR.l D1,(A6)+

; EOR.l, Dn,-(An)
	EOR.l D2,-(A7)

; EOR.l, Dn,d16(An)
	EOR.l D3,-32768(A2)
	EOR.l D4,0(A3)
	EOR.l D5,1234(A4)
	EOR.l D6,32767(A4)

; EOR.l, Dn,d8(An,Xn.L|W)
	EOR.l D7,123(A2,A3.w)
	EOR.l D0,-1(A3,D2.l)
	EOR.l D1,-128(A4,D3.w)

; EOR.l, Dn,(xxx).w
	EOR.l D2,($0000).w
	EOR.l D3,($1234).w
	EOR.l D4,($7FFF).w

; EOR.l, Dn,(xxx).l
	EOR.l D5,($00000000).l
	EOR.l D6,($12345678).l
	EOR.l D7,($FFFFFFFF).l

; EOR.l, Dn,d16(PC)
; EOR.l, Dn,d8(PC,Xn)
; EOR.l, Dn,Imm
