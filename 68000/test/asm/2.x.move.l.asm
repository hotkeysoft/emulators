	mc68000
	opt o-

; =========================================================
; MOVE.l <ea>, <ea>
; =========================================================


; ---------------------------------------------------------
; MOVE.l Dn, <ea>
; ---------------------------------------------------------

; MOVE.l, Dn, Dn
	MOVE.l D0,D0

; MOVE.l, Dn, An (not allowed for dest)

; MOVE.l, Dn, (An)
	MOVE.l D1,(A5)

; MOVE.l, Dn, (An)+
	MOVE.l D2,(A6)+

; MOVE.l, Dn, -(An)
	MOVE.l D3,-(A7)

; MOVE.l, Dn, d16(An)
	MOVE.l D2,-32768(A2)
	MOVE.l D2,0(A3)
	MOVE.l D3,1234(A4)
	MOVE.l D3,32767(A4)

; MOVE.l, Dn, d8(An, Xn.L|W)
	MOVE.l D2,123(A2,A3.w)
	MOVE.l D2,-1(A3,D2.l)
	MOVE.l D3,-128(A4,D3.w)

; MOVE.l, Dn, (xxx).w
	MOVE.l D0,($0000).w
	MOVE.l D1,($1234).w
	MOVE.l D2,($7FFF).w

; MOVE.l, Dn, (xxx).l
	MOVE.l D3,($00000000).l
	MOVE.l D4,($12345678).l
	MOVE.l D5,($FFFFFFFF).l

; MOVE.l, Dn, d16(PC) (not allowed for dest)
; MOVE.l, Dn, d8(PC, Xn) (not allowed for dest)
; MOVE.l, Dn, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l An, <ea>
; ---------------------------------------------------------

; MOVE.l, An, Dn
	MOVE.l A0,D0

; MOVE.l, An, An (not allowed for dest)

; MOVE.l, An, (An)
	MOVE.l A1,(A5)

; MOVE.l, An, (An)+
	MOVE.l A2,(A6)+

; MOVE.l, An, -(An)
	MOVE.l A3,-(A7)

; MOVE.l, An, d16(An)
	MOVE.l A2,-32768(A2)
	MOVE.l A2,0(A3)
	MOVE.l A3,1234(A4)
	MOVE.l A3,32767(A4)

; MOVE.l, An, d8(An, Xn.L|W)
	MOVE.l A2,123(A2,A3.w)
	MOVE.l A2,-1(A3,D2.l)
	MOVE.l A3,-128(A4,D3.w)

; MOVE.l, An, (xxx).w
	MOVE.l A0,($0000).w
	MOVE.l A1,($1234).w
	MOVE.l A2,($7FFF).w

; MOVE.l, An, (xxx).l
	MOVE.l A3,($00000000).l
	MOVE.l A4,($12345678).l
	MOVE.l A5,($FFFFFFFF).l

; MOVE.l, Dn, d16(PC) (not allowed for dest)
; MOVE.l, Dn, d8(PC, Xn) (not allowed for dest)
; MOVE.l, Dn, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l (An), <ea>
; ---------------------------------------------------------

; MOVE.l, (An), Dn
	MOVE.l (A1),D0

; MOVE.l, (An), An (not allowed for dest)

; MOVE.l, (An), (An)
	MOVE.l (A2),(A5)

; MOVE.l, (An), (An)+
	MOVE.l (A3),(A6)+

; MOVE.l, (An), -(An)
	MOVE.l (A4),-(A7)

; MOVE.l, (An), d16(An)
	MOVE.l (A2),-32768(A2)
	MOVE.l (A3),0(A3)
	MOVE.l (A4),1234(A4)
	MOVE.l (A5),32767(A4)

; MOVE.l, (An), d8(An, Xn.L|W)
	MOVE.l (A5),123(A2,A3.w)
	MOVE.l (A6),-1(A3,D2.l)
	MOVE.l (A7),-128(A4,D3.w)

