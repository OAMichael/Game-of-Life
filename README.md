## Conway's Game of Life

#### This is visualization of Game of Life using OpenGL for rendering and MPICH for calculations


## How to build and run

Clone the repository:
```console
git clone https://github.com/OAMichael/Game-of-Life.git
```

Then, being in main directory, create `build` directory:
```console
mkdir build && cd build
```

Being in build folder, type:
```console 
cmake ..
```

And finally make:
```console
make
```

To run with MPI use:
```console
mpiexec -np 4 ./main.exe
```

https://user-images.githubusercontent.com/70339407/230748532-3e047e83-d562-4d80-a9b9-9e2e11db0acc.mp4
