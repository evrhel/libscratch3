# libscratch3

libscratch3 is a library for running Scratch 3 projects. It comes with a command line interface that can run Scratch 3 projects from the command line.

Currently, some projects will run, but there is no way to interact with them or get graphical output. This is a work in progress.

The library adheres to the unofficial [Scratch 3 JSON project format](https://en.scratch-wiki.info/wiki/Scratch_File_Format).

The project runs on Windows but should eventually run on Linux and macOS as well.

## Building

[vcpkg](https://github.com/microsoft/vcpkg) is used as the dependency manager and installation is required to build the project alongside CMake. Other dependencies are included in the repository as submodules.

To build the project, first generate the build files with CMake, using the `windows-vcpkg` preset.

```bash
mkdir build
cd build
cmake .. --preset windows-vcpkg
```

Then build the project using the generated build files.

### Use in external projects

The easiest way to use libscratch3 is to add it as a subdirectory in your project. This can be done by adding the following line to your `CMakeLists.txt`:

```cmake
add_subdirectory(libscratch3)
```

Then, link your target to the library:
    
```cmake
target_link_libraries(your_target PRIVATE libscratch3)
```

You will need `vcpkg` as well, to ensure the dependencies are installed.

## Usage

To use libscratch3, include `scratch3/scratch3.h` in your project. A plain C interface is provided.

### Example

```c
#include <scratch3/scratch3.h>

#include <stdlib.h>

int main(int argc, char *argv[])
{
    Scratch3 *S;
    int rc;

    // Create a new Scratch 3 instance referencing the
    // given project file
    S = Scratch3_Create("path/to/project.sb3", NULL, NULL)
    if (S == NULL)
        return 1;

    // Compile the project
    rc = Scratch3_Compile(S);
    if (rc == -1)
    {
        Scratch3_Destroy(S);
        return 1;
    }

    // Run the project
    rc = Scratch3_Run(S);
    if (rc == -1)
    {
        Scratch3_Destroy(S);
        return 1;
    }

    // Wait for the project to finish
    rc = Scratch3_Wait(S, -1);
    if (rc == -1)
    {
        Scratch3_Destroy(S);
        return 1;
    }

    // Clean up
    Scratch3_Destroy(S);

    return 0;
}
```

## Running

To run a Scratch 3 project, use the `scratch3` command line tool. It takes a single argument, the path to the project file. Assuming the executable is in your path, projects can be run using:

```bash
scratch3 path/to/project.sb3
```
