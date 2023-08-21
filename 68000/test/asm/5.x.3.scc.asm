	mc68000
	opt o-

; =========================================================
; Scc.b <ea>
; =========================================================

; =========================================================
; ST.b <ea>
; =========================================================

; ST.b, Dn
	ST.b D0

; ST.b, An (not allowed for dest)

; ST.b, (An)
	ST.b (A5)

; ST.b, (An)+
	ST.b (A6)+

; ST.b, -(An)
	ST.b -(A7)

; ST.b, d16(An)
	ST.b -32768(A2)
	ST.b 0(A3)
	ST.b 1234(A4)
	ST.b 32767(A4)

; ST.b, d8(An, Xn.L|W)
	ST.b 123(A2,A3.w)
	ST.b -1(A3,D2.l)
	ST.b -128(A4,D3.w)

; ST.b, (xxx).w
	ST.b ($0000).w
	ST.b ($1234).w
	ST.b ($7FFF).w

; ST.b, (xxx).l
	ST.b ($00000000).l
	ST.b ($12345678).l
	ST.b ($FFFFFFFF).l

; Not supported
; ST.b, d16(PC)
; ST.b, d8(PC, Xn)
; ST.b, Imm

; =========================================================
; SF.b <ea>
; =========================================================

; SF.b, Dn
	SF.b D0

; SF.b, An (not allowed for dest)

; SF.b, (An)
	SF.b (A5)

; SF.b, (An)+
	SF.b (A6)+

; SF.b, -(An)
	SF.b -(A7)

; SF.b, d16(An)
	SF.b -32768(A2)
	SF.b 0(A3)
	SF.b 1234(A4)
	SF.b 32767(A4)

; SF.b, d8(An, Xn.L|W)
	SF.b 123(A2,A3.w)
	SF.b -1(A3,D2.l)
	SF.b -128(A4,D3.w)

; SF.b, (xxx).w
	SF.b ($0000).w
	SF.b ($1234).w
	SF.b ($7FFF).w

; SF.b, (xxx).l
	SF.b ($00000000).l
	SF.b ($12345678).l
	SF.b ($FFFFFFFF).l

; Not supported
; SF.b, d16(PC)
; SF.b, d8(PC, Xn)
; SF.b, Imm

; =========================================================
; SHI.b <ea>
; =========================================================

; SHI.b, Dn
	SHI.b D0

; SHI.b, An (not allowed for dest)

; SHI.b, (An)
	SHI.b (A5)

; SHI.b, (An)+
	SHI.b (A6)+

; SHI.b, -(An)
	SHI.b -(A7)

; SHI.b, d16(An)
	SHI.b -32768(A2)
	SHI.b 0(A3)
	SHI.b 1234(A4)
	SHI.b 32767(A4)

; SHI.b, d8(An, Xn.L|W)
	SHI.b 123(A2,A3.w)
	SHI.b -1(A3,D2.l)
	SHI.b -128(A4,D3.w)

; SHI.b, (xxx).w
	SHI.b ($0000).w
	SHI.b ($1234).w
	SHI.b ($7FFF).w

; SHI.b, (xxx).l
	SHI.b ($00000000).l
	SHI.b ($12345678).l
	SHI.b ($FFFFFFFF).l

; Not supported
; SHI.b, d16(PC)
; SHI.b, d8(PC, Xn)
; SHI.b, Imm

; =========================================================
; SLS.b <ea>
; =========================================================

; SLS.b, Dn
	SLS.b D0

; SLS.b, An (not allowed for dest)

; SLS.b, (An)
	SLS.b (A5)

; SLS.b, (An)+
	SLS.b (A6)+

; SLS.b, -(An)
	SLS.b -(A7)

; SLS.b, d16(An)
	SLS.b -32768(A2)
	SLS.b 0(A3)
	SLS.b 1234(A4)
	SLS.b 32767(A4)

; SLS.b, d8(An, Xn.L|W)
	SLS.b 123(A2,A3.w)
	SLS.b -1(A3,D2.l)
	SLS.b -128(A4,D3.w)

; SLS.b, (xxx).w
	SLS.b ($0000).w
	SLS.b ($1234).w
	SLS.b ($7FFF).w

; SLS.b, (xxx).l
	SLS.b ($00000000).l
	SLS.b ($12345678).l
	SLS.b ($FFFFFFFF).l

; Not supported
; SLS.b, d16(PC)
; SLS.b, d8(PC, Xn)
; SLS.b, Imm

