/*             | Talha BAYBURTLU | 150118066 | CSE2055-Project1 |                 *
 * ------------------------------------------------------------------------------ *
 * This program's goal is to read all documents under all categories              *
 * and finding higher order paths (Only 1st,2nd,3rd) between words                *
 * based on their location, finding most common words(TF) with their occurrences  *
 * for each category , finding most common words inverse document frequency with  *
 * logarithmic calculation multiplied with term frequency (TF*IDF).               */

/*             TABLE'S COLUMNS MAY SLIDE BECAUSE OF TURKISH CHARACTERS            */
/* THIS PROJECT CREATED WITH CYGWIN COMPILER, WHICH ACCEPTS UTF-8 FORMATTED TXT'S *
 * FOR TURKISH CHARACTERS. TXT'S FORMAT MAY CHANGE BASED ON WHAT COMPILER USED    *
 * WHILE RUNNING PROJECT. IF THERE IS A PROBLEM ABOUT TURKISH CHARACTERS PLEASE   *
 *                      TRANSFORM FORMATS TO UTF-8 OR ANSI                        */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <locale.h>
#include <dirent.h>
#include <math.h>

typedef struct category { // Category struct for holding general category locations or word categories.
    char *category;
    int occurrence, documentCounter; // Occurrence for word categories.
    struct wordStruct *frequentWords[5]; // For general categories.
    struct category *nextCategoryPtr;
} category, *categoryPtr;

typedef struct wordLocation { // WordLocation struct for holding word's location as strings.
    char *wordLocation;
    struct wordLocation *nextLocationPtr;
} wordLocation , *wordLocationPtr;

typedef struct wordStruct { // WordStruct struct for Master Linked List.
    char *word;
    categoryPtr headCategoryPtr;
    wordLocationPtr headLocationPtr;
    struct wordStruct *nextPtr;
    struct orderStruct *firstOrderPtr;
    struct orderStruct *secondOrderPtr;
    struct orderStruct *thirdOrderPtr;
} wordStruct , *wordStructPtr;

typedef struct orderStruct { // OrderStruct struct for holding word structs for any higher order paths.
    struct wordStruct *wordStructPtr;
    struct orderStruct *nextOrderStruct;
} orderStruct , *orderStructPtr;

/* ------------------------------------- FUNCTION DECLARATIONS ------------------------------------- */
char *readFromFile(FILE* filePtr);
void insert(wordStructPtr *headPtr ,char* word ,char *category ,char* wordLocationStr);
void printList(wordStructPtr headPtr);
int isDuplicate(wordStructPtr headPtr , char *word, char* category,char *wordLocationStr);
char *removeSequences(char *word);
void assignWordLocations(wordStructPtr headPtr , char *filePath);
void insertWordLocation(wordStructPtr nodePtr ,char *wordLocationStr);
char *formatWordLocation(char *filePath);
void createOrderPaths(wordStructPtr headPtr);
int isFirstOrder(wordStructPtr currentWordPtrV1 , wordStructPtr currentWordPtrV2);
int isSecondOrder(wordStructPtr wordStructPtrV1 , wordStructPtr wordStructPtrV2, wordStructPtr headPtr);
int isThirdOrder(wordStructPtr wordStructPtrV1 , wordStructPtr wordStructPtrV2, wordStructPtr headPtr);
void printOrders(wordStructPtr headPtr);
void sortLinkedList(wordStructPtr *headPtr);
void insertCategory(categoryPtr *headPtr, char *category, int occurrence);
void determineFrequentWords(wordStructPtr headPtr,categoryPtr generalCategories);
void sortFrequentWordsByIDF(categoryPtr generalCategories);
void printFrequentWords(categoryPtr generalCategories);
/* ------------------------------------- FUNCTION DECLARATIONS ------------------------------------- */

