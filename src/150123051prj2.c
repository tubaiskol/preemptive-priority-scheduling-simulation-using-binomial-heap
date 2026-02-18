#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//change the file name in one place
#define INPUT_FILENAME "input.txt"
//to set safe limits for arrays, do not have memory errors
#define MAX_PROCESSES 100
#define MAX_TIME 1000 

typedef struct {
    char name[10]; //P1,P2..
    int id;
    int e_original;  //original execution time c(e) total time to do work
    int e_remaining; //left time for running simulation when e_rem= work done 
    int t_arrival;   //when process appears
    int t_last_queue;  //to calculate waiting time (time =  entered the queue)
    int total_wait; //accumulate the total waited time in process
    double prio_val; //calculated priority value
} Process;

typedef struct BHNode {
    Process p; //process data in a node
    int degree;  //BH property: number of children
    struct BHNode *child;
    struct BHNode *parent;
    struct BHNode *sibling;
} BHNode;

typedef struct BinomialHeap {
    BHNode *head; //points to root 
} BinomialHeap;

int E_MAX = 0; //longest execution time

int gantt_chart[MAX_TIME];
int gantt_end = 0; //total duration of the simulation

//calculate priority value f(ei, tarr(i)) function in given project file
double calculate_priority_value(int e_rem, int e_max, int is_first) {
    double ratio, exponent;
    
    //First insertion priority = Execution Time c(ei)=1 prio = 1*ei
    if (is_first) 
        return (double)e_rem;
    //prevent division by zero    
    if (e_max == 0) 
        return (double)e_rem;
    //c(e) = 1 / exp( - (2*e / 3*e_max)^3 )    
    ratio = (2.0 * e_rem) / (3.0 * e_max);
    exponent = -pow(ratio, 3);
    //Result: c(e) * e_remaining
    return (1.0 / exp(exponent)) * e_rem;
}

//text output for priority values and process matching
// force_arrival: A flag (1 or 0). If 1, we MUST show "Arrival Time" instead of the formula.
void get_prio_string(char *buffer, Process p, int force_arrival) {
    double val;
    // If forced to show arrival. two have same original time and output showing arrival time
    if (force_arrival) {
        sprintf(buffer, "%s: %d", p.name, p.t_arrival);
    } 
    // If it's the very first insertion (Standard). has nor runned yet
    else if (p.e_remaining == p.e_original && p.t_last_queue == p.t_arrival) {
        sprintf(buffer, "%s: %d", p.name, p.e_remaining);
    } 
    // Otherwise show the formula, ran before and was preempted
    else {
        val = calculate_priority_value(p.e_remaining, E_MAX, 0);
        sprintf(buffer, "%s: (1/exp-(2*%d/(3*%d))^3)*%d = %.3f", 
                p.name, p.e_remaining, E_MAX, p.e_remaining, val);
    }
}

//allocate memory for a heap node
BHNode *nodeInit(Process p) {
    BHNode *node = (BHNode *)malloc(sizeof(BHNode));
    if (node != NULL) {
        node->p = p; 
        node->degree = 0;
        node->child = NULL; 
        node->parent = NULL;
        node->sibling = NULL;
    }
    return node;
}
//allocate memory for an empty heap 
BinomialHeap *heapInit() {
    BinomialHeap *heap = (BinomialHeap *)malloc(sizeof(BinomialHeap));
    if (heap != NULL) {
        heap->head = NULL; 
    }
    return heap;
}

//compare the process a vs process b
int isHigherPriority(BHNode *a, BHNode *b) {
    
    // If Original Execution Times are equal, strictly use Arrival Time
    // we ignore the formula. We strictly look at who arrived first.
    if (a->p.e_original == b->p.e_original) {
        return (a->p.t_arrival < b->p.t_arrival);
    }
    // Otherwise compare Calculated Priorities
    // If priorities are different, the LOWER number wins (Min-Heap property)
    if (a->p.prio_val != b->p.prio_val) {
        return (a->p.prio_val < b->p.prio_val);
    }
    // If logic fails(very rare), just pick the one who arrived first.
    return (a->p.t_arrival < b->p.t_arrival);
}

BHNode *heapMerge(BinomialHeap *h1, BinomialHeap *h2) {
    BHNode *head = NULL, *tail = NULL, *n1 = h1->head, *n2 = h2->head;
    if (!n1) 
        return n2; 
    if (!n2) 
        return n1;
    if (n1->degree <= n2->degree){
        head = n1; 
        n1 = n1->sibling;
    }
    else{ 
        head = n2; 
        n2 = n2->sibling; 
    }
    tail = head;
    while (n1 && n2) {
        if (n1->degree <= n2->degree){
            tail->sibling = n1;
            n1 = n1->sibling;
        }
        else{ 
            tail->sibling = n2;
            n2 = n2->sibling;
        }
        tail = tail->sibling;
    }
    tail->sibling = (n1) ? n1 : n2;
    return head;
}

