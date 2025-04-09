## gameOfLife
<a href="https://github.com/DD-exe/gameOfLife/blob/master/readme.md">ÖÐÎÄ°æ</a><br>
This project aims to implement a graphical Windows application for the visual simulation of Conway's Game of Life with adjustable parameters.<br>
In traditional discussions of Conway's Game of Life, the importance of initial parameters is often emphasized. If the survival conditions are too lenient, chaotic expansion occurs; if too strict, the simulation quickly dies out. However, most available Game of Life simulators do not allow free adjustment of these parameters, limiting users' ability to explore and understand the system's behavior.<br>
In this project, we introduce adjustable game parameters while maintaining a reasonable computational cost. Now, the simulation is no longer restricted to the classic "optimal" parameters¡ªyou can modify them at any time, even mid-simulation.<br>
Additionally, the project includes common features found in other Game of Life visualizations, such as adjustable grid size and iteration speed.<br><br>
In version 2.0 of the project, we further push the boundaries of the Game of Life using efficient algorithms and well-structured code frameworks:<br>
Building upon the existing program, we creatively designed and implemented a battle version of Game of Life rules by integrating a portable tabletop RPG dice-based decision mechanism. This enables simulations of multiple factions and evolutionary warfare.<br>
We designed a decentralized online multiplayer structure based on ENet, with a simplified connection process, and introduced more robust simulation constraints for online matches. Clearly, this is a necessary step for transitioning from a simulator to a more gamified experience.<br>
We have significantly optimized the previous bottlenecks in rendering performance and introduced user experience-oriented features such as infinite grid scrolling, faction color selection, and rule descriptions. Now, you can enjoy the flexibility similar to that of a professional drawing application.<br>
We added a JSON-based database system, allowing players to conveniently save and load data, with open access for direct modification.<br><br>
Below is the source code structure of the project:
### Project Structure
```
gameOfLife
|   about.cpp                       // Handles messages for the "About" dialog
|   color.cpp                       // Handles messages for the "Color Picker" dialog
|   gameOfLife.cpp                  // Main framework; creates the main window and manages the message loop
|   life.cpp                        // Implements data iteration under Game of Life rules
|   load.cpp                        // Implements save file loading
|   rules.cpp                       // Handles messages for the "Rules" dialog
|   save.cpp                        // Implements save file writing
|   saveBmp.cpp                     // Implements export to image
|   single.cpp                      // Handles messages for single-player simulation dialog
|   tools.cpp                       // Implements utility functions used by various components
|   trans.cpp                       // Implements network communication
|   vs.cpp                          // Handles messages for local multiplayer battle dialog
|   vsOnline.cpp                    // Handles messages for online battle dialog
|   framework.h                     // Project framework header file
|   gameOfLife.h                    // Header for gameOfLife.cpp
|   gameOfLife.rc                   // Project resources
|   resource.h                      // Resource interface definitions
| 
©¸©¤image                             // Backgrounds and icons
    |   main.png
    |   lin.bmp 
    ©¸©¤  OIP-C.bmp
©¸©¤  vcpkg_installed                 // Package manager resources

```