4C900100	MOVEM.w (A0),A0
4C918000	MOVEM.w (A1),A7
4C920001	MOVEM.w (A2),D0
4C930080	MOVEM.w (A3),D7
4C940300	MOVEM.w (A4),A0-A1
4C950003	MOVEM.w (A5),D0-D1
4C960100	MOVEM.w (A6),A0
4C970101	MOVEM.w (A7),D0/A0
4C9000FF	MOVEM.w (A0),D0-D7
4C91FF00	MOVEM.w (A1),A0-A7
4C92FFFF	MOVEM.w (A2),D0-D7/A0-A7
4C930F0F	MOVEM.w (A3),D0-D3/A0-A3
4C948001	MOVEM.w (A4),D0/A7
4C9D2100	MOVEM.w (A5)+,A0/A5
4C9D8000	MOVEM.w (A5)+,A7
4C9D8001	MOVEM.w (A5)+,D0/A7
4C9D000E	MOVEM.w (A5)+,D1-D3
4CAA04008000	MOVEM.w -32768(A2),A2
4CAB28000000	MOVEM.w 0(A3),A3/A5
4CAC001004D2	MOVEM.w 1234(A4),D4
4CAC00607FFF	MOVEM.w 32767(A4),D5-D6
4CB24000B07B	MOVEM.w 123(A2,A3.w),A6
4CB3800028FF	MOVEM.w -1(A3,D2.l),A7
4CB400013080	MOVEM.w -128(A4,D3.w),D0
4CB800020000	MOVEM.w ($0000).w,D1
4CB800041234	MOVEM.w ($1234).w,D2
4CB800F87FFF	MOVEM.w ($7FFF).w,D3-D7
4CB9000300000000	MOVEM.w ($00000000).l,D0-D1
4CB9002012345678	MOVEM.w ($12345678).l,D5
4CB900C0FFFFFFFF	MOVEM.w ($FFFFFFFF).l,D6-D7
4CB9008080808080	MOVEM.w ($80808080).l,D7
4CBA04008000	MOVEM.w -32768(PC),A2
4CBA08000000	MOVEM.w 0(PC),A3
4CBA001004D2	MOVEM.w 1234(PC),D4
4CBA81207FFF	MOVEM.w 32767(PC),D5/A0/A7
4CBB4000B07B	MOVEM.w 123(PC,A3.w),A6
4CBB800028FF	MOVEM.w -1(PC,D2.l),A7
4CBB00013080	MOVEM.w -128(PC,D3.w),D0
4CD00100	MOVEM.l (A0),A0
4CD18000	MOVEM.l (A1),A7
4CD20001	MOVEM.l (A2),D0
4CD30080	MOVEM.l (A3),D7
4CD40300	MOVEM.l (A4),A0-A1
4CD50003	MOVEM.l (A5),D0-D1
4CD60100	MOVEM.l (A6),A0
4CD70101	MOVEM.l (A7),D0/A0
4CD000FF	MOVEM.l (A0),D0-D7
4CD1FF00	MOVEM.l (A1),A0-A7
4CD2FFFF	MOVEM.l (A2),D0-D7/A0-A7
4CD30F0F	MOVEM.l (A3),D0-D3/A0-A3
4CD48001	MOVEM.l (A4),D0/A7
4CDD0100	MOVEM.l (A5)+,A0
4CDD8000	MOVEM.l (A5)+,A7
4CDD0001	MOVEM.l (A5)+,D0
4CDD0080	MOVEM.l (A5)+,D7
4CEA04008000	MOVEM.l -32768(A2),A2
4CEB08000000	MOVEM.l 0(A3),A3
4CEC001004D2	MOVEM.l 1234(A4),D4
4CEC00207FFF	MOVEM.l 32767(A4),D5
4CF24000B07B	MOVEM.l 123(A2,A3.w),A6
4CF3800028FF	MOVEM.l -1(A3,D2.l),A7
4CF400013080	MOVEM.l -128(A4,D3.w),D0
4CF800020000	MOVEM.l ($0000).w,D1
4CF800041234	MOVEM.l ($1234).w,D2
4CF800087FFF	MOVEM.l ($7FFF).w,D3
4CF9001000000000	MOVEM.l ($00000000).l,D4
4CF9002012345678	MOVEM.l ($12345678).l,D5
4CF90040FFFFFFFF	MOVEM.l ($FFFFFFFF).l,D6
4CF9008080808080	MOVEM.l ($80808080).l,D7
4CFA04008000	MOVEM.l -32768(PC),A2
4CFA08000000	MOVEM.l 0(PC),A3
4CFA001004D2	MOVEM.l 1234(PC),D4
4CFA00207FFF	MOVEM.l 32767(PC),D5
4CFB4000B07B	MOVEM.l 123(PC,A3.w),A6
4CFB800028FF	MOVEM.l -1(PC,D2.l),A7
4CFB00013080	MOVEM.l -128(PC,D3.w),D0
