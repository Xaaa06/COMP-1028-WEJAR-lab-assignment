Cyberbullying/Toxic Text Analyzer – COMP1028 Coursework

This project was developed as part of the COMP1028 module coursework.
It provides automated analysis of text and CSV files to generate reports on toxicity, lexical diversity, word frequency, and other advanced statistics.

____________________________________________________________________________________________________________________________________
Project Overview:
The program performs several analytical tasks:
i. Toxic ratio calculation
ii. Lexical diversity measurement
iii. Word frequency analysis
iv. Top toxic word extraction (global and category-based)
v. Advanced sorting using Bubble Sort and Merge Sort
vi. ASCII bar chart visualization
vii. Support for multi-file analysis
viii. Support for both TXT and CSV inputs

The program processes the input by cleaning, normalizing, and categorizing words using predefined toxic and stop-word dictionaries.
____________________________________________________________________________________________________________________________________
Development Environment:

The project was built using Visual Studio Code with the GCC compiler.

Compilation:
gcc stage5.c -o stage5

Execution:
./stage5
____________________________________________________________________________________________________________________________________
Folder Structure:

The main project directory contains three key folders:

i. analyze_source/
- Contains all input files (.txt or .csv) that will be analyzed by the system.

ii. dictionary_source/
- Contains the toxic-word dictionary and stop-word list used during analysis.

iii. analyze_result/
- Holds all generated output files, including:
Toxic ratio reports
Complexity reports
Sorted frequency results (txt/csv)
Algorithm comparison reports
____________________________________________________________________________________________________________________________________
Notes:
Ensure all files to be analyzed are placed inside analyze_source/.
All output reports will be saved in analyze_result/ automatically.
Toxic-word and stop-word dictionaries must remain in dictionary_source/.