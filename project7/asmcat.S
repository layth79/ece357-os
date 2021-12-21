.text

.global start

start: 

cardinal: # main loop
	movl 	$4096, %edx
	movl 	$buffer, %ecx
	movl 	$3, %eax
	movl 	$0, %ebx
	int     $0x80
	movl 	$0, %esi
	cmpl 	$0, %eax
	je 		depart
	jl		erratum

	movl    %eax, %edx
	movl    $buffer, %ecx
	movl    $1, %ebx
	movl    $4, %eax
	int     $0x80
	jmp 	cardinal

erratum: # error
	mov 	$-1, %ebx
	movl 	$1, %eax
	int 	$0x80

depart: # exit
	movl    $0, %ebx
	movl    $1, %eax
	int     $0x80

.data

buffer:
	.ascii	"give us an A please"