int main() {
    setlocale(LC_COLLATE, "tr_TR.utf8");

    DIR *dir; DIR *fileDir; // Two structs for opening directories for respectively categories and documents.
    FILE *filePtr = NULL;
    struct dirent *dirStructPtr = NULL;
    struct dirent *fileDirStructPtr = NULL;
    char *fileName = calloc(25 , sizeof(char));
    dir = opendir("dataset/");

    if (!dir) // Exit if dataset couldn't found.
        exit(-1);

    wordStructPtr headPtr = NULL;
    categoryPtr generalCategories = NULL;
    while ((dirStructPtr = readdir(dir)) != NULL) { // Reading all categories.
        sprintf(fileName, "dataset/%s" , dirStructPtr->d_name);
        fileDir = opendir(fileName);
        if (strcoll(dirStructPtr->d_name ,".") == 0 || strcoll(dirStructPtr->d_name ,"..") == 0) // Eliminating current and parent directories.
            continue;

        insertCategory(&generalCategories,dirStructPtr->d_name, -1); // Inserting general categories to a linked list.

        while((fileDirStructPtr = readdir(fileDir)) != NULL) { // Reading all documents.
            sprintf( fileName , "dataset/%s/%s" , dirStructPtr->d_name ,fileDirStructPtr->d_name);
            if (strcoll(fileDirStructPtr->d_name ,".") == 0 || strcoll(fileDirStructPtr->d_name ,"..") == 0) // Eliminating current and parent directories.
                continue;

            if ((filePtr = fopen(fileName , "r")) == NULL) // Opening file for reading purposes.
                continue;

            char *wordLocation = formatWordLocation(fileName); // Parsing word location.
            char *content = readFromFile(filePtr); // Reading words from file.
            char *tokenPtr = strtok(content, " ");

            insert(&headPtr,removeSequences(tokenPtr),dirStructPtr->d_name,wordLocation); // Inserting word to Master Linked List.

            while ((tokenPtr = strtok(NULL , " ")) != NULL) // Inserting all words to Master Linked List by tokenizing.
                insert(&headPtr,removeSequences(tokenPtr),dirStructPtr->d_name,wordLocation);

            categoryPtr currentCategory = generalCategories;
            while (currentCategory->nextCategoryPtr != NULL)
                currentCategory = currentCategory->nextCategoryPtr;

            currentCategory->documentCounter++; // Updating document counter for current general category.

            fclose(filePtr);
        }
    }
    closedir(dir);

    sortLinkedList(&headPtr); // Sorting Master Linked List.
    createOrderPaths(headPtr); // Creating higher order paths.

    printOrders(headPtr); // Printing higher order paths to console.
    determineFrequentWords(headPtr,generalCategories); // Determining frequent words inside Master Linked List.
    printFrequentWords(generalCategories); // Printing frequent words to console.
    return 0;
}

char *readFromFile(FILE* filePtr) { // Reads words and returns whole content as a string.
    char *content = malloc(256 * sizeof(char));
    char *holder = malloc(256 * sizeof(char));
    int count = 1;

    *content = '\0';

    while (fgets(holder, 255, filePtr) != NULL) { // Getting 255 characters.
        content = realloc(content , 256 * ++count); // Reallocating string with incremented size.
        strcat(content,holder);
    }

    free(holder);
    return content; // Returning whole content as a string.
}

void insert(wordStructPtr *headPtr ,char* word ,char *category ,char* wordLocationStr) { // Inserts word to Master Linked List.
    // Checks if it is duplicate or empty string.
    if (isDuplicate(*headPtr , word, category,wordLocationStr) || strcoll(word , " ") == 0 || strcoll(word , "") == 0 || strcoll(word , "  ") == 0)
        return;

    wordStructPtr nodePtr = malloc(sizeof(wordStruct)); // Allocating memory for new node.
    nodePtr->headLocationPtr = NULL;    nodePtr->headCategoryPtr = NULL;    nodePtr->nextPtr = NULL;
    nodePtr->firstOrderPtr = NULL;      nodePtr->secondOrderPtr = NULL;     nodePtr->thirdOrderPtr = NULL;

    nodePtr->word = calloc(20 , sizeof(char));
    strcpy(nodePtr->word , word); // Copying word.
    insertWordLocation(nodePtr, wordLocationStr);  // Inserting word's locations.
    insertCategory(&(nodePtr->headCategoryPtr),category,1); // Inserting word's categories.

    if (*headPtr != NULL) { // Inserting new node to at the end of the Master Linked List.
        wordStructPtr currentStructPtr = *headPtr;

        while (currentStructPtr->nextPtr != NULL)
            currentStructPtr = currentStructPtr->nextPtr;

        currentStructPtr->nextPtr = nodePtr;
    } else // Inserting new node as head node.
        *headPtr = nodePtr;
}

