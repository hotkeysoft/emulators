	mc68000
	opt o-

; =========================================================
; MOVE.w <ea>, <ea>
; =========================================================


; ---------------------------------------------------------
; MOVE.w Dn, <ea>
; ---------------------------------------------------------

; MOVE.w, Dn, Dn
	MOVE.w D0,D0

; MOVE.w, Dn, An (not allowed for dest)

; MOVE.w, Dn, (An)
	MOVE.w D1,(A5)

; MOVE.w, Dn, (An)+
	MOVE.w D2,(A6)+

; MOVE.w, Dn, -(An)
	MOVE.w D3,-(A7)

; MOVE.w, Dn, d16(An)
	MOVE.w D2,-32768(A2)
	MOVE.w D2,0(A3)
	MOVE.w D3,1234(A4)
	MOVE.w D3,32767(A4)

; MOVE.w, Dn, d8(An, Xn.L|W)
	MOVE.w D2,123(A2,A3.w)
	MOVE.w D2,-1(A3,D2.l)
	MOVE.w D3,-128(A4,D3.w)

; MOVE.w, Dn, (xxx).w
	MOVE.w D0,($0000).w
	MOVE.w D1,($1234).w
	MOVE.w D2,($7FFF).w

; MOVE.w, Dn, (xxx).l
	MOVE.w D3,($00000000).l
	MOVE.w D4,($12345678).l
	MOVE.w D5,($FFFFFFFF).l

; MOVE.w, Dn, d16(PC) (not allowed for dest)
; MOVE.w, Dn, d8(PC, Xn) (not allowed for dest)
; MOVE.w, Dn, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w An, <ea>
; ---------------------------------------------------------

; MOVE.w, An, Dn
	MOVE.w A0,D0

; MOVE.w, An, An (not allowed for dest)

; MOVE.w, An, (An)
	MOVE.w A1,(A5)

; MOVE.w, An, (An)+
	MOVE.w A2,(A6)+

; MOVE.w, An, -(An)
	MOVE.w A3,-(A7)

; MOVE.w, An, d16(An)
	MOVE.w A2,-32768(A2)
	MOVE.w A2,0(A3)
	MOVE.w A3,1234(A4)
	MOVE.w A3,32767(A4)

; MOVE.w, An, d8(An, Xn.L|W)
	MOVE.w A2,123(A2,A3.w)
	MOVE.w A2,-1(A3,D2.l)
	MOVE.w A3,-128(A4,D3.w)

; MOVE.w, An, (xxx).w
	MOVE.w A0,($0000).w
	MOVE.w A1,($1234).w
	MOVE.w A2,($7FFF).w

; MOVE.w, An, (xxx).l
	MOVE.w A3,($00000000).l
	MOVE.w A4,($12345678).l
	MOVE.w A5,($FFFFFFFF).l

; MOVE.w, Dn, d16(PC) (not allowed for dest)
; MOVE.w, Dn, d8(PC, Xn) (not allowed for dest)
; MOVE.w, Dn, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w (An), <ea>
; ---------------------------------------------------------

; MOVE.w, (An), Dn
	MOVE.w (A1),D0

; MOVE.w, (An), An (not allowed for dest)

; MOVE.w, (An), (An)
	MOVE.w (A2),(A5)

; MOVE.w, (An), (An)+
	MOVE.w (A3),(A6)+

; MOVE.w, (An), -(An)
	MOVE.w (A4),-(A7)

; MOVE.w, (An), d16(An)
	MOVE.w (A2),-32768(A2)
	MOVE.w (A3),0(A3)
	MOVE.w (A4),1234(A4)
	MOVE.w (A5),32767(A4)

; MOVE.w, (An), d8(An, Xn.L|W)
	MOVE.w (A5),123(A2,A3.w)
	MOVE.w (A6),-1(A3,D2.l)
	MOVE.w (A7),-128(A4,D3.w)

