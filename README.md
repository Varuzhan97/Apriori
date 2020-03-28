# Apriori
Apriori implementation using uthash table for best performance.

Apriori is an algorithm for frequent item set mining and association rule learning over relational databases. It proceeds by identifying the frequent individual items in the database and extending them to larger and larger item sets as long as those item sets appear sufficiently often in the database. The frequent item sets determined by Apriori can be used to determine association rules which highlight general trends in the database: this has applications in domains such as market basket analysis.
The Apriori algorithm was proposed by Agrawal and Srikant in 1994. Apriori is designed to operate on databases containing transactions (for example, collections of items bought by customers, or details of a website frequentation or IP addresses). Other algorithms are designed for finding association rules in data having no transactions (Winepi and Minepi), or having no timestamps (DNA sequencing). Each transaction is seen as a set of items (an itemset). Given a threshold C, the Apriori algorithm identifies the item sets which are subsets of at least C transactions in the database.
Apriori uses a "bottom up" approach, where frequent subsets are extended one item at a time (a step known as candidate generation), and groups of candidates are tested against the data. The algorithm terminates when no further successful extensions are found.
Apriori uses breadth-first search and a Hash tree structure to count candidate item sets efficiently. It generates candidate item sets of length k from item sets of length k-1. Then it prunes the candidates which have an infrequent sub pattern. According to the downward closure lemma, the candidate set contains all frequent k-length item sets. After that, it scans the transaction database to determine frequent item sets among the candidates.

Association rules analysis is a technique to uncover how items are associated to each other. There are three common ways to measure association.

Measure 1: Support. This says how popular an itemset is, as measured by the proportion of transactions in which an itemset appears.

           support{x} = (transactions containing x) / (total number of transactions)

Measure 2: Confidence. This says how likely item Y is purchased when item X is purchased, expressed as {X -> Y}. This is measured by the proportion of transactions with item X, in which item Y also appears.

           confidence{x->y} = (support{x,y}) / (support{x})

Measure 3: Lift. This says how likely item Y is purchased when item X is purchased, while controlling for how popular item Y is.

           lift{x->y} = (support{x,y}) / (support{x} X support{y})

Used library for best performance: https://github.com/troydhanson/uthash.

Compile syntax: g++ apriori.cpp -o a.out.

Output of algorithm is "output.csv" file, which contains itemsets with minimum suppurt, confidence, lift.

"config.cfg" file is for input and process configurations(minimum support, minimum confidence, database name).
