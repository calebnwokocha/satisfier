#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <omp.h>

typedef struct {
    int *orig;
    int  sz;
} Clause;

// Portable getline
static ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    if (!lineptr || !n || !stream) return -1;
    char *buf = *lineptr;
    size_t cap = *n, len = 0;
    for (;;) {
        int ch = fgetc(stream);
        if (ch == EOF) return len ? (ssize_t)len : -1;
        if (len + 1 >= cap) {
            size_t newcap = cap ? cap * 2 : 128;
            char *tmp = realloc(buf, newcap);
            if (!tmp) return -1;
            buf = tmp; cap = newcap;
        }
        buf[len++] = ch;
        if (ch == '\n') break;
    }
    buf[len] = '\0';
    *lineptr = buf; *n = cap;
    return (ssize_t)len;
}

static Clause* read_clauses(int *out_M) {
    Clause *C = NULL;
    int cap = 0, M = 0;
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    printf("Enter clause(s) and blank line to finish:\n");
    while ((linelen = getline(&line, &linecap, stdin)) != -1) {
        if (linelen == 1 && line[0] == '\n') break;
        if (M == cap) {
            cap = cap ? cap * 2 : 4;
            C = realloc(C, cap * sizeof(Clause));
        }
        int *temp = NULL, tcap = 0, tsz = 0;
        for (char *tok = strtok(line, " \t\n"); tok; tok = strtok(NULL, " \t\n")) {
            int lit = atoi(tok);
            if (!lit) continue;
            if (tsz == tcap) {
                tcap = tcap ? tcap * 2 : 4;
                temp = realloc(temp, tcap * sizeof(int));
            }
            temp[tsz++] = lit;
        }
        C[M].orig = temp;
        C[M].sz   = tsz;
        M++;
    }
    free(line);
    *out_M = M;
    return C;
}

// Process one clause: record forbidden bits per literal position
static void process_clause(const Clause *c,
                           int row_len,
                           unsigned char *out_row,
                           bool *early_unsat_flag,
                           int clause_idx)
{
    // Detect if same var appears both pos and neg in clause
    // Small local map: since clause small, O(n^2) is fine
    for (int i = 0; i < c->sz; i++) {
        for (int j = i + 1; j < c->sz; j++) {
            if (c->orig[i] + c->orig[j] == 0) {
                #pragma omp critical
                {
                    int v = abs(c->orig[i]);
                    printf("Unsatisfiable, clause (%d) contains both %d and -%d\n",
                           clause_idx + 1, v, v);
                    *early_unsat_flag = true;
                }
                return;
            }
        }
    }
    // Mark forbidden: 1 for negative literal, 0 for positive
    #pragma omp parallel for
    for (int i = 0; i < row_len; i++)
        out_row[i] = (c->orig[i] < 0) ? 1 : 0;
}

// Convert a row of length C to a number
static uint64_t forbidden_val(const unsigned char *row, int C) {
    int s = 0;
    while (s < C && row[s] == 0) s++;
    if (s == C) return 0;
    uint64_t v = 0;
    #pragma omp parallel for
    for (int j = s; j < C; j++)
        v = (v << 1) | row[j];
    return v;
}

// Partition for quicksort
static int partition_serial(uint64_t arr[], int low, int high) {
    uint64_t pivot = arr[high];
    int i = low - 1;
    #pragma omp parallel for
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            uint64_t tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
        }
    }
    uint64_t tmp = arr[i+1]; arr[i+1] = arr[high]; arr[high] = tmp;
    return i + 1;
}

// Parallel quicksort tasks
void parallel_quick_sort(uint64_t *arr, int low, int high) {
    if (low < high) {
        int p = partition_serial(arr, low, high);
        #pragma omp task
        parallel_quick_sort(arr, low, p - 1);
        #pragma omp task
        parallel_quick_sort(arr, p + 1, high);
    }
}

