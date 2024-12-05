#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define MAX_WORDS 10000

float first[26];
float firstTwo[26][26];
float triple[26][26][26];
float lastTwo[26][26];

typedef struct {
    char *string;
    float score;
} Word;

Word **anagrammes;
int cursor;

void parse_file(const char* path);
void add_word(const char* word, int length);
void make_tables();
float get_score(const char* word, int size);
void swap(char *x, char* y);
void permutation(char string[], int a, int b);
void insert_score(const char perm[], const int size);
void write_file();
int factorial(int n);

void parse_file(const char* path) {
    FILE* p;
    p = fopen(path, "r");
    if (p == NULL)
        exit(EXIT_FAILURE);
    char* line;
    size_t len;
    while (getline(&line, &len, p) != -1) {
        int realLen = strlen(line) - 1;
        if (realLen > 2)
            add_word(line, realLen);
    }
    if (fclose(p) != 0)
        printf("Error while closing the file\n");
    free(line); // error detected with valgrind
}

void add_word(const char* word, int length) {
    first[word[0]-'a'] += 1;
    firstTwo[word[0]-'a'][word[1]-'a'] += 1;
    lastTwo[word[length-2]-'a'][word[length-1]-'a'] += 1;
    for (int i=1; i<length-3; i++) {
        triple[word[i]-'a'][word[i+1]-'a'][word[i+2]-'a'] += 1;
    }
}


// in sum_array and make_tables the -O3 optimization flag gets differents
// results if the operators += and /= are used (word with -O1)
float sum_array(const float *array, int size) {
    float sum = 0;
    const float *tmp = array;
    int i=0;
    while (i<size) {
        sum += *tmp;
        tmp++;
        i++;
    }
    return sum;
}

void make_tables() {
    float fSum = sum_array(first, 26);
    float ftSum = sum_array(firstTwo, 26*26);
    float ltSum = sum_array(lastTwo, 26*26);
    float trSum = sum_array(triple, 26*26*26);
    for (int i=0; i<26; i++) {
        first[i] = first[i]/fSum;
        for (int j=0; j<26; j++) {
            firstTwo[i][j] = firstTwo[i][j]/ftSum;
            lastTwo[i][j] = lastTwo[i][j]/ltSum;
            for (int k=0; k<26; k++) {
                triple[i][j][k] = triple[i][j][k]/trSum;
            }
        }
    }
}

float get_score(const char* word, int size) {
    float score = 0;
    //score += first[word[0]-'a'];
    score += firstTwo[word[0]-'a'][word[1]-'a'];
    score += lastTwo[word[size-2]-'a'][word[size-1]-'a'];
    for (int i=0; i<size-2; i++) {
        float tmp = triple[word[i]-'a'][word[i+1]-'a'][word[i+2]-'a'];
        score += tmp;
    }
    return score;
}

void swap(char *x, char* y) {
    char tmp = *x;
    *x = *y;
    *y = tmp;
}

int should_swap(char string[], int a, int c) {
    for (int i=a; i<c; i++) {
        if (string[i] == string[c])
            return 0;
    }
    return 1;
}

void permutation(char string[], int a, int b) {
    if (a==b) {
        insert_score(string, b+1);
    } else {
        for (int i=a; i<=b; i++) {
            if (!should_swap(string, a, i))
                continue;
            char tmp = string[a];
            string[a] = string[i];
            string[i] = tmp;
            permutation(string, a+1, b);
            string[i] = string[a];
            string[a] = tmp;
        }
    }
}

void insert_score(const char perm[], const int size) {
    float score = get_score(perm, size);
    Word *new = malloc(sizeof(Word*));
    new->string = calloc(size, sizeof(char));
    strncpy(new->string, perm, size);
    new->score = score;
    anagrammes[cursor] = new;
    cursor++;
    if (cursor >= MAX_WORDS)
        return write_file();
}

void write_file() {
    FILE *fptr;
    fptr = fopen("result.txt", "a");
    if (fptr == NULL)
        exit(EXIT_FAILURE);
    for (int i=0; i<cursor; i++) {
        fprintf(fptr, "%s: %f\n", anagrammes[i]->string, anagrammes[i]->score);
        free(anagrammes[i]->string);
        free(anagrammes[i]);
    }
    if (fclose(fptr) != 0)
        printf("Failed to close\n");
    cursor = 0;
}

int word_cmp(const void *wa, const void *wb) {
    Word **a = (Word **)wa;
    Word **b = (Word **)wb;
    if ((*a)->score > (*b)->score)
        return 1;
    else if ((*a)->score < (*b)->score)
        return -1;
    return strcmp((*a)->string, (*b)->string);
}

int factorial(int n) {
    if (n==1)
        return n;
    return n * factorial(n-1);
}

int main(int argc, char* argv[]) {
    parse_file("lexique.txt");
    make_tables();
    int size = strlen(argv[1]);
    int prmt = factorial(size);
    cursor = 0;
    anagrammes = calloc(prmt, sizeof(Word*));
    permutation(argv[1], 0, size-1); // size-1 is the index of the last letter
    write_file();
    for (int i=0; i<cursor; i++) {
        free(anagrammes[i]->string);
        free(anagrammes[i]);
    }
    free(anagrammes);
    return 0;
}
