# Shell Lab實驗紀錄

## Intro

Files:

- `Makefile` &rarr; Compiles your shell program and runs the tests
- `README.md`  &rarr; This file
- `tsh.c` &rarr; The shell program that you will write and hand in
- `tshref` &rarr; The reference shell binary.

### The remaining files are used to test your shell
- `sdriver.pl` &rarr; The trace-driven shell driver
- `trace*.txt` &rarr; The 15 trace files that control the shell driver
- `tshref.out`  &rarr;  Example output of the reference shell on all 15 traces

### Little C programs that are called by the trace files
- `myspin.c` &rarr; Takes argument <n> and spins for <n> seconds
- `mysplit.c` &rarr; Forks a child that spins for <n> seconds
- `mystop.c ` &rarr; Spins for <n> seconds and sends SIGTSTP to itself
- `myint.c` &rarr; Spins for <n> seconds and sends SIGINT to itself

