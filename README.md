# Reptile-Emulator
Simply the simulated version of the CPU based on 'Reptile' design. It takes an assembly code file as input and shows the final state of all registers and data memory.

## Details

This is an emulator that parses the Reptile Assembly code and shows register and memory contents at the end. 
The instructions that we have in reptile design are listed as:
- Arithmetic Operations (ADD, SUB, AND, OR, XOR, MOV, NOT, INC, DEC)
- Load Immediate (LDI)
- Load (LD)
- Store (ST)
- Jump (JMP)
- Jump-if-zero (JZ)

For the assembly code itself, it is divided into two parts: **data section** and **code section**. 
***Data section*** includes variables which they store some values and addresses. 
***Code section*** includes the code itself including the labels, as well.

An example input is shown in here:

<pre>
.data
  N: 60
  sum:
.code
  ldi 0 N
  ld 0 0
  jz out
  loop inc 1
  add 2 2 1
  dec 0
  jz out
  jmp loop
  out ldi 0 sum
  st 2 0
</pre>

To be able to emulate this, we need to read the code section two times. First one is for reading labels and LDI locations. 
Then we execute the code as we have initialized in the first pass.

### Example Output

<pre>
REGISTER CONTENTS:
Register 0 : 0x52f0
Register 1 : 0x000a
Register 2 : 0xfffb
Register 3 : 0x0000

DATA MEMORY CONTENTS:
N : 0x000a
sum : 0xfffb
</pre>

Here is an example showing the output of the example input shown above. It shows register and memory contents in hexadecimal format.
