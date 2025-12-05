#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#define MAX_TOKENS 1000000       // Limit for word while processing analysation
#define MAX_DICT 2000            // Limit for toxic words (toxic_dictionary.txt)
#define MAX_WORD 200             // Limit charecter per a single word 
#define MAX_LINE 300             // Limit charecter per line in dictionary
#define MAX_STOPWORDS 200        // Limit for stop word (stopwords.txt)
#define MAX_UNIQUE_WORDS 100000  // Limit for tracking unique vocabulary 
#define MAX_SENTENCES 100000     // Limit for tracking sentence details

//toxic word (dictionary)
typedef struct {
    char word[MAX_WORD];
    char category[MAX_WORD];
    int severity;
    int count;
} ToxicWord;

//multifile (multiFiles)
typedef struct {
    char filename[200];
    int counts[MAX_DICT]; // count for analyzed toxic words, sort with id
    int totalWords;
} MultiResult;

//sortResult
typedef struct {
    int dictIndex;   // index to dictionary[]
    int severity;
    int count;
} SortEntry;

//Bar Chart
typedef struct {
    int index;
    int count;
} CountEntry;

//we limit maximun file = 3
MultiResult multiFiles[3];
int multiCount = 0;

//Max toxic word = Max capacity of dictionary
//Use dictionary to represent the structure of ToxicWord
ToxicWord dictionary[MAX_DICT];
int dictCount = 0;

//Max stop word = [MAX_STOPWORDS] in the length of[MAX_WORD]
char stopWords[MAX_STOPWORDS][MAX_WORD];
int stopWordCount = 0;

//Function use to replace symbol/number to alphebert 
//(to increase detect rate against words like sh1t, b*tch...)
char replaceChar(char c) {
    c = tolower((unsigned char)c);

    //while certain symbol below is detected, AnalayzeFile pass those symbol(char) here, return the replaced character afterward
    switch (c) {
        case '0': return 'o';
        case '1': return 'i';
        case '2': return 'z';
        case '3': return 'e';
        case '4': return 'a';
        case '5': return 's';
        case '6': return 'g';
        case '7': return 't';
        case '8': return 'b';
        case '9': return 'g';
        case '@': return 'a';
        case '$': return 's';
        case '!': return 'i';
        case '%': return 'o';
        case '#': return 'h'; 
        case '*': return 'i'; 
        default:  return c;
    }
}

// This function remove punctuation + do lowercase to letter 
void cleanWord(char *word) {
    char temp[MAX_WORD];
     // i = word's length and j is the character after clean 
    int i, j = 0;
    //word[i] limits reading , j < MAX_WORD - 1 limits writing
    for (i = 0; word[i] && j < MAX_WORD - 1; i++) {

        //unsigned to strict char value between 0 to 255
        unsigned char c = (unsigned char)word[i];

        if (isalpha(c)) {
            temp[j++] = tolower(c);
        }
        else {
            // call function to replace
            char replaced = replaceChar(c);

            // check is it alphabert, only keep alphabert
            if (isalpha((unsigned char)replaced)) {
                temp[j++] = replaced;
            }
            // every symbol, either replace with helper function, or remove totally
            
        }
    }

    // null terminator at last
    temp[j] = '\0';
    strcpy(word, temp);
}


// function to remove duplicate charecter
void normalizeWord(char *word) {
    char temp[MAX_WORD];
    int i, j = 0;
    int repeatCount = 1;

    if (word[0] == '\0') return;

    temp[j++] = word[0];

    for (i = 1; word[i] != '\0' && j < MAX_WORD - 1; i++) {

        // false if char is not repeating
        if (word[i] == word[i - 1]) {
            repeatCount++;
            
            // current char = last char, still keep it if only repeat once
            if (repeatCount == 2 && j < MAX_WORD - 1) {
                temp[j++] = word[i];
            }
            
            // current char = last two char, remain only 1 char, overwrite last one and not adding any repitive one
            else if (repeatCount == 3 && j > 0) {
                j--; 
            }
        } 
        else {
            repeatCount = 1;
            if (j < MAX_WORD - 1)
                temp[j++] = word[i];
        }
    }

    temp[j] = '\0';
    strcpy(word, temp);
}

// Load Toxic Dictionary (Default while running program) (Menu option 8 to reload)
void loadDictionary() {
    FILE *fp = fopen("dictionary_source/toxic_dictionary.txt", "r");
    dictCount = 0;
    // checking if file exists or not
    if (!fp) {
        printf("Warning: toxic_dictionary.txt not found in the folder dictionary_source. Processing without toxic_dictionary. \n");
        return;
    }


    char line[MAX_LINE];

    // Loop through the file, reading one line (\n) at a time until End Of File(EOF)
    while (fgets(line, sizeof(line), fp)) {
        
        // If it exceed array size stop laoding to prevent crash
        if (dictCount >= MAX_DICT) {
            printf("Dictionary limit reached (%d entries)\n", MAX_DICT);
            break;
        }

        // Use sscanf to split the line into word, category, and severity, and store them into the dictionary entry
        if (sscanf(line, " %[^|]| %[^|]| %d",
                   //Note from lab: Strings already provide an address (pointer), but integers no. So integers need & and strings don’t
                   dictionary[dictCount].word,
                   dictionary[dictCount].category,
                   &dictionary[dictCount].severity) == 3) {
            
            //Initialize the counter for this toxic word to be 0 and increment counter
            dictionary[dictCount].count = 0;
            dictCount++;
        }
    }

    fclose(fp);

    printf("Toxic dictionary loaded (%d entries).\n", dictCount);
}

