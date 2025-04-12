# Lab 2 Part 1 Design Doc:

## Instruction (Please Remove Upon Submission)
Follow the guidelines in [designdoc.md](designdoc.md).

This is a template for the lab 2 design doc, you are welcomed to make adjustments, but  **you must answer the provided design questions**.

Once your design doc is finished, you should **submit it on Gradescope as a pdf**.
You can generate a pdf of this markdown file by running
```
pandoc -f markdown -o lab2-dd.pdf lab/lab2-part1-design.md
```

Please include your names in the design doc you submit (helps us if you forget to tag partner on gradescope).

**Names:**

## Overview

> Explain the motivation and the goal for Lab 2. What do these system calls enable? Why do we need to add synchronization to open files first?

[ your text here ]

### Major Parts

> For each part below, explain the goals and key challenges to accomplishing these goals (e.g., the need to resume a child with
> almost the same execution state as the parent). 
> Please also explain how different parts of this lab interact with each other. For example, how does fork interact with file synchronization?

Synchronization For Open Files

  - [ your text here ]

Fork

  - [ your text here ]

Exit

  - [ your text here ]

Wait

  - [ your text here ]

Interactions

  - [ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->

## In-depth Analysis and Implementation

### Synchronization For Open Files

**Functions To Implement & Modify**

This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify the type of locks used and which critical sections you need to guard).
> Example: modify `file_open` to lock around access to the global file info table.

[ your text here ]

**Design Questions**

> The following questions are meant to guide you through your sychronization design. You must answer them in your design doc.

Why does the global file info table need to be protected? 

- [ your answer here ]

What are the trade-offs of using a single lock for all entries of the global file table vs. per-entry lock?

- [ your answer here ]

How long are the critical sections for your lock(s)? What operations (read/write memory, or I/O request) are done while holdings the lock(s)?

- [ your answer here ]

What are your locking decisions (coarse vs fine-grained, spinlock vs sleeplock) for the global file table and why?

- [ your answer here ]

Does the per-process file descriptor table need to be protected? Why or why not?

- [ your answer here ]


**Corner Cases**

Describe any special/edge cases and how your program logic handles these cases.
> Example: initialize the added lock(s) before the first process starts

[ your text here ]

**Test Plans**

List an order in which you will test the functionality of this component
and give a brief justification as to why your program logic will correctly handle
the listed test cases.
> Example: run lab1test to make sure they still work (justify why they will work) ...

[ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->

### Fork

**Functions To Implement & Modify**

This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify locks used and critical sections you need to guard).
> Example: Implement `fork` in `kernel/proc.c`.
>
> - `fork`:
>   - allocate a new PCB (`struct proc`) via `allocproc`
>   - initialize the child process's virtual address space (`vspaceinit`)
>   - ...

[ your text here ]

**Design Questions**

How does a new process become schedulable? Take a look at `scheduler` to see how the scheduler finds candidates to schedule.

- [ your answer here ]

At what point in `fork` can a new process be safely set to schedulable? Take a look at `userinit` and see when the first process becomes schedulable.
  
- [ your answer here ]

Who may access the scheduling state of a process? How does your design protect/synchronize these accesses? 

- [ your answer here ]

Take a look at `allocproc`, what field(s) of `struct proc` are protected by `ptable.lock` and what are not? Why is this a safe decision?

- [ your answer here ]

How do you plan to track the parent child relationship between the calling process and the newly created process?

- [ your answer here ]

How does the new process inherit its parent's open files? What needs to be updated as a result?

- [ your answer here ]

**Corner Cases**

[ your text here ]

**Test Plans**

List an order in which you will test the functionality of this component
and give a brief justification as to why your program logic will correctly handle
the listed test cases.
> Example: Although test cases in `user/lab2test.c` requires `fork`, `wait`, and `exit` to be implemented,
> there are parts to lab 2 mini shell that only relies on `fork` to succeed ...

[ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->

### Exit

**Functions To Implement & Modify**

This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify locks used and critical sections).
> Example: modify `sys_exit` in `kernel/sysproc.c` to call `exit`. 
>
> Implement `exit` in `kernel/proc.c`:
>
> - `exit`:
>   - close all open files
>   - ...

[ your text here ]

**Design Questions**

As a child process, how does the exiting process inform its parent? 

- [ your answer here ]

As a parent process, how does the exiting process ensure that its children get cleaned up if it exits first?

- [ your answer here ]

Why can't an exiting process clean up all of its resources?

- [ your answer here ]

Is it safe for the child process to access its parent's PCB? What happens if the parent has already exited? 

- [ your answer here ]

What fields of `struct proc` are accessed and/or modified in `exit`? How are these accesses protected?

- [ your answer here ]


**Corner Cases**

[ your text here ]


**Test Plans**
List an order in which you will test the functionality of this component
and give a brief justification as to why your program logic will correctly handle
your listed test cases.
> Example: the test program mini shell takes in an `exit` command that only relies on `fork` and `exit` ...

[ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->

### Wait

**Functions To Implement & Modify**

This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify locks used and critical sections).
> Example: remove the `while(1)` in `sys_wait`. 
>
> Implement `wait` in `kernel/proc.c`.
>
> - `wait`: 
>   - ...

[ your text here ]

**Design Questions**

How can a parent process find its children? Does this operation need any synchronization?

- [ your answer here ]

How do you ensure that a parent process cannot wait on the same child process twice?

- [ your answer here ]

When is it safe to deallocate the child's PCB (`struct proc`)? How do you deallocate a PCB? What fields need to be cleaned up?

- [ your answer here ]

**Corner Cases**

> Example: wait when no children

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
> Example: if a parent opens more files after `fork`, should the child process inherit them too?

[ your text here ]

### Staging of Work

[ your text here ]


### Time Estimation

> Example
>
> - File Synchronization (__ hours)
> - Fork (__ hours)
> - Exit (__ hours)
> - Wait (__ hours)
> - Edge cases and Error handling (__ hours)

[ your text here ]
