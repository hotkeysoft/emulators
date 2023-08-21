	mc68000
	opt o-

; =========================================================
; DBcc.w <ea>
; =========================================================

; Don't use generated code because there's no way to specify
; a raw offset (as it will appear in machine code)
; Everything is relative, so here everything points at
; absolute address '0', meaning all opcodes will have an
; increasing negative offset

;	DBT.w D0,0
;	DBF.w D1,0
;	DBHI.w D2,0
;	DBLS.w D3,0
;	DBCC.w D4,0
;	DBCS.w D5,0
;	DBNE.w D6,0
;	DBEQ.w D7,0
;	DBVC.w D0,0
;	DBVS.w D1,0
;	DBPL.w D2,0
;	DBMI.w D3,0
;	DBGE.w D4,0
;	DBLT.w D5,0
;	DBGT.w D6,0
;	DBLE.w D7,0