void sortLinkedList(wordStructPtr *headPtr) { // Sorts Master Linked List alphabetically.
    int keepSorting = 1; // Sorting condition.
    wordStructPtr tempStructPtr = NULL;

    while (keepSorting) {
        wordStructPtr currentStructPtr = *headPtr;
        wordStructPtr previousStructPtr = currentStructPtr;

        while (currentStructPtr->nextPtr != NULL) { // Comparing all words with the next one.
            if (strcoll(currentStructPtr->nextPtr->word, currentStructPtr->word) < 0) {
                if (currentStructPtr != *headPtr) { // Replacing nodes where they are not located at the beginning
                    tempStructPtr = currentStructPtr->nextPtr->nextPtr;
                    previousStructPtr->nextPtr = currentStructPtr->nextPtr;
                    currentStructPtr->nextPtr->nextPtr =currentStructPtr;
                    currentStructPtr->nextPtr = tempStructPtr;
                } else { // Replacing nodes where they are located at the beginning.
                    tempStructPtr = currentStructPtr->nextPtr->nextPtr;
                    *headPtr = currentStructPtr->nextPtr;
                    currentStructPtr->nextPtr->nextPtr = currentStructPtr;
                    currentStructPtr->nextPtr = tempStructPtr;
                }
                break;
            }

            previousStructPtr = currentStructPtr;
            currentStructPtr = currentStructPtr->nextPtr;

            if (currentStructPtr->nextPtr == NULL) // Checks all nodes are sorted or not.
                keepSorting = 0;
        }
    }
}

int isDuplicate(wordStructPtr headPtr , char *word, char* category,char *wordLocationStr) { // Checks given word is duplicated inside Master Linked List or not.
    wordStructPtr currentPtr = headPtr;

    while (currentPtr != NULL) { // Checking all words.
        if (currentPtr->word != '\0'  && strcoll(word,currentPtr->word) == 0) { // Comparing given word with current word.
            insertWordLocation(currentPtr,wordLocationStr); // Inserting word location if it's from another document.
            insertCategory(&(currentPtr->headCategoryPtr),category,1); // Inserting category if it's from another category.
            return 1;
        }
        currentPtr = currentPtr->nextPtr;
    }
    return 0;
}

char *removeSequences(char *word) { // Removes unwanted characters from words.
    char *updatedWord = calloc(1, sizeof(word));

    int i = 0, j= 0;
    for (; i < strlen(word) ; i++) {
        if (!iscntrl(word[i])  && !ispunct(word[i]) &&!isspace(word[i])) // Checking characher is an unwanted one.
            updatedWord[j++] = word[i];
    }

    updatedWord[strlen(word)] = '\0';
    return updatedWord;
}

void insertWordLocation(wordStructPtr nodePtr ,char *wordLocationStr) { // Inserts word location to word's word location linked list.
    wordLocationPtr currentLocationPtr = nodePtr->headLocationPtr;
    if (nodePtr->headLocationPtr != NULL) // Checking every word location for not duplicating it.
        while (currentLocationPtr != NULL) {
            if (strcoll(currentLocationPtr->wordLocation , wordLocationStr) == 0)
                return;
            else
                currentLocationPtr = currentLocationPtr->nextLocationPtr;
        }

    wordLocationPtr newLocationPtr = malloc(sizeof(wordLocation)); // Allocating new node.
    newLocationPtr->wordLocation = calloc(15, sizeof(char));
    newLocationPtr->nextLocationPtr = NULL;
    strcpy(newLocationPtr->wordLocation , wordLocationStr);

    if (nodePtr->headLocationPtr != NULL) { // Inserting new node to at the end of the word location linked list.
        currentLocationPtr = nodePtr->headLocationPtr;

        while (currentLocationPtr->nextLocationPtr != NULL)
            currentLocationPtr = currentLocationPtr->nextLocationPtr;

        currentLocationPtr->nextLocationPtr = newLocationPtr;
    } else // Inserting new node as head node.
        nodePtr->headLocationPtr = newLocationPtr;
}

