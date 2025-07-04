#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// Cross-platform dynamic line reading function
char* read_dynamic_line(size_t *capacity) {
    if (*capacity == 0) {
        *capacity = 1024;
    }

    char *line = malloc(*capacity);
    if (!line) return NULL;

    size_t length = 0;
    int c;

    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            line[length] = '\0';
            return line;
        }

        // Expand buffer if needed
        if (length >= *capacity - 1) {
            *capacity *= 2;
            char *new_line = realloc(line, *capacity);
            if (!new_line) {
                free(line);
                return NULL;
            }
            line = new_line;
        }

        line[length++] = c;
    }

    // Handle EOF
    if (length == 0) {
        free(line);
        return NULL;
    }

    line[length] = '\0';
    return line;
}

// Dynamic clause reading with unlimited capacity
void read_clauses(int ***clauses, int *num_clauses, int *num_vars) {
    printf("Format: each line is one clause with space-separated integers\n");
    printf("Example: 1 -2 3 (then press Enter)\n");
    printf("Press Enter on empty line to finish\n");

    *num_clauses = 0;
    *num_vars = 0;

    // Dynamic clause array capacity
    size_t clause_capacity = 100;
    *clauses = malloc(clause_capacity * sizeof(int*));
    if (!*clauses) {
        fprintf(stderr, "Memory allocation failed for clause array\n");
        exit(1);
    }

    size_t line_capacity = 1024;
    char *line;

    while ((line = read_dynamic_line(&line_capacity)) != NULL) {
        // Check for blank line (stop condition)
        if (strlen(line) == 0) {
            free(line);
            break;
        }

        // Dynamic clause with initial capacity
        size_t literal_capacity = 50;
        int *clause = malloc(literal_capacity * sizeof(int));
        if (!clause) {
            fprintf(stderr, "Memory allocation failed for clause\n");
            exit(1);
        }

        int size = 0;
        char *token = strtok(line, " \t");

        while (token) {
            int var = atoi(token);
            // Accept any non-zero integer as a literal
            if (var != 0) {
                // Expand clause if needed
                if (size >= literal_capacity) {
                    literal_capacity *= 2;
                    clause = realloc(clause, literal_capacity * sizeof(int));
                    if (!clause) {
                        fprintf(stderr, "Memory allocation failed for clause literals\n");
                        exit(1);
                    }
                }

                clause[size++] = var;
                if (abs(var) > *num_vars) *num_vars = abs(var);
            }
            token = strtok(NULL, " \t");
        }

        if (size > 0) {
            clause[size] = 0;  // Null terminate for processing

            // Expand clause array if needed
            if (*num_clauses >= clause_capacity) {
                clause_capacity *= 2;
                *clauses = realloc(*clauses, clause_capacity * sizeof(int*));
                if (!*clauses) {
                    fprintf(stderr, "Memory allocation failed for clause array\n");
                    exit(1);
                }
            }

            (*clauses)[(*num_clauses)++] = clause;
            printf("Added clause %d with %d literals\n", *num_clauses, size);
        } else {
            free(clause);
            printf("Empty clause ignored\n");
        }

        free(line);
    }
}

// Serial vector printing (printf is not thread-safe for ordered output)
void print_vector(int *vec, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", vec[i]);
    }
    printf("\n");
}

// Serial partition for quicksort (parallel version would be complex and inefficient)
int partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (abs(arr[j]) < abs(pivot)) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    int temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

// Parallel quicksort
void quicksort(int *arr, int low, int high) {
    if (low < high) {
        int pivot = partition(arr, low, high);
        #pragma omp parallel sections
        {
            #pragma omp section
            quicksort(arr, low, pivot - 1);
            #pragma omp section
            quicksort(arr, pivot + 1, high);
        }
    }
}

