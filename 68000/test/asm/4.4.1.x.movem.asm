	mc68000
	opt o-

; =========================================================
; MOVEM <register list>,<ea>
; =========================================================
; Register list bit mask :
; All other modes:   A7 A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0
; Predecrement mode: D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6 A7

; =========================================================
; MOVEM.w <register list>,<ea>
; =========================================================

; MOVEM.w, <regs>,Dn (not allowed for dest)
; MOVEM.w, <regs>,An (not allowed for dest)

; MOVEM.w, <regs>,(An)
	MOVEM.w A0,(A5)
	MOVEM.w A7,(A5)
	MOVEM.w D0,(A5)
	MOVEM.w D7,(A5)
	MOVEM.w A0-A1,(A5)
	MOVEM.w D0-D1,(A5)
	MOVEM.w A0,(A5)
	MOVEM.w D0/A0,(A5)
	MOVEM.w D0-D7,(A7)
	MOVEM.w A0-A7,(A0)
	MOVEM.w D0-D7/A0-A7,(A7)
	MOVEM.w D0-D3/A0-A3,(A0)
	MOVEM.w D0/A7,(A0)

; MOVEM.w, <regs>,(An)+  (not allowed for dest)

; MOVEM.w, <regs>,-(An)
; Note: Predecrement mode has an inverted bit mask
; (D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6 A7)
	MOVEM.w A0,-(A5)
	MOVEM.w A7,-(A5)
	MOVEM.w D0,-(A5)
	MOVEM.w D7,-(A5)
	MOVEM.w A0-A1,-(A5)
	MOVEM.w D0-D1,-(A5)
	MOVEM.w A0,-(A5)
	MOVEM.w D0/A0,-(A5)
	MOVEM.w D0-D7,-(A7)
	MOVEM.w A0-A7,-(A0)
	MOVEM.w D0-D7/A0-A7,-(A7)
	MOVEM.w D0-D3/A0-A3,-(A0)

; MOVEM.w, <regs>,d16(An)
	MOVEM.w A0-A7,-32768(A2)
	MOVEM.w A3,0(A3)
	MOVEM.w A4,1234(A4)
	MOVEM.w A5,32767(A4)

; MOVEM.w, <regs>,d8(An, Xn.L|W)
	MOVEM.w D0-D4,123(A2,A3.w)
	MOVEM.w A7,-1(A3,D2.l)
	MOVEM.w D0,-128(A4,D3.w)

; MOVEM.w, <regs>,(xxx).w
	MOVEM.w D0/A5,($0000).w
	MOVEM.w D2,($1234).w
	MOVEM.w D3,($7FFF).w

; MOVEM.w, <regs>,(xxx).l
	MOVEM.w D4,($00000000).l
	MOVEM.w D5,($12345678).l
	MOVEM.w D6,($FFFFFFFF).l
	MOVEM.w D7,($80808080).l

; Not supported
; MOVEM.w, <regs>,d16(PC)
; MOVEM.w, <regs>,d8(PC, Xn)
; MOVEM.w, <regs>,Imm

; =========================================================
; MOVEM.l <register list>,<ea>
; =========================================================

; MOVEM.l, <regs>,Dn (not allowed for dest)
; MOVEM.l, <regs>,An (not allowed for dest)

; MOVEM.l, <regs>,(An)
	MOVEM.l A0,(A5)

; MOVEM.l, <regs>,(An)+ (not allowed for dest)

; MOVEM.l, <regs>,-(An)
	MOVEM.l A1,-(A7)

; MOVEM.l, <regs>,d16(An)
	MOVEM.l A2,-32768(A2)
	MOVEM.l A3,0(A3)
	MOVEM.l A4,1234(A4)
	MOVEM.l A5,32767(A4)

; MOVEM.l, <regs>,d8(An, Xn.L|W)
	MOVEM.l A6,123(A2,A3.w)
	MOVEM.l A7,-1(A3,D2.l)
	MOVEM.l D0,-128(A4,D3.w)

; MOVEM.l, <regs>,(xxx).w
	MOVEM.l D1,($0000).w
	MOVEM.l D2,($1234).w
	MOVEM.l D3,($7FFF).w

; MOVEM.l, <regs>,(xxx).l
	MOVEM.l D4,($00000000).l
	MOVEM.l D5,($12345678).l
	MOVEM.l D6,($FFFFFFFF).l
	MOVEM.l D7,($80808080).l

; Not supported
; MOVEM.l, <regs>,d16(PC)
; MOVEM.l, <regs>,d8(PC, Xn)
; MOVEM.l, <regs>,Imm
