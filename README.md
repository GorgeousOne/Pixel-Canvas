# Scientific Visualization Lab Class Exercise Framework Setup

This document will help you to build and run the application that you will use to complete the assignment for the SciVis part of the Visualization course. Note that the instructions differ depending on your operating system. 

For a demonstration of the build process, see the accompanying video clips.

## Download
 * Download the assignment framework as a .zip file from the moodle page. Extract to your chosen location.
 * Extract the zipped data directory (`data.zip`) to a directory called `data/`.   

<!-- ## Prerequisites -->
<!-- In order to build the applications, a C++ compiler is required. If you have not programmed using C++ before, then you may need to install a compiler. We will also use the [CMake](https://cmake.org/download/) build tool. Operating system specific instructions are below. -->

## Building and Running the Application

### Windows

There are two suggested methods for building and running the application in Windows: using either Visual Studio or Visual Studio Code. 

Method 1: Visual Studio
 * If not already installed, install [CMake](https://cmake.org/download/) and [Microsoft Visual Studio](https://visualstudio.microsoft.com/downloads/). The Community version of VS can be downloaded for free. When the installer prompts you to choose ‘Workloads’, choose `Desktop development with C++`.
 * Open CMake GUI
 * In the first input field, labeled ‘where is the source code’, browse to the root directory of the framework (the directory should contain the file CMakeLists.txt, e.g. `C:\Users\name\Documents\ExSciVis2023`)
 * In the second input field, labeled ‘where to build the binaries’, copy the input from the first field and add ‘\build’ to the end of the path (e.g. `C:\Users\name\Documents\ExSciVis2023\build`)
 * Click ‘Configure’
 * If prompted, click ‘Yes’ to confirm that a new build directory should be created
 * Specify your Visual Studio version, and click ‘Finish’
 * Click ‘Generate’
 * Click ‘Open Project’ to open the project in Visual Studio. Alternatively, one can open the ‘.sln’ file generated inside the build folder
 * In the Solution Explorer window in Visual Studio, confirm that the `ray_casting` target is shown in bold. This means that it is set as the start-up project. If not, right-click on `ray-casting` and select ‘Set as Startup Project’
 * Run the application by clicking the run symbol (green triangle in the toolbar).


 Method 2: Visual Studio Code
 * Install [Visual Studio Code](https://code.visualstudio.com/). From the extensions window (View > Extensions), install the `CMake Tools` and `C/C++` extensions.
 * Open the ExSciVis202X folder (File > Open Folder...)
 * The toolbar at the bottom should show a button saying that CMake is ready.
 * If a button with `no kit selected` is shown, click on it and select a kit. 
 * Make sure the default build target is set to `[ray_casting]` instead of `[ALL_BUILD]`
 * Launch the build by clicking on the triangle in the toolbar. 





### Linux

Prerequisites:
 * install [CMake](https://cmake.org/download/) from either the linked page, from Ubuntu Software, or install using a package manager such as apt or snappy
 * if you have not developed using c++ before, install the build-essentials package, which contains g++, a C++ compiler. In the terminal, enter:

`$ sudo apt install build-essential`


Building and Running:
 * Open a terminal inside the root directory of the assignment framework, ExSciVis202X
 * Create a build directory, in which the applications will be built, and navigate into the directory: 

`$ mkdir build` 

`$ cd build`

 * Run CMake (note the two dots, to tell CMake that the build script is in the parent directory):

`$ ccmake .. `

 * Press ‘C’ to configure CMake (you need to do this twice when the framework is built for the first time).
 * Press ‘G’ to generate the makefiles. After this, the terminal interface of CMake will disappear.
 * Build the applications by entering:

`$ make`

 * To be able to run the applications, we need to navigate to the directory where the binaries are located:

`$ cd build/Release`

 * Start the applications by entering `./<name_of_application>`, for example:

`$ ./ray_casting`

### MacOS

Prerequisites:
 * The built in compiler should be sufficient.
 * Install [CMake](https://cmake.org/download/). To enable calling CMake from within a terminal window, enter:

`$ PATH="/Applications/CMake.app/Contents/bin":"$PATH"`

Building and running steps are the same as on Linux.


## Hints and Tips


### Syntax Highlighting

Syntax highlighting adds colored text in the code editor to make files easier to read. The shader files in this project are written in GLSL - a language that may not be supported by default in many code editors.

On Windows, [GLSL language integration](https://marketplace.visualstudio.com/items?itemName=DanielScherzer.GLSL) can be downloaded and added to Visual Studio.
The [Shader languages support for VS Code](https://marketplace.visualstudio.com/items?itemName=slevesque.shader) extension can be added to Visual Studio Code.

Other code editors may have similar extensions for GLSL. Alternatively, 'C' syntax highlighting works quite well for GLSL files.


## Troubleshooting

If you have any issues:
 * Please make sure you are in the correct directory when executing commands in the terminal (Mac & Linux)
 * Please make sure you have selected the correct directories in CMake GUI (Windows)



GUI created with ImGui
https://github.com/ocornut/imgui