void insertCategory(categoryPtr *headPtr, char *category, int occurrence) { // Inserts category to word's category linked list.
    categoryPtr currentCategoryPtr = *headPtr;
    while (currentCategoryPtr != NULL) { // Checking every category for not duplicating it.
        if (strcoll(currentCategoryPtr->category, category) == 0){
            currentCategoryPtr->occurrence++;
            return;
        }

        currentCategoryPtr = currentCategoryPtr->nextCategoryPtr;
    }

    categoryPtr categoryNodePtr = malloc(sizeof(struct category));  // Allocating new node.
    categoryNodePtr->category = calloc(1,sizeof(category));
    strcpy(categoryNodePtr->category,category);
    categoryNodePtr->occurrence = occurrence;
    categoryNodePtr->nextCategoryPtr = NULL;
    categoryNodePtr->documentCounter = 0;

    if (*headPtr != NULL) { // Inserting new node to at the end of the category linked list.
        currentCategoryPtr = *headPtr;

        while (currentCategoryPtr->nextCategoryPtr != NULL)
            currentCategoryPtr = currentCategoryPtr->nextCategoryPtr;

        currentCategoryPtr->nextCategoryPtr = categoryNodePtr;
    } else // Inserting new node as head node.
        *headPtr = categoryNodePtr;
}

char *formatWordLocation(char *filePath) { // Formats word location.
    char *category = calloc(10 , sizeof(char));
    char *document = calloc(3 , sizeof(char));
    char *wordLocation = calloc(15 , sizeof(char));
    char *copiedFilePath = calloc(1, sizeof(filePath));
    strcpy(copiedFilePath , filePath);

    strtok(copiedFilePath , "/"); // Tokenizing file path.
    category = strtok(NULL , "/");
    document = strtok(NULL , "/");
    document[strlen(document) - 4] = '\0';

    sprintf(wordLocation , "%s-%s" , category , document); // Formatting word location as "category-document"
    return wordLocation;
}

void createOrderPaths(wordStructPtr headPtr) { // Creates first-second-third order paths.
    int firstOrderCheck = 0 , secondOrderCheck = 0 , thirdOrderCheck = 0, finalCheck = 0, orderCoefficient = 0; // Order conditions.
    wordStructPtr currentWordPtrV1 = NULL;

    for (; orderCoefficient <= 2 ; orderCoefficient++) { // Looping for creating all order paths.
        currentWordPtrV1 = headPtr;
        while(currentWordPtrV1 != NULL) {
            wordStructPtr currentWordPtrV2 = currentWordPtrV1;

            while (currentWordPtrV2 != NULL) {
                if (currentWordPtrV2 == currentWordPtrV1) { // Checking words are the same or not.
                    currentWordPtrV2 = currentWordPtrV2->nextPtr;
                    continue;
                }

                switch (orderCoefficient) { // Creating order conditions based on current order coefficent.
                    case 2: thirdOrderCheck = isThirdOrder(currentWordPtrV1,currentWordPtrV2,headPtr);
                    case 1: secondOrderCheck = isSecondOrder(currentWordPtrV1,currentWordPtrV2,headPtr);
                    case 0: firstOrderCheck = isFirstOrder(currentWordPtrV1,currentWordPtrV2); break;
                }

                switch (orderCoefficient) { // Creating final check condition based on order coefficent.
                    case 2: finalCheck = thirdOrderCheck && !secondOrderCheck && !firstOrderCheck; break;
                    case 1: finalCheck = secondOrderCheck && !firstOrderCheck; break;
                    case 0: finalCheck = firstOrderCheck; break;
                }

                orderStructPtr currentOrderStructPtr = NULL;
                if (finalCheck) { // Checking suitable for creating current order.
                    switch (orderCoefficient) { // Determining current order struct pointer based on order coefficent.
                        case 0: currentOrderStructPtr = currentWordPtrV1->firstOrderPtr; break;
                        case 1: currentOrderStructPtr = currentWordPtrV1->secondOrderPtr; break;
                        case 2: currentOrderStructPtr = currentWordPtrV1->thirdOrderPtr; break;
                    }
                    orderStructPtr previousOrderStructPtr = currentOrderStructPtr;

                    while (currentOrderStructPtr != NULL) { // Making current order struct pointer as last current order struct pointer.
                        previousOrderStructPtr = currentOrderStructPtr;
                        currentOrderStructPtr = currentOrderStructPtr->nextOrderStruct;
                    }

                    currentOrderStructPtr = malloc(sizeof(orderStruct)); // Allocating memory for new node.
                    currentOrderStructPtr->wordStructPtr = currentWordPtrV2;
                    currentOrderStructPtr->nextOrderStruct = NULL;

                    switch (orderCoefficient) { // Inserting new node based on order coefficent.
                        case 0:
                            if (currentWordPtrV1->firstOrderPtr == NULL) // Inserting new node as head node.
                                currentWordPtrV1->firstOrderPtr = currentOrderStructPtr;
                            else  // Inserting new node to at the end of order linked list.
                                previousOrderStructPtr->nextOrderStruct = currentOrderStructPtr; break;
                        case 1:
                            if (currentWordPtrV1->secondOrderPtr == NULL) // Inserting new node as head node.
                                currentWordPtrV1->secondOrderPtr = currentOrderStructPtr;
                            else // Inserting new node to at the end of order linked list.
                                previousOrderStructPtr->nextOrderStruct = currentOrderStructPtr; break;
                        case 2:
                            if (currentWordPtrV1->thirdOrderPtr == NULL) // Inserting new node as head node.
                                currentWordPtrV1->thirdOrderPtr = currentOrderStructPtr;
                            else // Inserting new node to at the end of order linked list.
                                previousOrderStructPtr->nextOrderStruct = currentOrderStructPtr; break;
                    }
                }
                currentWordPtrV2 = currentWordPtrV2->nextPtr;
            }
            currentWordPtrV1 = currentWordPtrV1->nextPtr;
        }
    }
}

