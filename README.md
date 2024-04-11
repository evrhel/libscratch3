# libscratch3

libscratch3 is a library for running Scratch 3 projects. It comes with a command line interface that can run Scratch 3 projects from the command line.

Currently, all it does is parse a project file and convert it to an Abstract Syntax Tree (AST). Not all blocks are supported.

The library adheres to the unofficial [Scratch 3 JSON project format](https://en.scratch-wiki.info/wiki/Scratch_File_Format).

The project should build on Windows, macOS, and Linux.

## Building

Use CMake to build the project. The dependencies are provided as git submodules.

To build the project, first generate the build files with CMake:

```bash
mkdir build
cd build
cmake ..
```

The build the project using whatever build system you have.

### Use in external projects

The easiest way to use libscratch3 is to add it as a subdirectory in your project. This can be done by adding the following line to your `CMakeLists.txt`:

```cmake
add_subdirectory(libscratch3)
```

Then, link your target to the library:
    
```cmake
target_link_libraries(your_target PRIVATE libscratch3)
```

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

To run a Scratch 3 project, use the `scratch3` command line tool. It takes a single argument, the path to the project file. Assuming the executable is in your path, you can run a project like this:

```bash
scratch3 path/to/project.sb3
```

**Note**: This will only print the AST.
