#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// Parallel clause reading with dynamic allocation
void read_clauses(int ***clauses, int *num_clauses, int *num_vars) {
    printf("Enter clauses (use a blank line to stop):\n");
    *num_clauses = 0;
    *num_vars = 0;
    char line[1024];

    while (fgets(line, sizeof(line), stdin) && line[0] != '\n') {
        int *clause = malloc(sizeof(int) * 100);
        int size = 0;
        char *token = strtok(line, " ");

        while (token) {
            int var = atoi(token);
            if (var != 0) {
                clause[size++] = var;
                if (abs(var) > *num_vars) *num_vars = abs(var);
            }
            token = strtok(NULL, " ");
        }

        clause[size] = 0;
        if (size > 0) {
            *clauses = realloc(*clauses, (*num_clauses + 1) * sizeof(int*));
            (*clauses)[(*num_clauses)++] = clause;
        } else free(clause);
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

// Parallel literal analysis
void analyze_literals(int **clauses, int num_clauses, int num_vars, int **all_literals, int *literal_count, int **vars_with_both_signs, int *vars_with_both_count) {
    int *has_pos = calloc(num_vars + 1, sizeof(int));
    int *has_neg = calloc(num_vars + 1, sizeof(int));

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
        int idx = 0;
        for (int i = 1; i <= num_vars; i++) {
            if (has_pos[i] && has_neg[i]) (*vars_with_both_signs)[idx++] = i;
        }
    }

    // Collect all unique literals
    int *temp_literals = malloc(sizeof(int) * num_clauses * num_vars * 2);
    int temp_count = 0;

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
                temp_literals[temp_count++] = literal;
            }
        }
    }

    *all_literals = malloc(temp_count * sizeof(int));
    memcpy(*all_literals, temp_literals, temp_count * sizeof(int));
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

// Parallel vector creation for dual case
void create_dual_vectors(int *all_literals, int literal_count, int *vars_with_both_signs, int vars_with_both_count, int **G_pos, int *G_pos_size, int **G_neg, int *G_neg_size) {
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
        quicksort(*G_pos, 0, *G_pos_size - 1);
        #pragma omp section
        quicksort(*G_neg, 0, *G_neg_size - 1);
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

            printf("Sorted G vector:\n");
            print_vector(G_pos, G_pos_size);

            int *forbidden, *assignment;
            create_forbidden(G_pos, G_pos_size, &forbidden);
            create_assignment(forbidden, G_pos_size, &assignment);

            printf("Assignment Vector:\n");
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

            printf("Sorted G_pos vector:\n");
            print_vector(G_pos, G_pos_size);
            printf("Sorted G_neg vector:\n");
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

            printf("Assignment Vector (Positive):\n");
            print_vector(assignment_pos, G_pos_size);
            printf("Assignment Vector (Negative):\n");
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
