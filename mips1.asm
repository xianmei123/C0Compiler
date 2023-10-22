.data
str0:    .asciiz    "\n"
.text
li $k0,268501100
move $fp,$sp
addiu $sp,$sp,-44
sw $fp,-4($sp)
sw $ra,-8($sp)
addiu $fp,$sp,0
addiu $sp,$sp,-44
jal main
li $v0,10
syscall
myscanf:
li $v0,5
syscall
move $t8,$v0
sw $t8,-12($fp)
lw $t8,-12($fp)
addiu $t0,$t8,0
move $v1,$t0
jr $ra
myprintf:
lw $t8,-12($fp)
addiu $t0,$t8,0
move $a0,$t0
li $v0,1
syscall
la $a0,str0
li $v0,4
syscall
jr $ra
checkrange:
Label1:
lw $t8,-12($fp)
addiu $t0,$t8,0
li $t1,1073741824
ble $t0,$t1,Label2
lw $t8,-12($fp)
addiu $t0,$t8,-1073741824
addiu $t8,$t0,0
sw $t8,-12($fp)
j Label1
Label2:
Label3:
lw $t8,-12($fp)
addiu $t0,$t8,0
li $t1,0
bge $t0,$t1,Label4
lw $t8,-12($fp)
addiu $t0,$t8,1073741824
addiu $t8,$t0,0
sw $t8,-12($fp)
j Label3
Label4:
lw $t8,-12($fp)
addiu $t0,$t8,0
move $v1,$t0
jr $ra
reduce:
lw $t8,-12($fp)
addiu $t0,$t8,0
li $t1,0
bne $t0,$t1,Label6
lw $t8,-16($fp)
lw $t9,-20($fp)
addu $t0,$t8,$t9
sw $t0,-336($sp)
sw $fp,-328($sp)
sw $ra,-332($sp)
addiu $fp,$sp,-324
addiu $sp,$sp,-344
jal checkrange
addiu $sp,$sp,344
lw $fp,-328($sp)
lw $ra,-332($sp)
addiu $t0,$v1,0
move $v1,$t0
jr $ra
j Label5
Label6:
Label5:
lw $t8,-12($fp)
addiu $t0,$t8,0
li $t1,1
bne $t0,$t1,Label8
li $t0,0
addiu $t8,$t0,0
sw $t8,-24($fp)
li $t0,1
addiu $t8,$t0,0
sw $t8,-28($fp)
Label9:
lw $t8,-28($fp)
addiu $t0,$t8,0
li $t1,1073741824
bge $t0,$t1,Label10
lw $t8,-16($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t0
lw $t8,-16($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t1
div $t1,$t1,2
mul $t1,$t1,2
subu $t1,$t0,$t1
lw $t8,-20($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t0
lw $t8,-20($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t2
div $t2,$t2,2
mul $t2,$t2,2
subu $t2,$t0,$t2
bne $t1,$t2,Label12
lw $t8,-24($fp)
mul $t0,$t8,2
addiu $t8,$t0,0
sw $t8,-24($fp)
j Label11
Label12:
lw $t8,-24($fp)
mul $t0,$t8,2
addiu $t0,$t0,1
addiu $t8,$t0,0
sw $t8,-24($fp)
Label11:
lw $t8,-28($fp)
mul $t0,$t8,2
addiu $t8,$t0,0
sw $t8,-28($fp)
j Label9
Label10:
lw $t8,-24($fp)
addiu $t0,$t8,0
move $v1,$t0
jr $ra
j Label7
Label8:
Label7:
lw $t8,-12($fp)
addiu $t0,$t8,0
li $t1,2
bne $t0,$t1,Label14
lw $t8,-16($fp)
addiu $t0,$t8,0
lw $t8,-20($fp)
addiu $t1,$t8,0
ble $t0,$t1,Label16
lw $t8,-16($fp)
addiu $t0,$t8,0
move $v1,$t0
jr $ra
j Label15
Label16:
lw $t8,-20($fp)
addiu $t0,$t8,0
move $v1,$t0
jr $ra
Label15:
j Label13
Label14:
Label13:
lw $t8,-12($fp)
addiu $t0,$t8,0
li $t1,3
bne $t0,$t1,Label18
li $t0,0
addiu $t8,$t0,0
sw $t8,-24($fp)
li $t0,1
addiu $t8,$t0,0
sw $t8,-28($fp)
Label19:
lw $t8,-28($fp)
addiu $t0,$t8,0
li $t1,1073741824
bge $t0,$t1,Label20
lw $t8,-16($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t0
lw $t8,-16($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t1
div $t1,$t1,2
mul $t1,$t1,2
subu $t1,$t0,$t1
li $t0,1
bne $t1,$t0,Label22
lw $t8,-24($fp)
mul $t0,$t8,2
addiu $t0,$t0,1
addiu $t8,$t0,0
sw $t8,-24($fp)
j Label21
Label22:
lw $t8,-20($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t0
lw $t8,-20($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t1
div $t1,$t1,2
mul $t1,$t1,2
subu $t1,$t0,$t1
li $t0,1
bne $t1,$t0,Label24
lw $t8,-24($fp)
mul $t0,$t8,2
addiu $t0,$t0,1
addiu $t8,$t0,0
sw $t8,-24($fp)
j Label23
Label24:
Label23:
lw $t8,-24($fp)
mul $t0,$t8,2
addiu $t8,$t0,0
sw $t8,-24($fp)
Label21:
lw $t8,-28($fp)
mul $t0,$t8,2
addiu $t8,$t0,0
sw $t8,-28($fp)
j Label19
Label20:
lw $t8,-24($fp)
addiu $t0,$t8,0
move $v1,$t0
jr $ra
j Label17
Label18:
Label17:
lw $t8,-12($fp)
addiu $t0,$t8,0
li $t1,4
bne $t0,$t1,Label26
li $t0,0
addiu $t8,$t0,0
sw $t8,-24($fp)
li $t0,1
addiu $t8,$t0,0
sw $t8,-28($fp)
Label27:
lw $t8,-28($fp)
addiu $t0,$t8,0
li $t1,1073741824
bge $t0,$t1,Label28
lw $t8,-16($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t0
lw $t8,-16($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t1
div $t1,$t1,2
mul $t1,$t1,2
subu $t1,$t0,$t1
li $t0,1
bne $t1,$t0,Label30
lw $t8,-20($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t0
lw $t8,-20($fp)
lw $t9,-28($fp)
div $t8,$t9
mflo $t1
div $t1,$t1,2
mul $t1,$t1,2
subu $t1,$t0,$t1
li $t0,1
bne $t1,$t0,Label32
lw $t8,-24($fp)
mul $t0,$t8,2
addiu $t0,$t0,1
addiu $t8,$t0,0
sw $t8,-24($fp)
j Label31
Label32:
lw $t8,-24($fp)
mul $t0,$t8,2
addiu $t8,$t0,0
sw $t8,-24($fp)
Label31:
j Label29
Label30:
lw $t8,-24($fp)
mul $t0,$t8,2
addiu $t8,$t0,0
sw $t8,-24($fp)
Label29:
lw $t8,-28($fp)
mul $t0,$t8,2
addiu $t8,$t0,0
sw $t8,-28($fp)
j Label27
Label28:
lw $t8,-24($fp)
addiu $t0,$t8,0
move $v1,$t0
jr $ra
j Label25
Label26:
Label25:
li $t0,0
move $v1,$t0
jr $ra
getvalue:
lw $t8,-20($fp)
addiu $t0,$t8,0
li $t1,0
bge $t0,$t1,Label34
li $t0,0
move $v1,$t0
jr $ra
j Label33
Label34:
Label33:
lw $t8,-24($fp)
addiu $t0,$t8,0
li $t1,0
bge $t0,$t1,Label36
li $t0,0
move $v1,$t0
jr $ra
j Label35
Label36:
Label35:
lw $t8,-20($fp)
addiu $t0,$t8,0
lw $t8,-12($fp)
addiu $t1,$t8,0
blt $t0,$t1,Label38
li $t0,0
move $v1,$t0
jr $ra
j Label37
Label38:
Label37:
lw $t8,-24($fp)
addiu $t0,$t8,0
lw $t8,-16($fp)
addiu $t1,$t8,0
blt $t0,$t1,Label40
li $t0,0
move $v1,$t0
jr $ra
j Label39
Label40:
Label39:
lw $t8,-20($fp)
lw $t9,-16($fp)
mul $t0,$t8,$t9
lw $t9,-24($fp)
addu $t0,$t0,$t9
mul $t0,$t0,4
addiu $t0,$t0,0
addu $t0,$k0,$t0
lw $t1,0($t0)
move $v1,$t1
jr $ra
convn:
li $t8,0
sw $t8,-28($fp)
li $t8,0
sw $t8,-32($fp)
li $t8,1
sw $t8,-48($fp)
li $t8,1
sw $t8,-52($fp)
li $t8,1
sw $t8,-56($fp)
li $t8,1
sw $t8,-60($fp)
Label41:
lw $t8,-48($fp)
addiu $t0,$t8,0
li $t1,1
bne $t0,$t1,Label42
li $t0,0
addiu $t8,$t0,0
sw $t8,-32($fp)
Label43:
lw $t8,-52($fp)
addiu $t0,$t8,0
li $t1,1
bne $t0,$t1,Label44
li $t0,0
addiu $t8,$t0,0
sw $t8,-44($fp)
lw $t8,-24($fp)
div $t0,$t8,2
lw $t8,-28($fp)
subu $t0,$t8,$t0
addiu $t8,$t0,0
sw $t8,-36($fp)
Label45:
lw $t8,-56($fp)
addiu $t0,$t8,0
li $t1,1
bne $t0,$t1,Label46
lw $t8,-24($fp)
div $t0,$t8,2
lw $t8,-32($fp)
subu $t0,$t8,$t0
addiu $t8,$t0,0
sw $t8,-40($fp)
Label47:
lw $t8,-60($fp)
addiu $t0,$t8,0
li $t1,1
bne $t0,$t1,Label48
lw $t8,-12($fp)
addiu $t0,$t8,0
lw $t8,-44($fp)
addiu $t1,$t8,0
lw $t8,-16($fp)
addiu $t0,$t8,0
lw $t8,-20($fp)
addiu $t1,$t8,0
lw $t8,-36($fp)
addiu $t2,$t8,0
lw $t8,-40($fp)
addiu $t3,$t8,0
sw $t0,-200($sp)
sw $t1,-204($sp)
sw $t2,-208($sp)
sw $t3,-212($sp)
sw $fp,-192($sp)
sw $ra,-196($sp)
addiu $fp,$sp,-188
addiu $sp,$sp,-220
jal getvalue
addiu $sp,$sp,220
lw $fp,-192($sp)
lw $ra,-196($sp)
addiu $t0,$v1,0
sw $t0,-200($sp)
sw $t1,-204($sp)
sw $t0,-208($sp)
sw $fp,-192($sp)
sw $ra,-196($sp)
addiu $fp,$sp,-188
addiu $sp,$sp,-224
jal reduce
addiu $sp,$sp,224
lw $fp,-192($sp)
lw $ra,-196($sp)
addiu $t0,$v1,0
addiu $t8,$t0,0
sw $t8,-44($fp)
lw $t8,-40($fp)
addiu $t0,$t8,1
addiu $t8,$t0,0
sw $t8,-40($fp)
lw $t8,-40($fp)
addiu $t0,$t8,0
lw $t8,-24($fp)
div $t1,$t8,2
lw $t8,-32($fp)
addu $t1,$t8,$t1
blt $t0,$t1,Label50
li $t0,0
addiu $t8,$t0,0
sw $t8,-60($fp)
j Label49
Label50:
Label49:
j Label47
Label48:
li $t0,1
addiu $t8,$t0,0
sw $t8,-60($fp)
lw $t8,-36($fp)
addiu $t0,$t8,1
addiu $t8,$t0,0
sw $t8,-36($fp)
lw $t8,-36($fp)
addiu $t0,$t8,0
lw $t8,-24($fp)
div $t1,$t8,2
lw $t8,-28($fp)
addu $t1,$t8,$t1
blt $t0,$t1,Label52
li $t0,0
addiu $t8,$t0,0
sw $t8,-56($fp)
j Label51
Label52:
Label51:
j Label45
Label46:
li $t0,1
addiu $t8,$t0,0
sw $t8,-56($fp)
lw $t8,-28($fp)
lw $t9,-20($fp)
mul $t0,$t8,$t9
lw $t9,-32($fp)
addu $t0,$t0,$t9
lw $t8,-44($fp)
addiu $t1,$t8,0
mul $t0,$t0,4
addiu $t0,$t0,4000
addu $t0,$k0,$t0
sw $t1,0($t0)
lw $t8,-32($fp)
addiu $t1,$t8,1
addiu $t8,$t1,0
sw $t8,-32($fp)
lw $t8,-32($fp)
addiu $t0,$t8,0
lw $t8,-20($fp)
addiu $t1,$t8,0
blt $t0,$t1,Label54
li $t0,0
addiu $t8,$t0,0
sw $t8,-52($fp)
j Label53
Label54:
Label53:
j Label43
Label44:
li $t0,1
addiu $t8,$t0,0
sw $t8,-52($fp)
lw $t8,-28($fp)
addiu $t0,$t8,1
addiu $t8,$t0,0
sw $t8,-28($fp)
lw $t8,-28($fp)
addiu $t0,$t8,0
lw $t8,-16($fp)
addiu $t1,$t8,0
blt $t0,$t1,Label56
li $t0,0
addiu $t8,$t0,0
sw $t8,-48($fp)
j Label55
Label56:
Label55:
j Label41
Label42:
li $t0,0
move $v1,$t0
jr $ra
mymemmove:
li $t8,0
sw $t8,-16($fp)
Label57:
lw $t8,-16($fp)
addiu $t0,$t8,0
lw $t8,-12($fp)
addiu $t1,$t8,0
bge $t0,$t1,Label58
lw $t8,-16($fp)
addiu $t0,$t8,0
lw $t8,-16($fp)
addiu $t1,$t8,0
mul $t1,$t1,4
addiu $t1,$t1,4000
addu $t1,$k0,$t1
lw $t2,0($t1)
mul $t0,$t0,4
addiu $t0,$t0,0
addu $t0,$k0,$t0
sw $t2,0($t0)
lw $t8,-16($fp)
addiu $t0,$t8,1
addiu $t8,$t0,0
sw $t8,-16($fp)
j Label57
Label58:
jr $ra
main:
li $t8,0
sw $t8,-36($fp)
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-148
jal myscanf
addiu $sp,$sp,148
lw $fp,-132($sp)
lw $ra,-136($sp)
addiu $t0,$v1,0
addiu $t8,$t0,0
sw $t8,-12($fp)
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-148
jal myscanf
addiu $sp,$sp,148
lw $fp,-132($sp)
lw $ra,-136($sp)
addiu $t0,$v1,0
addiu $t8,$t0,0
sw $t8,-16($fp)
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-148
jal myscanf
addiu $sp,$sp,148
lw $fp,-132($sp)
lw $ra,-136($sp)
addiu $t0,$v1,0
addiu $t8,$t0,0
sw $t8,-20($fp)
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-148
jal myscanf
addiu $sp,$sp,148
lw $fp,-132($sp)
lw $ra,-136($sp)
addiu $t0,$v1,0
addiu $t8,$t0,0
sw $t8,-28($fp)
Label59:
lw $t8,-36($fp)
addiu $t0,$t8,0
lw $t8,-28($fp)
addiu $t1,$t8,0
bge $t0,$t1,Label60
lw $t8,-36($fp)
addiu $t0,$t8,0
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-148
jal myscanf
addiu $sp,$sp,148
lw $fp,-132($sp)
lw $ra,-136($sp)
addiu $t0,$v1,0
mul $t0,$t0,4
addiu $t0,$t0,0
addu $t0,$k0,$t0
sw $t0,0($t0)
lw $t8,-36($fp)
addiu $t0,$t8,1
addiu $t8,$t0,0
sw $t8,-36($fp)
j Label59
Label60:
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-148
jal myscanf
addiu $sp,$sp,148
lw $fp,-132($sp)
lw $ra,-136($sp)
addiu $t0,$v1,0
addiu $t8,$t0,0
sw $t8,-24($fp)
li $t0,0
addiu $t8,$t0,0
sw $t8,-36($fp)
Label61:
lw $t8,-36($fp)
addiu $t0,$t8,0
lw $t8,-24($fp)
addiu $t1,$t8,0
bge $t0,$t1,Label62
lw $t8,-36($fp)
addiu $t0,$t8,0
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-148
jal myscanf
addiu $sp,$sp,148
lw $fp,-132($sp)
lw $ra,-136($sp)
addiu $t0,$v1,0
mul $t0,$t0,4
addiu $t0,$t0,8000
addu $t0,$k0,$t0
sw $t0,0($t0)
lw $t8,-36($fp)
addiu $t0,$t8,1
addiu $t8,$t0,0
sw $t8,-36($fp)
j Label61
Label62:
li $t0,0
addiu $t8,$t0,0
sw $t8,-36($fp)
Label63:
lw $t8,-36($fp)
addiu $t0,$t8,0
lw $t8,-24($fp)
addiu $t1,$t8,0
bge $t0,$t1,Label64
lw $t8,-36($fp)
addiu $t0,$t8,0
mul $t0,$t0,4
addiu $t0,$t0,8000
addu $t0,$k0,$t0
lw $t1,0($t0)
lw $t8,-16($fp)
addiu $t0,$t8,0
lw $t8,-20($fp)
addiu $t2,$t8,0
lw $t8,-12($fp)
addiu $t3,$t8,0
sw $t1,-140($sp)
sw $t0,-144($sp)
sw $t2,-148($sp)
sw $t3,-152($sp)
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-196
jal convn
addiu $sp,$sp,196
lw $fp,-132($sp)
lw $ra,-136($sp)
lw $t8,-16($fp)
lw $t9,-20($fp)
mul $t0,$t8,$t9
sw $t0,-140($sp)
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-152
jal mymemmove
addiu $sp,$sp,152
lw $fp,-132($sp)
lw $ra,-136($sp)
lw $t8,-36($fp)
addiu $t0,$t8,1
addiu $t8,$t0,0
sw $t8,-36($fp)
j Label63
Label64:
li $t0,0
addiu $t8,$t0,0
sw $t8,-36($fp)
Label65:
lw $t8,-36($fp)
addiu $t0,$t8,0
lw $t8,-16($fp)
lw $t9,-20($fp)
mul $t1,$t8,$t9
bge $t0,$t1,Label66
lw $t8,-36($fp)
addiu $t0,$t8,0
mul $t0,$t0,4
addiu $t0,$t0,0
addu $t0,$k0,$t0
lw $t1,0($t0)
sw $t1,-140($sp)
sw $fp,-132($sp)
sw $ra,-136($sp)
addiu $fp,$sp,-128
addiu $sp,$sp,-148
jal myprintf
addiu $sp,$sp,148
lw $fp,-132($sp)
lw $ra,-136($sp)
lw $t8,-36($fp)
addiu $t0,$t8,1
addiu $t8,$t0,0
sw $t8,-36($fp)
j Label65
Label66:
jr $ra
