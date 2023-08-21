	mc68000
	opt o-

; Don't use generated code because there's no way to specify
; a raw offset (as it will appear in machine code)
; Everything is relative, so here everything points at
; absolute address '0', meaning all opcodes will have an
; increasing negative offset

; =========================================================
; BRA.b|w <displ>
; =========================================================

;	BRA.w 2
;
;	BRA.b 64
;	BRA.w 1024

; =========================================================
; BSR.b|w <displ>
; =========================================================

;	BSR.b 64
;	BSR.w 1024

; =========================================================
; Bcc.b|w <displ>
; =========================================================

;	BHI.b 64
;	BLS.b 64
;	BCC.b 64
;	BCS.b 64
;	BNE.b 64
;	BEQ.b 64
;	BVC.b 64
;	BVS.b 64
;	BPL.b 64
;	BMI.b 64
;	BGE.b 64
;	BLT.b 64
;	BGT.b 64
;	BLE.b 64
;
;	BHI.w 1024
;	BLS.w 1024
;	BCC.w 1024
;	BCS.w 1024
;	BNE.w 1024
;	BEQ.w 1024
;	BVC.w 1024
;	BVS.w 1024
;	BPL.w 1024
;	BMI.w 1024
;	BGE.w 1024
;	BLT.w 1024
;	BGT.w 1024
;	BLE.w 1024