// Load Stop Word (Default while running program) (Menu option 8 to reload)
void loadStopWords() {
    FILE *fp = fopen("dictionary_source/stopwords.txt", "r");
    stopWordCount = 0;
    
    if (!fp) {
        printf("Warning: stopwords.txt not found in the folder dictionary_source. Processing without filters.\n");
        return;
    }

    char line[MAX_WORD];
    while (fscanf(fp, "%199s", line) == 1) {
        if (stopWordCount < MAX_STOPWORDS) {
            strcpy(stopWords[stopWordCount], line);
            stopWordCount++;
        }
    }
    fclose(fp);
    printf("Stop words loaded (%d entries).\n", stopWordCount);
}

// function call from "analyzeTxt" to check whether a word is stop word that match in out stopwords.txt
int isStopWord(const char *word) {

    for (int i = 0; i < stopWordCount; i++) {
        if (strcmp(word, stopWords[i]) == 0) {
            return 1; // It is a stop word
        }
    }
    return 0; // It is not a stop word
}

// function call from "analyzeFile" and "analyzeComplexity" to check csv input
int isCsv(const char *filename) {
    int len = strlen(filename);

    // if last 4 chracter = .csv
    return (strcasecmp(filename + (len - 4), ".csv") == 0);
}

// function call from "analyzeFile" and "analyzeComplexity" to convert from .csv to .txt
int convertCsvToTxt(const char *csvFile, const char *txtFile, int column, int hasHeader) {
    FILE *csvin = fopen(csvFile, "r");
    if (!csvin) {
        printf("Could not open CSV file.\n");
        return 0;
    }

    FILE *txtout = fopen(txtFile, "w");
    if (!txtout) {
        printf("Could not create temp TXT file.\n");
        fclose(csvin);
        return 0;
    }

    //buffer and counter
    char line[50000];
    int currentLine = 0;

    // read line by line until EOF
    while (fgets(line, sizeof(line), csvin)) {

        currentLine++;

        // Skip header base on user input
        if (currentLine == 1 && hasHeader)
            continue;

        // CSV format is like a,b,c (comma seperated)
        // The logic here is to use strtok, and base on user input, dynamically choose the column we want to be written into txt file
        int columnIndex = 1;
        char *token = strtok(line, ",");

        while (token != NULL) {
            if (columnIndex == column) {
                fprintf(txtout, "%s\n", token);
                break;
            }
            // increase and do strtok to move until "column" (user input)
            columnIndex++;
            token = strtok(NULL, ",");
        }
    }

    fclose(csvin);
    fclose(txtout);
    return 1;
}

// Function call "analyzeFile" after save text in token, analyse is it in toxic dictionary
int analyzeTxt(char *filename) {

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("[ERROR] File not found in analyze_source folder.\n");
        printf("Please enter a valid .txt or .csv file.\n");
        return 1;
    }

    // Malloc for toxic words
    char (*Dict)[MAX_WORD] = malloc(dictCount * sizeof(*Dict));
    if (!Dict) {
        printf("Memory allocation failed for Dict.\n");
        fclose(fp);
        return 1;
    }

    // Malloc for tokens
    char (*tokens)[MAX_WORD] = malloc(MAX_TOKENS * sizeof(*tokens));
    if (!tokens) {
        printf("Memory allocation failed for tokens.\n");
        free(Dict);
        fclose(fp);
        return 1;
    }

    // call both "cleanWord" and "normalizeWord" to clean and normalize word in dictionary, then save into Dict[i]
    for (int i = 0; i < dictCount; i++) {
        strcpy(Dict[i], dictionary[i].word);
        cleanWord(Dict[i]);
        normalizeWord(Dict[i]);
        //initial count = 0
        dictionary[i].count = 0;
    }

    // Counter and buffer
    int tokenCount = 0;
    char raw[MAX_WORD];

    // Error handling if token needed > max token
    while (fscanf(fp, "%199s", raw) == 1) {
        if (tokenCount >= MAX_TOKENS) {
            printf("Warning: too many tokens needed, program can't handle\n");
            printf("Please try smaller folder to analyse.\n");
            free(Dict);
            break;
        }

        // clean and normalize raw text
        cleanWord(raw);
        normalizeWord(raw);

        // continue if the return value of raw is like nothing
        if (strlen(raw) == 0)
            continue;

        // copy from raw to tokens for further analization
        strcpy(tokens[tokenCount], raw);
        tokenCount++;
    }

    fclose(fp);

    //biagram and trigram detector
    int i = 0;
    while (i < tokenCount) {

        int consumed = 0;

        //for trigram and biagram, we didnt call "isStopWord", so something like "fuck you"
        //will still be recorded, even "you" is in our stopword dictionary

        if (i + 2 < tokenCount) {
            char trigram[MAX_WORD];
            //snprintf(buffer, buffer size, "format", value);
            snprintf(trigram, sizeof(trigram), "%s %s %s",tokens[i], tokens[i+1], tokens[i+2]);

            //check for trigram
            for (int d = 0; d < dictCount; d++) {
                if (strcmp(Dict[d], trigram) == 0) {
                    // count for this trigram ++
                    dictionary[d].count++;
                    consumed = 3;
                    break;
                }
            }
        }
        
        //if trigram is checked, skip this biagram
        if (consumed == 0 && i + 1 < tokenCount) {
            char bigram[MAX_WORD];
            //snprintf(buffer, buffer size, "format", value);
            snprintf(bigram, sizeof(bigram), "%s %s",tokens[i], tokens[i+1]);

            //check for biagram
            for (int d = 0; d < dictCount; d++) {
                if (strcmp(Dict[d], bigram) == 0) {
                    // count for this biagram ++
                    dictionary[d].count++;
                    consumed = 2;
                    break;
                }
            }
        }

        //if trigram or biagram checked, skip single word
        if (consumed == 0) {
            //check is it a stop word
            if (!isStopWord(tokens[i])) {
                for (int d = 0; d < dictCount; d++) {
                    if (strcmp(Dict[d], tokens[i]) == 0) {
                        // count for this toxic word ++
                        dictionary[d].count++;
                    }
                }
            }
            consumed = 1;
        }

        i += consumed;
    }

    free(tokens);
    free(Dict);

    printf("Analysis (max-match) completed.\n");
    return tokenCount;
}

