Skip to [SOLUTION.md](./SOLUTION.md) for my solution.

# Zeta Motion LTD  - Technical Challenge for Software Engineer (CPP001)

In this challenge, you will be asked to write a simple application in C/C++ to find the bounding boxes of the objects in an
image, as shown in Figure 1. An object is defined as a continuous "Blob" of non-black pixels.

| Figure 1.  Detected Objects       |
| ------------- |
|  ![Object Detection](image.png "Object Detection Simple") |

Your task is to

1. Use `main.cpp` as a starting point to load `data.bin` file into memory
2. Read in `data.bin`, which contains an array of `unsigned char` numbers (0 - 255), which holds image frames with shape (10, 600, 800). Each image is stored in row-major format.
3. Implement an algorithm that identifies the bounding boxes of the objects in each image frame. 
All bounding boxes should be exported in format (top, left, height, width) to a txt file; One line for each image frame.

The success criteria for your programme are:

1. Accurate number of bounding boxes
2. Accurate positions and sizes
3. High performance (please use the macro provided to measure running time)
4. Bug free

Please note:

1. Don't use any third party library to implement the algorithm (you can do better than them). However, you can use any tool to visualise the images for debugging purposes.
2. Make comments wherever relevant