; =========================================================
; SCC.b <ea>
; =========================================================

; SCC.b, Dn
	SCC.b D0

; SCC.b, An (not allowed for dest)

; SCC.b, (An)
	SCC.b (A5)

; SCC.b, (An)+
	SCC.b (A6)+

; SCC.b, -(An)
	SCC.b -(A7)

; SCC.b, d16(An)
	SCC.b -32768(A2)
	SCC.b 0(A3)
	SCC.b 1234(A4)
	SCC.b 32767(A4)

; SCC.b, d8(An, Xn.L|W)
	SCC.b 123(A2,A3.w)
	SCC.b -1(A3,D2.l)
	SCC.b -128(A4,D3.w)

; SCC.b, (xxx).w
	SCC.b ($0000).w
	SCC.b ($1234).w
	SCC.b ($7FFF).w

; SCC.b, (xxx).l
	SCC.b ($00000000).l
	SCC.b ($12345678).l
	SCC.b ($FFFFFFFF).l

; Not supported
; SCC.b, d16(PC)
; SCC.b, d8(PC, Xn)
; SCC.b, Imm

; =========================================================
; SCS.b <ea>
; =========================================================

; SCS.b, Dn
	SCS.b D0

; SCS.b, An (not allowed for dest)

; SCS.b, (An)
	SCS.b (A5)

; SCS.b, (An)+
	SCS.b (A6)+

; SCS.b, -(An)
	SCS.b -(A7)

; SCS.b, d16(An)
	SCS.b -32768(A2)
	SCS.b 0(A3)
	SCS.b 1234(A4)
	SCS.b 32767(A4)

; SCS.b, d8(An, Xn.L|W)
	SCS.b 123(A2,A3.w)
	SCS.b -1(A3,D2.l)
	SCS.b -128(A4,D3.w)

; SCS.b, (xxx).w
	SCS.b ($0000).w
	SCS.b ($1234).w
	SCS.b ($7FFF).w

; SCS.b, (xxx).l
	SCS.b ($00000000).l
	SCS.b ($12345678).l
	SCS.b ($FFFFFFFF).l

; Not supported
; SCS.b, d16(PC)
; SCS.b, d8(PC, Xn)
; SCS.b, Imm

; =========================================================
; SNE.b <ea>
; =========================================================

; SNE.b, Dn
	SNE.b D0

; SNE.b, An (not allowed for dest)

; SNE.b, (An)
	SNE.b (A5)

; SNE.b, (An)+
	SNE.b (A6)+

; SNE.b, -(An)
	SNE.b -(A7)

; SNE.b, d16(An)
	SNE.b -32768(A2)
	SNE.b 0(A3)
	SNE.b 1234(A4)
	SNE.b 32767(A4)

; SNE.b, d8(An, Xn.L|W)
	SNE.b 123(A2,A3.w)
	SNE.b -1(A3,D2.l)
	SNE.b -128(A4,D3.w)

; SNE.b, (xxx).w
	SNE.b ($0000).w
	SNE.b ($1234).w
	SNE.b ($7FFF).w

; SNE.b, (xxx).l
	SNE.b ($00000000).l
	SNE.b ($12345678).l
	SNE.b ($FFFFFFFF).l

; Not supported
; SNE.b, d16(PC)
; SNE.b, d8(PC, Xn)
; SNE.b, Imm

; =========================================================
; SEQ.b <ea>
; =========================================================

; SEQ.b, Dn
	SEQ.b D0

; SEQ.b, An (not allowed for dest)

; SEQ.b, (An)
	SEQ.b (A5)

; SEQ.b, (An)+
	SEQ.b (A6)+

; SEQ.b, -(An)
	SEQ.b -(A7)

; SEQ.b, d16(An)
	SEQ.b -32768(A2)
	SEQ.b 0(A3)
	SEQ.b 1234(A4)
	SEQ.b 32767(A4)

; SEQ.b, d8(An, Xn.L|W)
	SEQ.b 123(A2,A3.w)
	SEQ.b -1(A3,D2.l)
	SEQ.b -128(A4,D3.w)

; SEQ.b, (xxx).w
	SEQ.b ($0000).w
	SEQ.b ($1234).w
	SEQ.b ($7FFF).w

; SEQ.b, (xxx).l
	SEQ.b ($00000000).l
	SEQ.b ($12345678).l
	SEQ.b ($FFFFFFFF).l

; Not supported
; SEQ.b, d16(PC)
; SEQ.b, d8(PC, Xn)
; SEQ.b, Imm

