#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <omp.h>

typedef struct {
    int *orig;   // original literals
    int  sz;     // number of original literals
} Clause;

// Phase 1: read clauses, skip any lit == 0
static Clause* read_clauses(int *out_M, int *out_max_id, bool **out_seen) {
    Clause *C = NULL;
    int cap = 0, M = 0, max_id = 0;
    bool *seen = calloc(1024, sizeof(bool));
    char line[2048];

    printf("Enter clause(s) and blank line to finish:\n");
    while (fgets(line, sizeof(line), stdin) && *line != '\n') {
        if (M == cap) {
            cap = cap ? cap * 2 : 4;
            C   = realloc(C, cap * sizeof(Clause));
        }
        // parse into a temporary vector
        int *temp = NULL, tcap = 0, tsz = 0;
        for (char *tok = strtok(line, " \t\n"); tok; tok = strtok(NULL, " \t\n")) {
            int lit = atoi(tok);
            if (lit == 0) continue;        // ← skip zeros entirely
            if (tsz == tcap) {
                tcap = tcap ? tcap*2 : 4;
                temp = realloc(temp, tcap * sizeof(int));
            }
            temp[tsz++] = lit;
            int v = abs(lit);
            if (v > max_id) max_id = v;
            if (v < 1024)  seen[v] = true;
        }
        C[M].orig = temp;
        C[M].sz   = tsz;
        M++;
    }

    *out_M      = M;
    *out_max_id = max_id;
    *out_seen   = seen;
    return C;
}

// Phase 2: build sorted list of distinct vars
static int* build_vars(bool *seen, int max_id, int *out_n) {
    // Determine upper bound for v (ensures v < 1024)
    int upper = (max_id < 1023) ? max_id : 1023;

    // 1️⃣ Count how many vars are present
    int n = 0;
    #pragma omp parallel for reduction(+ : n)
    for (int v = 1; v <= upper; v++) {
        if (seen[v]) n++;
    }
    *out_n = n;

    int *vars = malloc(n * sizeof(int));
    if (!vars) return NULL;

    // Prepare per-thread counters
    int num_threads = omp_get_max_threads();
    int *thread_counts = calloc(num_threads, sizeof(int));
    if (!thread_counts) { free(vars); return NULL; }

    // 2️⃣ Each thread counts how many items it will write
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int local_count = 0;
        #pragma omp for nowait schedule(static)
        for (int v = 1; v <= upper; v++) {
            if (seen[v]) local_count++;
        }
        thread_counts[tid] = local_count;
    }

    // Compute prefix sums to get each thread’s starting index
    int *thread_offset = malloc((num_threads + 1) * sizeof(int));
    if (!thread_offset) {
        free(vars);
        free(thread_counts);
        return NULL;
    }
    thread_offset[0] = 0;
    for (int t = 0; t < num_threads; t++) {
        thread_offset[t + 1] = thread_offset[t] + thread_counts[t];
    }

    // 3️⃣ Each thread writes its portion into vars[], preserving order
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int idx = thread_offset[tid];
        #pragma omp for schedule(static)
        for (int v = 1; v <= upper; v++) {
            if (seen[v]) {
                vars[idx++] = v;
            }
        }
    }

    free(thread_counts);
    free(thread_offset);
    return vars;
}

// Phase 3: scan, pad+“sort”, build forbidden in one pass
static void process_clause(const Clause *c,
                           int *vars, int *var_to_idx, int n,
                           int *out_sorted, unsigned char *out_forbidden,
                           bool *early_unsat_flag, int clause_idx)
{
    bool seen_pos[n], seen_neg[n];
    memset(seen_pos, 0, n * sizeof(bool));
    memset(seen_neg, 0, n * sizeof(bool));

    // detect p vs. ¬p
    for (int i = 0; i < c->sz; i++) {
        int lit = c->orig[i];
        int v   = abs(lit);
        int idx = var_to_idx[v];
        if (lit > 0) {
            if (seen_neg[idx]) {
                #pragma omp critical
                {
                    printf("Unsatisfiable, clause (%d) contains both %d and -%d\n",
                           clause_idx+1, v, v);
                    *early_unsat_flag = true;
                }
                return;
            }
            seen_pos[idx] = true;
        } else {
            if (seen_pos[idx]) {
                #pragma omp critical
                {
                    printf("Unsatisfiable, clause (%d) contains both %d and -%d\n",
                           clause_idx+1, v, v);
                    *early_unsat_flag = true;
                }
                return;
            }
            seen_neg[idx] = true;
        }
    }

    // build padded+sorted & forbidden
    for (int j = 0; j < n; j++) {
        if (seen_neg[j]) {
            out_sorted[j]  = -vars[j];
            out_forbidden[clause_idx * n + j] = 1;  // Store forbidden vector here
        } else {
            // either seen_pos[j] or padded positive
            out_sorted[j]  =  vars[j];
            out_forbidden[clause_idx * n + j] = 0;
        }
    }
}

