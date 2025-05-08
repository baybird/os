# Lab 3 Design Doc: Address Space Management

## Instruction (Please Remove Upon Submission)
Follow the guidelines in [designdoc.md](designdoc.md).

This is a template for lab 3 design doc, you are welcomed to make adjustments, but  **you must answer the provided design questions**.

Once your design doc is finished, you should **submit it on Gradescope as a pdf**.
You can generate a pdf of this markdown file by running
```
pandoc -f markdown -o lab3-dd.pdf lab/lab3-design.md
```

Please include your names in the design doc you submit (helps us if you forget to tag partner on gradescope).

**Names:**

## Overview

[ your text here ]

### Major Parts
> For each part below, explain the goals and key challenges to accomplishing these goals.
> Please also explain how different parts of this lab interact with each other.

Heap Growth

  - [ your text here ]

Stack Growth

  - [ your text here ]

Copy-on-write Fork

  - [ your text here ]

Interactions with Page Fault Handler

  - [ your text here ]


<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->

## In-depth Analysis and Implementation

### Heap Growth

**Functions To Implement & Modify**

> This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify the type of locks used and which critical sections you need to guard).

[ your text here ]

**Design Questions**

> Heap growth is not demand paged in xk, meaning that the kernel allocates frames and maps them (`vregionaddmap`) as needed upon `sbrk`. 
How does `vregionaddmap` allocates a physical frame? What structures are updated as a result? 

- [ your answer here ]

> Does `vregionaddmap` update the machine dependent page table? If so, what line performs the update? If not, how should you update the machine dependent page table?

- [ your answer here ]

> Does `sbrk` always cause new page(s) to be mapped? Justify your answer.

- [ your answer here ]

> `sbrk` cannot succeed if the resulting heap will collide with an already allocated region within the virtual address space. 
How do you determine whether growing the heap will cause a collision?

- [ your answer here ]


**Corner Cases**

> Describe any special/edge cases and how your program logic handles these cases.

[ your text here ]

**Test Plans**

> List the specific behaviors/cases that the provided tests cover (along with any
other important cases that you might think of that the tests don't cover) **and** 
justify how and why your program logic will correctly handle the tested behaviors.

[ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->

### Stack Growth

**Functions To Implement & Modify**

> This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify the type of locks used and which critical sections you need to guard).

[ your text here ]

**Design Questions**

> Stack growth must be demand paged, meaning that the growth needs to be handled through a page fault. 
Upon a page fault, what value(s) do we expect as the present bit (b0) of the error code (`tf->err`)?

- [ your answer here ]

> Upon a page fault, what value(s) do we expect as the read/write bit (b1) of the error code (`tf->err`)?
Can a stack growth happen as a result of read and write?

- [ your answer here ]

> Upon a page fault, what value(s) do we expect as the user/kernel bit (b2) of the error code (`tf->err`)?
Can a stack growth occur within the kernel? If yes, explain a scenario that leads to it, if not, explain why not.

- [ your answer here ]

> Upon a page fault, how do you tell whether the faulting address is within the the stack range?

- [ your answer here ]

> Is there any synchronization necessary when handling stack growth?

- [ your answer here ]

> After handling the stack growth, is it necessary to flush the TLB? Why or why not?

- [ your answer here ]


**Corner Cases**

> Describe any special/edge cases and how your program logic handles these cases.

[ your text here ]

**Test Plans**

> List the specific behaviors/cases that the provided tests cover (along with any
other important cases that you might think of that the tests don't cover) **and** 
justify how and why your program logic will correctly handle the tested behaviors.

[ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->

### Copy-on-write Fork

**Functions To Implement & Modify**

> This section should describe the behavior of the functions in sufficient detail that
another student/TA would be convinced of its correctness. (include any data structures accessed, 
structs modified and specify the type of locks used and which critical sections you need to guard).

[ your text here ]

**Design Questions**

> At fork time, what needs to be changed in parent's vspace to reflect the copy-on-write?

- [ your answer here ]

> After a cow fork, is it necessary to flush the TLB? Why or why not?

- [ your answer here ]

> Upon a cow page fault, what value(s) do we expect as the present bit (b0) of the error code (`tf->err`)?

- [ your answer here ]

> Upon a cow page fault, what value(s) do we expect as the read/write bit (b1) of the error code (`tf->err`)?

- [ your answer here ]

> Upon a cow page fault, what value(s) do we expect as the user/kernel bit (b2) of the error code (`tf->err`)?
Can a cow occur within the kernel? If yes, explain a scenario that leads to it, if not, explain why not.

- [ your answer here ]

> How do you distinguish a cow page fault from an actual permission violation? 

- [ your answer here ]

> Is there any synchronization necessary when handling a cow fault? How do you plan to handle multiple cow page faults backed by the same frame?

- [ your answer here ]

> After handling the cow, is it necessary to flush the TLB? Why or why not?

- [ your answer here ]

> A parent may fork a child who then forks another child, how do you handle the nested fork case?
Please explain what needs to be considered at `fork` time and at page fault time.

- [ your answer here ]


**Corner Cases**

> Describe any special/edge cases and how your program logic handles these cases.

[ your text here ]


**Test Plans**

> List the specific behaviors/cases that the provided tests cover (along with any
other important cases that you might think of that the tests don't cover) **and** 
justify how and why your program logic will correctly handle the tested behaviors.

[ your text here ]

<!-- for formatting, do not remove -->
\newpage
<!-- for formatting, do not remove -->


## Risk Analysis

### Unanswered Questions

[ your text here ]

### Staging of Work

[ your text here ]

### Time Estimation

[ your text here ]
