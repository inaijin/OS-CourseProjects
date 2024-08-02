# Operating Systems Projects Spring 2023

This repository contains a collection of projects completed as part of the Operating Systems Course at UT-CE in SP-2023. These projects cover various aspects of operating systems, including socket programming, Map-Reduce, and serial/parallel processing.

## Project Descriptions

1. [CA1: Socket Programming](#ca1-socket-programming)
2. [CA2: Map-Reduce](#ca2-map-reduce)
3. [CA3: Serial/Parallel Image Processing](#ca3-serialparallel-image-processing)

### [CA1: Socket Programming](https://github.com/inaijin/OS-CourseProjects/tree/main/CA1_Socket_Programming)
**Description**: In this project, we utilized socket programming to implement a restaurant-client-suppliers connection. The restaurant requests ingredients from the suppliers, and customers request food from restaurants. The system is designed to handle multiple instances simultaneously without blocking.

**Features**:
- Asynchronous communication: Supports multiple clients and suppliers concurrently.
- Non-blocking sockets: Ensures that no process is halted while waiting for responses.
- Real-world simulation: Models the interaction between restaurants, suppliers, and customers.

### [CA2: Map-Reduce](https://github.com/inaijin/OS-CourseProjects/tree/main/CA2_Map_Reduce)
**Description**: This project involves using Map-Reduce techniques to calculate the utility bills for gas, water, and electricity. A main process creates a facility, which in turn creates(forks) houses. Each house forks controllers that calculate the bills for different utilities.

**Features**:
- Hierarchical process management: Main process, facility, houses, and controllers.
- Bill computation: Calculates gas, water, and electricity bills using Map-Reduce.
- Resource efficiency: Demonstrates efficient use of resources through parallel computation.

### [CA3: Serial/Parallel Image Processing](https://github.com/inaijin/OS-CourseProjects/tree/main/CA3_Parallel_Image_Processing)
**Description**: In this project, we explored the impact of parallel processing using pthreads compared to serial processing. We applied these techniques to image processing tasks, such as blurring, reversing, and color manipulation (e.g., making images purple and such).

**Features**:
- Parallel vs. serial comparison: Measures the performance difference between parallel and serial processing.
- Image processing tasks: Includes blurring, reversing, and color changes.
- Pthreads utilization: Demonstrates the use of pthreads for parallel execution.

## Contributing
Feel free to fork this repository, open issues, or submit pull requests. Any contributions are welcome!
