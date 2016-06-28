#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

//Pensar nas seguintes estruturas:
//  -Solução - matrix formiga*solucao
//  -Dados 
//  -Feromônios - Matrix formiga*feromonios
//  -Informação Heurística - Matrix formiga*feromonios
//  -Probabilidades
//  -Componentes Candidatos
//
//  -> Como Achar Componentes Candidatos

#define Q 100.0f

typedef struct _Session {
    float alpha;
    float beta;
    float rho;
    int antNumber;

    int universeSet; 
    int numberOfSets;

    double * weightSet;
    int * sets;

    float * pheromone;     
} Session;

typedef struct _Solution {
    bool * setsChoosed;
    bool * numbersCovered;
    double weight;
} Solution;

void printAllSets(Session * session) {
    int i, j;

    printf("Numero de Conjuntos: %d ", session->numberOfSets);
    printf("Tamanho do Universo: %d \n", session->universeSet);
    for (i=0; i<session->numberOfSets; i++) {
        for (j=0; j<session->universeSet; j++) 
            printf("%d ", session->sets[i*session->universeSet + j]);
        printf("\n");
    }
}

void printSet(Session * session, int set) {
    int j=0;

    while (j<session->universeSet 
            && session->sets[set*session->universeSet + j] != -1) {
        printf("%d ", session->sets[set*session->universeSet + j]);
        j++;
    }
    printf("\nSet Weight: %f\n", session->weightSet[set]);
}

void printPheromone(Session * session) {
    int i;

    printf("Pheromone:\n");
    for (i=0; i<session->numberOfSets; i++) {
        printf("%f ", session->pheromone[i]);
    }
    printf("\n");
}

void printSolution(Session * session, Solution * solution) {
    int i, cont;

    printf("Sets choosed: \n");
    for (i=0; i<session->numberOfSets; i++) {
        if (cont == session->universeSet) {
            printf("\n");
            cont = 1;
        } else cont++;
        printf("%d ", solution->setsChoosed[i]);
    }
    
    printf("\nNumbers Covered: \n");
    
    for (i=0; i<session->universeSet; i++) 
            printf("%d ", solution->numbersCovered[i]);
    
    printf("\nWeight: %f\n", solution->weight);
}

void printBestSolution(Session * session, Solution * solution) {
    int i, cont;

    printf("Sets choosed: \n");
    for (i=0; i<session->numberOfSets; i++) {
        if (solution->setsChoosed[i]) 
            printf("%d ", i);
    }
    
    printf("\nWeight: %f\n", solution->weight);
}

void readFile(char filename[], Session * session) {
    FILE* input;
    size_t len = 1;
    ssize_t read;
    char buffer[512];
    char *line = NULL;
    int i;

    input = fopen(filename, "r");
    if (input == NULL) exit(1);

    //Read U
    read = getline(&line, &len, input);

    strtok(line, "\n");
    strtok(line, " ");

    line = strtok(NULL, " ");

    session->universeSet = atoi(line);
    //printf("%d\n", session->universeSet);

    //Read Set number
    read = getline(&line, &len, input);

    strtok(line, "\n");
    strtok(line, " ");

    line = strtok(NULL, " ");

    session->numberOfSets = atoi(line);
    //printf("%d\n", session->numberOfSets);

    session->weightSet = (double *)malloc(session->numberOfSets*sizeof(double));
    session->sets = (int *)malloc(session->numberOfSets*session->universeSet*sizeof(int));
    memset(session->sets, -1, session->numberOfSets*session->universeSet*sizeof(float));

    read = getline(&line, &len, input);

    for (i=0; i<session->numberOfSets; i++) {
        read = getline(&line, &len, input);

        strtok(line, "\n");
        strtok(line, " ");
    
        line = strtok(NULL, " ");

        session->weightSet[i] = atof(line);
        //printf("%f\n", session->weightSet[i]);

        line = strtok(NULL, " ");

        int j = 0;
        while (line != NULL) {
            session->sets[i*session->universeSet + j] = atoi(line) - 1;
            line = strtok(NULL, " ");
            //printf("%d ", session->sets[i*session->universeSet + j]);
            j++;
        }
        //printf("\n");
    }
}

float heuristic(Session * session, Solution * solution, int setNumber) {
    int individual, j=0;
    float intersect = 0.0f;
    while (session->sets[setNumber*session->universeSet + j] != -1) {
        individual = session->sets[setNumber*session->universeSet + j];
        if (!solution->numbersCovered[individual]) intersect++;
        //printf("Number: %d I: %f\n", individual, intersect);
        j++;
    }
    //printf("--\n");

    return intersect/(float)session->universeSet;
}

int getMax(Session * session, float * prob) {
    int maxID = 0, i;
    for (i=0; i<session->numberOfSets; i++) {
        if (prob[i] == prob[maxID]) 
            if (0 == rand()%2)
                maxID = i;
        if (prob[i] > prob[maxID]) maxID = i;
        //printf("prob[%d] = %f \n", i, prob[i]);
    }
    return maxID;
}

//choose next
int calculateProbability(Session * session, Solution * solution) {
    int i;
    float * uppers = malloc(session->numberOfSets*sizeof(float)); //remember to free
    float * prob = malloc(session->numberOfSets*sizeof(float));  // remember to free
    float lower = 0;

    for (i=0; i<session->numberOfSets; i++) {
        if (!solution->setsChoosed[i]) 
            uppers[i] = pow(session->pheromone[i], session->alpha) *
                        pow(heuristic(session, solution, i), session->beta);
        else uppers[i] = 0;
        lower += uppers[i];

        //printf("H: %f P: %f\n", pow(heuristic(session, solution, i), session->beta), pow(session->pheromone[i], session->alpha));
    }

    for (i=0; i<session->numberOfSets; i++) {
        prob[i] = uppers[i]/ lower;
    }

    int max = getMax(session, prob);
    
    free(uppers);
    free(prob);

    return max;
}