; MOVE.l, (An), (xxx).w
	MOVE.l (A0),($0000).w
	MOVE.l (A1),($1234).w
	MOVE.l (A2),($7FFF).w

; MOVE.l, (An), (xxx).l
	MOVE.l (A3),($00000000).l
	MOVE.l (A4),($12345678).l
	MOVE.l (A5),($FFFFFFFF).l

; MOVE.l, (An), d16(PC) (not allowed for dest)
; MOVE.l, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.l, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l (An)+, <ea>
; ---------------------------------------------------------

; MOVE.l, (An)+, Dn
	MOVE.l (A1)+,D0

; MOVE.l, (An)+, An (not allowed for dest)

; MOVE.l, (An)+, (An)
	MOVE.l (A2)+,(A5)

; MOVE.l, (An)+, (An)+
	MOVE.l (A3)+,(A6)+

; MOVE.l, (An)+, -(An)
	MOVE.l (A4)+,-(A7)

; MOVE.l, (An)+, d16(An)
	MOVE.l (A2)+,-32768(A2)
	MOVE.l (A3)+,0(A3)
	MOVE.l (A4)+,1234(A4)
	MOVE.l (A5)+,32767(A4)

; MOVE.l, (An)+, d8(An, Xn.L|W)
	MOVE.l (A5)+,123(A2,A3.w)
	MOVE.l (A6)+,-1(A3,D2.l)
	MOVE.l (A7)+,-128(A4,D3.w)

; MOVE.l, (An)+, (xxx).w
	MOVE.l (A0)+,($0000).w
	MOVE.l (A1)+,($1234).w
	MOVE.l (A2)+,($7FFF).w

; MOVE.l, (An)+, (xxx).l
	MOVE.l (A3)+,($00000000).l
	MOVE.l (A4)+,($12345678).l
	MOVE.l (A5)+,($FFFFFFFF).l

; MOVE.l, (An)+, d16(PC) (not allowed for dest)
; MOVE.l, (An)+, d8(PC, Xn) (not allowed for dest)
; MOVE.l, (An)+, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l -(An), <ea>
; ---------------------------------------------------------

; MOVE.l, -(An), Dn
	MOVE.l -(A1),D0

; MOVE.l, -(An), An (not allowed for dest)

; MOVE.l, -(An), (An)
	MOVE.l -(A2),(A5)

; MOVE.l, -(An), (An)+
	MOVE.l -(A3),(A6)+

; MOVE.l, -(An), -(An)
	MOVE.l -(A4),-(A7)

; MOVE.l, -(An), d16(An)
	MOVE.l -(A2),-32768(A2)
	MOVE.l -(A3),0(A3)
	MOVE.l -(A4),1234(A4)
	MOVE.l -(A5),32767(A4)

; MOVE.l, -(An), d8(An, Xn.L|W)
	MOVE.l -(A5),123(A2,A3.w)
	MOVE.l -(A6),-1(A3,D2.l)
	MOVE.l -(A7),-128(A4,D3.w)

; MOVE.l, -(An), (xxx).w
	MOVE.l -(A0),($0000).w
	MOVE.l -(A1),($1234).w
	MOVE.l -(A2),($7FFF).w

; MOVE.l, -(An), (xxx).l
	MOVE.l -(A3),($00000000).l
	MOVE.l -(A4),($12345678).l
	MOVE.l -(A5),($FFFFFFFF).l

