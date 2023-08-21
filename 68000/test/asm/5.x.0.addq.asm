	mc68000
	opt o-

; =========================================================
; ADDQ.b #<data>,<ea>
; =========================================================

; ADDQ.b, #<data>,Dn
	ADDQ.b #1,D0
	ADDQ.b #2,D1
	ADDQ.b #3,D2
	ADDQ.b #4,D3
	ADDQ.b #5,D4
	ADDQ.b #6,D5
	ADDQ.b #7,D6
	ADDQ.b #8,D7

; ADDQ.b, #<data>,An (not allowed for byte)

; ADDQ.b, #<data>,(An)
	ADDQ.b #1,(A5)

; ADDQ.b, #<data>,(An)+
	ADDQ.b #2,(A6)+

; ADDQ.b, #<data>,-(An)
	ADDQ.b #3,-(A7)

; ADDQ.b, #<data>,d16(An)
	ADDQ.b #4,-32768(A2)
	ADDQ.b #5,0(A3)
	ADDQ.b #6,1234(A4)
	ADDQ.b #7,32767(A4)

; ADDQ.b, #<data>,d8(An, Xn.L|W)
	ADDQ.b #8,123(A2,A3.w)
	ADDQ.b #1,-1(A3,D2.l)
	ADDQ.b #2,-128(A4,D3.w)

; ADDQ.b, #<data>,(xxx).w
	ADDQ.b #3,($0000).w
	ADDQ.b #4,($1234).w
	ADDQ.b #5,($7FFF).w

; ADDQ.b, #<data>,(xxx).l
	ADDQ.b #6,($00000000).l
	ADDQ.b #7,($12345678).l
	ADDQ.b #8,($FFFFFFFF).l

; Not supported
; ADDQ.b, d16(PC)
; ADDQ.b, d8(PC, Xn)
; ADDQ.b, Imm

; =========================================================
; ADDQ.w #<data>,<ea>
; =========================================================

; ADDQ.w, #<data>,Dn
	ADDQ.w #1,D0
	ADDQ.w #2,D1
	ADDQ.w #3,D2
	ADDQ.w #4,D3
	ADDQ.w #5,D4
	ADDQ.w #6,D5
	ADDQ.w #7,D6
	ADDQ.w #8,D7

; ADDQ.w, #<data>,An
	ADDQ.w #1,(A0)

; ADDQ.w, #<data>,(An)
	ADDQ.w #2,(A5)

; ADDQ.w, #<data>,(An)+
	ADDQ.w #3,(A6)+

; ADDQ.w, #<data>,-(An)
	ADDQ.w #4,-(A7)

; ADDQ.w, #<data>,d16(An)
	ADDQ.w #5,-32768(A2)
	ADDQ.w #6,0(A3)
	ADDQ.w #7,1234(A4)
	ADDQ.w #8,32767(A4)

; ADDQ.w, #<data>,d8(An, Xn.L|W)
	ADDQ.w #1,123(A2,A3.w)
	ADDQ.w #2,-1(A3,D2.l)
	ADDQ.w #3,-128(A4,D3.w)

; ADDQ.w, #<data>,(xxx).w
	ADDQ.w #4,($0000).w
	ADDQ.w #5,($1234).w
	ADDQ.w #6,($7FFF).w

; ADDQ.w, #<data>,(xxx).l
	ADDQ.w #7,($00000000).l
	ADDQ.w #8,($12345678).l
	ADDQ.w #1,($FFFFFFFF).l

; Not supported
; ADDQ.w, d16(PC)
; ADDQ.w, d8(PC, Xn)
; ADDQ.w, Imm

; =========================================================
; ADDQ.l <ea>
; =========================================================

; ADDQ.l, #<data>,Dn
	ADDQ.l #1,D0
	ADDQ.l #2,D1
	ADDQ.l #3,D2
	ADDQ.l #4,D3
	ADDQ.l #5,D4
	ADDQ.l #6,D5
	ADDQ.l #7,D6
	ADDQ.l #8,D7

; ADDQ.l, #<data>,An
	ADDQ.l #1,A0

; ADDQ.l, #<data>,(An)
	ADDQ.l #2,(A5)

; ADDQ.l, #<data>,(An)+
	ADDQ.l #3,(A6)+

; ADDQ.l, #<data>,-(An)
	ADDQ.l #4,-(A7)

; ADDQ.l, #<data>,d16(An)
	ADDQ.l #5,-32768(A2)
	ADDQ.l #6,0(A3)
	ADDQ.l #7,1234(A4)
	ADDQ.l #8,32767(A4)

; ADDQ.l, #<data>,d8(An, Xn.L|W)
	ADDQ.l #1,123(A2,A3.w)
	ADDQ.l #2,-1(A3,D2.l)
	ADDQ.l #3,-128(A4,D3.w)

; ADDQ.l, #<data>,(xxx).w
	ADDQ.l #4,($0000).w
	ADDQ.l #5,($1234).w
	ADDQ.l #6,($7FFF).w

; ADDQ.l, #<data>,(xxx).l
	ADDQ.l #7,($00000000).l
	ADDQ.l #8,($12345678).l
	ADDQ.l #1,($FFFFFFFF).l

; Not supported
; ADDQ.l, d16(PC)
; ADDQ.l, d8(PC, Xn)
; ADDQ.l, Imm
