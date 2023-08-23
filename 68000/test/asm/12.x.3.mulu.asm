	mc68000
	opt o-

; =========================================================
; MULU.w <ea>,Dn
; =========================================================

; MULU.w, Dn,Dn
	MULU.w D7,D0
	MULU.w D6,D1
	MULU.w D5,D2
	MULU.w D4,D3
	MULU.w D3,D4
	MULU.w D2,D5
	MULU.w D1,D6
	MULU.w D0,D7

; MULU.w, An,Dn (not allowed)

; MULU.w, (An),Dn
	MULU.w (A5),D0

; MULU.w, (An)+,Dn
	MULU.w (A6)+,D1

; MULU.w, -(An),Dn
	MULU.w -(A7),D2

; MULU.w, d16(An),Dn
	MULU.w -32768(A2),D3
	MULU.w 0(A3),D4
	MULU.w 1234(A4),D5
	MULU.w 32767(A4),D6

; MULU.w, d8(An, Xn.L|W),Dn
	MULU.w 123(A2,A3.w),D7
	MULU.w -1(A3,D2.l),D0
	MULU.w -128(A4,D3.w),D1

; MULU.w, (xxx).w,Dn
	MULU.w ($0000).w,D2
	MULU.w ($1234).w,D3
	MULU.w ($7FFF).w,D4

; MULU.w, (xxx).l,Dn
	MULU.w ($00000000).l,D5
	MULU.w ($12345678).l,D6
	MULU.w ($FFFFFFFF).l,D7

; MULU.w, d16(PC),Dn
	MULU.w -32768(PC),D0
	MULU.w 0(PC),D1
	MULU.w 1234(PC),D2
	MULU.w 32767(PC),D3

; MULU.w, d8(PC, Xn),Dn
	MULU.w 123(PC,A3.w),D4
	MULU.w -1(PC,D2.l),D5
	MULU.w -128(PC,D3.w),D6

; MULU.w, Imm,Dn
	MULU.w #$0000,D0
	MULU.w #$8000,D1
	MULU.w #$FFFF,D2
	MULU.w #$FF00,D3
	MULU.w #$00FF,D4
	MULU.w #$F0F0,D5
	MULU.w #$AA55,D6
	MULU.w #$1234,D7