; =========================================================
; SVC.b <ea>
; =========================================================

; SVC.b, Dn
	SVC.b D0

; SVC.b, An (not allowed for dest)

; SVC.b, (An)
	SVC.b (A5)

; SVC.b, (An)+
	SVC.b (A6)+

; SVC.b, -(An)
	SVC.b -(A7)

; SVC.b, d16(An)
	SVC.b -32768(A2)
	SVC.b 0(A3)
	SVC.b 1234(A4)
	SVC.b 32767(A4)

; SVC.b, d8(An, Xn.L|W)
	SVC.b 123(A2,A3.w)
	SVC.b -1(A3,D2.l)
	SVC.b -128(A4,D3.w)

; SVC.b, (xxx).w
	SVC.b ($0000).w
	SVC.b ($1234).w
	SVC.b ($7FFF).w

; SVC.b, (xxx).l
	SVC.b ($00000000).l
	SVC.b ($12345678).l
	SVC.b ($FFFFFFFF).l

; Not supported
; SVC.b, d16(PC)
; SVC.b, d8(PC, Xn)
; SVC.b, Imm

; =========================================================
; SVS.b <ea>
; =========================================================

; SVS.b, Dn
	SVS.b D0

; SVS.b, An (not allowed for dest)

; SVS.b, (An)
	SVS.b (A5)

; SVS.b, (An)+
	SVS.b (A6)+

; SVS.b, -(An)
	SVS.b -(A7)

; SVS.b, d16(An)
	SVS.b -32768(A2)
	SVS.b 0(A3)
	SVS.b 1234(A4)
	SVS.b 32767(A4)

; SVS.b, d8(An, Xn.L|W)
	SVS.b 123(A2,A3.w)
	SVS.b -1(A3,D2.l)
	SVS.b -128(A4,D3.w)

; SVS.b, (xxx).w
	SVS.b ($0000).w
	SVS.b ($1234).w
	SVS.b ($7FFF).w

; SVS.b, (xxx).l
	SVS.b ($00000000).l
	SVS.b ($12345678).l
	SVS.b ($FFFFFFFF).l

; Not supported
; SVS.b, d16(PC)
; SVS.b, d8(PC, Xn)
; SVS.b, Imm

; =========================================================
; SPL.b <ea>
; =========================================================

; SPL.b, Dn
	SPL.b D0

; SPL.b, An (not allowed for dest)

; SPL.b, (An)
	SPL.b (A5)

; SPL.b, (An)+
	SPL.b (A6)+

; SPL.b, -(An)
	SPL.b -(A7)

; SPL.b, d16(An)
	SPL.b -32768(A2)
	SPL.b 0(A3)
	SPL.b 1234(A4)
	SPL.b 32767(A4)

; SPL.b, d8(An, Xn.L|W)
	SPL.b 123(A2,A3.w)
	SPL.b -1(A3,D2.l)
	SPL.b -128(A4,D3.w)

; SPL.b, (xxx).w
	SPL.b ($0000).w
	SPL.b ($1234).w
	SPL.b ($7FFF).w

; SPL.b, (xxx).l
	SPL.b ($00000000).l
	SPL.b ($12345678).l
	SPL.b ($FFFFFFFF).l

; Not supported
; SPL.b, d16(PC)
; SPL.b, d8(PC, Xn)
; SPL.b, Imm

; =========================================================
; SMI.b <ea>
; =========================================================

; SMI.b, Dn
	SMI.b D0

; SMI.b, An (not allowed for dest)

; SMI.b, (An)
	SMI.b (A5)

; SMI.b, (An)+
	SMI.b (A6)+

; SMI.b, -(An)
	SMI.b -(A7)

; SMI.b, d16(An)
	SMI.b -32768(A2)
	SMI.b 0(A3)
	SMI.b 1234(A4)
	SMI.b 32767(A4)

; SMI.b, d8(An, Xn.L|W)
	SMI.b 123(A2,A3.w)
	SMI.b -1(A3,D2.l)
	SMI.b -128(A4,D3.w)

; SMI.b, (xxx).w
	SMI.b ($0000).w
	SMI.b ($1234).w
	SMI.b ($7FFF).w

; SMI.b, (xxx).l
	SMI.b ($00000000).l
	SMI.b ($12345678).l
	SMI.b ($FFFFFFFF).l

; Not supported
; SMI.b, d16(PC)
; SMI.b, d8(PC, Xn)
; SMI.b, Imm

