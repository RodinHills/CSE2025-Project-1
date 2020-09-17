# Data-Structers-Project-1
A project that finds first-second-third order paths between words in various categories and documents by linked list notation within Data Structers course.

## How it works
Program transforms all words (no duplicate) to a master linked list(MLL). First order paths get created between words which is in same document. Second order paths get created between words which is in different documents that has common another word. Third order paths get created between words which is in documents that has no common word, but has a common word with another document. Finally program prints first-second-third order paths , term frequency (most common words) and inverse term frequency (ln[(Total number of documents)/(Number of documents with term t in it)]).
