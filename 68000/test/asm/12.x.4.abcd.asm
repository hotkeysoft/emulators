	mc68000
	opt o-

; =========================================================
; MULU.b Dy, Dx
; =========================================================

	ABCD.b D7,D0
	ABCD.b D6,D1
	ABCD.b D5,D2
	ABCD.b D4,D3
	ABCD.b D3,D4
	ABCD.b D2,D5
	ABCD.b D1,D6
	ABCD.b D0,D7

; =========================================================
; ABCD.b -(Ay), -(Ax)
; =========================================================

	ABCD.b -(A7),-(A0)
	ABCD.b -(A6),-(A1)
	ABCD.b -(A5),-(A2)
	ABCD.b -(A4),-(A3)
	ABCD.b -(A3),-(A4)
	ABCD.b -(A2),-(A5)
	ABCD.b -(A1),-(A6)
	ABCD.b -(A0),-(A7)