void updatePheromone(Session * session, Solution * solution) {
    int i,j; 
    double totalDelta;
    for (i=0; i<session->numberOfSets; i++) {
        totalDelta = 0;
        for (j=0; j<session->antNumber; j++) {
            if (solution[j].setsChoosed[i]) totalDelta += Q/solution[j].weight;
        }
        session->pheromone[i] = session->rho*session->pheromone[i] + totalDelta;
    } 
}

bool isSolution(Session * session, Solution * solution) {
    int i;
    for (i=0; i<session->universeSet; i++) 
        if (!solution->numbersCovered[i]) return false;
    return true;
}

bool changeSolution(Session * session, Solution * solution, int set) {
   int individual, j=0;
    if (solution->setsChoosed[set] != 0) return false;
    else solution->setsChoosed[set] = 1;
    
    while (session->sets[set*session->universeSet + j] != -1) {
        individual = session->sets[set*session->universeSet + j];
        if (!solution->numbersCovered[individual]) { 
            solution->numbersCovered[individual] = 1;
        }
        j++;
    }

    solution->weight += session->weightSet[individual];
    return true;
}

void applyConstruction(Session * session, Solution * solution) {
    int next; 
    
    while (!isSolution(session, solution)) {
        next = calculateProbability(session, solution);

        //printf("NEXT: %d\n", next);
        //printSet(session, next);

        changeSolution(session, solution, next);

        //printSolution(session, solution);
        //printf("\n\n\n");

    }
}

void buildSolutions(Session * session, Solution * solution) {
    int i;

    for (i=0; i<session->antNumber; i++) {
        int root = rand()%session->numberOfSets;
        //printf("ROOT: %d\n", root);
        //printSet(session, root);

        changeSolution(session, &(solution[i]), root);

        //printSolution(session, solution);
        //printf("\n\n\n");

        applyConstruction(session,&(solution[i]));
    } 
}

void reinitializeSolutions(Session * session, Solution * solutions, Solution * bestSolution) {
    int j;

    for (j=0; j<session->antNumber; j++) {
        if (solutions[j].weight < bestSolution->weight) {
            memcpy(bestSolution->setsChoosed, solutions[j].setsChoosed, session->numberOfSets*sizeof(bool));
            memcpy(bestSolution->numbersCovered, solutions[j].numbersCovered, session->numberOfSets*sizeof(bool));
            bestSolution->weight = solutions[j].weight;
        }
        memset(solutions[j].setsChoosed, 0, session->numberOfSets*sizeof(bool));
        memset(solutions[j].numbersCovered, 0, session->universeSet*sizeof(bool));

        solutions[j].weight = 0;
    }
}

void initializeSolutions(Session * session, Solution * solutions) {
    int j;
    for (j=0; j<session->antNumber; j++) {
        solutions[j].setsChoosed = (bool *)malloc(session->numberOfSets*sizeof(bool));
        solutions[j].numbersCovered = (bool *)malloc(session->universeSet*sizeof(bool));

        memset(solutions[j].setsChoosed, 0, session->numberOfSets*sizeof(bool));
        memset(solutions[j].numbersCovered, 0, session->universeSet*sizeof(bool));

        solutions[j].weight = 0;
    }    
}

int main(int argc, char *argv[]) {
    FILE *in;
    char filename[50];
    int oc, cycles;
    clock_t stime, ftime;
    time_t seed = time(NULL);

    Session session; 

    //Falta: 
    while ((oc = getopt(argc, argv, "a:b:r:f:n:c:s:")) != -1) {
        switch(oc) {
            case 'a':
                session.alpha = atof(optarg);
                break;
            case 'b':
                session.beta = atof(optarg);
                break;
            case 'r':
                session.rho = atof(optarg);
                break;
                case 'f':
                strcpy(filename, optarg);
                break;
            case 'n':
                session.antNumber = atoi(optarg);
                break;
            case 'c':
                cycles = atoi(optarg);
                break;
            case 's':
                seed = (time_t)atoi(optarg);
                break;
        }
    }

    srand(seed);
    readFile(filename, &session);

    Solution * solution = malloc(session.antNumber*sizeof(Solution));

    session.pheromone = (float *)malloc(session.numberOfSets*sizeof(float));    
    
    int i;
    for (i=0; i<session.numberOfSets; i++) session.pheromone[i] = 1.0f;
    
    initializeSolutions(&session, solution);

    //printSet(&session);

    //changeSolution(&session, &solution, 1);

    //printSet(&session, 1);
    //printSolution(&session, &solution);

    //
    //int next = calculateProbability(&session, &solution);
    //printf("Next: %d\n\n\n", next);
        
    //changeSolution(&session, &solution, next);

    //printSet(&session, next);
    //printSolution(&session, &solution);

    //printf("Next: %d\n\n\n", calculateProbability(&session, &solution));

    //int i;
    //printPheromone(&session);
    
    Solution bestSolution;
    bestSolution.setsChoosed = malloc(session.numberOfSets*sizeof(bool));
    bestSolution.numbersCovered = malloc(session.numberOfSets*sizeof(bool));
    bestSolution.weight = INFINITY;

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (i=0; i<cycles; i++) {
        buildSolutions(&session, solution);
        updatePheromone(&session, solution);
        reinitializeSolutions(&session, solution, &bestSolution);
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    //printPheromone(&session);
    
    printf("Total time: %f\n\n", elapsed);
    printBestSolution(&session, &bestSolution);

    return 0;
}