BHNode *heapUnion(BinomialHeap *orig, BinomialHeap *uni) {
    BHNode *new_head = heapMerge(orig, uni);
    orig->head = NULL; 
    uni->head = NULL;
    if (!new_head) 
        return NULL;
    BHNode *prev = NULL, *aux = new_head, *next = aux->sibling;
    while (next) {
        if ((aux->degree != next->degree) || (next->sibling && next->sibling->degree == aux->degree)) {
            prev = aux; aux = next;
        } else {
            if (isHigherPriority(aux, next)) {
                aux->sibling = next->sibling; next->parent = aux; next->sibling = aux->child; aux->child = next; aux->degree++;
            } else {
                if (!prev) new_head = next; 
                else prev->sibling = next;
                aux->parent = next; aux->sibling = next->child; next->child = aux; next->degree++; aux = next;
            }
        }
        next = aux->sibling;
    }
    return new_head;
}

void heapInsert(BinomialHeap *heap, Process p) {
    BinomialHeap *temp = heapInit();
    if (temp != NULL) {
        temp->head = nodeInit(p);
        if (temp->head != NULL) {
            heap->head = heapUnion(heap, temp);
        }
        free(temp);
    }
}

/* Finds the node with the highest priority (using isHigherPriority)
removes it, repairs the heap, and returns the process data */
Process extract_process(BinomialHeap *heap) {
    Process ret = {0};
    BHNode *min, *min_prev, *prev, *curr;
    BHNode *child, *new_head, *next;
    BinomialHeap *temp;

    if (!heap->head) 
        return ret;
    
    min = heap->head;
    min_prev = NULL;
    prev = min;
    curr = min->sibling;
    
    while (curr) {
        if (isHigherPriority(curr, min)){
            min = curr; min_prev = prev;
        }
        prev = curr; 
        curr = curr->sibling;
    }
    ret = min->p;
    if (min == heap->head)
        heap->head = min->sibling;
    else if 
        (min_prev) min_prev->sibling = min->sibling;
    
    child = min->child;
    new_head = NULL;
    
    while (child) {
        next = child->sibling;
        child->sibling = new_head;
        child->parent = NULL;
        new_head = child; 
        child = next;
    }
    temp = heapInit();
    if (temp != NULL) {
        temp->head = new_head;
        heap->head = heapUnion(heap, temp);
        free(temp); 
    }
    free(min);
    return ret;
}

int heapIsEmpty(BinomialHeap *heap){ 
    return (heap->head == NULL); 
}

void collect_nodes(BHNode *node, Process *list, int *count) {
    if (!node) 
        return;
    list[(*count)++] = node->p;
    collect_nodes(node->child, list, count);
    collect_nodes(node->sibling, list, count);
}

// prints the detailed trace table.
void print_heap_snapshot(BinomialHeap *heap, int time) {
    //Collect all nodes into a simple array 'list', coppies every process from heap to array
    Process list[MAX_PROCESSES];
    Process temp;
    int count = 0;
    int i, j, k; // Loop variables declared here
    char names[1024] = ""; //will hold text P4,P5
    char formulas[2048] = "";  // will hold text like P4: 5, P5: 6
    int special_tie_note = 0; //a flag 0 or 1 when two processes share the same e_original(p4 = p5)0
    char buf[256];
    int show_arrival;

    collect_nodes(heap->head, list, &count);

    //bubble sort by id in temp list
    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (list[j].id > list[j + 1].id) {
                temp = list[j]; 
                list[j] = list[j + 1]; 
                list[j + 1] = temp;
            }
        }
    }

    if (count == 0) {
        strcpy(names, "EMPTY");
    } else {
        for (i = 0; i < count; i++) {
            //build tne name string
            strcat(names, list[i].name); // Add "P1" to names string
            if (i < count - 1)
                strcat(names, ", ");  // Add comma if it's not the last one

            show_arrival = 0; 

            /*
            If we find another process that has the exact same original length 
            (like P4 and P5 both have e=2 at time=6) 
            AND they are both newly arriving, 
            we must display their Arrival Time instead of the priority formula. 
             */ 
            for (k = 0; k < count; k++) {
                if (i != k){ // Don't compare a process to itself
                    if (list[i].e_original == list[k].e_original && //if same original execution time
                        list[i].e_remaining == list[i].e_original && // if "i"th first insertion
                        list[k].e_remaining == list[k].e_original) { // if "k"th first insertion
                        show_arrival = 1;  // YES: Switch display mode to Arrival Time
                        special_tie_note = 1;// YES: Turn on the flag to print the note later
                    }
                }
            }
            /* in get_prio_string function , show_arrival 
            If 1: It returns just the arrival time (e.g., "P4: 5").
            If 0: It returns the calculated formula (e.g., "P1: (1/exp...)").
            */
            get_prio_string(buf, list[i], show_arrival);
            //appends new text to main formula
            strcat(formulas, buf);
            if (i < count - 1)
                strcat(formulas, ", ");
        }
        
        //if we detected that specific tie condition (because we found P4 and P5
        if (special_tie_note) {
             strcat(formulas, " (both have the same e value, so priority is tarr)");
        }
    }
    //Print the formatted row with Time, Names, and Formulas
    printf("%-5d %-20s %s\n", time, names, formulas);
}

