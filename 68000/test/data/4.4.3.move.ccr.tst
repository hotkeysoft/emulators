44C0	MOVE.w D0,CCR
44D5	MOVE.w (A5),CCR
44DE	MOVE.w (A6)+,CCR
44E7	MOVE.w -(A7),CCR
44EA8000	MOVE.w -32768(A2),CCR
44EB0000	MOVE.w 0(A3),CCR
44EC04D2	MOVE.w 1234(A4),CCR
44EC7FFF	MOVE.w 32767(A4),CCR
44F2B07B	MOVE.w 123(A2,A3.w),CCR
44F328FF	MOVE.w -1(A3,D2.l),CCR
44F43080	MOVE.w -128(A4,D3.w),CCR
44F80000	MOVE.w ($0000).w,CCR
44F81234	MOVE.w ($1234).w,CCR
44F87FFF	MOVE.w ($7FFF).w,CCR
44F900000000	MOVE.w ($00000000).l,CCR
44F912345678	MOVE.w ($12345678).l,CCR
44F9FFFFFFFF	MOVE.w ($FFFFFFFF).l,CCR
44FA8000	MOVE.w -32768(PC),CCR
44FA0000	MOVE.w 0(PC),CCR
44FA04D2	MOVE.w 1234(PC),CCR
44FA7FFF	MOVE.w 32767(PC),CCR
44FBB07B	MOVE.w 123(PC,A3.w),CCR
44FB28FF	MOVE.w -1(PC,D2.l),CCR
44FB3080	MOVE.w -128(PC,D3.w),CCR
44FC0000	MOVE.w #$0000,CCR
44FCFFFF	MOVE.w #$FFFF,CCR
44FC7FFF	MOVE.w #$7FFF,CCR
44FC1234	MOVE.w #$1234,CCR