; =========================================================
; SGE.b <ea>
; =========================================================

; SGE.b, Dn
	SGE.b D0

; SGE.b, An (not allowed for dest)

; SGE.b, (An)
	SGE.b (A5)

; SGE.b, (An)+
	SGE.b (A6)+

; SGE.b, -(An)
	SGE.b -(A7)

; SGE.b, d16(An)
	SGE.b -32768(A2)
	SGE.b 0(A3)
	SGE.b 1234(A4)
	SGE.b 32767(A4)

; SGE.b, d8(An, Xn.L|W)
	SGE.b 123(A2,A3.w)
	SGE.b -1(A3,D2.l)
	SGE.b -128(A4,D3.w)

; SGE.b, (xxx).w
	SGE.b ($0000).w
	SGE.b ($1234).w
	SGE.b ($7FFF).w

; SGE.b, (xxx).l
	SGE.b ($00000000).l
	SGE.b ($12345678).l
	SGE.b ($FFFFFFFF).l

; Not supported
; SGE.b, d16(PC)
; SGE.b, d8(PC, Xn)
; SGE.b, Imm

; =========================================================
; SLT.b <ea>
; =========================================================

; SLT.b, Dn
	SLT.b D0

; SLT.b, An (not allowed for dest)

; SLT.b, (An)
	SLT.b (A5)

; SLT.b, (An)+
	SLT.b (A6)+

; SLT.b, -(An)
	SLT.b -(A7)

; SLT.b, d16(An)
	SLT.b -32768(A2)
	SLT.b 0(A3)
	SLT.b 1234(A4)
	SLT.b 32767(A4)

; SLT.b, d8(An, Xn.L|W)
	SLT.b 123(A2,A3.w)
	SLT.b -1(A3,D2.l)
	SLT.b -128(A4,D3.w)

; SLT.b, (xxx).w
	SLT.b ($0000).w
	SLT.b ($1234).w
	SLT.b ($7FFF).w

; SLT.b, (xxx).l
	SLT.b ($00000000).l
	SLT.b ($12345678).l
	SLT.b ($FFFFFFFF).l

; Not supported
; SLT.b, d16(PC)
; SLT.b, d8(PC, Xn)
; SLT.b, Imm

; =========================================================
; SGT.b <ea>
; =========================================================

; SGT.b, Dn
	SGT.b D0

; SGT.b, An (not allowed for dest)

; SGT.b, (An)
	SGT.b (A5)

; SGT.b, (An)+
	SGT.b (A6)+

; SGT.b, -(An)
	SGT.b -(A7)

; SGT.b, d16(An)
	SGT.b -32768(A2)
	SGT.b 0(A3)
	SGT.b 1234(A4)
	SGT.b 32767(A4)

; SGT.b, d8(An, Xn.L|W)
	SGT.b 123(A2,A3.w)
	SGT.b -1(A3,D2.l)
	SGT.b -128(A4,D3.w)

; SGT.b, (xxx).w
	SGT.b ($0000).w
	SGT.b ($1234).w
	SGT.b ($7FFF).w

; SGT.b, (xxx).l
	SGT.b ($00000000).l
	SGT.b ($12345678).l
	SGT.b ($FFFFFFFF).l

; Not supported
; SGT.b, d16(PC)
; SGT.b, d8(PC, Xn)
; SGT.b, Imm

; =========================================================
; SLE.b <ea>
; =========================================================

; SLE.b, Dn
	SLE.b D0

; SLE.b, An (not allowed for dest)

; SLE.b, (An)
	SLE.b (A5)

; SLE.b, (An)+
	SLE.b (A6)+

; SLE.b, -(An)
	SLE.b -(A7)

; SLE.b, d16(An)
	SLE.b -32768(A2)
	SLE.b 0(A3)
	SLE.b 1234(A4)
	SLE.b 32767(A4)

; SLE.b, d8(An, Xn.L|W)
	SLE.b 123(A2,A3.w)
	SLE.b -1(A3,D2.l)
	SLE.b -128(A4,D3.w)

; SLE.b, (xxx).w
	SLE.b ($0000).w
	SLE.b ($1234).w
	SLE.b ($7FFF).w

; SLE.b, (xxx).l
	SLE.b ($00000000).l
	SLE.b ($12345678).l
	SLE.b ($FFFFFFFF).l

; Not supported
; SLE.b, d16(PC)
; SLE.b, d8(PC, Xn)
; SLE.b, Imm