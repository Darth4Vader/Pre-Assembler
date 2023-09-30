; input operand addressing can't be ADDRESSING_IMMEDIATE and ADDRESSING_REGISTER
lea #1, r3
lea r3, LABEL

; output operand can't be ADDRESSING_IMMEDIATE
; group 1
mov r3, #1
lea LABEL, #15
; group 2 (not parameter addressing)
not #1

; parameter addressing output operand can't be ADDRESSING_IMMEDIATE and ADDRESSING_REGISTER
bne #1(r4,r3)
bne r3(r4,r3)