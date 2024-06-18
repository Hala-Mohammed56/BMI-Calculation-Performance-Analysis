# Comparative Performance Analysis of BMI Calculation Approaches in Linux Environment

This repository contains the code and report for the project assigned in the ENCS3390: Operating System Concepts course, Second Semester 2023/2024.

## Project Overview

The project involves writing a program to calculate the average BMI (Body Mass Index) for a given dataset using different approaches. The performance of each approach is compared by measuring the execution time. The approaches considered are:

1. **Naive Approach**: A single-threaded program.
2. **Multiprocessing Approach**: A program that uses multiple child processes running in parallel.
3. **Multithreading Approach**: A program that uses multiple threads running in parallel.

## Objectives

- Implement and measure the performance of different BMI calculation approaches.
- Analyze the results according to Amdahl’s law.
- Determine the optimal number of child processes or threads for the best performance.

## Requirements

- A computer with at least 4 cores (if using a virtual machine, ensure it has at least 4 allocated cores).
- Programming language and platform of your choice.
- Linux environment.

## Project Structure

- `code/`: Contains the source code for the different approaches.
- `data/`: The dataset used for BMI calculations.
- `report/`: The project report discussing the methods, analysis, and conclusions.

## Getting Started

1. Clone the repository:
    ```bash
    git clone https://github.com/your-username/BMI-Calculation-Performance-Analysis.git
    cd BMI-Calculation-Performance-Analysis
    ```

2. Navigate to the `code` directory and follow the instructions in the README to run each approach.

## Analysis and Results

The report includes:

- Methodology for achieving multiprocessing and multithreading.
- Analysis based on Amdahl’s law.
- Performance comparison table.
- Discussion on performance differences and conclusions.

## License

This project is licensed under the MIT License.

## Acknowledgments

- Faculty of Engineering & Technology
- Department of Electrical & Computer Engineering