/* Checks for new arrivals at 'current_time' and adds them to the Heap
behaves as a receptionist: 
look at the list of incoming jobs and decide if anyone new has arrived at the current clock tick 
If they have, it calculates their initial priority and puts them into the waiting line (the Heap)*/

void check_arrivals(BinomialHeap *bh, Process *jobs, int count, int current_time, int *job_idx) {
    //Process* jobs: sorted by arrival time, count: total number of jobs
    while (*job_idx < count && jobs[*job_idx].t_arrival <= current_time) {
        jobs[*job_idx].prio_val = calculate_priority_value(jobs[*job_idx].e_original, E_MAX, 1);
        //job entered the waiting line
        jobs[*job_idx].t_last_queue = current_time;
        heapInsert(bh, jobs[*job_idx]);
        (*job_idx)++; // move to next job
    }
}

// Prints the Gantt Chart and the Waiting Time Table
void print_final_report(Process *jobs, int count, int gantt_end) {
    int i, k; // Loop variables
    double grand_total_wait = 0; // to calculate final average of waiting time 

    printf("\nGantt Chart:\nTime: ");
    for(i=0; i<gantt_end; i++) 
        printf("%-3d ", i);
    //gantt_end: holds final time value
    
    printf("\nPID : ");
    for(i=0; i<gantt_end; i++) {
        //gantt_chart: check array in global at time "i" ,filled in run_simulation
        //stores the id of the running process
        if(gantt_chart[i] == -1) 
            printf("    "); //Print 4 spaces to match the time column width
        else
            printf("P%-2d ", jobs[gantt_chart[i]].id + 1); //Print "P" followed by the ID number
    }

    printf("\n\nPID   Waiting time\n");
    
    // iterate through IDs 0, 1, 2... (P1, P2...)
    for(i=0; i<count; i++) {
        //Search the shuffled 'jobs' array to find the matching ID
        for(k=0; k<count; k++) {
            //found the job that corresponds to ID 'i'?
            if(jobs[k].id == i) {
                //Print its name and total wait time
                printf("P%-2d   %d\n", i + 1, jobs[k].total_wait);
                // Add to the grand total sum
                grand_total_wait += jobs[k].total_wait;
            }
        }
    }
    //grand_total_wait/count: performs the final division (8/6 etc.)
    printf("\nAWT = %.0f/%d = %.2f\n", grand_total_wait, count, grand_total_wait/count);
}

