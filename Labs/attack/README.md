# Attack Lab實驗紀錄

## Intro

This file contains materials for one instance of the **attacklab**.

Files:

```bash
ctarget
```

Linux binary with code-injection vulnerability.  To be used for phases
1-3 of the assignment.

    rtarget

Linux binary with return-oriented programming vulnerability.  To be
used for phases 4-5 of the assignment.

```bash
 cookie.txt
```

Text file containing 4-byte signature required for this lab instance.

```bash
 farm.c
```

Source code for gadget farm present in this instance of `rtarget`.  You
can compile (use flag `-Og`) and disassemble it to look for gadgets.

```bash
 hex2raw
```

Utility program to generate byte sequences.  See documentation in lab
handout.

