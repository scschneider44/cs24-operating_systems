	.file	"example.c"
	.text
	.globl	ex
	.type	ex, @function
ex:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)   # a
	movl	%esi, -24(%rbp)   # b
	movl	%edx, -28(%rbp)   # c
	movl	%ecx, -32(%rbp)   # d
	movl	-24(%rbp), %eax
	subl	-28(%rbp), %eax		# subtraction occurs here. b - c goes into %eax
	imull	-20(%rbp), %eax	  # multiplication occurs here a * (b - c) goes into eax
	movl	%eax, %edx        # puts a * (b - c) in %edx
	movl	-32(%rbp), %eax   # puts d in %eax
	addl	%edx, %eax        # addition occurs here, a * (b - c) + d goes indo %eax
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	ex, .-ex
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
