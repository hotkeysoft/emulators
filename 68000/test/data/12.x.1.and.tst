C007	AND.b D7,D0
C206	AND.b D6,D1
C405	AND.b D5,D2
C604	AND.b D4,D3
C803	AND.b D3,D4
CA02	AND.b D2,D5
CC01	AND.b D1,D6
CE00	AND.b D0,D7
C015	AND.b (A5),D0
C21E	AND.b (A6)+,D1
C427	AND.b -(A7),D2
C62A8000	AND.b -32768(A2),D3
C82B0000	AND.b 0(A3),D4
CA2C04D2	AND.b 1234(A4),D5
CC2C7FFF	AND.b 32767(A4),D6
CE32B07B	AND.b 123(A2,A3.w),D7
C03328FF	AND.b -1(A3,D2.l),D0
C2343080	AND.b -128(A4,D3.w),D1
C4380000	AND.b ($0000).w,D2
C6381234	AND.b ($1234).w,D3
C8387FFF	AND.b ($7FFF).w,D4
CA3900000000	AND.b ($00000000).l,D5
CC3912345678	AND.b ($12345678).l,D6
CE39FFFFFFFF	AND.b ($FFFFFFFF).l,D7
C03A8000	AND.b -32768(PC),D0
C23A0000	AND.b 0(PC),D1
C43A04D2	AND.b 1234(PC),D2
C63A7FFF	AND.b 32767(PC),D3
C83BB07B	AND.b 123(PC,A3.w),D4
CA3B28FF	AND.b -1(PC,D2.l),D5
CC3B3080	AND.b -128(PC,D3.w),D6
C03C0000	AND.b #$00,D0
C23C0012	AND.b #$12,D1
C43C00AB	AND.b #$AB,D2
C63C0080	AND.b #$80,D3
C83C00F0	AND.b #$F0,D4
CA3C00AA	AND.b #$AA,D5
CC3C000F	AND.b #$0F,D6
CE3C00FF	AND.b #$FF,D7
C047	AND.w D7,D0
C246	AND.w D6,D1
C445	AND.w D5,D2
C644	AND.w D4,D3
C843	AND.w D3,D4
CA42	AND.w D2,D5
CC41	AND.w D1,D6
CE40	AND.w D0,D7
C055	AND.w (A5),D0
C25E	AND.w (A6)+,D1
C467	AND.w -(A7),D2
C66A8000	AND.w -32768(A2),D3
C86B0000	AND.w 0(A3),D4
CA6C04D2	AND.w 1234(A4),D5
CC6C7FFF	AND.w 32767(A4),D6
CE72B07B	AND.w 123(A2,A3.w),D7
C07328FF	AND.w -1(A3,D2.l),D0
C2743080	AND.w -128(A4,D3.w),D1
C4780000	AND.w ($0000).w,D2
C6781234	AND.w ($1234).w,D3
C8787FFF	AND.w ($7FFF).w,D4
CA7900000000	AND.w ($00000000).l,D5
CC7912345678	AND.w ($12345678).l,D6
CE79FFFFFFFF	AND.w ($FFFFFFFF).l,D7
C07A8000	AND.w -32768(PC),D0
C27A0000	AND.w 0(PC),D1
C47A04D2	AND.w 1234(PC),D2
C67A7FFF	AND.w 32767(PC),D3
C87BB07B	AND.w 123(PC,A3.w),D4
CA7B28FF	AND.w -1(PC,D2.l),D5
CC7B3080	AND.w -128(PC,D3.w),D6
C07C0000	AND.w #$0000,D0
C27C1234	AND.w #$1234,D1
C47CABCD	AND.w #$ABCD,D2
C67C8000	AND.w #$8000,D3
C87CF0F0	AND.w #$F0F0,D4
CA7CAA55	AND.w #$AA55,D5
CC7C0F0F	AND.w #$0F0F,D6
CE7CFFFF	AND.w #$FFFF,D7
C087	AND.l D7,D0
C286	AND.l D6,D1
C485	AND.l D5,D2
C684	AND.l D4,D3
C883	AND.l D3,D4
CA82	AND.l D2,D5
CC81	AND.l D1,D6
CE80	AND.l D0,D7
C095	AND.l (A5),D0
C29E	AND.l (A6)+,D1
C4A7	AND.l -(A7),D2
C6AA8000	AND.l -32768(A2),D3
C8AB0000	AND.l 0(A3),D4
CAAC04D2	AND.l 1234(A4),D5
CCAC7FFF	AND.l 32767(A4),D6
CEB2B07B	AND.l 123(A2,A3.w),D7
C0B328FF	AND.l -1(A3,D2.l),D0
C2B43080	AND.l -128(A4,D3.w),D1
C4B80000	AND.l ($0000).w,D2
C6B81234	AND.l ($1234).w,D3
C8B87FFF	AND.l ($7FFF).w,D4
CAB900000000	AND.l ($00000000).l,D5
CCB912345678	AND.l ($12345678).l,D6
CEB9FFFFFFFF	AND.l ($FFFFFFFF).l,D7
C0BA8000	AND.l -32768(PC),D0
C2BA0000	AND.l 0(PC),D1
C4BA04D2	AND.l 1234(PC),D2
C6BA7FFF	AND.l 32767(PC),D3
C8BBB07B	AND.l 123(PC,A3.w),D4
CABB28FF	AND.l -1(PC,D2.l),D5
CCBB3080	AND.l -128(PC,D3.w),D6
C0BC00000000	AND.l #$00000000,D0
C2BC12341234	AND.l #$12341234,D1
C4BCABCDABCD	AND.l #$ABCDABCD,D2
C6BC80008000	AND.l #$80008000,D3
C8BCF0F0F0F0	AND.l #$F0F0F0F0,D4
CABCAA55AA55	AND.l #$AA55AA55,D5
CCBC0F0F0F0F	AND.l #$0F0F0F0F,D6
CEBCFFFFFFFF	AND.l #$FFFFFFFF,D7
