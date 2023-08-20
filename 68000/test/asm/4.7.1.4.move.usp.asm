	mc68000
	opt o-

; =========================================================
; MOVE USP, An
; =========================================================

	MOVE.l USP,A0
	MOVE.l USP,A1
	MOVE.l USP,A2
	MOVE.l USP,A3
	MOVE.l USP,A4
	MOVE.l USP,A5
	MOVE.l USP,A6
	MOVE.l USP,A7

; =========================================================
; MOVE An, USP
; =========================================================

	MOVE.l A0,USP
	MOVE.l A1,USP
	MOVE.l A2,USP
	MOVE.l A3,USP
	MOVE.l A4,USP
	MOVE.l A5,USP
	MOVE.l A6,USP
	MOVE.l A7,USP