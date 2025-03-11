## gameOfLife
<a href="https://github.com/DD-exe/gameOfLife/blob/master/readme.md">ÖÐÎÄ°æ</a><br>
This project aims to implement a graphical Windows application for the visual simulation of Conway's Game of Life with adjustable parameters.<br>
In traditional discussions of Conway's Game of Life, the importance of initial parameters is often emphasized. If the survival conditions are too lenient, chaotic expansion occurs; if too strict, the simulation quickly dies out. However, most available Game of Life simulators do not allow free adjustment of these parameters, limiting users' ability to explore and understand the system's behavior.<br>
In this project, we introduce adjustable game parameters while maintaining a reasonable computational cost. Now, the simulation is no longer restricted to the classic "optimal" parameters¡ªyou can modify them at any time, even mid-simulation.<br>
Additionally, the project includes common features found in other Game of Life visualizations, such as adjustable grid size and iteration speed.<br>
### Project Structure
```
gameOfLife
©¦  about.cpp            // Handles the "About" dialog messages
©¦  downloads.cpp        // Implements built-in simulation result saving
©¦  gameOfLife.cpp       /* Main framework of the program  
©¦                         Handles main window creation and message loop */
©¦  life.cpp             // Implements the iteration logic for Game of Life
©¦  tools.cpp            // Implements various utility functions
©¦  gameOfLife.h         // Header file for gameOfLife.cpp
©¦  framework.h          // Project framework header file
©¦  resource.h           // Resource interface definitions
©¦  targetver.h
©¦  gameOfLife.rc        // Project resources
©¦
©¸©¤image                // Icons and images
        lin.bmp
        OIP-C.bmp
        OIP-C.ico
        small.ico

```