// Helper: convert a C bit row (row[0] is leftmost bit) to its integer value
static uint64_t forbidden_val(const unsigned char *row, int C) {
    // skip leading zeros
    int s = 0;
    while (s < C && row[s] == 0) s++;
    if (s == C) return 0;
    // parse bits row[s..C-1]
    uint64_t v = 0;
    for (int j = s; j < C; j++)
        v = (v << 1) | row[j];
    return v;
}

// Comparator for qsort
static int cmp_uint64(const void *pa, const void *pb) {
    uint64_t a = *(const uint64_t*)pa;
    uint64_t b = *(const uint64_t*)pb;
    return (a < b) ? -1 : (a > b) ? +1 : 0;
}

static bool get_assignments(int M, int C, const unsigned char *forbidden) {
    // 1) build array of forbidden values
    uint64_t *vals = malloc((size_t)M * sizeof *vals);
    if (!vals) { perror("malloc"); exit(1); }
    #pragma omp parallel for
    for (int i = 0; i < M; i++) {
        vals[i] = forbidden_val(forbidden + (size_t)i * C, C);
    }

    // 2) sort ascending
    qsort(vals, M, sizeof vals[0], cmp_uint64);
    bool any = false;

    // 3) enumerate each gap between consecutive forbidden values
    for (int i = 0; i + 1 < M; i++) {
        uint64_t ai = vals[i];
        uint64_t bi = vals[i+1];
        if (bi > ai + 1) {
            for (uint64_t val = ai + 1; val < bi; val++) {
                any = true;
                // print the C bit binary (with spaces)
                for (int b = C - 1; b >= 0; b--) {
                    putchar((val >> b) & 1 ? '1' : '0');
                    if (b > 0) putchar(' ');
                }
                putchar('\n');
            }
        }
    }

    // 4) now enumerate *tail* assignments > vals[M-1], up to 2^C−1
    uint64_t last = vals[M-1];
    uint64_t maxv = ((uint64_t)1 << C) - 1;
    if (last < maxv) {
        for (uint64_t val = last + 1; val <= maxv; val++) {
            any = true;
            for (int b = C - 1; b >= 0; b--) {
                putchar((val >> b) & 1 ? '1' : '0');
                if (b > 0) putchar(' ');
            }
            putchar('\n');
        }
    }

    if (any)
        printf("\n");
    free(vals);
    return any;
}

int main(void) {
    omp_set_num_threads(omp_get_max_threads());
    while (1) {
        int M, max_id, n;
        bool *seen;
        Clause *clause = read_clauses(&M, &max_id, &seen);

        int *vars       = build_vars(seen, max_id, &n);
        int *var_to_idx = calloc(max_id+1, sizeof(int));
        #pragma omp parallel for
        for (int i = 0; i < n; i++)
            var_to_idx[vars[i]] = i;

        int  (*sorted)[n] = malloc(M * sizeof *sorted);
        unsigned char *forbidden = malloc(M * n * sizeof(unsigned char));
        bool early_unsat = false;

        printf("Result:\n");

        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < M; i++) {
            if (!early_unsat) {
                process_clause(&clause[i],
                               vars, var_to_idx, n,
                               sorted[i], forbidden,
                               &early_unsat, i);
            }
        }

        if (early_unsat) {
            for (int i = 0; i < M; i++)
                free(clause[i].orig);
            free(clause); free(seen); free(vars);
            free(var_to_idx); free(sorted); free(forbidden);
            printf("\n");
            continue;
        }

        bool has_any = get_assignments(M, n, forbidden);
        if (!has_any) {
            printf("Unsatisfiable, no gap or tail of the SAT instance, or invalid input\n");
            for (int i = 0; i < M; i++)
                free(clause[i].orig);
            free(clause); free(seen); free(vars);
            free(var_to_idx); free(sorted); free(forbidden);
            printf("\n");
            continue;
        }

        for (int i = 0; i < M; i++)
            free(clause[i].orig);
        free(clause);
        free(seen);
        free(vars);
        free(var_to_idx);
        free(sorted);
        free(forbidden);
    }
    return 0;
}
