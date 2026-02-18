# Preemptive Priority Scheduler using Binomial Heap (C)

This project implements a **preemptive priority CPU scheduling simulation** in ANSI C using a **Binomial Heap** as the priority queue data structure.

The scheduler dynamically recalculates priorities based on a mathematical function and evaluates performance through Average Waiting Time (AWT) optimization across different quantum values.

---

##  Project Overview

The system simulates a preemptive CPU scheduling environment where:

- Processes arrive dynamically.
- Priority is computed using a mathematical formula.
- Running processes may be preempted.
- A Binomial Heap maintains the ready queue.
- Performance is evaluated using Average Waiting Time (AWT).

---

##  Core Features

-  Binomial Heap implementation from scratch
-  Dynamic priority recalculation after preemption
-  Strict tie-breaking based on arrival time
-  Real-time process arrival handling
-  Gantt Chart generation
-  Waiting Time table per process
-  Quantum optimization analysis

---

##  Priority Function

For preempted processes, priority is recalculated as:

c(e) = (1 / exp(-(2e / 3Emax)^3)) * e

Where:

- `e` = remaining execution time  
- `Emax` = maximum execution time among all processes  

If two processes share the same original execution time, arrival time is used as a strict tie-breaker.

---

##  Input Format

The program reads from:

input.txt

Each line must contain:

ProcessName ExecutionTime ArrivalTime

Example:

P1 5 0

P2 3 1

P3 7 2

---

## ▶ Compilation

Compile using:

```bash
gcc -o scheduler main.c -lm
```

Run:

```bash
./scheduler
```

Note: `-lm` is required for linking the math library (pow, exp).

---
## Output

### 1) Detailed Trace Mode (Quantum = 1, 2, 3)

For predefined quantum values, the simulation prints:

- Heap snapshot at each time unit
- Priority values of processes inside the Binomial Heap
- Gantt Chart visualization
- Individual Waiting Times
- Average Waiting Time (AWT)

### 2) Optimization Summary

After the detailed scenarios, the program runs silent simulations for multiple quantum values:

Quantum | AWT

It determines the quantum that minimizes the Average Waiting Time.

---
It determines the quantum that minimizes the Average Waiting Time.

---

## Data Structures Used

- struct Process for process modeling
- Binomial Heap (tree-based priority queue)
- Linked node structure (parent, child, sibling)
- Global time tracking using a Gantt chart array

---

## Time Complexity

- Heap insertion: O(log n)
- Extract highest priority: O(log n)
- Overall simulation complexity depends on total execution time and quantum size

---

## Documentation

The detailed project report is available under:

/docs/150123051_prj2.docx

The report includes:

- Design considerations
- Priority function explanation
- Tie-breaking rules
- Optimization analysis

---

## Academic Context

Course: Data Structures  
Topic: Preemptive Priority Scheduling  
Language: ANSI C  

---

## Author
Tuba İşkol  
Marmara University - Computer Engineering



