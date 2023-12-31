#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

#define CHARACTER_SET_SIZE 128
#define TRIE_PREFIX_SIZE 256
#define TRIE_MATCHES_SIZE 256

typedef struct TrieNode {
    struct TrieNode *children[CHARACTER_SET_SIZE];
    bool isEndOfWord;
} TrieNode;

typedef struct Trie {
    struct TrieNode *root;
    char *prefix;
    int prefixSize;
    char **matches;
    int matchesCount;
    int matchesSize;
} Trie;

typedef struct DirecTrie {
    struct Trie *trie;
    char** directory;
    int dir_count;
} DirecTrie;

DirecTrie *initDirecTrie();
Trie *initTrie();
TrieNode *createNode();

void insert(TrieNode *root, char *key);
void direc_search(DirecTrie *d, char *key);
void search(Trie *t, char *key);
void searchHelper(TrieNode *current, Trie *t, int level);
void printAllWords(TrieNode *root, char *prefix, int level);
void insertFilesInDirectory(DirecTrie *d, char *dirPath);
void freeTrie(TrieNode *root);

int main() {

    DirecTrie* d = initDirecTrie();
    insertFilesInDirectory(d, "/usr/bin");
    insertFilesInDirectory(d, "/usr/local/bin");

    char *partialText = (char*) malloc(sizeof(char) * 50);
    strcpy(partialText, "sta");
    direc_search(d, partialText); 

    if (d->trie->matchesCount == 0) {
        printf("No match found for: \n%s\n", partialText);
    } else if (d->trie->matchesCount == 1) {
        printf("Match found: \n%s\n", d->trie->matches[0]);
    } else {
        printf("Multiple matches found:\n");
        for (int i = 0; i < d->trie->matchesCount; i++) {
            printf("%s\n", d->trie->matches[i]);
        }
    }

    strcpy(partialText, "lsb");
    direc_search(d, partialText); 

    if (d->trie->matchesCount == 0) {
        printf("No match found for: \n%s\n", partialText);
    } else if (d->trie->matchesCount == 1) {
        printf("Match found: \n%s\n", d->trie->matches[0]);
    } else {
        printf("Multiple matches found:\n");
        for (int i = 0; i < d->trie->matchesCount; i++) {
            printf("%s\n", d->trie->matches[i]);
        }
    }

    free(partialText);

    freeTrie(d->trie->root);

    for (int i = 0; i < d->trie->matchesSize*TRIE_MATCHES_SIZE; i++) {
        free(d->trie->matches[i]);
    } 
    free(d->trie->matches);
    free(d->trie->prefix);
    free(d->trie);

    for(int i = 0; i < d->dir_count; i++) {
        free(d->directory[i]);
    }
    free(d->directory);
    free(d);

    return 0;
}

void direc_search(DirecTrie *d, char *key) {

    char* path = (char*)calloc(TRIE_PREFIX_SIZE, sizeof(char));
    int idx = 0;
    d->trie->matchesCount = 0;

    for(int i = 0; i < d->dir_count; i++) {
        strcpy(path, d->directory[i]);
        if(path[strlen(d->directory[i])-1] != '/') 
            strcat(path, "/");
        strcat(path, key);
        path[strlen(path)]='\0';
        
        search(d->trie, path);
    }

    free(path);
}

DirecTrie *initDirecTrie() {

    DirecTrie *d = (DirecTrie*) malloc(sizeof(DirecTrie));

    d->directory = NULL;
    d->dir_count = 0;
    
    d->trie = initTrie();

    return d; 
}

Trie *initTrie() {

    Trie* t = (Trie*) malloc (sizeof(Trie));

    t->prefix = (char*)malloc((TRIE_PREFIX_SIZE + 1) * sizeof(char));
    memset(t->prefix, '\0', (TRIE_PREFIX_SIZE + 1) * sizeof(char));

    t->prefixSize = TRIE_PREFIX_SIZE;

    t->matches = (char**) malloc(sizeof(char*) * TRIE_MATCHES_SIZE);
    t->matchesCount = 0;
    t->matchesSize = 1;

    t->root = createNode();

    return t;
}

TrieNode *createNode() {

    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));

    if (node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++) {
        node->children[i] = NULL;
    }

    node->isEndOfWord = false;
    return node;
}

void insertFilesInDirectory(DirecTrie *d, char *dirPath) {

    DIR *directory;
    struct dirent *entry;

    if ((directory = opendir(dirPath)) == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char filePath[PATH_MAX];
            snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

            insert(d->trie->root, filePath);

            if (entry->d_type == DT_DIR) {
                // insertFilesInDirectory(d, filePath);
            }
        }
    }

    closedir(directory);

    
    d->directory = realloc(d->directory, (d->dir_count+1) * sizeof(char*));
    d->directory[d->dir_count] = (char*)malloc(strlen(dirPath) + 1);
    strcpy(d->directory[d->dir_count], dirPath);
    d->dir_count++;
}

void insert(TrieNode *root, char *key) {

    TrieNode *current = root;
    int len = strlen(key);

    for (int level = 0; level < len; level++) {
        int index = (unsigned char)key[level];
        if (!current->children[index]) {
            current->children[index] = createNode();
        }
        current = current->children[index];
    }

    current->isEndOfWord = true;
}

void search(Trie *t, char *key) {

    TrieNode *current = t->root;
    int len = strlen(key);
    int prefixSize = len;

    for (int level = 0; level < len; level++) {
        int index = (unsigned char)key[level];
        if (!current->children[index]) {
            return;
        }

        if(prefixSize >= TRIE_PREFIX_SIZE)
            t->prefix = realloc(t->prefix, (prefixSize + 1) * sizeof(char));
        if (t->prefix == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        t->prefix[level] = key[level];
        t->prefix[level + 1] = '\0';

        current = current->children[index];
    }

    searchHelper(current, t, len);
}

void searchHelper(TrieNode *current, Trie *t, int level) {

    if (current->isEndOfWord) {
        if(t->matchesCount+1 >= TRIE_MATCHES_SIZE * t->matchesSize) {
            t->matches = realloc(t->matches, (++t->matchesSize * TRIE_MATCHES_SIZE) * sizeof(char *));
        }
        if (t->matches == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        if(t->matches[t->matchesCount])
            free(t->matches[t->matchesCount]);
        t->matches[t->matchesCount++] = strdup(t->prefix); // mem leak (valgrind)
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++) {
        if (current->children[i]) {
            t->prefix[level] = i;
            searchHelper(current->children[i], t, level + 1);
            t->prefix[level] = '\0'; 
        }
    }
}

void printAllWords(TrieNode *root, char *prefix, int level) {

    if (root->isEndOfWord) {
        prefix[level] = '\0';
        printf("%s\n", prefix);
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++) {
        if (root->children[i]) {
            prefix[level] = i;
            printAllWords(root->children[i], prefix, level + 1);
        }
    }
}

void freeTrie(TrieNode *root) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++) {
        freeTrie(root->children[i]);
    }

    free(root);
}
