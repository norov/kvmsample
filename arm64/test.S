# A test code for kvmsample

.section .text

.globl start
start:
	mov	x19, #0
    
loop1:
	add	x19, x19, #1
	hvc	#0
	b loop1

