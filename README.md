# Suitor and Local Dominant Graph Matching 
This repository contains my final freestyle assigment from the "Efficient and Parallel C++" course I took at university. 
It implements and evaluates the sequential and parallel versions of the suitor and local dominant matching algorithm
explained in: 

* [M. Halappanavar, J. Feo, O. Villa, A. Tumeo, and A. Pothen, “Approximate weighted
  matching on emerging manycore and multithreaded architectures,” Int. J. High Perf.
  Comput. App., vol. 26, no. 4, pp. 413– 430, 2012](https://journals.sagepub.com/doi/full/10.1177/1094342012452893)
  
* [Manne, Halappanavar: Suitor algorithm for 1/2-approx. edge weighted matching, IPDPS
  2014](http://www.ii.uib.no/~fredrikm/ipdps2014.pdf)


## Repo structure
* The ``Assignment.pdf`` contains the exact task, the ``Endpresentation.pdf`` contains my final presentation.
* ``external` contains code which is not mine. In general it contains a very efficient concurrent queue, the RMAT graph generator 
and a script to read edgelists from one of the tutors of the course
* ``evaluation`` contains my measurments and plots
* ``implementation`` contains the header (hpp), ``source`` the source (cpp) files
* ``tools`` contains a file to transform the edgelist created by RMAT into another format with weigths (more on it below)

## General Class Interface 
All classes have the same interface

``Input edgeLst = readEdges("../data/graphfile.raw_graph);``

``[Algorithm] test_1(edgeLst) // see algos below`` e.g. ``ParallelSuitor test_1(edgeLst)``

``test_1.simple_procedure(); // run normal matching, the parallel variants take the number of threads as argument``

``test_1.test_matching(); // test if result is a matching``

``test_1.print_matching(); // print matching``

``double weight = test_1.matching_quality(); // return weight of matching``

``test_1.clear_mate(); // This resets the mate / sutor array (as well as some other stuff). This is nessesary if one wants to run a matching again with the same instance of the class``

``test_1.improved_procedure(); // Run the heavy matching quality improvment explained in Manne and Halappanavar (if implemented);  the parallel variants take the number of threads as argument``

See also main function in each cpp file

Algorithms: ``SequentialDominant, SequentialDominant, ParallelSuitor, ParallelDominant, ParallelFreeSuitor``

## General Usage from Command Line 
``cd cmake-build-debug``

`` make `` 

Available commands: 

* ``./[algo] 0 path/to/data``:  runs correctness test on ``simple_procedure()`` (simple matching)
* ``./[algo] -1 path/to/data``: runs correctness test on ``simple_procedure()`` and ``improved_procedure()`` (only for the algos that have heavy matching implemented)
* ``./[algo] 1 path/to/data``: runs runtime test on ``simple_procedure()``
* ``./[algo] 2 path/to/data``: runs runtime test on ``simple_procedure()`` and ``improved_procedure()`` (if implemented)

Available algorithms: 
* ``seq_domi``: Sequential Local Dominant 
* ``par_domi``: Parallel Local Dominant
* ``seq_suitor``: Sequential Suitor 
* ``par_suitor``: Parallel Suitor (using Lock) 
* ``par_free_suitor``: Parallel Lockfree Suitor 

## Detailed description 
In the header file of the respective implementation their is a short description of what each function does and what each member is for


## Bonus / Imporved Procedure 
Heaving Matching is implementred for SeqSuitor (as required by exercise), SeqDominant (Bonus) and ParSuitor (Bonus).
I gave my best to mark the corresponding parts of the code that are bonus. Note that the ``improved_procedure()`` (Heavy Matching) is not particularly fast
(see the paper for details).



## Graph Generator / Data Format 
See readme in external/PaRMAT-master

In short:

``cd`` to ``Release``

``make``

``./PaRMAT -nVertices XX -nEdges XX -noEdgeToSelf -noDuplicateEdges -undirected -output testfile.raw_graph``

``-noEdgeToSelf -noDuplicateEdges -undirected`` are extremly important.

Then their are two options:
 - Read the file as it is with ``implementation/edge_list2.hpp`` (default). Note that the file does not have any weights. Those are generated
 randomly when reading the edges. Therefore the quality of the matching is not comparable. See `data/test.raw_graph`` for an example.
 It's an simple edgelist separated by spaces with no weights (just the node ids). 

- Adjust the format to what ``external/edge_list.hpp`` expects using ``tools/trans_file.cpp``. This creates a new file that contains
the weights as well as the number of vertrices (note that RMAT does not exactly generate the nVertrices that are passed, therefore
the script has to find the highest index first). Note that the ``#include`` has to be adjusted in the header file of the respective graph
(``graph2_adj_array.hpp`` for lock-free suitor, ``graph_adj_array.hpp`` for the rest) to ``external/edge_list.hpp``.
This option is not particularly fast. See ``data/test.graph`` for an example. The first line contains information on the number of 
nodes, followed by an edgelist separated by spaces (nodeid nodeid weight). 

``cd cmake-build-debug``

`` make trans_file``

``./trans_file inputpath.raw_graph outputpath.graph``



## Evaluation 
In the evaluation dir you can find the runtime tests. Note that the Rscripts in each subdirectory (one per exerciese) assume that
'evaluation‘ is set as the working directory. Some plots contain tests for the HeavyMatching. They have an Imp (Imporved) suffix,
while the normal versions have an Simp (Simple) suffix. Note that each Figure with SeqDomiImp and ParSuitorImp is displaying bonus
content. The directory does contain a file notes.txt which states what graphs have been used for evaluation. 

## Example Usage 
```
cd external/PaRMAT-master/Release/
./PaRMAT -nVertices 1000000 -nEdges 100000000 -noEdgeToSelf -noDuplicateEdges -undirected -output ../../../data/test1.raw_graph -threads 48
cd ../../../cmake-build-debug/
./seq_suitor 1 ../data/test1.raw_graph
```

The graph file is around 1GB in size. 

## OS
I have tested it on MacOS and Linux. 