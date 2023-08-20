	mc68000
	opt o-

; =========================================================
; MOVE.b <ea>, <ea>
; =========================================================


; ---------------------------------------------------------
; MOVE.b Dn, <ea>
; ---------------------------------------------------------

; MOVE.b, Dn, Dn
	MOVE.b D0,D0

; MOVE.b, Dn, An (not allowed for dest)

; MOVE.b, Dn, (An)
	MOVE.b D1,(A5)

; MOVE.b, Dn, (An)+
	MOVE.b D2,(A6)+

; MOVE.b, Dn, -(An)
	MOVE.b D3,-(A7)

; MOVE.b, Dn, d16(An)
	MOVE.b D2,-32768(A2)
	MOVE.b D2,0(A3)
	MOVE.b D3,1234(A4)
	MOVE.b D3,32767(A4)

; MOVE.b, Dn, d8(An, Xn.L|W)
	MOVE.b D2,123(A2,A3.w)
	MOVE.b D2,-1(A3,D2.l)
	MOVE.b D3,-128(A4,D3.w)

; MOVE.b, Dn, (xxx).w
	MOVE.b D0,($0000).w
	MOVE.b D1,($1234).w
	MOVE.b D2,($7FFF).w

; MOVE.b, Dn, (xxx).l
	MOVE.b D3,($00000000).l
	MOVE.b D4,($12345678).l
	MOVE.b D5,($FFFFFFFF).l

; MOVE.b, Dn, d16(PC) (not allowed for dest)
; MOVE.b, Dn, d8(PC, Xn) (not allowed for dest)
; MOVE.b, Dn, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b (An), <ea>
; ---------------------------------------------------------

; MOVE.b, (An), Dn
	MOVE.b (A1),D0

; MOVE.b, (An), An (not allowed for dest)

; MOVE.b, (An), (An)
	MOVE.b (A2),(A5)

; MOVE.b, (An), (An)+
	MOVE.b (A3),(A6)+

; MOVE.b, (An), -(An)
	MOVE.b (A4),-(A7)

; MOVE.b, (An), d16(An)
	MOVE.b (A2),-32768(A2)
	MOVE.b (A3),0(A3)
	MOVE.b (A4),1234(A4)
	MOVE.b (A5),32767(A4)

; MOVE.b, (An), d8(An, Xn.L|W)
	MOVE.b (A5),123(A2,A3.w)
	MOVE.b (A6),-1(A3,D2.l)
	MOVE.b (A7),-128(A4,D3.w)

; MOVE.b, (An), (xxx).w
	MOVE.b (A0),($0000).w
	MOVE.b (A1),($1234).w
	MOVE.b (A2),($7FFF).w

; MOVE.b, (An), (xxx).l
	MOVE.b (A3),($00000000).l
	MOVE.b (A4),($12345678).l
	MOVE.b (A5),($FFFFFFFF).l

; MOVE.b, (An), d16(PC) (not allowed for dest)
; MOVE.b, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.b, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b (An)+, <ea>
; ---------------------------------------------------------

; MOVE.b, (An)+, Dn
	MOVE.b (A1)+,D0

; MOVE.b, (An)+, An (not allowed for dest)

; MOVE.b, (An)+, (An)
	MOVE.b (A2)+,(A5)

; MOVE.b, (An)+, (An)+
	MOVE.b (A3)+,(A6)+

; MOVE.b, (An)+, -(An)
	MOVE.b (A4)+,-(A7)

; MOVE.b, (An)+, d16(An)
	MOVE.b (A2)+,-32768(A2)
	MOVE.b (A3)+,0(A3)
	MOVE.b (A4)+,1234(A4)
	MOVE.b (A5)+,32767(A4)

; MOVE.b, (An)+, d8(An, Xn.L|W)
	MOVE.b (A5)+,123(A2,A3.w)
	MOVE.b (A6)+,-1(A3,D2.l)
	MOVE.b (A7)+,-128(A4,D3.w)

; MOVE.b, (An)+, (xxx).w
	MOVE.b (A0)+,($0000).w
	MOVE.b (A1)+,($1234).w
	MOVE.b (A2)+,($7FFF).w

; MOVE.b, (An)+, (xxx).l
	MOVE.b (A3)+,($00000000).l
	MOVE.b (A4)+,($12345678).l
	MOVE.b (A5)+,($FFFFFFFF).l

; MOVE.b, (An)+, d16(PC) (not allowed for dest)
; MOVE.b, (An)+, d8(PC, Xn) (not allowed for dest)
; MOVE.b, (An)+, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b -(An), <ea>
; ---------------------------------------------------------

