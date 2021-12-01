# Marker for PC

Implementation on PC (linux & windows)

------

To compile from source:

1. Init submodules
   ```bash
   git submodule update --init --recursive
   ```
2. Install OpenCV on your system  
3. Configure for OpenCV:  
   If Windows, edit `CMakeLists.txt` and set OpenCV path in `find_package` command  
   If Linux, make sure to install following dependencies:  
   ```bash
   sudo pacman -S opencv hdf5 vtk pugixml
   ```
4. Compile:
   ```bash
   mkdir build && cd build
   cmake ..
   ```
   For Linux:
   ```bash
   make -j4
   ```
   For Windows:
   ```bash
   cmake --build . --config Release
   ```
5. Executable can be found in `bin` folder