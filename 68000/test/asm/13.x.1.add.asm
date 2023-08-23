	mc68000
	opt o-

; =========================================================
; ADD.b Dn,<ea>
; =========================================================

; ADD.b, Dn,Dn (not allowed)
; ADD.b, Dn,An (not allowed)

; ADD.b, Dn,(An)
	ADD.b D0,(A5)

; ADD.b, Dn,(An)+
	ADD.b D1,(A6)+

; ADD.b, Dn,-(An)
	ADD.b D2,-(A7)

; ADD.b, Dn,d16(An)
	ADD.b D3,-32768(A2)
	ADD.b D4,0(A3)
	ADD.b D5,1234(A4)
	ADD.b D6,32767(A4)

; ADD.b, Dn,d8(An,Xn.L|W)
	ADD.b D7,123(A2,A3.w)
	ADD.b D0,-1(A3,D2.l)
	ADD.b D1,-128(A4,D3.w)

; ADD.b, Dn,(xxx).w
	ADD.b D2,($0000).w
	ADD.b D3,($1234).w
	ADD.b D4,($7FFF).w

; ADD.b, Dn,(xxx).l
	ADD.b D5,($00000000).l
	ADD.b D6,($12345678).l
	ADD.b D7,($FFFFFFFF).l

; Not allowed
; ADD.b, Dn,d16(PC)
; ADD.b, Dn,d8(PC,Xn)
; ADD.b, Dn,Imm

; =========================================================
; ADD.w Dn,<ea>
; =========================================================

; ADD.w, Dn,Dn (not allowed)
; ADD.w, Dn,An (not allowed)

; ADD.w, Dn,(An)
	ADD.w D0,(A5)

; ADD.w, Dn,(An)+
	ADD.w D1,(A6)+

; ADD.w, Dn,-(An)
	ADD.w D2,-(A7)

; ADD.w, Dn,d16(An)
	ADD.w D3,-32768(A2)
	ADD.w D4,0(A3)
	ADD.w D5,1234(A4)
	ADD.w D6,32767(A4)

; ADD.w, Dn,d8(An,Xn.L|W)
	ADD.w D7,123(A2,A3.w)
	ADD.w D0,-1(A3,D2.l)
	ADD.w D1,-128(A4,D3.w)

; ADD.w, Dn,(xxx).w
	ADD.w D2,($0000).w
	ADD.w D3,($1234).w
	ADD.w D4,($7FFF).w

; ADD.w, Dn,(xxx).l
	ADD.w D5,($00000000).l
	ADD.w D6,($12345678).l
	ADD.w D7,($FFFFFFFF).l

; Not allowed
; ADD.w, Dn,d16(PC)
; ADD.w, Dn,d8(PC,Xn)
; ADD.w, Dn,Imm

; =========================================================
; ADD.l Dn,<ea>
; =========================================================

; ADD.l, Dn,Dn
; ADD.l, Dn,An (not allowed)

; ADD.l, Dn,(An)
	ADD.l D0,(A5)

; ADD.l, Dn,(An)+
	ADD.l D1,(A6)+

; ADD.l, Dn,-(An)
	ADD.l D2,-(A7)

; ADD.l, Dn,d16(An)
	ADD.l D3,-32768(A2)
	ADD.l D4,0(A3)
	ADD.l D5,1234(A4)
	ADD.l D6,32767(A4)

; ADD.l, Dn,d8(An,Xn.L|W)
	ADD.l D7,123(A2,A3.w)
	ADD.l D0,-1(A3,D2.l)
	ADD.l D1,-128(A4,D3.w)

; ADD.l, Dn,(xxx).w
	ADD.l D2,($0000).w
	ADD.l D3,($1234).w
	ADD.l D4,($7FFF).w

; ADD.l, Dn,(xxx).l
	ADD.l D5,($00000000).l
	ADD.l D6,($12345678).l
	ADD.l D7,($FFFFFFFF).l

; ADD.l, Dn,d16(PC)
; ADD.l, Dn,d8(PC,Xn)
; ADD.l, Dn,Imm
