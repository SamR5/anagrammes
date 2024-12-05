#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define MAX_WORDS 20

float first[26];
float firstTwo[26][26];
float triple[26][26][26];
float lastTwo[26][26];

float scores[MAX_WORDS] = {-1};
char *words[MAX_WORDS];

void parse_file(const char* path);
void add_word(const char* word, int length);
void make_tables();
float get_score(const char* word, int size);
float sum_array(const float *array, int size);
void permutation(char string[], int a, int b);
int min_index(float* arr, int size);
void insert_score(const char* perm, const int size);
int cmpw(const void *wa, const void *wb);
int cmps(const void *sa, const void *sb);


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
        if (tmp == 0)
            return 0;
        score += tmp;
    }
    return score;
}

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

int min_index(float arr[], int size) {
    float x = arr[0];
    int index=0;
    for (int i=1; i<size; i++) {
        if (x > arr[i]) {
            x = arr[i];
            index = i;
        }
    }
    return index;
}

void insert_score(const char perm[], const int size) {
    float score = get_score(perm, size);
    for (int i=0; i<MAX_WORDS; i++) {
        // first check scores, if different then words are differents
        if (scores[i] == score && strncmp(words[i], perm, size) == 0)
            return;
    }
    int minimum = min_index(scores, MAX_WORDS);
    if (score > scores[minimum]) {
        scores[minimum] = score;
        strncpy(words[minimum], perm, size);
    }
}

int cmpw(const void *wa, const void *wb) {
    //return strcmp(*(const char**)wa, *(const char**)wb);
    int size = 8;
    const char *aa = *(const char**)wa;
    const char *bb = *(const char**)wb;
    float a = get_score(aa, size);
    float b = get_score(bb, size);
    if (a-b < 0)
        return 1;
    if (a-b > 0)
        return -1;
    return 0;
}

int cmps(const void *sa, const void *sb) {
    float *a = sa;
    float *b = sb;
    if (*a-*b < 0)
        return 1;
    if (*a-*b > 0)
        return -1;
    return 0;
}

int main(int argc, char* argv[]) {
    parse_file("lexique.txt");
    make_tables();
    int size = strlen(argv[1]);
    printf("%s_%d\n", argv[1], size);
    for (int i=0; i<MAX_WORDS; i++) {
        words[i] = calloc(size, sizeof(char));
    }
    permutation(argv[1], 0, size-1); // size-1 is the index of the last letter
    qsort(words, MAX_WORDS, sizeof(char*), cmpw);
    qsort(scores, MAX_WORDS, sizeof(float), cmps);
    for (int i=0; i<MAX_WORDS; i++) {
        printf("%s: %f\n", words[i], scores[i]);
    }
    return 0;
}
