	mc68000
	opt o-

; =========================================================
; SBCD.b Dy, Dx
; =========================================================

	SBCD.b D7,D0
	SBCD.b D6,D1
	SBCD.b D5,D2
	SBCD.b D4,D3
	SBCD.b D3,D4
	SBCD.b D2,D5
	SBCD.b D1,D6
	SBCD.b D0,D7

; =========================================================
; SBCD.b -(Ay), -(Ax)
; =========================================================

	SBCD.b -(A7),-(A0)
	SBCD.b -(A6),-(A1)
	SBCD.b -(A5),-(A2)
	SBCD.b -(A4),-(A3)
	SBCD.b -(A3),-(A4)
	SBCD.b -(A2),-(A5)
	SBCD.b -(A1),-(A6)
	SBCD.b -(A0),-(A7)