int isFirstOrder(wordStructPtr currentWordPtrV1 , wordStructPtr currentWordPtrV2) { // Checks there should be first order path between words.
    wordLocationPtr wordLocationPtrV1 = currentWordPtrV1->headLocationPtr;
    while (wordLocationPtrV1 != NULL) {
        wordLocationPtr wordLocationPtrV2 = currentWordPtrV2->headLocationPtr;
        while (wordLocationPtrV2 != NULL) {
            // Checking words has same location or not.
            if (strcoll(wordLocationPtrV1->wordLocation, wordLocationPtrV2->wordLocation) == 0)
                return 1;

            wordLocationPtrV2 = wordLocationPtrV2->nextLocationPtr;
        }
        wordLocationPtrV1 = wordLocationPtrV1->nextLocationPtr;
    }
    return 0;
}

int isSecondOrder(wordStructPtr wordStructPtrV1 , wordStructPtr wordStructPtrV2, wordStructPtr headPtr) { // Checks there should be second order path between words.
   wordStructPtr currentWordStructPtr = headPtr;

   while (currentWordStructPtr != NULL) {
       // Checking words making first order path with current word.
        if (isFirstOrder(currentWordStructPtr,wordStructPtrV1) && isFirstOrder(currentWordStructPtr,wordStructPtrV2))
            return 1;
       currentWordStructPtr = currentWordStructPtr->nextPtr;
   }

   return 0;
}

int isThirdOrder(wordStructPtr wordStructPtrV1 , wordStructPtr wordStructPtrV2, wordStructPtr headPtr) { // Checks there should be third order path between words.
    wordStructPtr currentWordStructPtr = headPtr;

    while (currentWordStructPtr != NULL) {
        // Checking words making two second order with current word.
        if ((isSecondOrder(currentWordStructPtr,wordStructPtrV1,headPtr) && isSecondOrder(currentWordStructPtr,wordStructPtrV2,headPtr)))
            return 1;
        currentWordStructPtr = currentWordStructPtr->nextPtr;
    }

    return 0;
}

void printOrders(wordStructPtr headPtr) { // Printing all higher order paths.
    wordStructPtr currentWordPtr = NULL;
    orderStructPtr orderStructPtr = NULL;
    int orderCoefficient = 0;

    for (; orderCoefficient <= 2 ; orderCoefficient++) { // Looping for printing all order paths.
        currentWordPtr = headPtr;
        switch (orderCoefficient) { // Printing main message based on order coefficient.
            case 0: printf("1st-order term co-occurrence : "); break;
            case 1:printf("2nd-order term co-occurrence : "); break;
            case 2: printf("3rd-order term co-occurrence : "); break;
        }

        while (currentWordPtr != NULL) {
            switch (orderCoefficient) { // Taking order struct ptr based on order coefficient.
                case 0: orderStructPtr = currentWordPtr->firstOrderPtr; break;
                case 1: orderStructPtr = currentWordPtr->secondOrderPtr; break;
                case 2: orderStructPtr = currentWordPtr->thirdOrderPtr; break;
            }

            while (orderStructPtr != NULL) { // Printing path.
                printf("{%s,%s} ," , currentWordPtr->word , orderStructPtr->wordStructPtr->word);
                orderStructPtr = orderStructPtr->nextOrderStruct;
            }
            currentWordPtr = currentWordPtr->nextPtr;
        }
        printf("\n");
    }
    printf("\nBig O -> O((n*(n + n (n^2 + n* n^2) + n)) + (nlogn) + (((n * (n+1) /2) * n) + 2n ) + (n^2 * (2n)) + ( 2n + n^2+ nlogn + 2n + n^2) -> O(n^5) \n");
}