// Parallel literal analysis with dynamic allocation
void analyze_literals(int **clauses, int num_clauses, int num_vars, int **all_literals, int *literal_count, int **vars_with_both_signs, int *vars_with_both_count) {
    if (num_vars == 0) {
        *all_literals = NULL;
        *literal_count = 0;
        *vars_with_both_signs = NULL;
        *vars_with_both_count = 0;
        return;
    }

    int *has_pos = calloc(num_vars + 1, sizeof(int));
    int *has_neg = calloc(num_vars + 1, sizeof(int));

    if (!has_pos || !has_neg) {
        fprintf(stderr, "Memory allocation failed for literal tracking\n");
        exit(1);
    }

    // Mark which variables have positive and negative literals
    #pragma omp parallel for
    for (int i = 0; i < num_clauses; i++) {
        for (int j = 0; clauses[i][j] != 0; j++) {
            int literal = clauses[i][j];
            int var = abs(literal);
            #pragma omp critical
            {
                if (literal > 0) has_pos[var] = 1;
                else has_neg[var] = 1;
            }
        }
    }

    // Count variables with both positive and negative literals
    int both_count = 0;
    for (int i = 1; i <= num_vars; i++) {
        if (has_pos[i] && has_neg[i]) both_count++;
    }
    *vars_with_both_count = both_count;

    if (*vars_with_both_count > 0) {
        *vars_with_both_signs = malloc(*vars_with_both_count * sizeof(int));
        if (!*vars_with_both_signs) {
            fprintf(stderr, "Memory allocation failed for vars_with_both_signs\n");
            exit(1);
        }

        int idx = 0;
        for (int i = 1; i <= num_vars; i++) {
            if (has_pos[i] && has_neg[i]) (*vars_with_both_signs)[idx++] = i;
        }
    }

    // Estimate initial capacity for literals (2 * num_vars is reasonable upper bound)
    size_t literal_capacity = (num_vars * 2 > 1000) ? num_vars * 2 : 1000;
    int *temp_literals = malloc(literal_capacity * sizeof(int));
    if (!temp_literals) {
        fprintf(stderr, "Memory allocation failed for temp_literals\n");
        exit(1);
    }

    int temp_count = 0;

    // Collect all unique literals
    for (int i = 0; i < num_clauses; i++) {
        for (int j = 0; clauses[i][j] != 0; j++) {
            int literal = clauses[i][j];
            int found = 0;

            // Check if literal already exists
            for (int k = 0; k < temp_count; k++) {
                if (temp_literals[k] == literal) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                // Expand if needed
                if (temp_count >= literal_capacity) {
                    literal_capacity *= 2;
                    temp_literals = realloc(temp_literals, literal_capacity * sizeof(int));
                    if (!temp_literals) {
                        fprintf(stderr, "Memory allocation failed expanding temp_literals\n");
                        exit(1);
                    }
                }
                temp_literals[temp_count++] = literal;
            }
        }
    }

    *all_literals = malloc(temp_count * sizeof(int));
    if (!*all_literals && temp_count > 0) {
        fprintf(stderr, "Memory allocation failed for all_literals\n");
        exit(1);
    }

    if (temp_count > 0) {
        memcpy(*all_literals, temp_literals, temp_count * sizeof(int));
    }
    *literal_count = temp_count;

    free(temp_literals);
    free(has_pos);
    free(has_neg);
}

// Parallel vector creation for single case
void create_single_vector(int *all_literals, int literal_count, int **G, int *G_size) {
    *G_size = literal_count;
    *G = malloc(*G_size * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < literal_count; i++) {
        (*G)[i] = all_literals[i];
    }

    quicksort(*G, 0, *G_size - 1);
}

// Helper function to check if variable has both signs
int has_both_signs(int var, int *vars_with_both_signs, int vars_with_both_count) {
    for (int i = 0; i < vars_with_both_count; i++) {
        if (vars_with_both_signs[i] == var) {
            return 1;
        }
    }
    return 0;
}

// Parallel vector creation for dual case with dynamic allocation
void create_dual_vectors(int *all_literals, int literal_count, int *vars_with_both_signs, int vars_with_both_count, int **G_pos, int *G_pos_size, int **G_neg, int *G_neg_size) {
    if (literal_count == 0) {
        *G_pos = NULL;
        *G_neg = NULL;
        *G_pos_size = 0;
        *G_neg_size = 0;
        return;
    }

    // First pass: count how many literals go in each vector
    int pos_count = 0, neg_count = 0;

    for (int i = 0; i < literal_count; i++) {
        int literal = all_literals[i];
        int var = abs(literal);
        int has_both = has_both_signs(var, vars_with_both_signs, vars_with_both_count);

        if ((has_both && literal > 0) || !has_both) {
            pos_count++;
        }
        if ((has_both && literal < 0) || !has_both) {
            neg_count++;
        }
    }

    // Allocate arrays
    *G_pos = malloc(pos_count * sizeof(int));
    *G_neg = malloc(neg_count * sizeof(int));

    if ((!*G_pos && pos_count > 0) || (!*G_neg && neg_count > 0)) {
        fprintf(stderr, "Memory allocation failed for G_pos or G_neg vectors\n");
        exit(1);
    }

    *G_pos_size = 0;
    *G_neg_size = 0;

    // Second pass: fill the arrays
    for (int i = 0; i < literal_count; i++) {
        int literal = all_literals[i];
        int var = abs(literal);
        int has_both = has_both_signs(var, vars_with_both_signs, vars_with_both_count);

        if ((has_both && literal > 0) || !has_both) {
            (*G_pos)[(*G_pos_size)++] = literal;
        }
        if ((has_both && literal < 0) || !has_both) {
            (*G_neg)[(*G_neg_size)++] = literal;
        }
    }

    // Sort both vectors in parallel
    #pragma omp parallel sections
    {
        #pragma omp section
        if (*G_pos_size > 1) quicksort(*G_pos, 0, *G_pos_size - 1);
        #pragma omp section
        if (*G_neg_size > 1) quicksort(*G_neg, 0, *G_neg_size - 1);
    }
}

