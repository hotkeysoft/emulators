3040	MOVEA.w D0,A0
3241	MOVEA.w D1,A1
3442	MOVEA.w D2,A2
3643	MOVEA.w D3,A3
3844	MOVEA.w D4,A4
3A45	MOVEA.w D5,A5
3C46	MOVEA.w D6,A6
3E47	MOVEA.w D7,A7
3E48	MOVEA.w A0,A7
3C49	MOVEA.w A1,A6
3A4A	MOVEA.w A2,A5
384B	MOVEA.w A3,A4
364C	MOVEA.w A4,A3
344D	MOVEA.w A5,A2
324E	MOVEA.w A6,A1
304F	MOVEA.w A7,A0
3048	MOVEA.w A0,A0
3E50	MOVEA.w (A0),A7
3C51	MOVEA.w (A1),A6
3A52	MOVEA.w (A2),A5
3853	MOVEA.w (A3),A4
3654	MOVEA.w (A4),A3
3455	MOVEA.w (A5),A2
3256	MOVEA.w (A6),A1
3057	MOVEA.w (A7),A0
3050	MOVEA.w (A0),A0
3E58	MOVEA.w (A0)+,A7
3C59	MOVEA.w (A1)+,A6
3A5A	MOVEA.w (A2)+,A5
385B	MOVEA.w (A3)+,A4
365C	MOVEA.w (A4)+,A3
345D	MOVEA.w (A5)+,A2
325E	MOVEA.w (A6)+,A1
305F	MOVEA.w (A7)+,A0
3058	MOVEA.w (A0)+,A0
3E60	MOVEA.w -(A0),A7
3C61	MOVEA.w -(A1),A6
3A62	MOVEA.w -(A2),A5
3863	MOVEA.w -(A3),A4
3664	MOVEA.w -(A4),A3
3465	MOVEA.w -(A5),A2
3266	MOVEA.w -(A6),A1
3067	MOVEA.w -(A7),A0
3060	MOVEA.w -(A0),A0
3E688000	MOVEA.w -32768(A0),A7
3C698000	MOVEA.w -32768(A1),A6
3A6A8000	MOVEA.w -32768(A2),A5
386B8000	MOVEA.w -32768(A3),A4
366C7FFF	MOVEA.w 32767(A4),A3
346D7FFF	MOVEA.w 32767(A5),A2
326E7FFF	MOVEA.w 32767(A6),A1
306F7FFF	MOVEA.w 32767(A7),A0
30687FFF	MOVEA.w 32767(A0),A0
3E700080	MOVEA.w -128(A0,D0.w),A7
3E711080	MOVEA.w -128(A1,D1.w),A7
3E722080	MOVEA.w -128(A2,D2.w),A7
3E733080	MOVEA.w -128(A3,D3.w),A7
3E74407F	MOVEA.w 127(A4,D4.w),A7
3E75507F	MOVEA.w 127(A5,D5.w),A7
3E76607F	MOVEA.w 127(A6,D6.w),A7
3E77707F	MOVEA.w 127(A7,D7.w),A7
3C708080	MOVEA.w -128(A0,A0.w),A6
3C719080	MOVEA.w -128(A1,A1.w),A6
3C72A080	MOVEA.w -128(A2,A2.w),A6
3C73B080	MOVEA.w -128(A3,A3.w),A6
3C74C07F	MOVEA.w 127(A4,A4.w),A6
3C75D07F	MOVEA.w 127(A5,A5.w),A6
3C76E07F	MOVEA.w 127(A6,A6.w),A6
3C77F07F	MOVEA.w 127(A7,A7.w),A6
3A700880	MOVEA.w -128(A0,D0.l),A5
3A711880	MOVEA.w -128(A1,D1.l),A5
3A722880	MOVEA.w -128(A2,D2.l),A5
3A733880	MOVEA.w -128(A3,D3.l),A5
3A74487F	MOVEA.w 127(A4,D4.l),A5
3A75587F	MOVEA.w 127(A5,D5.l),A5
3A76687F	MOVEA.w 127(A6,D6.l),A5
3A77787F	MOVEA.w 127(A7,D7.l),A5
38708880	MOVEA.w -128(A0,A0.l),A4
38719880	MOVEA.w -128(A1,A1.l),A4
3872A880	MOVEA.w -128(A2,A2.l),A4
3873B880	MOVEA.w -128(A3,A3.l),A4
3874C87F	MOVEA.w 127(A4,A4.l),A4
3875D87F	MOVEA.w 127(A5,A5.l),A4
3876E87F	MOVEA.w 127(A6,A6.l),A4
3877F87F	MOVEA.w 127(A7,A7.l),A4
30780000	MOVEA.w ($0000).w,A0
32781234	MOVEA.w ($1234).w,A1
34787FFF	MOVEA.w ($7FFF).w,A2
367900000000	MOVEA.w ($00000000).l,A3
367912345678	MOVEA.w ($12345678).l,A3
3679FFFFFFFF	MOVEA.w ($FFFFFFFF).l,A3
307A8000	MOVEA.w -32768(PC),A0
327A0000	MOVEA.w 0(PC),A1
347A7FFF	MOVEA.w 32767(PC),A2
3E7B0080	MOVEA.w -128(PC,D0.w),A7
3E7B1080	MOVEA.w -128(PC,D1.w),A7
3E7B2080	MOVEA.w -128(PC,D2.w),A7
3E7B3080	MOVEA.w -128(PC,D3.w),A7
3E7B407F	MOVEA.w 127(PC,D4.w),A7
3E7B507F	MOVEA.w 127(PC,D5.w),A7
3E7B607F	MOVEA.w 127(PC,D6.w),A7
3E7B707F	MOVEA.w 127(PC,D7.w),A7
3C7B8080	MOVEA.w -128(PC,A0.w),A6
3C7B9080	MOVEA.w -128(PC,A1.w),A6
3C7BA080	MOVEA.w -128(PC,A2.w),A6
3C7BB080	MOVEA.w -128(PC,A3.w),A6
3C7BC07F	MOVEA.w 127(PC,A4.w),A6
3C7BD07F	MOVEA.w 127(PC,A5.w),A6
3C7BE07F	MOVEA.w 127(PC,A6.w),A6
3C7BF07F	MOVEA.w 127(PC,A7.w),A6
3A7B0880	MOVEA.w -128(PC,D0.l),A5
3A7B1880	MOVEA.w -128(PC,D1.l),A5
3A7B2880	MOVEA.w -128(PC,D2.l),A5
3A7B3880	MOVEA.w -128(PC,D3.l),A5
3A7B487F	MOVEA.w 127(PC,D4.l),A5
3A7B587F	MOVEA.w 127(PC,D5.l),A5
3A7B687F	MOVEA.w 127(PC,D6.l),A5
3A7B787F	MOVEA.w 127(PC,D7.l),A5
387B8880	MOVEA.w -128(PC,A0.l),A4
387B9880	MOVEA.w -128(PC,A1.l),A4
387BA880	MOVEA.w -128(PC,A2.l),A4
387BB880	MOVEA.w -128(PC,A3.l),A4
387BC87F	MOVEA.w 127(PC,A4.l),A4
387BD87F	MOVEA.w 127(PC,A5.l),A4
387BE87F	MOVEA.w 127(PC,A6.l),A4
387BF87F	MOVEA.w 127(PC,A7.l),A4
327C0000	MOVEA.w #$0000,A1
347C1234	MOVEA.w #$1234,A2
367CFFFF	MOVEA.w #$FFFF,A3
