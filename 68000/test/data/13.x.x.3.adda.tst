D0C7	ADDA.w D7,A0
D2C6	ADDA.w D6,A1
D4C5	ADDA.w D5,A2
D6C4	ADDA.w D4,A3
D8C3	ADDA.w D3,A4
DAC2	ADDA.w D2,A5
DCC1	ADDA.w D1,A6
DEC0	ADDA.w D0,A7
D0CF	ADDA.w A7,A0
D2CE	ADDA.w A6,A1
D4CD	ADDA.w A5,A2
D6CC	ADDA.w A4,A3
D8CB	ADDA.w A3,A4
DACA	ADDA.w A2,A5
DCC9	ADDA.w A1,A6
DEC8	ADDA.w A0,A7
D0D5	ADDA.w (A5),A0
D2DE	ADDA.w (A6)+,A1
D4E7	ADDA.w -(A7),A2
D6EA8000	ADDA.w -32768(A2),A3
D8EB0000	ADDA.w 0(A3),A4
DAEC04D2	ADDA.w 1234(A4),A5
DCEC7FFF	ADDA.w 32767(A4),A6
DEF2B07B	ADDA.w 123(A2,A3.w),A7
D0F328FF	ADDA.w -1(A3,D2.l),A0
D2F43080	ADDA.w -128(A4,D3.w),A1
D4F80000	ADDA.w ($0000).w,A2
D6F81234	ADDA.w ($1234).w,A3
D8F87FFF	ADDA.w ($7FFF).w,A4
DAF900000000	ADDA.w ($00000000).l,A5
DCF912345678	ADDA.w ($12345678).l,A6
DEF9FFFFFFFF	ADDA.w ($FFFFFFFF).l,A7
D0FA8000	ADDA.w -32768(PC),A0
D2FA0000	ADDA.w 0(PC),A1
D4FA04D2	ADDA.w 1234(PC),A2
D6FA7FFF	ADDA.w 32767(PC),A3
D8FBB07B	ADDA.w 123(PC,A3.w),A4
DAFB28FF	ADDA.w -1(PC,D2.l),A5
DCFB3080	ADDA.w -128(PC,D3.w),A6
D0FC0000	ADDA.w #$0000,A0
D2FC1234	ADDA.w #$1234,A1
D4FCABCD	ADDA.w #$ABCD,A2
D6FC8000	ADDA.w #$8000,A3
D8FCF0F0	ADDA.w #$F0F0,A4
DAFCAA55	ADDA.w #$AA55,A5
DCFC0F0F	ADDA.w #$0F0F,A6
DEFCFFFF	ADDA.w #$FFFF,A7
D1C7	ADDA.l D7,A0
D3C6	ADDA.l D6,A1
D5C5	ADDA.l D5,A2
D7C4	ADDA.l D4,A3
D9C3	ADDA.l D3,A4
DBC2	ADDA.l D2,A5
DDC1	ADDA.l D1,A6
DFC0	ADDA.l D0,A7
D1CF	ADDA.l A7,A0
D3CE	ADDA.l A6,A1
D5CD	ADDA.l A5,A2
D7CC	ADDA.l A4,A3
D9CB	ADDA.l A3,A4
DBCA	ADDA.l A2,A5
DDC9	ADDA.l A1,A6
DFC8	ADDA.l A0,A7
D1D5	ADDA.l (A5),A0
D3DE	ADDA.l (A6)+,A1
D5E7	ADDA.l -(A7),A2
D7EA8000	ADDA.l -32768(A2),A3
D9EB0000	ADDA.l 0(A3),A4
DBEC04D2	ADDA.l 1234(A4),A5
DDEC7FFF	ADDA.l 32767(A4),A6
DFF2B07B	ADDA.l 123(A2,A3.w),A7
D1F328FF	ADDA.l -1(A3,D2.l),A0
D3F43080	ADDA.l -128(A4,D3.w),A1
D5F80000	ADDA.l ($0000).w,A2
D7F81234	ADDA.l ($1234).w,A3
D9F87FFF	ADDA.l ($7FFF).w,A4
DBF900000000	ADDA.l ($00000000).l,A5
DDF912345678	ADDA.l ($12345678).l,A6
DFF9FFFFFFFF	ADDA.l ($FFFFFFFF).l,A7
D1FA8000	ADDA.l -32768(PC),A0
D3FA0000	ADDA.l 0(PC),A1
D5FA04D2	ADDA.l 1234(PC),A2
D7FA7FFF	ADDA.l 32767(PC),A3
D9FB307B	ADDA.l 123(PC,D3.w),A4
DBFB28FF	ADDA.l -1(PC,D2.l),A5
DDFBB080	ADDA.l -128(PC,A3.w),A6
D1FC00000000	ADDA.l #$00000000,A0
D3FC12341234	ADDA.l #$12341234,A1
D5FCABCDABCD	ADDA.l #$ABCDABCD,A2
D7FC80008000	ADDA.l #$80008000,A3
D9FCF0F0F0F0	ADDA.l #$F0F0F0F0,A4
DBFCAA55AA55	ADDA.l #$AA55AA55,A5
DDFC0F0F0F0F	ADDA.l #$0F0F0F0F,A6
DFFCFFFFFFFF	ADDA.l #$FFFFFFFF,A7