// Analyze file (call from MtoONE)
int analyzeFile(char *filename) {

    // Check if it's a Csv
    if (isCsv(filename)) {

        int csvIndex = 1;

        int column;
        int hasHeader;

        // ask the column from user
        printf("CSV detected. Enter the column number containing the text: ");
        scanf("%d", &column);

        // ask whether is there a header from user
        printf("Does the CSV have a header row? (1=yes, 0=no): ");
        scanf("%d", &hasHeader);

        // open a temp file for csv to convert in
        char tempFile[100];
        sprintf(tempFile, "analyze_source/temp_csv_extract_%d.txt", csvIndex);
        csvIndex++; 

        if (!convertCsvToTxt(filename, tempFile, column, hasHeader)) {
            printf("CSV conversion failed. Please use txt file instead\n");
            return 0;
        }

        // now analyze the extracted txt after convert from csv to txt
        return analyzeTxt(tempFile);
    }

    // if not csv, its a txt, analyze the text
    return analyzeTxt(filename);
}

// Analyze file (Option 1 in menu)
void MtoONE() {

    multiCount = 0; // reset

    int fileCount;
    printf("How many files to analyze? (1 - 3): ");

    //error handling for invalid value type or out of range
    while (1) {
    if (scanf("%d", &fileCount) == 1 && fileCount >= 1 && fileCount <= 3) {
        break;  
    }
    printf("Invalid input. Please enter an integer between 1 and 3.\n");
    printf("How many files to analyze? (1 - 3): ");
    while (getchar() != '\n');
    }

    for (int i = 1; i <= fileCount; i++) {
        // buffer for file name
        char filename[200];
        int totalWords = 0;

        // getting file name from user input
        while (1) {
            printf("Enter file %d name: ", i);
            scanf("%199s", filename);

            // Build full path
            // buffer for file name including folder path
            char fullPath[300];
            sprintf(fullPath, "analyze_source/%s", filename);

            printf("Analyzing %s...\n", fullPath);

            // analyzeFile return token count
            totalWords = analyzeFile(fullPath);

            if (totalWords > 1) {
                // Save the full path as the filename used
                strcpy(multiFiles[multiCount].filename, fullPath);
                break;
            } else if (totalWords == 0) {
                printf("File is empty or contains no valid words. Please enter another file.\n");
            }
    }


        // Save info for 1 file after successful analyze
        strcpy(multiFiles[multiCount].filename, filename);

        // count total words after cleaning and normalize (token count)
        multiFiles[multiCount].totalWords = totalWords;

        // Save the count for analyzed toxic words
        for (int d = 0; d < dictCount; d++) {
            multiFiles[multiCount].counts[d] = dictionary[d].count;
        }
        multiCount++;
    }

    printf("\n>> Finished analyzing %d files.\n", multiCount);
    if(fileCount == multiCount){
        printf(">> Choose menu option 3 to save results as TXT or option 4 as CSV.\n");
        printf(">> Choose menu option 5 to show ASCII bar chart or option 6 for advance statistic");
    }
}