// Parallel forbidden vector creation
void create_forbidden(int *G, int G_size, int **forbidden) {
    *forbidden = malloc(sizeof(int) * G_size);

    #pragma omp parallel for
    for (int i = 0; i < G_size; i++) {
        (*forbidden)[i] = (G[i] < 0) ? 1 : 0;
    }
}

// Parallel assignment vector creation
void create_assignment(int *forbidden, int size, int **assignment) {
    *assignment = malloc(sizeof(int) * size);

    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        (*assignment)[i] = (forbidden[i] == 0) ? 1 : 0;
    }
}

// Parallel memory cleanup
void cleanup_memory(int **clauses, int num_clauses, int *all_literals, int *vars_with_both_signs, int *G_pos, int *G_neg) {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (clauses != NULL) {
                for (int i = 0; i < num_clauses; i++) {
                    free(clauses[i]);
                }
                free(clauses);
            }
        }
        #pragma omp section
        if (all_literals != NULL) free(all_literals);
        #pragma omp section
        if (vars_with_both_signs != NULL) free(vars_with_both_signs);
        #pragma omp section
        if (G_pos != NULL) free(G_pos);
        #pragma omp section
        if (G_neg != NULL) free(G_neg);
    }
}

// Main parallel processing function
int main() {
    omp_set_num_threads(omp_get_max_threads());

    while (1) {
        int **clauses = NULL;
        int num_clauses = 0, num_vars = 0;

        read_clauses(&clauses, &num_clauses, &num_vars);

        if (num_clauses == 0) {
            printf("No clauses entered. Exiting...\n");
            break;
        }

        int *all_literals = NULL;
        int literal_count = 0;
        int *vars_with_both_signs = NULL;
        int vars_with_both_count = 0;

        analyze_literals(clauses, num_clauses, num_vars, &all_literals, &literal_count, &vars_with_both_signs, &vars_with_both_count);

        int *G_pos = NULL, *G_neg = NULL;
        int G_pos_size = 0, G_neg_size = 0;

        if (vars_with_both_count == 0) {
            create_single_vector(all_literals, literal_count, &G_pos, &G_pos_size);

            printf("Sorted G vector: ");
            print_vector(G_pos, G_pos_size);

            int *forbidden, *assignment;
            create_forbidden(G_pos, G_pos_size, &forbidden);
            create_assignment(forbidden, G_pos_size, &assignment);

            printf("Assignment Vector: ");
            print_vector(assignment, G_pos_size);

            #pragma omp parallel sections
            {
                #pragma omp section
                free(forbidden);
                #pragma omp section
                free(assignment);
            }
        } else {
            create_dual_vectors(all_literals, literal_count, vars_with_both_signs, vars_with_both_count, &G_pos, &G_pos_size, &G_neg, &G_neg_size);

            printf("Sorted G_pos vector: ");
            print_vector(G_pos, G_pos_size);
            printf("Sorted G_neg vector: ");
            print_vector(G_neg, G_neg_size);

            int *forbidden_pos, *forbidden_neg, *assignment_pos, *assignment_neg;

            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    create_forbidden(G_pos, G_pos_size, &forbidden_pos);
                    create_assignment(forbidden_pos, G_pos_size, &assignment_pos);
                }
                #pragma omp section
                {
                    create_forbidden(G_neg, G_neg_size, &forbidden_neg);
                    create_assignment(forbidden_neg, G_neg_size, &assignment_neg);
                }
            }

            printf("Assignment Vector (Positive): ");
            print_vector(assignment_pos, G_pos_size);
            printf("Assignment Vector (Negative): ");
            print_vector(assignment_neg, G_neg_size);

            #pragma omp parallel sections
            {
                #pragma omp section
                free(forbidden_pos);
                #pragma omp section
                free(forbidden_neg);
                #pragma omp section
                free(assignment_pos);
                #pragma omp section
                free(assignment_neg);
            }
        }

        cleanup_memory(clauses, num_clauses, all_literals, vars_with_both_signs, G_pos, G_neg);

        char continue_input[10];
        printf("Do you want to enter more clauses? (y/n): ");
        fgets(continue_input, sizeof(continue_input), stdin);

        if (continue_input[0] == 'n' || continue_input[0] == 'N') {
            printf("Exiting...\n");
            break;
        }
    }

    return 0;
}
