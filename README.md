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
```
make
```

To run with MPI use:
```console
mpiexec -np 4 ./main.exe
```


<video width="600" height="600" controls>
  <source src="./pictures/GameOfLife.ogv" type="video/ogv">
</video>