// function call from "saveReportTXT" and "saveReportCSV" to sort analysis result. With the sequence priority of severity, then count
// SortEntry: arr[i] = {dictIndex=5, severity=3, count=12} , passed from function with variable "list"
void sortResults(SortEntry arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {

            // Sort by severity first, then count
            if (arr[j].severity > arr[i].severity ||(arr[j].severity == arr[i].severity &&arr[j].count > arr[i].count)) {
                //same struct type to swap two struct elements
                SortEntry tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    }
}

void saveToxicRatioReport() {

    // error handling and friendly user indication
    if (multiCount == 0) {
        printf("No analyzed data available. Run option 1 first.\n");
        return;
    }
    FILE *fp = fopen("analyze_result/toxic_ratio_report.txt", "w");
    if (!fp) {
        printf("Error creating toxic_ratio_report.txt at the folder analyze_result\n");
        return;
    }

    fprintf(fp, "=========== TOXIC RATIO REPORT ===========\n");
    fprintf(fp, "Formula Using : ToxicityIndex = (boostedScore * 100) / totalWords \n\n");

    // variable use for comparison
    float highestRatio = 0;
    float lowestRatio = 99999;
    int highestIndex = 0;
    int lowestIndex = 0;

    // analyse file by file
    for (int f = 0; f < multiCount; f++) {

        int boostedScore = 0;
        int totalWords = multiFiles[f].totalWords;
        int totalToxic = 0;

        for (int d = 0; d < dictCount; d++) {
            //toxic score count
            boostedScore += multiFiles[f].counts[d] *dictionary[d].severity * dictionary[d].severity;
            //toxic word count
            totalToxic += multiFiles[f].counts[d];
        }

        // with a weighted 100 formula, similar to Coleman-Liau index. (inspired by cs50 2025, week2, readability)
        float toxicRatio = (float)boostedScore * 100.0f / (float)totalWords;
        float ToxicPercent = (float)totalToxic/(float)totalWords *100;

        fprintf(fp, "FILE: %s\n", multiFiles[f].filename);
        fprintf(fp, "Total Words (cleaned)  : %d\n", totalWords);
        fprintf(fp, "Total Words (toxic)    : %d\n", totalToxic);
        fprintf(fp, "Toxic Words percentage : %.2f%%\n", ToxicPercent);
        fprintf(fp, "Boosted Toxic Score    : %d\n", boostedScore);
        fprintf(fp, "Toxicity Ratio         : %.2f\n\n", toxicRatio);

        // conclusion for toxic ratio per file
        if (toxicRatio < 10) {
            fprintf(fp, "Conclusion:Toxic Ratio < 10: The file is generally non-toxic.\n           It may contain isolated toxic words, but they are either mild or contextually harmless.\n\n");
        }
        else if (toxicRatio < 20) {
            fprintf(fp, "Conclusion: Toxic Ratio between 10-20: The file contains noticeable toxic elements.\n            Either low-severity toxic words are used more frequently, or a few high-severity words appear.\n\n");
        }
        else if (toxicRatio < 30) {
            fprintf(fp, "Conclusion: Toxic Ratio between 20-30: The file shows moderate toxicity.\n            Toxic language appears regularly, and its usage affects the overall tone of the content.\n\n");
        }
        else if (toxicRatio < 40) {
            fprintf(fp, "Conclusion: Toxic Ratio between 30-40: The file is highly toxic.\n            Strong or repeated toxic words dominate parts of the content, significantly impacting readability and appropriateness.\n\n");
        }
        else if (toxicRatio < 50) {
            fprintf(fp, "Conclusion: Toxic Ratio between 40-50: The file is very toxic.\n            High-severity expressions occur frequently, and the overall message is aggressive, harmful, or hostile.\n\n");
        }
        else if (toxicRatio >= 50) {
            fprintf(fp, "Conclusion: Toxic Ratio >= 50: The file is extremely toxic.\n            Severe toxic language is used excessively, indicating highly abusive, threatening, or hateful content.\n\n");
        }

        // this section is data for comparison
        if (toxicRatio > highestRatio) {
            highestRatio = toxicRatio;
            highestIndex = f;
        }
        if (toxicRatio < lowestRatio) {
            lowestRatio = toxicRatio;
            lowestIndex = f;
        }
    }
        // this section is output of comparison and only happend if there is more than 1 file
    if (multiCount > 1){
        fprintf(fp, "===========================================\n");
        fprintf(fp, "        TOXICITY COMPARISON SUMMARY\n");
        fprintf(fp, "===========================================\n\n");

        fprintf(fp, "Most Toxic File:\n");
        fprintf(fp, "  File Name      : %s\n", multiFiles[highestIndex].filename);
        fprintf(fp, "  Toxicity Ratio : %.2f\n\n", highestRatio);

        fprintf(fp, "Least Toxic File:\n");
        fprintf(fp, "  File Name      : %s\n", multiFiles[lowestIndex].filename);
        fprintf(fp, "  Toxicity Ratio : %.2f\n\n", lowestRatio);

        // we want to display conclusion here between the two output file, but we feel like
        // there is already conclusion up there for certain file, so we didn't put it here
    }

    fclose(fp);
    printf(">> Toxic ratio report saved to analyze_result/toxic_ratio_report.txt\n");
}

// Save report as txt after analysis (Option 3 in menu)
void saveReportTXT() {

    // error handling and friendly user indication
    if (multiCount == 0) {
        printf("[ERROR] No analysis available. Run option 1 first.\n");
        return;
    }
    FILE *fp = fopen("analyze_result/analysis_report.txt", "w");
    if (!fp) {
        printf("Error creating analysis_report.txt at the folder analyze_result\n");
        return;
    }

    // output file one by one
    for (int f = 0; f < multiCount; f++) {

        // format for report
        fprintf(fp, "===== ANALYSIS REPORT FOR: %s =====\n",
                multiFiles[f].filename);

        fprintf(fp, "%-25s %-20s %-10s %-10s\n",
                "Phrase/Word", "Category", "Severity", "Count");
        fprintf(fp, "-------------------------------------------------------------\n");

        // build list base on result
        SortEntry list[MAX_DICT];
        // total toxic word for current file
        int used = 0;

        for (int d = 0; d < dictCount; d++) {
            int c = multiFiles[f].counts[d];
            // only list recorded toxic word, they are data that pass to function "sortResults"
            if (c > 0) {
                list[used].dictIndex = d;
                list[used].severity = dictionary[d].severity;
                list[used].count = c;
                used++;
            }
        }

        // call "sortResults" to sort result in sequence of severity, then count
        sortResults(list, used);

        // print sorted result
        for (int i = 0; i < used; i++) {
            int d = list[i].dictIndex;

            fprintf(fp, "%-25s %-20s %-10d %-10d\n",
                    // base on index for dictionary to get the word,category and severity 
                    dictionary[d].word,
                    dictionary[d].category,
                    dictionary[d].severity,
                    // we have save counts from analyzeTxt to MtoONE
                    multiFiles[f].counts[d]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("Saved ALL results to analyze_result/analysis_report.txt\n");
}

// Save report as csv after analysis (Option 4 in menu)
void saveReportCSV() {

    // error handling and friendly user indication
    if (multiCount == 0) {
        printf("[ERROR] No analysis available. Run option 1 first.\n");
        return;
    }
    FILE *fp = fopen("analyze_result/analysis_report.csv", "w");
    if (!fp) {
        printf("Error creating analysis_report.csv at the folder analyze_result\n");
        return;
    }

    // header for .csv report
    fprintf(fp, "File,Phrase,Category,Severity,Count\n");

    for (int f = 0; f < multiCount; f++) {

        // build list base on result
        SortEntry list[MAX_DICT];
        // total toxic word for current file
        int used = 0;

        for (int d = 0; d < dictCount; d++) {
            int c = multiFiles[f].counts[d];
            // only list recorded toxic word, they are data that pass to function "sortResults"
            if (c > 0) {
                list[used].dictIndex = d;
                list[used].severity = dictionary[d].severity;
                list[used].count = c;
                used++;
            }
        }

        // call "sortResults" to sort result in sequence of severity, then count
        sortResults(list, used);

        for (int i = 0; i < used; i++) {
            int d = list[i].dictIndex;

            //format for .csv a,b,c,d...
            fprintf(fp, "%s,%s,%s,%d,%d\n",
                    multiFiles[f].filename,
                    dictionary[d].word,
                    dictionary[d].category,
                    dictionary[d].severity,
                    // we have save counts from analyzeTxt to MtoONE
                    multiFiles[f].counts[d]);
        }
    }

    fclose(fp);
    printf("Saved ALL results to analyze_result/analysis_report.csv\n");
}


// print ASCII Bar Chart for top 20 toxic words (Option 5 in menu)
void showBarChart() {

    // Error handling and friendly user guidance
    if (multiCount == 0) {
        printf("\n[ERROR] No analysis available. Run option 1 first.\n");
        return;
    }

    // Go through analyzed files
    for (int f = 0; f < multiCount; f++) {
        printf("\n_______________________________________________________\n");
        printf(" TOP 20 TOXIC WORDS FOR FILE: %s\n", multiFiles[f].filename);
        printf("_______________________________________________________\n");

        // this structure is to keep track of which word belongs to which count during sorting.
        // typedef struct {
        //    int index;
        //    int count;
        // } CountEntry;

        CountEntry list[MAX_DICT];
        int used = 0;

        // loop through dictionary to find matches
        for (int d = 0; d < dictCount; d++) {
            int c = multiFiles[f].counts[d];
            if (c > 0) {
                // store represent dictionary index and count
                list[used].index = d;
                list[used].count = c;
                used++;
            }
        }

        if (used == 0) {
            printf("(No toxic words found in this file)\n\n");
            continue;
        }

        // limit result to 20
        int limit = used;
        if (limit > 20) {
            limit = 20;
        }
        
        // Sort descending by count
        for (int i = 0; i < limit; i++) {
            for (int j = i + 1; j < used; j++) {
                if (list[j].count > list[i].count) {
                    //same struct type to swap two struct elements.
                    CountEntry tmp = list[i];
                    list[i] = list[j];
                    list[j] = tmp;
                }
            }
        }

        for (int r = 0; r < limit; r++) {

            // retrieve the Dictionary Index and Frequency Count from our sorted list
            int idx = list[r].index;
            int count = list[r].count;

            printf("%2d) %-22s | ", r + 1, dictionary[idx].word);

            for (int j = 0; j < count; j++) {
                // wrap every 50 #
                if (j > 0 && j % 50 == 0) printf("\n%27s", "");

                printf("#");    
            }

            //print total count per toxic word
            printf(" (%d)\n", count); 
        }
        printf("\n");
    }
}

// Bubble sort
void BubbleSort(ToxicWord arr[], int n, int *steps) {
    int i, j;
    // Reset counter for multi file
    *steps = 0; 
    
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {

            // Count the Comparison
            (*steps)++; 

            if (arr[j].count < arr[j + 1].count) {
                ToxicWord temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                
                (*steps)++; // Count the Swap
            }
        }
    }
}

// Recursive mergesort, merge part
// merge(arr, left, mid, right, steps);
void merge(ToxicWord arr[], int left, int mid, int right, int *steps) {
    int i, j, k;
    int lefthalves = mid - left + 1;
    int righthalves = right - mid;

    ToxicWord *L = (ToxicWord *)malloc(lefthalves * sizeof(ToxicWord));
    ToxicWord *R = (ToxicWord *)malloc(righthalves * sizeof(ToxicWord));

    if (L == NULL || R == NULL) { 
        printf("Mem error\n");
        return; 
    }

    // copy raw value to temporary array in "merge"
    for (i = 0; i < lefthalves; i++) {
        L[i] = arr[left + i];
    }
    for (j = 0; j < righthalves; j++) {
        R[j] = arr[mid + 1 + j];
    }


    i = 0; j = 0; k = left;
    while (i < lefthalves && j < righthalves) {
        
        (*steps)++; // Count the Comparison
        
        // If left's value bigger
        if (L[i].count >= R[j].count) {
            arr[k] = L[i];
            i++;
        // If right's value bigger
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < lefthalves)  
    // copy remain left half
    {arr[k] = L[i]; i++; k++;}

    while (j < righthalves) 
    // copy remain right half
    {arr[k] = R[j]; j++; k++;}

    free(L);
    free(R);
}

// Recursive mergesort, main part
// MergeSort(mergeList, 0, used - 1, &mergeSteps);
void MergeSort(ToxicWord arr[], int left, int right, int *steps) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        MergeSort(arr, left, mid, steps);
        MergeSort(arr, mid + 1, right, steps);
        merge(arr, left, mid, right, steps);
    }
}

// function to run bubble and merge sort and output their result (Menu option 6)
void BandMsort() {

    // Error handling and user firendly guidance
    if (multiCount == 0) {
        printf("\n[ERROR] No analyzed files. Run option 1 first.\n");
        return;
    }

    // User input to determine the output size
    int n;
    printf("Enter N (Top results to save per file): ");
    scanf("%d", &n);

    FILE *fp = fopen("analyze_result/algorithm_report.txt", "w");
    // Error handling
    if (fp == NULL) {
        printf("Fail to create algorithm_report.txt at the folder analyze_result\n");
        return;
    }

    fprintf(fp, "================ SORT & ALGORITHM REPORT ================\n\n");

    // analyze file by file
    for (int f = 0; f < multiCount; f++) {

        fprintf(fp, "=========================================================\n");
        fprintf(fp, "SORT RESULTS FOR FILE: %s\n", multiFiles[f].filename);
        fprintf(fp, "=========================================================\n\n");

        // Build list for bubble and merge sort result
        ToxicWord bubbleList[MAX_DICT];
        ToxicWord mergeList[MAX_DICT];

        // "used" is bubble/merge list index, they still use 'd' as index for dictionary's value
        int used = 0;
        // assign value into bubble/merge list array from dictionary
        for (int d = 0; d < dictCount; d++) {
            if (multiFiles[f].counts[d] > 0) {

                bubbleList[used] = dictionary[d];
                bubbleList[used].count = multiFiles[f].counts[d];

                mergeList[used] = dictionary[d];
                mergeList[used].count = multiFiles[f].counts[d];

                used++;
            }
        }

        if (used == 0) {
            fprintf(fp, "No toxic words found in this file.\n\n");
            continue;
        }

        // Section for bubble sort's result
        // count step and time
        int bubbleSteps = 0;
        clock_t start = clock();
        BubbleSort(bubbleList, used, &bubbleSteps);
        clock_t end = clock();
        double bubbleTime = (double)(end - start) / CLOCKS_PER_SEC;

        // print report format
        fprintf(fp, "--- [ALGORITHM 1] BUBBLE SORT RESULTS ---\n");
        fprintf(fp, "Time Taken:   %.6f seconds\n", bubbleTime);
        fprintf(fp, "Total Steps:  %lld\n", bubbleSteps);
        fprintf(fp, "---------------------------------------------------------------\n");
        fprintf(fp, "%-20s %-20s %s\n", "WORD", "CATEGORY", "FREQUENCY");
        fprintf(fp, "---------------------------------------------------------------\n");

        // print value from bubblesort
        // either it's end of list or either it hits N frequency of toxic words
        for (int i = 0; i < used && i < n; i++) {
            fprintf(fp, "%-20s %-20s %d\n",
                    bubbleList[i].word,
                    bubbleList[i].category,
                    bubbleList[i].count);
        }
        fprintf(fp, "\n\n");

        // Section for merge sort's result
        // count step and time
        int mergeSteps = 0;
        start = clock();
        MergeSort(mergeList, 0, used - 1, &mergeSteps);
        end = clock();
        double mergeTime = (double)(end - start) / CLOCKS_PER_SEC;

        // print report format
        fprintf(fp, "--- [ALGORITHM 2] MERGE SORT RESULTS ---\n");
        fprintf(fp, "Time Taken:   %.6f seconds\n", mergeTime);
        fprintf(fp, "Total Steps:  %lld\n", mergeSteps);
        fprintf(fp, "---------------------------------------------------------------\n");
        fprintf(fp, "%-20s %-20s %s\n", "WORD", "CATEGORY", "FREQUENCY");
        fprintf(fp, "---------------------------------------------------------------\n");

        // print value from merge sort
        // either it's end of list or either it hits N frequency of toxic words
        for (int i = 0; i < used && i < n; i++) {
            fprintf(fp, "%-20s %-20s %d\n",
                    mergeList[i].word,
                    mergeList[i].category,
                    mergeList[i].count);
        }

        fprintf(fp, "\n\n");

        // comparison section
        // print comparison format
        fprintf(fp, " [ PERFORMANCE COMPARISON FOR %s ] \n", multiFiles[f].filename);
        fprintf(fp, "---------------------------------------------------------------\n");
        fprintf(fp, "METRIC          | BUBBLE SORT         | MERGE SORT\n");
        fprintf(fp, "---------------------------------------------------------------\n");
        fprintf(fp, "Time Taken      | %-19f | %f\n", bubbleTime, mergeTime);
        fprintf(fp, "Steps           | %-19d | %d\n", bubbleSteps, mergeSteps);
        fprintf(fp, "---------------------------------------------------------------\n\n");

        // conclusion section
        fprintf(fp, "Conclusion: ");
        if (mergeSteps < bubbleSteps)
            fprintf(fp, "Merge Sort is more efficient.\n\n");
        else if (mergeSteps == bubbleSteps)
            fprintf(fp, "Both are equally efficient.\n\n");
        else
            fprintf(fp, "Bubble Sort is unexpectedly more efficient.\n\n");

        fprintf(fp, "---------------------------------------------------------------\n\n");

        // Sort toxic word by categories
        fprintf(fp, "--- TOP 3 TOXIC WORDS PER CATEGORY ---\n\n");

        // unique category
        char printedCats[MAX_DICT][50];
        int catCount = 0;

        for (int i = 0; i < used; i++) {

            char *katagori = mergeList[i].category;

            // Check is this category was already handled
            int already = 0;
            for (int c = 0; c < catCount; c++) {
                // check current category is already printed or not
                if (strcmp(katagori, printedCats[c]) == 0) {
                    already = 1;
                    break;
                }
            }
            // if handled, j
            if (already) continue;

            // Mark category as printed
            strcpy(printedCats[catCount], katagori);
            catCount++;

            // Print category title
            fprintf(fp, "[%s]\n", katagori);

            // Now get top 3 for this category
            // because mergelist is already sorted in descending value, we only need to
            // match the category, find the first 3 of it, they will 3 of the highest count
            int found = 0;
            // j < used loop through all toxic word in list
            // found < 3 limit the discovery to 3 data that match this category
            for (int j = 0; j < used && found < 3; j++) {
                if (strcmp(mergeList[j].category, katagori) == 0) {
                    fprintf(fp, "%d. %-20s %d\n",
                            found + 1,
                            mergeList[j].word,
                            mergeList[j].count);
                    found++;
                }
            }
            fprintf(fp, "\n");
        }
    }
    fprintf(fp, "---------------------------------------------------------------\n\n");
    fclose(fp);

    printf("\n>> SUCCESS: Sorted ALL analyzed files.\n");
    printf(">> Results saved to analyze_result/algorithm_report.txt\n");
}

// 
void analyzeComplexity(char *filename, FILE *out) {

    // check is input a .csv file
    if (isCsv(filename)) {

        // separate counter, different from analyze function
        int csvIndex2 = 1;   

        int column;
        int hasHeader;

        printf("CSV detected for complexity analyzer.\n");
        printf("Enter the column number containing the text: ");
        scanf("%d", &column);

        printf("Does the CSV have a header row? (1=yes, 0=no): ");
        scanf("%d", &hasHeader);

        char tempFile[200];
        sprintf(tempFile, "analyze_source/temp_complexity_csv_%d.txt", csvIndex2++);

        if (!convertCsvToTxt(filename, tempFile, column, hasHeader)) {
            fprintf(out, "CSV conversion failed for file: %s\n\n", filename);
            return;
        }

        // now analyze the extracted txt after convert from csv to txt
        analyzeComplexity(tempFile, out);
        return;
    }

    // Errror handling
    FILE *fp = fopen(filename, "r");
    if (!fp) { 
        fprintf(out, "File not found: %s\n\n", filename);
        return; 
    }

    char raw[MAX_WORD];
    char clean[MAX_WORD];

    static char uniqueWords[MAX_UNIQUE_WORDS][MAX_WORD];
    int uniqueCount = 0;

    int sentenceLengths[MAX_SENTENCES];
    int sentenceCount = 0;
    int currentSentLen = 0;
    int totalTokens = 0;

    // average word length's counter
    int totalChars = 0;  

    // scan word by eord
    while (fscanf(fp, "%199s", raw) == 1) {

        strcpy(clean, raw);
        // clean and normalize word
        cleanWord(clean);
        normalizeWord(clean);

        if (strlen(clean) > 0) {
            totalTokens++;
            currentSentLen++;

            totalChars += strlen(clean);

            int found = 0;

            // if the word is already in uniqueCount, its not unique anymore , uniqueCount do no increment
            // its like if you found your "unique one" is cheating with others, then she is not unique anymore
            // but you still has her count as one of your past, because the essence even if vanish, it's still belongs to your history and memory
            // here'something philosophy for your day, hope you won't feel tired for marking that much of stuff :)

            for (int i = 0; i < uniqueCount; i++) {
                // loop through every unique word, see is there any one that is same
                if (strcmp(uniqueWords[i], clean) == 0) {
                    found = 1; break;
                }
            }

                // if there isnt any, mark it as unique one
            if (!found) {
                strcpy(uniqueWords[uniqueCount], clean);
                uniqueCount++;
            }
        }

        // we need to use raw value here, cause clean value already remove symbol with "cleanWord" function
        int len = strlen(raw);
        if (len > 0) {
            char last = raw[len - 1];
            if (last == '.' || last == '!' || last == '?') {
                sentenceLengths[sentenceCount++] = currentSentLen;
                // reset sentence length and start another sentence
                currentSentLen = 0;
            }
        }
    }

    fclose(fp);

    if (currentSentLen > 0)
        sentenceLengths[sentenceCount++] = currentSentLen;

    // to prevent calculation divide 0
    if (totalTokens == 0) totalTokens = 1;
    if (sentenceCount == 0) sentenceCount = 1;

    // calculation with formula
    float lexicalDiversity = (float)uniqueCount / totalTokens;
    float avgSentenceLength = (float)totalTokens / sentenceCount;
    float avgWordLength = (float)totalChars / totalTokens;

    int minLen = 100000, maxLen = 0;
    for (int i = 0; i < sentenceCount; i++) {
        if (sentenceLengths[i] < minLen) minLen = sentenceLengths[i];
        if (sentenceLengths[i] > maxLen) maxLen = sentenceLengths[i];
    }

    if (minLen == 0) minLen = 1;

    fprintf(out, "===== COMPLEXITY REPORT FOR: %s =====\n", filename);
    fprintf(out, "Total Words:          %d\n", totalTokens);
    fprintf(out, "Unique Words:         %d\n", uniqueCount);
    fprintf(out, "Total Sentences:      %d\n", sentenceCount);
    fprintf(out, "Lexical Diversity:    %.3f\n", lexicalDiversity);
    fprintf(out, "Avg Word Length:      %.2f characters\n", avgWordLength);
    fprintf(out, "Avg Sentence Length:  %.2f words\n", avgSentenceLength);
    fprintf(out, "Max Sentence Length:  %d\n", maxLen);
    fprintf(out, "Min Sentence Length:  %d\n", minLen);

    fprintf(out, "\n--------------------------------------\n\n");
}

// Analyze lexical diversity (Option 7 in menu)
void MtoONE2() {
    int fileCount;

    printf("How many files do you want to analyze? (1 - 3): ");

    //error handling for invalid value type or out of range
    while (1) {
        if (scanf("%d", &fileCount) == 1 && fileCount >= 1 && fileCount <= 3) {
            break;
        }
        printf("Invalid input. Please enter an integer between 1 to 3.\n");
        printf("How many files to analyze? (1 - 3): ");
        while (getchar() != '\n');
    }

    // error handling
    FILE *out = fopen("analyze_result/complexity_report.txt", "w");
    if (!out) {
        printf("Failed to create complexity_report.txt at the folder analyze_result\n");
        return;
    }

    // analyze file one by one
    for (int i = 1; i <= fileCount; i++) {

        // buffer for file name and buffer for file name including folder path
        char filename[200];
        char fullPath[300];

        while (1) {
            printf("Enter file %d name: ", i);
            scanf("%199s", filename);

            // Build full path to analyze_source/
            sprintf(fullPath, "analyze_source/%s", filename);

            // Error handling
            FILE *fp = fopen(fullPath, "r");
            if (!fp) {
                printf("[ERROR] File not found in analyze_source folder.\n");
                printf("Please enter a valid .txt or .csv file.\n");
                continue;
            }
            fclose(fp);
            break;
        }

        analyzeComplexity(fullPath, out);
    }

    fclose(out);
    printf("\n>> Saved ALL results to analyze_result/complexity_report.txt\n");
}


// Expand Toxic Dictionary (Option 9 in menu)
void expandDictionary() {
    char input[256];
    char word[100], category[100];
    int severity;

    // UI
    printf("\n=== EXPAND TOXIC DICTIONARY ===\n");
    printf("Format: word|category|severity (severity 1-5)\n");
    printf("Example: idiot|insult|3\n");
    printf("Enter 'exit' to return to menu.\n");

    while (1) {
        printf("\nEnter new entry: ");
        // limit it to 255 character (user inpput), and read until '\n'
        scanf(" %255[^\n]", input);   // 

        // Way to Exit 
        if (strcasecmp(input, "exit") == 0) {
            printf("Returning to menu...\n");
            return;
        }

        // format: word|category|severity
        // use strtok to differentiate them
        char *p1 = strtok(input, "|");
        char *p2 = strtok(NULL, "|");
        char *p3 = strtok(NULL, "|");

        // error handling, if: invalid format
        if (!p1 || !p2 || !p3) {
            printf("[ERROR] Invalid format! Use: word|category|severity\n");
            continue;
        }

        // copy 3 parts and put into variables
        strcpy(word, p1);
        strcpy(category, p2);

        //  checking value from p3, if it not an integer or not between 1 to 5, error
        if (sscanf(p3, "%d", &severity) != 1 || severity < 1 || severity > 5) {
            printf("[ERROR] Severity must be an integer from 1 to 5.\n");
            continue;
        }
        
        // append mode to write the new toxic words in
        FILE *fp = fopen("dictionary_source/toxic_dictionary.txt", "a");
        // error handling
        if (!fp) {
            printf("[ERROR] Could not open toxic_dictionary.txt for writing in the folder dictionary_source.\n");
            return;
        }

        fprintf(fp, "%s|%s|%d\n", word, category, severity);
        fclose(fp);

        printf("[SUCCESS] Added: %s | %s | %d\n", word, category, severity);
        printf("Enter 'exit' to return to menu.\n");
    }
}

// Menu (Frindly User Interface)
void menu() {
    int choice;

    do {
        printf("\n====== COMP1028 GROUP WEJAR LAB COURSEWORK ======\n");
        printf("1. Analyze text or csv file (support multi-file)\n");
        printf("2. Output toxic ratio OR compare toxic ratio (when file > 1)\n");
        printf("3. Save last analysis to .txt file\n");         
        printf("4. Save analysis to .csv file\n");               
        printf("5. Show ASCII bar chart with top 20 toxic words\n");
        printf("6. Sort top N frequency toxic words with category(Bubble & Merge Sort)\n");
        printf("7. Analyze lexical diversity (support multi-file)\n");
        printf("8. Reload dictionaries (Toxic & Stop words)\n"); 
        printf("9. Expand toxic words dictionary\n");                       
        printf("10. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch(choice) {

            case 1: 
                MtoONE();               // Analyze File
                break;

            case 2:
                saveToxicRatioReport(); // Toxic ratio report
                break;

            case 3:  
                saveReportTXT();        // Save as .txt
                break;

            case 4:  
                saveReportCSV();        // Save as .csv
                break;

            case 5:  
                showBarChart();         // Show top 20 ASCII bar chart
                break;

            case 6:   
                BandMsort();            // Bubble and Merge Sort advance statistic
                break;

            case 7:   
                MtoONE2();              // Lexical Diversity
                break;


            case 8:                     // Reload toxic dictionary and stopwords
                loadDictionary();
                loadStopWords();
                break;

            case 9:
                expandDictionary();     // Expand toxic dictionary (user input)
                break;

            case 10:
                printf("Exiting...\n"); // Exit program
                break;

            default:
                printf("Invalid choice!\n");
        }

    } while(choice != 10);
}

// Main
int main() {
    loadDictionary();
    loadStopWords();
    menu();
    return 0;
}
