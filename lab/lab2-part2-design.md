# Lab 2 Part 2 Design Doc:

## Instruction (Please Remove Upon Submission)
Follow the guidelines in [designdoc.md](designdoc.md).

This is a template for the lab 2 design doc, you are welcomed to make adjustments, but  **you must answer the provided design questions**.

Once your design doc is finished, you should **submit it on Gradescope as a pdf**.
You can generate a pdf of this markdown file by running
```
pandoc -f markdown -o lab2-dd.pdf lab/lab2-part2-design.md
```

Please include your names in the design doc you submit (helps us if you forget to tag partner on gradescope).

**Names:**

## Overview

> Explain the motivation and the goal for Lab 2. What do these system calls enable? Why do we need to add synchronization to open files first?

[ your text here ]

## Part 2: Pipe & Exec

### Major Parts

> For each part below, explain the goals and key challenges to accomplishing these goals. 
> Please also explain how they interact with Lab 1 and Lab 2 part 1.
> For example, how does Pipe interact with file syscalls from Lab 1?

Pipe 

- [ your text here, please cover pipe creation, read, write, and close ]

Exec

- [ your text here ]

Interactions

- [ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->

## In-depth Analysis and Implementation


### Pipe

**Functions To Implement & Modify**

This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify locks used and critical sections).
> Example: implement `sys_pipe` from `kernel/sysfile.c`. 
> Implement `pipe_open`, `pipe_read`, `pipe_write`, `pipe_close`.

[ your text here ]

**Design Questions**

###### Pipe Creation

`pipe` returns 2 file descriptors: one for read end and one for write end. 
How many open files should be allocated as a result of `pipe` call? 
Can the read end and write end point to the same open file?

- [ your answer here ]

The `pipe` syscall should dynamically allocate a kernel buffer used for pipe data.
How do you dynamically allocate kernel memory in xk? What is the size of the allocated kernel memory?
What is the size of your pipe metadata & data?

- [ your answer here ]

###### Pipe Read

Once a `pipe` is created, a process can invoke `read` with the read end. 
Just like reading a normal file, the read offsets advances upon each read. 
What metadata must a pipe track to support this?

- [ your answer here ]

Although we allow for partial reads, a read from an empty pipe should still block. 
What metadata must a pipe track to support this? 

- [ your answer here ]

###### Pipe Write

Invoking `write` on a pipe write end should advance the write offset upon each write. 
What metadata must a pipe track to support this?

- [ your answer here ]

When there is no room to write in the buffer, a writer should block until there is space to write. 
Is it always true that some space in the buffer will become available? Please explain.

- [ your answer here ]

In addition, `write` on a pipe must be complete (all requested bytes must be written unless there are no more read ends). 
How does your design support write sizes larger than the size of your pipe buffer?

- [ your answer here ]

Lastly, `write` must be atomic (no interleaving writes). 
How do you ensure that a new writer cannot write until the current writer is done? What if the current writer is blocked?

- [ your answer here ]

###### Pipe Close

How do you plan to track the life time of a pipe? When can a pipe be deallocated? 

- [ your answer here ]

Is it important to distinguish between a close on a read end vs a write end of the pipe?
Why or why not?

- [ your answer here ]

What happens if the last read end calls `close` while there are still blocking writers?
How does your design avoid this situation?

- [ your answer here ]

What happens if all write ends are closed while there are still blocking readers? 
How does your design avoid this situation?

- [ your answer here ]


**Corner Cases**

> Example: error checking for pipe allocation

[ your text here ]

**Test Plans**

List an order in which you will test the functionality of this component
and give a brief justification as to why your program logic will correctly handle
your listed test cases.
[ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->


### Exec

**Functions To Implement & Modify**

This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify locks used and critical sections).
> Example: Implement `sys_exec` in `kernel/sysexec.c`.

[ your text here ]


**Design Questions**

`exec` takes in a string path and a string array `argv`. 
How do you plan to traverse through the null terminated `argv`?
What validation must you perform during the traversal?

- [ your answer here ]

`exec` sets up a new address space for a process and frees the old address space. 
When is it safe to free the old address space?

- [ your answer here ]

When copying arguments onto the new stack, which address space is in use? Explain why.
 
- [ your answer here ]


**Corner Cases**

> Example: add padding before setting up the argv array because it must be word size aligned

[ your text here ]

**Test Plans** 

List an order in which you will test the functionality of this component
and give a brief justification as to why your program logic will correctly handle
your listed test cases.
[ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->


## Risk Analysis

### Unanswered Questions
> Example: is the amount of extra padding added when setting up the stack for `exec` the same every time?

[ your text here ]

### Staging of Work

[ your text here ]


### Time Estimation

> Example
>
> - Pipe (__ hours)
> - Exec (__ hours)
> - Edge cases and Error handling (__ hours)

[ your text here ]