//runs the actual simulation for one specific Quantum value (q=1 etc.)
double run_simulation(int quantum, Process *input_processes, int count, int verbose) {
    BinomialHeap *bh = heapInit(); // Create an empty priority queue for waiting Line
    Process *jobs = (Process *)malloc(sizeof(Process) * count);
    int i, j, t, k; // Loop variables
    Process temp;
    int current_time = 0; // The CPU Clock. Starts at 0.
    int completed_jobs = 0; // Counter. When this equals 'count', all jobs will be done
    int job_idx = 0; // Pointer to the next job in our sorted list.
    int wait_period;
    int run_time;
    double total_wait_sum = 0;
    Process cur;

    for(i=0; i<count; i++) {
        jobs[i] = input_processes[i];               // Copy the static data (Name, ID, Arrival) to not destroy original data
        jobs[i].e_remaining = jobs[i].e_original;   // Reset work left to full
        jobs[i].total_wait = 0;                     // Reset wait accumulator
        jobs[i].t_last_queue = -1;                  // Reset timestamp
    }
    
    // Bubble sort by arrival time
    //jobs[0] is guaranteed to be the first one to arrive next one will be arrived
    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (jobs[j].t_arrival > jobs[j+1].t_arrival) {
                temp = jobs[j]; 
                jobs[j] = jobs[j+1]; 
                jobs[j+1] = temp;
            }
        }
    }

    // Clear the chart history.
    for(i=0; i<MAX_TIME; i++) 
        gantt_chart[i] = -1;

    if(verbose) {
        printf("\nTime  Processes in BH      Priority value of processes in BH\n");
        printf("------------------------------------------------------------\n");
    }

    //iterate until every single job is finished 
    while (completed_jobs < count) {
        
        // Enqueue Arrivals:  If the next job's arrival time is $\le$ current_time
        //calculate its initial priority and put it into the Heap.
        check_arrivals(bh, jobs, count, current_time, &job_idx);

        if(verbose) 
            print_heap_snapshot(bh, current_time);

        // Handle Idle CPU
        if (heapIsEmpty(bh)) {
            if (job_idx < count) { // If heap is empty BUT jobs are still coming
                gantt_chart[current_time] = -1; 
                current_time++; // Tick the clock forward
                continue;  // Restart the loop to check for arrivals again
            } else
                break;  //Heap empty AND no more jobs coming. We are done
        }

        // extract the highest priority job out of the heap
        cur = extract_process(bh);
        
        /* Calculate Wait Time
           Current Time (Now) - t_last_queue (Time it got in line)
           How long it waited this specific time*/
        wait_period = current_time - cur.t_last_queue;
        for(k=0; k<count; k++) {
            if(jobs[k].id == cur.id) { 
                jobs[k].total_wait += wait_period;
                break; }
        }

        //Run for 'quantum' OR 'e_remaining' (whichever is smaller)
        run_time = (cur.e_remaining < quantum) ? cur.e_remaining : quantum;
        
        /*Preemption: If P1 is running, and P2 arrives in the middle of P1's turn
         P2 must be added to the queue immediately 
         so it's ready to be considered as soon as this quantum finishes*/
        for (t = 0; t < run_time; t++) {
            gantt_chart[current_time] = cur.id; 
            current_time++;
            // Check for arrivals during execution
            check_arrivals(bh, jobs, count, current_time, &job_idx);
        }

        // Finish or Preempt
        cur.e_remaining -= run_time; // Update work done
        if (cur.e_remaining > 0) {
            //job is not done, recalculate priority 
            cur.prio_val = calculate_priority_value(cur.e_remaining, E_MAX, 0);
            //re-entering the queue
            cur.t_last_queue = current_time;
            //Put it back in the heap
            heapInsert(bh, cur);
        } else {
            //job is done
            completed_jobs++;
        }
    }
    

    if(verbose) 
        printf("%-5d %-20s \n", current_time, "EMPTY"); // Final Trace line
    gantt_end = current_time;

    if(verbose) {
        //print the chart and table
        print_final_report(jobs, count, gantt_end);
    }

    //sum up the every process
    for(k=0; k<count; k++) 
        total_wait_sum += jobs[k].total_wait;
    
    free(jobs); // Clean up memory to prevent leaks.
    return total_wait_sum / count; // Calculate AWT for return value
}

int main(void) {
    Process input_list[MAX_PROCESSES]; //holding the processes read from the file
    int n = 0;  //counter for actual processes loaded
    char line[100];  // Buffer to hold one text line
    int scenarios[3] = {1, 2, 3}; //list of scenarios to run
    int s, q; // Loop variables
    double min_awt = 1e9; // Initialize with a huge number (1 billion)
    int best_q = -1; // Placeholder for the best quantum
    double awt;

    FILE *fp = fopen(INPUT_FILENAME, "r");
    if(!fp) { 
        printf("Error: input.txt missing\n"); 
        return 1; }

    E_MAX = 0; // Reset Global Max Execution Time
    printf("Reading input...\n");
    
    //read the file line-by-line into line buffer
    while(fgets(line, sizeof(line), fp)) {
        //extract three specific values: check if we successfully found all 3 items
        //Process Name, Execution Time, Arrival Time
        if(sscanf(line, "%s %d %d", input_list[n].name, &input_list[n].e_original, &input_list[n].t_arrival) == 3) {
            input_list[n].id = n;
            if(input_list[n].e_original > E_MAX) 
                E_MAX = input_list[n].e_original;
            n++; //added a process, increment the count
        }
    }
    fclose(fp);

    for(s=0; s<3; s++) {
        q = scenarios[s]; // Pick the quantum (1, then 2, then 3)
        printf("\n================ SCENARIO q=%d ================\n", q);
        // RUN WITH VERBOSE = 1
        run_simulation(q, input_list, n, 1);
    }

    printf("\n\n================ OPTIMIZATION SUMMARY ================\n");
    printf("Quantum | AWT\n");
    
    for(q=1; q <= E_MAX+2; q++) {
        // RUN WITH VERBOSE = 0 : Don't print tables or charts
        awt = run_simulation(q, input_list, n, 0);
        printf("%-7d | %.4f\n", q, awt); // Print row in the summary table
        if(awt < min_awt) {
            min_awt = awt; 
            best_q = q; }
    }
    printf("Minimum AWT: %.4f found at q=%d\n", min_awt, best_q);
    return 0;
}