; MOVE.b, -(An), Dn
	MOVE.b -(A1),D0

; MOVE.b, -(An), An (not allowed for dest)

; MOVE.b, -(An), (An)
	MOVE.b -(A2),(A5)

; MOVE.b, -(An), (An)+
	MOVE.b -(A3),(A6)+

; MOVE.b, -(An), -(An)
	MOVE.b -(A4),-(A7)

; MOVE.b, -(An), d16(An)
	MOVE.b -(A2),-32768(A2)
	MOVE.b -(A3),0(A3)
	MOVE.b -(A4),1234(A4)
	MOVE.b -(A5),32767(A4)

; MOVE.b, -(An), d8(An, Xn.L|W)
	MOVE.b -(A5),123(A2,A3.w)
	MOVE.b -(A6),-1(A3,D2.l)
	MOVE.b -(A7),-128(A4,D3.w)

; MOVE.b, -(An), (xxx).w
	MOVE.b -(A0),($0000).w
	MOVE.b -(A1),($1234).w
	MOVE.b -(A2),($7FFF).w

; MOVE.b, -(An), (xxx).l
	MOVE.b -(A3),($00000000).l
	MOVE.b -(A4),($12345678).l
	MOVE.b -(A5),($FFFFFFFF).l

; MOVE.b, -(An), d16(PC) (not allowed for dest)
; MOVE.b, -(An), d8(PC, Xn) (not allowed for dest)
; MOVE.b, -(An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b d16(An), <ea>
; ---------------------------------------------------------

; MOVE.b, d16(An), Dn
	MOVE.b 32767(A1),D0

; MOVE.b, d16(An), An (not allowed for dest)

; MOVE.b, d16(An), (An)
	MOVE.b -32768(A2),(A5)

; MOVE.b, d16(An), (An)+
	MOVE.b 0(A3),(A6)+

; MOVE.b, d16(An), -(An)
	MOVE.b 255(A4),-(A7)

; MOVE.b, d16(An), d16(An)
	MOVE.b 123(A2),-32768(A2)
	MOVE.b 234(A3),0(A3)
	MOVE.b 345(A4),1234(A4)
	MOVE.b 567(A5),32767(A4)

; MOVE.b, d16(An), d8(An, Xn.L|W)
	MOVE.b 1(A5),123(A2,A3.w)
	MOVE.b 2(A6),-1(A3,D2.l)
	MOVE.b 3(A7),-128(A4,D3.w)

; MOVE.b, d16(An), (xxx).w
	MOVE.b -1(A0),($0000).w
	MOVE.b -2(A1),($1234).w
	MOVE.b -3(A2),($7FFF).w

; MOVE.b, d16(An), (xxx).l
	MOVE.b 128(A3),($00000000).l
	MOVE.b 128(A4),($12345678).l
	MOVE.b 128(A5),($FFFFFFFF).l

; MOVE.b, (An), d16(PC) (not allowed for dest)
; MOVE.b, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.b, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b d8(An,Xn.L|W), <ea>
; ---------------------------------------------------------

; MOVE.b, d8(An,Xn.L|W), Dn
	MOVE.b 127(A1,D0.w),D0

; MOVE.b, d8(An,Xn.L|W), An (not allowed for dest)

; MOVE.b, d8(An,Xn.L|W), (An)
	MOVE.b -1(A2,D0.l),(A5)

; MOVE.b, d8(An,Xn.L|W), (An)+
	MOVE.b 0(A3,A0.w),(A6)+

; MOVE.b, d8(An,Xn.L|W), -(An)
	MOVE.b -128(A4,A0.l),-(A7)

; MOVE.b, d8(An,Xn.L|W), d16(An)
	MOVE.b 64(A2,D1.w),-32768(A2)
	MOVE.b 64(A3,D2.w),0(A3)
	MOVE.b -64(A4,A3.l),1234(A4)
	MOVE.b -64(A5,A4.l),32767(A4)

; MOVE.b, d8(An,Xn.L|W), d8(An, Xn.L|W)
	MOVE.b 1(A5,D0.l),123(A2,A3.w)
	MOVE.b 2(A6,D1.l),-1(A3,D2.l)
	MOVE.b 3(A7,A7.l),-128(A4,D3.w)

; MOVE.b, d8(An,Xn.L|W), (xxx).w
	MOVE.b -1(A0,D5.w),($0000).w
	MOVE.b -2(A1,D6.w),($1234).w
	MOVE.b -3(A2,D7.w),($7FFF).w

; MOVE.b, d8(An,Xn.L|W), (xxx).l
	MOVE.b 127(A3,D3.l),($00000000).l
	MOVE.b -128(A4,D4.l),($12345678).l
	MOVE.b 127(A5,D5.l),($FFFFFFFF).l

; MOVE.b, (An), d16(PC) (not allowed for dest)
; MOVE.b, (An), d8(PC, Xn) (not allowed for dest)
; MOVE.b, (An), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b (xxx).w, <ea>
; ---------------------------------------------------------

; MOVE.b, (xxx).w, Dn
	MOVE.b ($0000).w,D0

; MOVE.b, (xxx).w, An (not allowed for dest)

; MOVE.b, (xxx).w, (An)
	MOVE.b ($0BCD).w,(A5)

; MOVE.b, (xxx).w, (An)+
	MOVE.b ($7FFF).w,(A6)+

; MOVE.b, (xxx).w, -(An)
	MOVE.b ($55AA).w,-(A7)

; MOVE.b, (xxx).w, d16(An)
	MOVE.b ($55AA).w,-32768(A2)
	MOVE.b ($7000).w,0(A3)
	MOVE.b ($000F).w,1234(A4)
	MOVE.b ($0000).w,32767(A4)

; MOVE.b, (xxx).w, d8(An, Xn.L|W)
	MOVE.b ($7FFF).w,123(A2,A3.w)
	MOVE.b ($1234).w,-1(A3,D2.l)
	MOVE.b ($0000).w,-128(A4,D3.w)

; MOVE.b, (xxx).w, (xxx).w
	MOVE.b ($0000).w,($0000).w
	MOVE.b ($1234).w,($1234).w
	MOVE.b ($7FFF).w,($7FFF).w

; MOVE.b, (xxx).w, (xxx).l
	MOVE.b ($7FFF).w,($00000000).l
	MOVE.b ($2345).w,($12345678).l
	MOVE.b ($0000).w,($FFFFFFFF).l

; MOVE.b, (xxx).w, d16(PC) (not allowed for dest)
; MOVE.b, (xxx).w, d8(PC, Xn) (not allowed for dest)
; MOVE.b, (xxx).w, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b (xxx).l, <ea>
; ---------------------------------------------------------

; MOVE.b, (xxx).l, Dn
	MOVE.b ($00000000).l,D0

; MOVE.b, (xxx).l, An (not allowed for dest)

; MOVE.b, (xxx).l, (An)
	MOVE.b ($ABCDEF00).l,(A5)

; MOVE.b, (xxx).l, (An)+
	MOVE.b ($7FFFFFFF).l,(A6)+

; MOVE.b, (xxx).l, -(An)
	MOVE.b ($55AA55AA).l,-(A7)

; MOVE.b, (xxx).l, d16(An)
	MOVE.b ($55AA55AA).l,-32768(A2)
	MOVE.b ($70000007).l,0(A3)
	MOVE.b ($000FF000).l,1234(A4)
	MOVE.b ($00000000).l,32767(A4)

; MOVE.b, (xxx).l, d8(An, Xn.L|W)
	MOVE.b ($FFFFFFFF).l,123(A2,A3.w)
	MOVE.b ($12345678).l,-1(A3,D2.l)
	MOVE.b ($00000001).l,-128(A4,D3.w)

; MOVE.b, (xxx).l, (xxx).w
	MOVE.b ($00000000).l,($0000).w
	MOVE.b ($12345678).l,($1234).w
	MOVE.b ($FFFFFFFF).l,($7FFF).w

; MOVE.b, (xxx).l, (xxx).l
	MOVE.b ($F0F0F0F0).l,($00000000).l
	MOVE.b ($0F0F0F0F).l,($12345678).l
	MOVE.b ($ABABABAB).l,($FFFFFFFF).l

; MOVE.b, (xxx).l, d16(PC) (not allowed for dest)
; MOVE.b, (xxx).l, d8(PC, Xn) (not allowed for dest)
; MOVE.b, (xxx).l, Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b d16(PC), <ea>
; ---------------------------------------------------------

; MOVE.b, d16(PC), Dn
	MOVE.b 32767(PC),D0

; MOVE.b, d16(PC), An (not allowed for dest)

; MOVE.b, d16(PC), (An)
	MOVE.b -32768(PC),(A5)

; MOVE.b, d16(PC), (An)+
	MOVE.b 0(PC),(A6)+

; MOVE.b, d16(PC), -(An)
	MOVE.b 255(PC),-(A7)

; MOVE.b, d16(PC), d16(An)
	MOVE.b 123(PC),-32768(A2)
	MOVE.b 234(PC),0(A3)
	MOVE.b 345(PC),1234(A4)
	MOVE.b 567(PC),32767(A4)

; MOVE.b, d16(PC), d8(An, Xn.L|W)
	MOVE.b 1(PC),123(A2,A3.w)
	MOVE.b 2(PC),-1(A3,D2.l)
	MOVE.b 3(PC),-128(A4,D3.w)

; MOVE.b, d16(PC), (xxx).w
	MOVE.b -1(PC),($0000).w
	MOVE.b -2(PC),($1234).w
	MOVE.b -3(PC),($7FFF).w

; MOVE.b, d16(PC), (xxx).l
	MOVE.b 128(PC),($00000000).l
	MOVE.b 128(PC),($12345678).l
	MOVE.b 128(PC),($FFFFFFFF).l

; MOVE.b, d16(PC), d16(PC) (not allowed for dest)
; MOVE.b, d16(PC), d8(PC, Xn) (not allowed for dest)
; MOVE.b, d16(PC), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b d8(PC,Xn.L|W), <ea>
; ---------------------------------------------------------

; MOVE.b, d8(PC,Xn.L|W), Dn
	MOVE.b 127(PC,D0.w),D0

; MOVE.b, d8(PC,Xn.L|W), An (not allowed for dest)

; MOVE.b, d8(PC,Xn.L|W), (An)
	MOVE.b -1(PC,D0.l),(A5)

; MOVE.b, d8(PC,Xn.L|W), (An)+
	MOVE.b 0(PC,A0.w),(A6)+

; MOVE.b, d8(PC,Xn.L|W), -(An)
	MOVE.b -128(PC,A0.l),-(A7)

; MOVE.b, d8(PC,Xn.L|W), d16(An)
	MOVE.b 64(PC,D1.w),-32768(A2)
	MOVE.b 64(PC,D2.w),0(A3)
	MOVE.b -64(PC,A3.l),1234(A4)
	MOVE.b -64(PC,A4.l),32767(A4)

; MOVE.b, d8(PC,Xn.L|W), d8(An, Xn.L|W)
	MOVE.b 1(PC,D0.l),123(A2,A3.w)
	MOVE.b 2(PC,D1.l),-1(A3,D2.l)
	MOVE.b 3(PC,A7.l),-128(A4,D3.w)

; MOVE.b, d8(PC,Xn.L|W), (xxx).w
	MOVE.b -1(PC,D5.w),($0000).w
	MOVE.b -2(PC,D6.w),($1234).w
	MOVE.b -3(PC,D7.w),($7FFF).w

; MOVE.b, d8(PC,Xn.L|W), (xxx).l
	MOVE.b 127(PC,D3.l),($00000000).l
	MOVE.b -128(PC,D4.l),($12345678).l
	MOVE.b 127(PC,D5.l),($FFFFFFFF).l

; MOVE.b, d8(PC,Xn.L|W), d16(PC) (not allowed for dest)
; MOVE.b, d8(PC,Xn.L|W), d8(PC, Xn) (not allowed for dest)
; MOVE.b, d8(PC,Xn.L|W), Imm (not allowed for dest)

; ---------------------------------------------------------
; MOVE.b Imm, <ea>
; ---------------------------------------------------------

; MOVE.b, Imm, Dn
	MOVE.b #$00,D0

; MOVE.b, Imm, An (not allowed for dest)

; MOVE.b, Imm, (An)
	MOVE.b #$FF,(A5)

; MOVE.b, Imm, (An)+
	MOVE.b #$7F,(A6)+

; MOVE.b, Imm, -(An)
	MOVE.b #$55,-(A7)

; MOVE.b, Imm, d16(An)
	MOVE.b #$55,-32768(A2)
	MOVE.b #$70,0(A3)
	MOVE.b #$0F,1234(A4)
	MOVE.b #$80,32767(A4)

; MOVE.b, Imm, d8(An, Xn.L|W)
	MOVE.b #$FF,123(A2,A3.w)
	MOVE.b #$34,-1(A3,D2.l)
	MOVE.b #$00,-128(A4,D3.w)

; MOVE.b, Imm, (xxx).w
	MOVE.b #$00,($0000).w
	MOVE.b #$12,($1234).w
	MOVE.b #$AA,($7FFF).w

; MOVE.b, Imm, (xxx).l
	MOVE.b #$BB,($00000000).l
	MOVE.b #$CC,($12345678).l
	MOVE.b #$DD,($FFFFFFFF).l

; MOVE.b, Imm, d16(PC) (not allowed for dest)
; MOVE.b, Imm, d8(PC, Xn) (not allowed for dest)
; MOVE.b, Imm, Imm (not allowed for dest)