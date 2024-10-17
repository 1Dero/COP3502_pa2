/*
Darel Gomez
COP3502
February 7, 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define NUM_QUEUES 12
#define NAME_SIZE 51 // Length 50

typedef struct Customer {
    char name[NAME_SIZE]; 
    int tickets; // number of tickets that will be bought
    int line; // line number
    int ToA; // time of arrival
    int PT; // processing time = 30 + tickets * 5
}Customer;

typedef struct node {
    Customer* data;
    struct node* next;
}node;

typedef struct queue {
    node* front;
    node* back;
    int size;
    int seen; // number of customers seen
    int booth; // booth number
}queue;

typedef struct Booth {
    queue** queues;
    int size;
}Booth; 

/* Initalizaiton Functions*/
Customer* init_Customer(char* name, int tickets, int ToA);
node* init_Node(Customer* c);
queue* init_Queue();
Booth* init_Booths(queue* lines[NUM_QUEUES], int b);

/* Queue functions */
int Enqueue(queue* q, Customer* c);
Customer* Dequeue(queue* q);
Customer* peek(queue* q);
int isEmpty(queue* q);
int qSize(queue* q);
int qSeen(queue* q);

/* Operation functions */
int assignQ(queue* lines[NUM_QUEUES], Customer* c);
int leastQ(queue* lines[NUM_QUEUES]);
void processB(Booth *b);
queue* pullFrom(Booth *b);

/* Print functions */
void printC(Customer* c);
void printQ(queue* q);

/* Free functions */
void free_Queues(queue* lines[NUM_QUEUES]);
void free_Booth(Booth* b);

int main() {
    // Initializing an array of queues
    queue* lines[NUM_QUEUES];
    for(int i = 0; i < NUM_QUEUES; i++) {
        lines[i] = init_Queue(lines[i]);
        if(lines[i] == NULL) {
            int t = 0;
            while(lines[i] == NULL && t < 5) {
                lines[i] = init_Queue(lines[i]);
            }
            if(lines[i] == NULL) {
                exit(-1);
            }
        }
    }

    int n; // Number of customers
    int b; // Number of booths
    scanf("%d %d", &n, &b);

    // Initializing customers and adding them to queues
    for(int i = 0; i < n; i++) {
        Customer* c;
        char name[NAME_SIZE];
        int tickets, line, ToA;

        scanf("%s %d %d", name, &tickets, &ToA);
        c = init_Customer(name, tickets, ToA);
        if(c == NULL) {
            int t = 0;
            while(c == NULL && t < 5) {
                c = init_Customer(name, tickets, ToA);
                t++;
            }
            if(c == NULL) {
                exit(-2);
            }
        }
        //printf("Allocated memory for %s. Calling assignQ...\n", c->name);
        if(!assignQ(lines, c)) {
            int t = 0;
            while(!assignQ(lines, c) && t < 5);
            if(!assignQ(lines, c)) {
                exit(-3);
            }
        }
    }

    // Assigning queues to booths
    Booth* booths = init_Booths(lines, b);
    if(booths == NULL) {
        int t = 0;
        while(booths == NULL && t < 5) {
            booths = init_Booths(lines, b);
            t++;
        }
        if(booths == NULL) {
            exit(-4);
        }
    }
    
    for(int i = 0; i < b; i++) {
        printf("Booth %d\n", i+1);
        processB(&booths[i]);
        printf("\n");
        free_Booth(&booths[i]);
    }
    free(booths);
    free_Queues(lines);
    return 0;
}

/* Initalizaiton Functions*/

// Precondition: A customer has arrived.
// Postcondition: Memory has been successfully allocated for a Customer, returns pointer to Customer structure.
Customer* init_Customer(char* name, int tickets, int ToA) {
    Customer* c = (Customer*) malloc(sizeof(Customer));

    if(c == NULL) return c;

    strcpy(c->name, name);
    c->tickets = tickets;
    c->line = -1; // No line
    c->ToA = ToA;
    c->PT = 30 + tickets * 5;

    return c;
}