; MOVE.l, -(An), d16(PC) (not allowed for dest)
; MOVE.l, -(An), d8(PC, Xn) (not allowed for dest)
; MOVE.l, -(An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l d16(An), <ea>
; ---------------------------------------------------------

; MOVE.l, d16(An), Dn
	MOVE.l 32767(A1),D0

; MOVE.l, d16(An), An (not allowed for dest)

; MOVE.l, d16(An), (An)
	MOVE.l -32768(A2),(A5)

; MOVE.l, d16(An), (An)+
	MOVE.l 0(A3),(A6)+

; MOVE.l, d16(An), -(An)
	MOVE.l 255(A4),-(A7)

; MOVE.l, d16(An), d16(An)
	MOVE.l 123(A2),-32768(A2)
	MOVE.l 234(A3),0(A3)
	MOVE.l 345(A4),1234(A4)
	MOVE.l 567(A5),32767(A4)

; MOVE.l, d16(An), d8(An, Xn.L|W)
	MOVE.l 1(A5),123(A2,A3.w)
	MOVE.l 2(A6),-1(A3,D2.l)
	MOVE.l 3(A7),-128(A4,D3.w)

; MOVE.l, d16(An), (xxx).w
	MOVE.l -1(A0),($0000).w
	MOVE.l -2(A1),($1234).w
	MOVE.l -3(A2),($7FFF).w

; MOVE.l, d16(An), (xxx).l
	MOVE.l 128(A3),($00000000).l
	MOVE.l 128(A4),($12345678).l
	MOVE.l 128(A5),($FFFFFFFF).l

; MOVE.l, (An), d16(PC) (not allowed for dest)
; MOVE.l, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.l, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l d8(An,Xn.L|W), <ea>
; ---------------------------------------------------------

; MOVE.l, d8(An,Xn.L|W), Dn
	MOVE.l 127(A1,D0.w),D0

; MOVE.l, d8(An,Xn.L|W), An (not allowed for dest)

; MOVE.l, d8(An,Xn.L|W), (An)
	MOVE.l -1(A2,D0.l),(A5)

; MOVE.l, d8(An,Xn.L|W), (An)+
	MOVE.l 0(A3,A0.w),(A6)+

; MOVE.l, d8(An,Xn.L|W), -(An)
	MOVE.l -128(A4,A0.l),-(A7)

; MOVE.l, d8(An,Xn.L|W), d16(An)
	MOVE.l 64(A2,D1.w),-32768(A2)
	MOVE.l 64(A3,D2.w),0(A3)
	MOVE.l -64(A4,A3.l),1234(A4)
	MOVE.l -64(A5,A4.l),32767(A4)

; MOVE.l, d8(An,Xn.L|W), d8(An, Xn.L|W)
	MOVE.l 1(A5,D0.l),123(A2,A3.w)
	MOVE.l 2(A6,D1.l),-1(A3,D2.l)
	MOVE.l 3(A7,A7.l),-128(A4,D3.w)

; MOVE.l, d8(An,Xn.L|W), (xxx).w
	MOVE.l -1(A0,D5.w),($0000).w
	MOVE.l -2(A1,D6.w),($1234).w
	MOVE.l -3(A2,D7.w),($7FFF).w

; MOVE.l, d8(An,Xn.L|W), (xxx).l
	MOVE.l 127(A3,D3.l),($00000000).l
	MOVE.l -128(A4,D4.l),($12345678).l
	MOVE.l 127(A5,D5.l),($FFFFFFFF).l

; MOVE.l, (An), d16(PC) (not allowed for dest)
; MOVE.l, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.l, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l (xxx).w, <ea>
; ---------------------------------------------------------

; MOVE.l, (xxx).w, Dn
	MOVE.l ($0000).w,D0

; MOVE.l, (xxx).w, An (not allowed for dest)

; MOVE.l, (xxx).w, (An)
	MOVE.l ($0BCD).w,(A5)

; MOVE.l, (xxx).w, (An)+
	MOVE.l ($7FFF).w,(A6)+

; MOVE.l, (xxx).w, -(An)
	MOVE.l ($55AA).w,-(A7)

; MOVE.l, (xxx).w, d16(An)
	MOVE.l ($55AA).w,-32768(A2)
	MOVE.l ($7000).w,0(A3)
	MOVE.l ($000F).w,1234(A4)
	MOVE.l ($0000).w,32767(A4)

; MOVE.l, (xxx).w, d8(An, Xn.L|W)
	MOVE.l ($7FFF).w,123(A2,A3.w)
	MOVE.l ($1234).w,-1(A3,D2.l)
	MOVE.l ($0000).w,-128(A4,D3.w)

; MOVE.l, (xxx).w, (xxx).w
	MOVE.l ($0000).w,($0000).w
	MOVE.l ($1234).w,($1234).w
	MOVE.l ($7FFF).w,($7FFF).w

; MOVE.l, (xxx).w, (xxx).l
	MOVE.l ($7FFF).w,($00000000).l
	MOVE.l ($2345).w,($12345678).l
	MOVE.l ($0000).w,($FFFFFFFF).l

; MOVE.l, (xxx).w, d16(PC) (not allowed for dest)
; MOVE.l, (xxx).w, d8(PC, Xn) (not allowed for dest)
; MOVE.l, (xxx).w, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l (xxx).l, <ea>
; ---------------------------------------------------------

; MOVE.l, (xxx).l, Dn
	MOVE.l ($00000000).l,D0

; MOVE.l, (xxx).l, An (not allowed for dest)

; MOVE.l, (xxx).l, (An)
	MOVE.l ($ABCDEF00).l,(A5)

; MOVE.l, (xxx).l, (An)+
	MOVE.l ($7FFFFFFF).l,(A6)+

; MOVE.l, (xxx).l, -(An)
	MOVE.l ($55AA55AA).l,-(A7)

; MOVE.l, (xxx).l, d16(An)
	MOVE.l ($55AA55AA).l,-32768(A2)
	MOVE.l ($70000007).l,0(A3)
	MOVE.l ($000FF000).l,1234(A4)
	MOVE.l ($00000000).l,32767(A4)

; MOVE.l, (xxx).l, d8(An, Xn.L|W)
	MOVE.l ($FFFFFFFF).l,123(A2,A3.w)
	MOVE.l ($12345678).l,-1(A3,D2.l)
	MOVE.l ($00000001).l,-128(A4,D3.w)

; MOVE.l, (xxx).l, (xxx).w
	MOVE.l ($00000000).l,($0000).w
	MOVE.l ($12345678).l,($1234).w
	MOVE.l ($FFFFFFFF).l,($7FFF).w

; MOVE.l, (xxx).l, (xxx).l
	MOVE.l ($F0F0F0F0).l,($00000000).l
	MOVE.l ($0F0F0F0F).l,($12345678).l
	MOVE.l ($ABABABAB).l,($FFFFFFFF).l

; MOVE.l, (xxx).l, d16(PC) (not allowed for dest)
; MOVE.l, (xxx).l, d8(PC, Xn) (not allowed for dest)
; MOVE.l, (xxx).l, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l d16(PC), <ea>
; ---------------------------------------------------------

; MOVE.l, d16(PC), Dn
	MOVE.l 32767(PC),D0

; MOVE.l, d16(PC), An (not allowed for dest)

; MOVE.l, d16(PC), (An)
	MOVE.l -32768(PC),(A5)

; MOVE.l, d16(PC), (An)+
	MOVE.l 0(PC),(A6)+

; MOVE.l, d16(PC), -(An)
	MOVE.l 255(PC),-(A7)

; MOVE.l, d16(PC), d16(An)
	MOVE.l 123(PC),-32768(A2)
	MOVE.l 234(PC),0(A3)
	MOVE.l 345(PC),1234(A4)
	MOVE.l 567(PC),32767(A4)

; MOVE.l, d16(PC), d8(An, Xn.L|W)
	MOVE.l 1(PC),123(A2,A3.w)
	MOVE.l 2(PC),-1(A3,D2.l)
	MOVE.l 3(PC),-128(A4,D3.w)

; MOVE.l, d16(PC), (xxx).w
	MOVE.l -1(PC),($0000).w
	MOVE.l -2(PC),($1234).w
	MOVE.l -3(PC),($7FFF).w

; MOVE.l, d16(PC), (xxx).l
	MOVE.l 128(PC),($00000000).l
	MOVE.l 128(PC),($12345678).l
	MOVE.l 128(PC),($FFFFFFFF).l

; MOVE.l, d16(PC), d16(PC) (not allowed for dest)
; MOVE.l, d16(PC), d8(PC, Xn) (not allowed for dest)
; MOVE.l, d16(PC), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l d8(PC,Xn.L|W), <ea>
; ---------------------------------------------------------

; MOVE.l, d8(PC,Xn.L|W), Dn
	MOVE.l 127(PC,D0.w),D0

; MOVE.l, d8(PC,Xn.L|W), An (not allowed for dest)

; MOVE.l, d8(PC,Xn.L|W), (An)
	MOVE.l -1(PC,D0.l),(A5)

; MOVE.l, d8(PC,Xn.L|W), (An)+
	MOVE.l 0(PC,A0.w),(A6)+

; MOVE.l, d8(PC,Xn.L|W), -(An)
	MOVE.l -128(PC,A0.l),-(A7)

; MOVE.l, d8(PC,Xn.L|W), d16(An)
	MOVE.l 64(PC,D1.w),-32768(A2)
	MOVE.l 64(PC,D2.w),0(A3)
	MOVE.l -64(PC,A3.l),1234(A4)
	MOVE.l -64(PC,A4.l),32767(A4)

; MOVE.l, d8(PC,Xn.L|W), d8(An, Xn.L|W)
	MOVE.l 1(PC,D0.l),123(A2,A3.w)
	MOVE.l 2(PC,D1.l),-1(A3,D2.l)
	MOVE.l 3(PC,A7.l),-128(A4,D3.w)

; MOVE.l, d8(PC,Xn.L|W), (xxx).w
	MOVE.l -1(PC,D5.w),($0000).w
	MOVE.l -2(PC,D6.w),($1234).w
	MOVE.l -3(PC,D7.w),($7FFF).w

; MOVE.l, d8(PC,Xn.L|W), (xxx).l
	MOVE.l 127(PC,D3.l),($00000000).l
	MOVE.l -128(PC,D4.l),($12345678).l
	MOVE.l 127(PC,D5.l),($FFFFFFFF).l

; MOVE.l, d8(PC,Xn.L|W), d16(PC) (not allowed for dest)
; MOVE.l, d8(PC,Xn.L|W), d8(PC, Xn) (not allowed for dest)
; MOVE.l, d8(PC,Xn.L|W), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.l Imm, <ea>
; ---------------------------------------------------------

; MOVE.l, Imm, Dn
	MOVE.l #$00000000,D0

; MOVE.l, Imm, An (not allowed for dest)

; MOVE.l, Imm, (An)
	MOVE.l #$FFFFFFFF,(A5)

; MOVE.l, Imm, (An)+
	MOVE.l #$7F7FA0A0,(A6)+

; MOVE.l, Imm, -(An)
	MOVE.l #$55AAAA55,-(A7)

; MOVE.l, Imm, d16(An)
	MOVE.l #$55550000,-32768(A2)
	MOVE.l #$70707070,0(A3)
	MOVE.l #$0F0FF0F0,1234(A4)
	MOVE.l #$80088008,32767(A4)

; MOVE.l, Imm, d8(An, Xn.L|W)
	MOVE.l #$FF0000FF,123(A2,A3.w)
	MOVE.l #$34433443,-1(A3,D2.l)
	MOVE.l #$01010101,-128(A4,D3.w)

; MOVE.l, Imm, (xxx).w
	MOVE.l #$00000001,($0000).w
	MOVE.l #$12345678,($1234).w
	MOVE.l #$AABBCCDD,($7FFF).w

; MOVE.l, Imm, (xxx).l
	MOVE.l #$BBCCDDEE,($00000000).l
	MOVE.l #$CCDDEEFF,($12345678).l
	MOVE.l #$DDEEFF00,($FFFFFFFF).l

; MOVE.l, Imm, d16(PC) (not allowed for dest)
; MOVE.l, Imm, d8(PC, Xn) (not allowed for dest)
; MOVE.l, Imm, Imm (not allowed for dest)