; MOVE.w, (An), (xxx).w
	MOVE.w (A0),($0000).w
	MOVE.w (A1),($1234).w
	MOVE.w (A2),($7FFF).w

; MOVE.w, (An), (xxx).l
	MOVE.w (A3),($00000000).l
	MOVE.w (A4),($12345678).l
	MOVE.w (A5),($FFFFFFFF).l

; MOVE.w, (An), d16(PC) (not allowed for dest)
; MOVE.w, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.w, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w (An)+, <ea>
; ---------------------------------------------------------

; MOVE.w, (An)+, Dn
	MOVE.w (A1)+,D0

; MOVE.w, (An)+, An (not allowed for dest)

; MOVE.w, (An)+, (An)
	MOVE.w (A2)+,(A5)

; MOVE.w, (An)+, (An)+
	MOVE.w (A3)+,(A6)+

; MOVE.w, (An)+, -(An)
	MOVE.w (A4)+,-(A7)

; MOVE.w, (An)+, d16(An)
	MOVE.w (A2)+,-32768(A2)
	MOVE.w (A3)+,0(A3)
	MOVE.w (A4)+,1234(A4)
	MOVE.w (A5)+,32767(A4)

; MOVE.w, (An)+, d8(An, Xn.L|W)
	MOVE.w (A5)+,123(A2,A3.w)
	MOVE.w (A6)+,-1(A3,D2.l)
	MOVE.w (A7)+,-128(A4,D3.w)

; MOVE.w, (An)+, (xxx).w
	MOVE.w (A0)+,($0000).w
	MOVE.w (A1)+,($1234).w
	MOVE.w (A2)+,($7FFF).w

; MOVE.w, (An)+, (xxx).l
	MOVE.w (A3)+,($00000000).l
	MOVE.w (A4)+,($12345678).l
	MOVE.w (A5)+,($FFFFFFFF).l

; MOVE.w, (An)+, d16(PC) (not allowed for dest)
; MOVE.w, (An)+, d8(PC, Xn) (not allowed for dest)
; MOVE.w, (An)+, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w -(An), <ea>
; ---------------------------------------------------------

; MOVE.w, -(An), Dn
	MOVE.w -(A1),D0

; MOVE.w, -(An), An (not allowed for dest)

; MOVE.w, -(An), (An)
	MOVE.w -(A2),(A5)

; MOVE.w, -(An), (An)+
	MOVE.w -(A3),(A6)+

; MOVE.w, -(An), -(An)
	MOVE.w -(A4),-(A7)

; MOVE.w, -(An), d16(An)
	MOVE.w -(A2),-32768(A2)
	MOVE.w -(A3),0(A3)
	MOVE.w -(A4),1234(A4)
	MOVE.w -(A5),32767(A4)

; MOVE.w, -(An), d8(An, Xn.L|W)
	MOVE.w -(A5),123(A2,A3.w)
	MOVE.w -(A6),-1(A3,D2.l)
	MOVE.w -(A7),-128(A4,D3.w)

; MOVE.w, -(An), (xxx).w
	MOVE.w -(A0),($0000).w
	MOVE.w -(A1),($1234).w
	MOVE.w -(A2),($7FFF).w

; MOVE.w, -(An), (xxx).l
	MOVE.w -(A3),($00000000).l
	MOVE.w -(A4),($12345678).l
	MOVE.w -(A5),($FFFFFFFF).l

