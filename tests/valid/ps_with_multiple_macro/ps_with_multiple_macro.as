; file ps_with_multiple_macro.as
.entry LENGTH
.extern W
mcr m2
    END: stop
    STR: .string "abcdef"
endmcr
MAIN: mov r3 ,LENGTH
LOOP: jmp L1(#-1,r6)
mcr m1
    sub r1, r4
    bne L3
endmcr
prn #-5
mcr m3
    bne LOOP(K,W)
endmcr
bne W(r4,r5)
m1
L1: inc K
.entry LOOP
m3
m2
LENGTH: .data 6,-9,15
K: .data 22
.extern L3