void determineFrequentWords(wordStructPtr headPtr,categoryPtr generalCategories) { // Determines frequent words in Master Linked List.
    categoryPtr currentCategoryPtrV1 = generalCategories;

    while (currentCategoryPtrV1 != NULL) {
        wordStructPtr frequentWords[5] = {NULL, NULL, NULL, NULL, NULL}; // Holding frequent words array for filling purposes.
        wordStructPtr currentStructPtr = headPtr;
        while (currentStructPtr != NULL) {
            categoryPtr currentCategoryPtrV2 = currentStructPtr->headCategoryPtr;
            while (currentCategoryPtrV2 != NULL) {
                if(strcoll(currentCategoryPtrV2->category , currentCategoryPtrV1->category) == 0) // Checking word's category is the same as general category.
                    break;

                currentCategoryPtrV2 = currentCategoryPtrV2->nextCategoryPtr;
            }

            if (currentCategoryPtrV2 != NULL) {
                for (int i = 0; i < 5 ; i++) { // Determining all indexes of frequent word array.
                    int currentCategoryValue , frequentWordValue;

                    categoryPtr currentCategoryPtrV3 = frequentWords[i] != NULL ? frequentWords[i]->headCategoryPtr : NULL;
                    while (currentCategoryPtrV3 != NULL) {
                        // Checking word's category is the same as current frequent words array index.
                        if (strcoll(currentCategoryPtrV3->category,currentCategoryPtrV2->category) == 0)
                            break;
                        currentCategoryPtrV3=currentCategoryPtrV3->nextCategoryPtr;
                    }

                    // Checking word's occurence is higher than current frequnt words array index.
                    if (frequentWords[i] == NULL || currentCategoryPtrV2->occurrence > currentCategoryPtrV3->occurrence) {
                        for (int j = 5 - 2 ; j >= i; j--) // Shifting indexes for making a place for new word.
                            frequentWords[j + 1] = frequentWords[j];
                        frequentWords[i] = currentStructPtr;
                        break;
                    }
                }
            }
            currentStructPtr = currentStructPtr->nextPtr;
        }

        for (int i = 0; i < 5 ; i++) // Copying contents of temporary array to current general category's frequent words array.
            currentCategoryPtrV1->frequentWords[i] = frequentWords[i];

        currentCategoryPtrV1 = currentCategoryPtrV1->nextCategoryPtr;
    }
}

void sortFrequentWordsByIDF(categoryPtr generalCategories) { // Sorts frequent words by TF*IDF.
    categoryPtr currentCategoryPtrV1 = generalCategories;

    while (currentCategoryPtrV1 != NULL) {
        int keepSorting = 1; // Sorting condition.
        while (keepSorting) {
            keepSorting = 0;
            for(int i = 0; i < 4 ; i++) {
                categoryPtr currentCategoryPtrV2 = currentCategoryPtrV1->frequentWords[i]->headCategoryPtr;
                while (currentCategoryPtrV2 != NULL) {
                    // Checking word's category is the same as current frequent words array index.
                    if (strcoll(currentCategoryPtrV2->category,currentCategoryPtrV1->category) == 0)
                        break;
                    currentCategoryPtrV2=currentCategoryPtrV2->nextCategoryPtr;
                }

                categoryPtr currentCategoryPtrV3 = currentCategoryPtrV1->frequentWords[i+1]->headCategoryPtr;
                while (currentCategoryPtrV3 != NULL) {
                    // Checking word's category is the same as current frequent words array index.
                    if (strcoll(currentCategoryPtrV3->category,currentCategoryPtrV1->category) == 0)
                        break;
                    currentCategoryPtrV3=currentCategoryPtrV3->nextCategoryPtr;
                }

                // Checking two index it is sorted or not.
                if (currentCategoryPtrV3->occurrence * log((float)currentCategoryPtrV1->documentCounter / currentCategoryPtrV3->occurrence) >
                    currentCategoryPtrV2->occurrence * log((float)currentCategoryPtrV1->documentCounter / currentCategoryPtrV2->occurrence)) {
                    wordStructPtr tempPtr = currentCategoryPtrV1->frequentWords[i];
                    currentCategoryPtrV1->frequentWords[i] = currentCategoryPtrV1->frequentWords[i+1];
                    currentCategoryPtrV1->frequentWords[i+1] = tempPtr;
                    keepSorting = 1 ; break;
                }
            }
        }
        currentCategoryPtrV1 = currentCategoryPtrV1->nextCategoryPtr;
    }
}

