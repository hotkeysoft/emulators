C1C7	MULS.w D7,D0
C3C6	MULS.w D6,D1
C5C5	MULS.w D5,D2
C7C4	MULS.w D4,D3
C9C3	MULS.w D3,D4
CBC2	MULS.w D2,D5
CDC1	MULS.w D1,D6
CFC0	MULS.w D0,D7
C1D5	MULS.w (A5),D0
C3DE	MULS.w (A6)+,D1
C5E7	MULS.w -(A7),D2
C7EA8000	MULS.w -32768(A2),D3
C9EB0000	MULS.w 0(A3),D4
CBEC04D2	MULS.w 1234(A4),D5
CDEC7FFF	MULS.w 32767(A4),D6
CFF2B07B	MULS.w 123(A2,A3.w),D7
C1F328FF	MULS.w -1(A3,D2.l),D0
C3F43080	MULS.w -128(A4,D3.w),D1
C5F80000	MULS.w ($0000).w,D2
C7F81234	MULS.w ($1234).w,D3
C9F87FFF	MULS.w ($7FFF).w,D4
CBF900000000	MULS.w ($00000000).l,D5
CDF912345678	MULS.w ($12345678).l,D6
CFF9FFFFFFFF	MULS.w ($FFFFFFFF).l,D7
C1FA8000	MULS.w -32768(PC),D0
C3FA0000	MULS.w 0(PC),D1
C5FA04D2	MULS.w 1234(PC),D2
C7FA7FFF	MULS.w 32767(PC),D3
C9FBB07B	MULS.w 123(PC,A3.w),D4
CBFB28FF	MULS.w -1(PC,D2.l),D5
CDFB3080	MULS.w -128(PC,D3.w),D6
C1FC0000	MULS.w #$0000,D0
C3FC8000	MULS.w #$8000,D1
C5FCFFFF	MULS.w #$FFFF,D2
C7FCFF00	MULS.w #$FF00,D3
C9FC00FF	MULS.w #$00FF,D4
CBFCF0F0	MULS.w #$F0F0,D5
CDFCAA55	MULS.w #$AA55,D6
CFFC1234	MULS.w #$1234,D7
