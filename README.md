This repository is an implementation of a simple simulator for the LC-3 processor. 


##Command line front end

All of these user input commands are supported by the simulator

- step [n]

  If no argument, Executes 1 instruction.
  Otherwise, executes n instructions

- quit

  Quits the simulator.

- continue

  Runs until the program halts.

- registers

  Dumps the registers of the machine. It displays each register in both hexadecimal and
  signed decimal. It also dumps the value of the PC (in hex) and the current condition code.

- dump start [end]

  Dumps the contents of memory from start to end, inclusive.
  (start and end will be in HEXADECIMAL i.e. x0, x1000, xFFFF)
  If end is not given then it will only print out memory[start]

- list start [end]

  Disassembles the contents of memory from start to end, inclusive.
  (start and end will be in HEXADECIMAL i.e. x0, x1000, xFFFF)
  If end is not given then it will disassemble only memory[start]

- setaddr addr value

  Sets memory address addr to value. Addr is in hexadecimal and
  value is in decimal

- setreg Rn value

  Sets a register to a value ex setreg R0 10 will set R0 to 10.

The [] syntax means that the argument is optional.