// Find missing assignments across all clauses
static bool get_assignments(int M,
                            unsigned char **forbidden,
                            int *clause_sizes)
{
    // Map each clause's forbidden row to a numeric value
    uint64_t *vals = malloc((size_t)M * sizeof *vals);
    if (!vals) { perror("malloc"); exit(1); }

    #pragma omp parallel for
    for (int i = 0; i < M; i++)
        vals[i] = forbidden_val(forbidden[i], clause_sizes[i]);

    #pragma omp parallel
    #pragma omp single
    parallel_quick_sort(vals, 0, M - 1);

    // 1) Head: assignments from 0 up to vals[0]-1
    bool any = false;
    if (vals[0] > 0) {
        any = true;
        #pragma omp parallel for
        for (uint64_t v = 0; v < vals[0]; v++) {
            for (int bb = clause_sizes[0]-1; bb >= 0; bb--) {
                putchar((v >> bb) & 1 ? '1' : '0');
                if (bb) putchar(' ');
            }
            putchar('\n');
        }
    }

    // 2) Gaps between consecutive forbidden values
    #pragma omp parallel for
    for (int i = 0; i < M - 1; i++) {
        uint64_t a = vals[i], b = vals[i+1];
        if (b > a + 1) {
            any = true;
            for (uint64_t v = a + 1; v < b; v++) {
                for (int bb = clause_sizes[0] - 1; bb >= 0; bb--) {
                    putchar((v >> bb) & 1 ? '1' : '0');
                    if (bb) putchar(' ');
                }
                putchar('\n');
            }
        }
    }
    // 3) Tail: assignments after vals[M-1]
    uint64_t last = vals[M-1];
    int C0 = clause_sizes[0];
    uint64_t maxv = ((uint64_t)1 << C0) - 1;
    if (last < maxv) {
        any = true;
        #pragma omp parallel for
        for (uint64_t v = last + 1; v <= maxv; v++) {
            for (int bb = C0 -1; bb >= 0; bb--) {
                putchar((v >> bb) & 1 ? '1' : '0');
                if (bb) putchar(' ');
            }
            putchar('\n');
        }
    }

    free(vals);
    return any;
}

static bool validate_clauses(Clause *clause, int M) {
    if (M == 0) {
        printf("Result:\n");
        printf("No input\n\n");
        free(clause);
        return false;
    }
    int first_sz = clause[0].sz;
    for (int i = 1; i < M; i++) {
        if (clause[i].sz != first_sz) {
            printf("Result:\n");
            printf(
              "Invalid input, clause (1) and clause (%d) have different number of literals\n\n",
              i + 1
            );
            // clean up
            #pragma omp parallel for
            for (int j = 0; j < M; j++)
                free(clause[j].orig);
            free(clause);
            return false;
        }
    }
    // Check for duplicated literals within each clause
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < clause[i].sz; j++) {
            for (int k = j + 1; k < clause[i].sz; k++) {
                if (clause[i].orig[j] == clause[i].orig[k]) {
                    printf("Result:\n");
                    printf("Invalid input, literal %d is duplicated in clause (%d)\n\n", clause[i].orig[j], i + 1);
                    #pragma omp parallel for
                    for (int t = 0; t < M; t++) free(clause[t].orig);
                    free(clause);
                    return false;
                }
            }
        }
    }
    return true;
}

int main(void) {
    omp_set_num_threads(omp_get_max_threads());
    while (1) {
        int M;
        Clause *clause = read_clauses(&M);

        if (!validate_clauses(clause, M))
            continue;

        unsigned char **forbidden   = malloc(M * sizeof(*forbidden));
        int            *clause_sizes = malloc(M * sizeof(*clause_sizes));
        bool            early_unsat = false;

        printf("Result:\n");
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < M; i++) {
            clause_sizes[i] = clause[i].sz;
            forbidden[i]    = malloc(clause_sizes[i]);
            if (!__atomic_load_n(&early_unsat, __ATOMIC_RELAXED))
                process_clause(&clause[i], clause_sizes[i], forbidden[i], &early_unsat, i);
        }

        if (!early_unsat) {
           /* printf("Forbidden matrix (%d clauses):\n", M);
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < clause_sizes[i]; j++)
                    printf("%d ", forbidden[i][j]);
                printf("\n");
            }
            printf("\n");*/
            bool has_any = get_assignments(M, forbidden, clause_sizes);
            if (!has_any) printf("Unsatisfiable, no head or gap or tail of the SAT instance, or the input is invalid\n");
        }

        #pragma omp parallel for
        for (int i = 0; i < M; i++) {
            free(clause[i].orig);
            free(forbidden[i]);
        }
        free(clause);
        free(forbidden);
        free(clause_sizes);
        printf("\n");
    }
    return 0;
}