// Precondition: A customer has been created.
// Postcondition: Memory has been successfully allcoated for a node in a Customer linked list, returns pointer to node structure.
node* init_Node(Customer* c) {
    node* n = (node*) malloc(sizeof(node));

    if(n == NULL) return n;

    n->data = c;
    n->next = NULL;

    return n;
}

// Precondition: An array of queues has been created.
// Postcondition: Memory has been successfully allocated for a queue, returns pointer to queue structure.
queue* init_Queue() {
    queue* q = (queue*) malloc(sizeof(queue));

    if(q == NULL) return q;

    q->front = NULL;
    q->back = NULL;
    q->size = 0;
    q->seen = 0;
    q->booth = -1; // No booth

    return q;
}

// Precondition: All customers have been assigned to a queue.
// Postcondition: Nonempty queues will be assigned to a booth.
//                A booth is defined as an array of queues.
//                A queue, q, is assigned to a booth as follows:
//                - Queues will be assigned in numberical order
//                - k is the amount of nonempty queues
//                - n=k%b, m=k/b
//                - The first n booths will have m+1 queues, and the rest will have m queues.   
//                Returns the booth, and returns null if there are no queues.        
Booth* init_Booths(queue* lines[NUM_QUEUES], int b) {
    Booth* booths = (Booth*) calloc(b, sizeof(Booth));
    if(booths == NULL) return NULL;

    int k = 0;
    for(int i = 0; i < NUM_QUEUES; i++) {
        if(!isEmpty(lines[i])) {
            k++;
        }
    }
    if(k == 0) return NULL;

    int n = k%b;
    int m = k/b;
    int q = -1;
    for(int i = 0; i < b; i++) {
        if(i < n) {
            booths[i].queues = (queue**) calloc(m+1, sizeof(queue*));
            if(booths[i].queues == NULL) {
                int t = 0;
                while(booths[i].queues == NULL && t < 5) {
                    booths[i].queues = (queue**) calloc(m+1, sizeof(queue*));
                }
                if(booths[i].queues == NULL) {
                    return NULL;
                }
            }
            booths[i].size = m+1;
            for(int j = 0; j < m+1; j++) {
                while(isEmpty(lines[++q])); // Goes to nonempty queue
                booths[i].queues[j] = lines[q];
                booths[i].queues[j]->booth = i+1;
            }
        }
        else {
            booths[i].queues = (queue**) calloc(m, sizeof(queue*));
            if(booths[i].queues == NULL) {
                int t = 0;
                while(booths[i].queues == NULL && t < 5) {
                    booths[i].queues = (queue**) calloc(m+1, sizeof(queue*));
                }
                if(booths[i].queues == NULL) {
                    return NULL;
                }
            }
            booths[i].size = m;
            for(int j = 0; j < m; j++) {
                while(isEmpty(lines[++q])); // Goes to nonempty queue
                booths[i].queues[j] = lines[q];
                booths[i].queues[j]->booth = i+1;
            }
        }
    }

    return booths;
}

/* Queue functions */

// Precondition: q points to a queue structure, c points to a Customer structure.
// Postcondition: adds Customer c to the back of the queue, returns 1 if this operation was successful or returns 0 if it was unable to create a node for c.
int Enqueue(queue* q, Customer* c) {
    node* temp = init_Node(c);

    if(temp == NULL) return 0;

    if(isEmpty(q)) {
        q->front = temp;
        q->back = temp;
    }
    else {
        q->back->next = temp;
        q->back = temp;
    }

    q->size++;
    q->seen++;

    return 1;
}

// Precondition: q points to a queue structure.
// Postcondition: removes the Customer at the front of the queue, returns said Customer.
Customer* Dequeue(queue* q) {
    node* temp = q->front;
    Customer* c = temp->data;
    q->front = q->front->next;
    q->size--;

    free(temp);

    return c;
}

// Precondition: q points to a queue structure.
// Postcondition: returns Customer at the front of the queue.
Customer* peek(queue* q) {
    return q->front->data;
}

// Precondition: q points to a queue structure.
// Postcondition: returns 1 if queue is empty or returns 0 if queue is not empty.
int isEmpty(queue* q) {
    if(q->size == 0) return 1;
    else return 0;
}

