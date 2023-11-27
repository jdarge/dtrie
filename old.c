#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

#define CHARACTER_SET_SIZE 128

typedef struct TrieNode {
    struct TrieNode *children[CHARACTER_SET_SIZE];
    bool isEndOfWord;
} TrieNode;

TrieNode *createNode();
void insert(TrieNode *root, const char *key);
void search(TrieNode *root, const char *key, char **prefix, int *count, char ***matches);
void searchHelper(TrieNode *root, char *prefix, int level, int *count, char ***matches);
void printAllWords(TrieNode *root, char *prefix, int level);
void freeTrie(TrieNode *root);
void insertFilesInDirectory(TrieNode *root, const char *dirPath);

int main() {
    
    TrieNode *root = createNode();
    
    const char *dirPath1 = "/usr/bin";
    const char *dirPath2 = "/usr/local/bin";

    insertFilesInDirectory(root, dirPath1);
    insertFilesInDirectory(root, dirPath2);

    char *prefix = NULL;
    int prefixSize = 0;

    char **matches = NULL;
    int matchesCount = 0;
    int matchesSize = 0;

    char *partialText = (char*) malloc(sizeof(char) * 50);
    strcpy(partialText, "/usr/bin/lsb");
    search(root, partialText, &prefix, &matchesCount, &matches);

    // char tmp[100];
    // printAllWords(root, tmp, 0);

    if (matchesCount == 0) {
        printf("No match found for: %s\n", partialText);
    } else if (matchesCount == 1) {
        printf("Match found: %s\n", matches[0]);
    } else {
        printf("Multiple matches found:\n");
        for (int i = 0; i < matchesCount; i++) {
            printf("%s\n", matches[i]);
        }
    }

    matchesCount = 0;
    strcpy(partialText, "/usr/local/bin/timer");
    search(root, partialText, &prefix, &matchesCount, &matches);

    if (matchesCount == 0) {
        printf("No match found for:\n%s\n", partialText);
    } else if (matchesCount == 1) {
        printf("Match found:\n%s\n", matches[0]);
    } else {
        printf("Multiple matches found:\n");
        for (int i = 0; i < matchesCount; i++) {
            printf("%s\n", matches[i]);
        }
    }

    free(prefix);
    for (int i = 0; i < matchesCount; i++) {
        free(matches[i]);
    }
    free(matches);

    freeTrie(root);

    return 0;
}

void insertFilesInDirectory(TrieNode *root, const char *dirPath) {
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

            insert(root, filePath);

            if (entry->d_type == DT_DIR) {
                insertFilesInDirectory(root, filePath);
            }
        }
    }

    closedir(directory);
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

void insert(TrieNode *root, const char *key) {
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

void search(TrieNode *root, const char *key, char **prefix, int *count, char ***matches) {
    TrieNode *current = root;
    int len = strlen(key);
    int prefixSize = len;

    for (int level = 0; level < len; level++) {
        int index = (unsigned char)key[level];
        if (!current->children[index]) {
            return;
        }

        *prefix = realloc(*prefix, (prefixSize + 1) * sizeof(char));
        if (*prefix == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        (*prefix)[level] = key[level];
        (*prefix)[level + 1] = '\0';

        current = current->children[index];
    }

    searchHelper(current, *prefix, len, count, matches);
}

void searchHelper(TrieNode *root, char *prefix, int level, int *count, char ***matches) {
    if (root->isEndOfWord) {
        *matches = realloc(*matches, (*count + 1) * sizeof(char *));
        if (*matches == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        (*matches)[*count] = strdup(prefix);
        (*count)++;
    }

    for (int i = 0; i < CHARACTER_SET_SIZE; i++) {
        if (root->children[i]) {
            prefix[level] = i;
            searchHelper(root->children[i], prefix, level + 1, count, matches);
            prefix[level] = '\0'; 
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
