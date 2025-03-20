# Versioned File System

This project implements a versioned file management system in C++. It allows users to create, open, read, write, and manage multiple versions of files efficiently.

## How to Use the Makefile

To compile the program: make

To run the program: make run

To clean all generated files (.o, executable, and .txt files): make clean

## Completed Tasks

- Design the architecture of the library, including the API for reading and writing.
- Define data structures for file versions and metadata.
- Create the basic structure for file representation.
- Implement initialization of the library.
- Implement the Copy-On-Write (COW) mechanism for writing.
- Implement logic to track version history.
- Ensure read operations always access the latest version.
- Create a function to list all file versions.
- Write documentation for the API and how to use the library.
- Create an example to demonstrate the usage of the library.

## Pending Tasks

- Develop functions to copy blocks of data and update metadata.         (In progress)
- Create unit tests for write functionality.
- Design and implement efficient data recovery mechanisms.              (In progress)
- Optimize read performance by minimizing data duplication.             (In progress)
- Implement memory optimization techniques to reduce storage overhead.  (In progress)
- Develop a garbage collection mechanism to free unused data blocks.    (In progress)
- Analyze memory usage and identify possible optimizations.
- Perform extensive testing to ensure stability.
- Conduct a performance analysis to evaluate the efficiency of COW.
- Write a final report summarizing findings and conclusions.