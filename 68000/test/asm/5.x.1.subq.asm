	mc68000
	opt o-

; =========================================================
; SUBQ.b #<data>,<ea>
; =========================================================

; SUBQ.b, #<data>,Dn
	SUBQ.b #1,D0
	SUBQ.b #2,D1
	SUBQ.b #3,D2
	SUBQ.b #4,D3
	SUBQ.b #5,D4
	SUBQ.b #6,D5
	SUBQ.b #7,D6
	SUBQ.b #8,D7

; SUBQ.b, #<data>,An (not allowed for byte)

; SUBQ.b, #<data>,(An)
	SUBQ.b #1,(A5)

; SUBQ.b, #<data>,(An)+
	SUBQ.b #2,(A6)+

; SUBQ.b, #<data>,-(An)
	SUBQ.b #3,-(A7)

; SUBQ.b, #<data>,d16(An)
	SUBQ.b #4,-32768(A2)
	SUBQ.b #5,0(A3)
	SUBQ.b #6,1234(A4)
	SUBQ.b #7,32767(A4)

; SUBQ.b, #<data>,d8(An, Xn.L|W)
	SUBQ.b #8,123(A2,A3.w)
	SUBQ.b #1,-1(A3,D2.l)
	SUBQ.b #2,-128(A4,D3.w)

; SUBQ.b, #<data>,(xxx).w
	SUBQ.b #3,($0000).w
	SUBQ.b #4,($1234).w
	SUBQ.b #5,($7FFF).w

; SUBQ.b, #<data>,(xxx).l
	SUBQ.b #6,($00000000).l
	SUBQ.b #7,($12345678).l
	SUBQ.b #8,($FFFFFFFF).l

; Not supported
; SUBQ.b, d16(PC)
; SUBQ.b, d8(PC, Xn)
; SUBQ.b, Imm

; =========================================================
; SUBQ.w #<data>,<ea>
; =========================================================

; SUBQ.w, #<data>,Dn
	SUBQ.w #1,D0
	SUBQ.w #2,D1
	SUBQ.w #3,D2
	SUBQ.w #4,D3
	SUBQ.w #5,D4
	SUBQ.w #6,D5
	SUBQ.w #7,D6
	SUBQ.w #8,D7

; SUBQ.w, #<data>,An
	SUBQ.w #1,(A0)

; SUBQ.w, #<data>,(An)
	SUBQ.w #2,(A5)

; SUBQ.w, #<data>,(An)+
	SUBQ.w #3,(A6)+

; SUBQ.w, #<data>,-(An)
	SUBQ.w #4,-(A7)

; SUBQ.w, #<data>,d16(An)
	SUBQ.w #5,-32768(A2)
	SUBQ.w #6,0(A3)
	SUBQ.w #7,1234(A4)
	SUBQ.w #8,32767(A4)

; SUBQ.w, #<data>,d8(An, Xn.L|W)
	SUBQ.w #1,123(A2,A3.w)
	SUBQ.w #2,-1(A3,D2.l)
	SUBQ.w #3,-128(A4,D3.w)

; SUBQ.w, #<data>,(xxx).w
	SUBQ.w #4,($0000).w
	SUBQ.w #5,($1234).w
	SUBQ.w #6,($7FFF).w

; SUBQ.w, #<data>,(xxx).l
	SUBQ.w #7,($00000000).l
	SUBQ.w #8,($12345678).l
	SUBQ.w #1,($FFFFFFFF).l

; Not supported
; SUBQ.w, d16(PC)
; SUBQ.w, d8(PC, Xn)
; SUBQ.w, Imm

; =========================================================
; SUBQ.l <ea>
; =========================================================

; SUBQ.l, #<data>,Dn
	SUBQ.l #1,D0
	SUBQ.l #2,D1
	SUBQ.l #3,D2
	SUBQ.l #4,D3
	SUBQ.l #5,D4
	SUBQ.l #6,D5
	SUBQ.l #7,D6
	SUBQ.l #8,D7

; SUBQ.l, #<data>,An
	SUBQ.l #1,A0

; SUBQ.l, #<data>,(An)
	SUBQ.l #2,(A5)

; SUBQ.l, #<data>,(An)+
	SUBQ.l #3,(A6)+

; SUBQ.l, #<data>,-(An)
	SUBQ.l #4,-(A7)

; SUBQ.l, #<data>,d16(An)
	SUBQ.l #5,-32768(A2)
	SUBQ.l #6,0(A3)
	SUBQ.l #7,1234(A4)
	SUBQ.l #8,32767(A4)

; SUBQ.l, #<data>,d8(An, Xn.L|W)
	SUBQ.l #1,123(A2,A3.w)
	SUBQ.l #2,-1(A3,D2.l)
	SUBQ.l #3,-128(A4,D3.w)

; SUBQ.l, #<data>,(xxx).w
	SUBQ.l #4,($0000).w
	SUBQ.l #5,($1234).w
	SUBQ.l #6,($7FFF).w

; SUBQ.l, #<data>,(xxx).l
	SUBQ.l #7,($00000000).l
	SUBQ.l #8,($12345678).l
	SUBQ.l #1,($FFFFFFFF).l

; Not supported
; SUBQ.l, d16(PC)
; SUBQ.l, d8(PC, Xn)
; SUBQ.l, Imm
