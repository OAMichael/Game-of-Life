#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../headers/LifeGame.hpp"
#include <mpi.h>


#define gridW 64
#define gridH 64



void updateCellsMPI(std::vector<int>& CellsArray, const int gridWidth, const int gridHeight, processInfo* info, int& updateFlag) {
    
    const int rank     = info->rank;
    const int commsize = info->commsize;
    
    if(commsize > 1)
        MPI_Bcast(&updateFlag, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);
    
    if(updateFlag == UPDATE_CELLS_MPI_END) {
        CellsArray[0] = UPDATE_CELLS_MPI_END;
        return;
    }

    static const int recvIndex_1 = ((info->start + gridHeight - 1) % gridHeight) * gridWidth;
    static const int recvIndex_2 = (info->end % gridHeight) * gridWidth;

    static const int sendIndex_1 = ((info->end - 1) % gridHeight) * gridWidth;
    static const int sendIndex_2 = (info->start % gridHeight) * gridWidth;

    if(commsize > 1) {
        if(updateFlag == UPDATE_CELLS_MPI_NEED_UPDATE)
            MPI_Bcast(CellsArray.data(), CellsArray.size(), MPI_INT, 0, MPI_COMM_WORLD);
        else {
            MPI_Status status;
            if(rank) {
                MPI_Recv(&CellsArray[recvIndex_1], gridWidth, MPI_INT, (rank - 1) % commsize, 0, MPI_COMM_WORLD, &status);
                MPI_Send(&CellsArray[sendIndex_1], gridWidth, MPI_INT, (rank + 1) % commsize, 0, MPI_COMM_WORLD);


                MPI_Recv(&CellsArray[recvIndex_2], gridWidth, MPI_INT, (rank + 1) % commsize, 0, MPI_COMM_WORLD, &status);
                MPI_Send(&CellsArray[sendIndex_2], gridWidth, MPI_INT, (rank - 1) % commsize, 0, MPI_COMM_WORLD);

            }
            else {
                MPI_Send(&CellsArray[sendIndex_1], gridWidth, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                MPI_Recv(&CellsArray[recvIndex_1], gridWidth, MPI_INT, commsize - 1, 0, MPI_COMM_WORLD, &status);


                MPI_Send(&CellsArray[sendIndex_2], gridWidth, MPI_INT, commsize - 1, 0, MPI_COMM_WORLD);
                MPI_Recv(&CellsArray[recvIndex_2], gridWidth, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }


    std::vector<int> cellStates;
    cellStates.resize(CellsArray.size());

    for(int i = info->start; i < info->end; ++i) {
        for(int j = 0; j < gridWidth; ++j) {
            int currentCellState = CellsArray[j + i * gridWidth];

            int idxLeft   = (j - 1 + gridWidth) % gridWidth;
            int idxMidX   = (j + 0) % gridWidth;
            int idxRight  = (j + 1) % gridWidth;

            int idxBottom = ((i - 1 + gridHeight) % gridHeight);
            int idxMidY   = ((i + 0) % gridHeight);
            int idxUp     = ((i + 1) % gridHeight);

            int neighbStates[8] = {

                CellsArray[idxLeft  + idxBottom * gridWidth],
                CellsArray[idxLeft  + idxMidY   * gridWidth],
                CellsArray[idxLeft  + idxUp     * gridWidth],

                CellsArray[idxMidX  + idxBottom * gridWidth],
                CellsArray[idxMidX  + idxUp     * gridWidth],

                CellsArray[idxRight + idxBottom * gridWidth],
                CellsArray[idxRight + idxMidY   * gridWidth],
                CellsArray[idxRight + idxUp     * gridWidth]
            };

            int neighbAlive = 0;
            for(int k = 0; k < 8; ++k)
                if(neighbStates[k] == 1)
                    ++neighbAlive;

            if(currentCellState == 0)
                if(neighbAlive == 3)
                    cellStates[j + i * gridWidth] = 1;
                else
                    cellStates[j + i * gridWidth] = 0;


            if(currentCellState == 1) 
                if(neighbAlive == 2 || neighbAlive == 3)
                    cellStates[j + i * gridWidth] = 1;
                else
                    cellStates[j + i * gridWidth] = 0;
        }
    }

    if(commsize > 1) {
        int* recvcnts = (int*)malloc(commsize * sizeof(int));
        int* displs = (int*)malloc(commsize * sizeof(int));
        int thisProcessSendCount = (info->end - info->start) * gridWidth;
        
        MPI_Gather(&thisProcessSendCount, 1, MPI_INT, recvcnts, 1, MPI_INT, 0, MPI_COMM_WORLD);

        displs[0] = 0;
        for(int i = 1; i < commsize; ++i) {
            displs[i] = displs[i - 1] + recvcnts[i - 1]; 
        }

        MPI_Gatherv(&cellStates[info->start * gridWidth], (info->end - info->start) * gridWidth, MPI_INT, CellsArray.data(), recvcnts, displs, MPI_INT, 0, MPI_COMM_WORLD);

        free(recvcnts);
        free(displs);


        if(rank > 0)
            std::copy(cellStates.begin() + info->start * gridWidth, cellStates.begin() + info->end * gridWidth, CellsArray.begin() + info->start * gridWidth);    
    }
    else {
        std::copy(cellStates.begin(), cellStates.end(), CellsArray.begin());
    }
}




int main(int argc, char* argv[]) {
    // Initializing MPI
    MPI_Init(&argc, &argv);

    // Getting the rank and communicator size
    int commsize, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    const int diff  = gridH / commsize;
    int start = diff * rank;
    int end   = diff * (rank + 1);

    if(gridH % commsize) {
        if(rank < gridH % commsize) {
            start += rank;
            end   += rank + 1;
        }
        else {
            start += (gridH % commsize);
            end   += (gridH % commsize);
        }
    }

    processInfo info{rank, commsize, start, end};


    MPI_Request myRequest;
    
    if(!rank) {
        LifeGame Game{gridW, gridH, info};

        const int xSize = (gridW > gridH) ? 1000 : 1000 * (float)gridW / gridH;
        const int ySize = (gridH > gridW) ? 1000 : 1000 * (float)gridH / gridW;

        Game.createWindow(3, 3, xSize, ySize, "Game of Life");
        Game.setCellUpdater(updateCellsMPI);
        Game.renderToWindow();
        Game.terminateWindow();
    }
    else {

        std::vector<int> thisRankAllCells;
        thisRankAllCells.resize(gridW * gridH);

        int stop;
        while(thisRankAllCells[0] != UPDATE_CELLS_MPI_END) {
            updateCellsMPI(thisRankAllCells, gridW, gridH, &info, stop);
        }
    }
    
    // Finalizing MPI
    MPI_Finalize();
    
    return 0;
}