// Precondition: q points to a queue structure.
// Postcondition: returns the size of the queue.
int qSize(queue* q) {
    return q->size;
}

// Precondition: q points to a queue structure.
// Postcondition: returns the total number of Customers the queue has seen
int qSeen(queue*q) {
    return q->seen;
}

/* Operation functions */

// Precondition: the array of queues lines has been properly initialized, Customer c has been properly initialized.
// Postcondition: Customer c is added into a queue.
//                The queue in which a customer is added to is based on the first letter of their name p.
//                p is assigned the following values: {A = 0, B = 1, ..., Z = 25}.
//                The line number q which c will be assigned to is calculated with p%(NUM_QUEUES+1).
//                Returns 1 if successful and 0 if not.
int assignQ(queue* lines[NUM_QUEUES], Customer* c) {
    //printf("inside assignQ");
    int p = c->name[0] - 'A';
    int q = p%(NUM_QUEUES+1);

    //printf("%d%%(%d) = %d", p, NUM_QUEUES+1, q);
    if(q == 0) {
        q = leastQ(lines);
    }

    c->line = q;

    return Enqueue(lines[q-1], c);
}

// Precondition: the array of queues has been properly initialized.
// Postcondition: returns the line number of the nonempty queue with the least amount of Customers seen.
//                if multiple/all queues have the same minimum size amount, returns the first line number it appears in (line# = index + 1).
int leastQ(queue* lines[NUM_QUEUES]) {
    int min = INT_MAX;
    int index = 0;
    for(int i = 0; i < NUM_QUEUES; i++) {
        if(lines[i] == NULL) exit(-1);
        if(!isEmpty(lines[i]) && lines[i]->size < min) {
            min = lines[i]->size;
            index = i;
        }
    }
    return index+1;
}

// Precondition: b is pointing to a booth structure.
// Postcondition: processes all the queues in b.
//                prints: "CUSTOMER from line X checks out at time T."
//                - CUSTOMER = customer name
//                - X = line number
//                - T = time of exit
//                T = ToA + PT (if ToA >= T)
//                T = T + PT (if ToA <= T)
void processB(Booth *b) {
    int T = 0;
    queue* q;
    Customer* c;
    while(1) {
        q = pullFrom(b);
        if(q == NULL) break;

        c = Dequeue(q);
        if(c == NULL) break;
        if((c->ToA) > T) T = c->ToA + c->PT;
        else T += c->PT;
        printf("%s from line %d checks out at time %d.\n", c->name, c->line, T);
        free(c);
    }
}

// Precondition: processing a booth b.
// Postcondition: returns the queue with the first customer that will be processed.
//                returns NULL if the booth has no more queues to process
queue* pullFrom(Booth *b) {
    queue* q = NULL;
    int min = INT_MAX;
    int index = -1;
    for(int i = 0; i < b->size; i++) {
        q = b->queues[i];
        if(isEmpty(q)) continue;
        else if((peek(q)->ToA) < min) {
            min = peek(q)->ToA;
            index = i;
        }
    }
    if(index == -1) return NULL;
    return b->queues[index];
}

/* Print functions */

// Precondition: c points to a Customer structure.
// Postcondition: prints the queue.
void printC(Customer* c) {
    printf("%s", c->name);
}

// Precondition: q points to a queue structure.
// Postcondition: prints the queue.
void printQ(queue* q) {
    node* walker = q->front;
    if(isEmpty(q)) {
        printf("EMPTY\n");
        return;
    }
    while(walker != q->back) {
        printC(walker->data);
        printf("-%d ", walker->data->ToA);
        walker = walker->next;
    }
    printC(walker->data);
    printf("-%d ", walker->data->ToA);
    printf("\n");
}

/* Free functions */

// Precondition: All queues have been processed.
// Postcondition: Freed the memory that was allocated for the array of queues.
void free_Queues(queue* lines[NUM_QUEUES]) {
    for(int i = 0; i < NUM_QUEUES; i++) {
        free(lines[i]);
    }
}

// Precondition: All booths have finished processing.
// Postcondition: Freed the memory that was allocated for the booth.
void free_Booth(Booth* booth) {
    free(booth->queues);
}