; MOVE.w, -(An), d16(PC) (not allowed for dest)
; MOVE.w, -(An), d8(PC, Xn) (not allowed for dest)
; MOVE.w, -(An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w d16(An), <ea>
; ---------------------------------------------------------

; MOVE.w, d16(An), Dn
	MOVE.w 32767(A1),D0

; MOVE.w, d16(An), An (not allowed for dest)

; MOVE.w, d16(An), (An)
	MOVE.w -32768(A2),(A5)

; MOVE.w, d16(An), (An)+
	MOVE.w 0(A3),(A6)+

; MOVE.w, d16(An), -(An)
	MOVE.w 255(A4),-(A7)

; MOVE.w, d16(An), d16(An)
	MOVE.w 123(A2),-32768(A2)
	MOVE.w 234(A3),0(A3)
	MOVE.w 345(A4),1234(A4)
	MOVE.w 567(A5),32767(A4)

; MOVE.w, d16(An), d8(An, Xn.L|W)
	MOVE.w 1(A5),123(A2,A3.w)
	MOVE.w 2(A6),-1(A3,D2.l)
	MOVE.w 3(A7),-128(A4,D3.w)

; MOVE.w, d16(An), (xxx).w
	MOVE.w -1(A0),($0000).w
	MOVE.w -2(A1),($1234).w
	MOVE.w -3(A2),($7FFF).w

; MOVE.w, d16(An), (xxx).l
	MOVE.w 128(A3),($00000000).l
	MOVE.w 128(A4),($12345678).l
	MOVE.w 128(A5),($FFFFFFFF).l

; MOVE.w, (An), d16(PC) (not allowed for dest)
; MOVE.w, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.w, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w d8(An,Xn.L|W), <ea>
; ---------------------------------------------------------

; MOVE.w, d8(An,Xn.L|W), Dn
	MOVE.w 127(A1,D0.w),D0

; MOVE.w, d8(An,Xn.L|W), An (not allowed for dest)

; MOVE.w, d8(An,Xn.L|W), (An)
	MOVE.w -1(A2,D0.l),(A5)

; MOVE.w, d8(An,Xn.L|W), (An)+
	MOVE.w 0(A3,A0.w),(A6)+

; MOVE.w, d8(An,Xn.L|W), -(An)
	MOVE.w -128(A4,A0.l),-(A7)

; MOVE.w, d8(An,Xn.L|W), d16(An)
	MOVE.w 64(A2,D1.w),-32768(A2)
	MOVE.w 64(A3,D2.w),0(A3)
	MOVE.w -64(A4,A3.l),1234(A4)
	MOVE.w -64(A5,A4.l),32767(A4)

; MOVE.w, d8(An,Xn.L|W), d8(An, Xn.L|W)
	MOVE.w 1(A5,D0.l),123(A2,A3.w)
	MOVE.w 2(A6,D1.l),-1(A3,D2.l)
	MOVE.w 3(A7,A7.l),-128(A4,D3.w)

; MOVE.w, d8(An,Xn.L|W), (xxx).w
	MOVE.w -1(A0,D5.w),($0000).w
	MOVE.w -2(A1,D6.w),($1234).w
	MOVE.w -3(A2,D7.w),($7FFF).w

; MOVE.w, d8(An,Xn.L|W), (xxx).l
	MOVE.w 127(A3,D3.l),($00000000).l
	MOVE.w -128(A4,D4.l),($12345678).l
	MOVE.w 127(A5,D5.l),($FFFFFFFF).l

; MOVE.w, (An), d16(PC) (not allowed for dest)
; MOVE.w, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.w, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w (xxx).w, <ea>
; ---------------------------------------------------------

; MOVE.w, (xxx).w, Dn
	MOVE.w ($0000).w,D0

; MOVE.w, (xxx).w, An (not allowed for dest)

; MOVE.w, (xxx).w, (An)
	MOVE.w ($0BCD).w,(A5)

; MOVE.w, (xxx).w, (An)+
	MOVE.w ($7FFF).w,(A6)+

; MOVE.w, (xxx).w, -(An)
	MOVE.w ($55AA).w,-(A7)

; MOVE.w, (xxx).w, d16(An)
	MOVE.w ($55AA).w,-32768(A2)
	MOVE.w ($7000).w,0(A3)
	MOVE.w ($000F).w,1234(A4)
	MOVE.w ($0000).w,32767(A4)

; MOVE.w, (xxx).w, d8(An, Xn.L|W)
	MOVE.w ($7FFF).w,123(A2,A3.w)
	MOVE.w ($1234).w,-1(A3,D2.l)
	MOVE.w ($0000).w,-128(A4,D3.w)

; MOVE.w, (xxx).w, (xxx).w
	MOVE.w ($0000).w,($0000).w
	MOVE.w ($1234).w,($1234).w
	MOVE.w ($7FFF).w,($7FFF).w

; MOVE.w, (xxx).w, (xxx).l
	MOVE.w ($7FFF).w,($00000000).l
	MOVE.w ($2345).w,($12345678).l
	MOVE.w ($0000).w,($FFFFFFFF).l

; MOVE.w, (xxx).w, d16(PC) (not allowed for dest)
; MOVE.w, (xxx).w, d8(PC, Xn) (not allowed for dest)
; MOVE.w, (xxx).w, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w (xxx).l, <ea>
; ---------------------------------------------------------

; MOVE.w, (xxx).l, Dn
	MOVE.w ($00000000).l,D0

; MOVE.w, (xxx).l, An (not allowed for dest)

; MOVE.w, (xxx).l, (An)
	MOVE.w ($ABCDEF00).l,(A5)

; MOVE.w, (xxx).l, (An)+
	MOVE.w ($7FFFFFFF).l,(A6)+

; MOVE.w, (xxx).l, -(An)
	MOVE.w ($55AA55AA).l,-(A7)

; MOVE.w, (xxx).l, d16(An)
	MOVE.w ($55AA55AA).l,-32768(A2)
	MOVE.w ($70000007).l,0(A3)
	MOVE.w ($000FF000).l,1234(A4)
	MOVE.w ($00000000).l,32767(A4)

; MOVE.w, (xxx).l, d8(An, Xn.L|W)
	MOVE.w ($FFFFFFFF).l,123(A2,A3.w)
	MOVE.w ($12345678).l,-1(A3,D2.l)
	MOVE.w ($00000001).l,-128(A4,D3.w)

; MOVE.w, (xxx).l, (xxx).w
	MOVE.w ($00000000).l,($0000).w
	MOVE.w ($12345678).l,($1234).w
	MOVE.w ($FFFFFFFF).l,($7FFF).w

; MOVE.w, (xxx).l, (xxx).l
	MOVE.w ($F0F0F0F0).l,($00000000).l
	MOVE.w ($0F0F0F0F).l,($12345678).l
	MOVE.w ($ABABABAB).l,($FFFFFFFF).l

; MOVE.w, (xxx).l, d16(PC) (not allowed for dest)
; MOVE.w, (xxx).l, d8(PC, Xn) (not allowed for dest)
; MOVE.w, (xxx).l, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w d16(PC), <ea>
; ---------------------------------------------------------

; MOVE.w, d16(PC), Dn
	MOVE.w 32767(PC),D0

; MOVE.w, d16(PC), An (not allowed for dest)

; MOVE.w, d16(PC), (An)
	MOVE.w -32768(PC),(A5)

; MOVE.w, d16(PC), (An)+
	MOVE.w 0(PC),(A6)+

; MOVE.w, d16(PC), -(An)
	MOVE.w 255(PC),-(A7)

; MOVE.w, d16(PC), d16(An)
	MOVE.w 123(PC),-32768(A2)
	MOVE.w 234(PC),0(A3)
	MOVE.w 345(PC),1234(A4)
	MOVE.w 567(PC),32767(A4)

; MOVE.w, d16(PC), d8(An, Xn.L|W)
	MOVE.w 1(PC),123(A2,A3.w)
	MOVE.w 2(PC),-1(A3,D2.l)
	MOVE.w 3(PC),-128(A4,D3.w)

; MOVE.w, d16(PC), (xxx).w
	MOVE.w -1(PC),($0000).w
	MOVE.w -2(PC),($1234).w
	MOVE.w -3(PC),($7FFF).w

; MOVE.w, d16(PC), (xxx).l
	MOVE.w 128(PC),($00000000).l
	MOVE.w 128(PC),($12345678).l
	MOVE.w 128(PC),($FFFFFFFF).l

; MOVE.w, d16(PC), d16(PC) (not allowed for dest)
; MOVE.w, d16(PC), d8(PC, Xn) (not allowed for dest)
; MOVE.w, d16(PC), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w d8(PC,Xn.L|W), <ea>
; ---------------------------------------------------------

; MOVE.w, d8(PC,Xn.L|W), Dn
	MOVE.w 127(PC,D0.w),D0

; MOVE.w, d8(PC,Xn.L|W), An (not allowed for dest)

; MOVE.w, d8(PC,Xn.L|W), (An)
	MOVE.w -1(PC,D0.l),(A5)

; MOVE.w, d8(PC,Xn.L|W), (An)+
	MOVE.w 0(PC,A0.w),(A6)+

; MOVE.w, d8(PC,Xn.L|W), -(An)
	MOVE.w -128(PC,A0.l),-(A7)

; MOVE.w, d8(PC,Xn.L|W), d16(An)
	MOVE.w 64(PC,D1.w),-32768(A2)
	MOVE.w 64(PC,D2.w),0(A3)
	MOVE.w -64(PC,A3.l),1234(A4)
	MOVE.w -64(PC,A4.l),32767(A4)

; MOVE.w, d8(PC,Xn.L|W), d8(An, Xn.L|W)
	MOVE.w 1(PC,D0.l),123(A2,A3.w)
	MOVE.w 2(PC,D1.l),-1(A3,D2.l)
	MOVE.w 3(PC,A7.l),-128(A4,D3.w)

; MOVE.w, d8(PC,Xn.L|W), (xxx).w
	MOVE.w -1(PC,D5.w),($0000).w
	MOVE.w -2(PC,D6.w),($1234).w
	MOVE.w -3(PC,D7.w),($7FFF).w

; MOVE.w, d8(PC,Xn.L|W), (xxx).l
	MOVE.w 127(PC,D3.l),($00000000).l
	MOVE.w -128(PC,D4.l),($12345678).l
	MOVE.w 127(PC,D5.l),($FFFFFFFF).l

; MOVE.w, d8(PC,Xn.L|W), d16(PC) (not allowed for dest)
; MOVE.w, d8(PC,Xn.L|W), d8(PC, Xn) (not allowed for dest)
; MOVE.w, d8(PC,Xn.L|W), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.w Imm, <ea>
; ---------------------------------------------------------

; MOVE.w, Imm, Dn
	MOVE.w #$0000,D0

; MOVE.w, Imm, An (not allowed for dest)

; MOVE.w, Imm, (An)
	MOVE.w #$FFFF,(A5)

; MOVE.w, Imm, (An)+
	MOVE.w #$A0A0,(A6)+

; MOVE.w, Imm, -(An)
	MOVE.w #$55AA,-(A7)

; MOVE.w, Imm, d16(An)
	MOVE.w #$5555,-32768(A2)
	MOVE.w #$7070,0(A3)
	MOVE.w #$0F0F,1234(A4)
	MOVE.w #$8008,32767(A4)

; MOVE.w, Imm, d8(An, Xn.L|W)
	MOVE.w #$FF00,123(A2,A3.w)
	MOVE.w #$3443,-1(A3,D2.l)
	MOVE.w #$0101,-128(A4,D3.w)

; MOVE.w, Imm, (xxx).w
	MOVE.w #$0000,($0000).w
	MOVE.w #$1234,($1234).w
	MOVE.w #$AABB,($7FFF).w

; MOVE.w, Imm, (xxx).l
	MOVE.w #$BBCC,($00000000).l
	MOVE.w #$CCDD,($12345678).l
	MOVE.w #$DDEE,($FFFFFFFF).l

; MOVE.w, Imm, d16(PC) (not allowed for dest)
; MOVE.w, Imm, d8(PC, Xn) (not allowed for dest)
; MOVE.w, Imm, Imm (not allowed for dest)