void printFrequentWords(categoryPtr generalCategories) { // Prints frequent words formatted as table.
    categoryPtr currentCategoryPtrV1 = generalCategories;

    printf("\n");
    while (currentCategoryPtrV1 != NULL) { // Printing all category's information.
        printf("|%-15s %16s| " ,currentCategoryPtrV1->category , "tf");
        currentCategoryPtrV1 = currentCategoryPtrV1->nextCategoryPtr;
    }
    printf("\n");
    currentCategoryPtrV1 =generalCategories;
    while (currentCategoryPtrV1 != NULL) { // Printing dashes below all category's information.
        printf("|--------------------------------| ");
        currentCategoryPtrV1 = currentCategoryPtrV1->nextCategoryPtr;
    }
    printf("\n");

    for (int i = 0; i < 5 ; i++) { // Printing all indexes.
        currentCategoryPtrV1 = generalCategories;
        while (currentCategoryPtrV1 != NULL) {
            categoryPtr currentCategoryPtrV2 = currentCategoryPtrV1->frequentWords[i]->headCategoryPtr;
            while (currentCategoryPtrV2 != NULL) { // Finding word's matching category.
                if (strcoll(currentCategoryPtrV1->category , currentCategoryPtrV2->category) == 0)
                    break;
                currentCategoryPtrV2 = currentCategoryPtrV2->nextCategoryPtr;
            }

            // Printing word's information.
            printf("|%-15s %16d| " , currentCategoryPtrV1->frequentWords[i]->word ,currentCategoryPtrV2->occurrence);
            currentCategoryPtrV1 = currentCategoryPtrV1->nextCategoryPtr;
        }
        printf("\n");
    }

    sortFrequentWordsByIDF(generalCategories);

    currentCategoryPtrV1 = generalCategories;
    printf("\n");
    while (currentCategoryPtrV1 != NULL) { // Printing all category's information.
        printf("|%-15s %8s %7s| " ,currentCategoryPtrV1->category , "tf", "tf*idf");
        currentCategoryPtrV1 = currentCategoryPtrV1->nextCategoryPtr;
    }
    printf("\n");
    currentCategoryPtrV1 =generalCategories;
    while (currentCategoryPtrV1 != NULL) { // Printing dashes below all category's information.
        printf("|--------------------------------| ");
        currentCategoryPtrV1 = currentCategoryPtrV1->nextCategoryPtr;
    }
    printf("\n");

    for (int i = 0; i < 5 ; i++) { // Printing all indexes.
        currentCategoryPtrV1 = generalCategories;
        while (currentCategoryPtrV1 != NULL) {
            categoryPtr currentCategoryPtrV2 = currentCategoryPtrV1->frequentWords[i]->headCategoryPtr;
            while (currentCategoryPtrV2 != NULL) { // Finding word's matching category.
                if (strcoll(currentCategoryPtrV1->category , currentCategoryPtrV2->category) == 0)
                    break;
                currentCategoryPtrV2 = currentCategoryPtrV2->nextCategoryPtr;
            }

            // Printing word's information.
            printf("|%-15s %8d %7.2f| " , currentCategoryPtrV1->frequentWords[i]->word ,currentCategoryPtrV2->occurrence,
                    currentCategoryPtrV2->occurrence * log((float)currentCategoryPtrV1->documentCounter / currentCategoryPtrV2->occurrence));
            currentCategoryPtrV1 = currentCategoryPtrV1->nextCategoryPtr;
        }
        printf("\n");
    }
}