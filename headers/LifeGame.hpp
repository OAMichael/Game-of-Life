#ifndef LIFEGAME_HPP
#define LIFEGAME_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../headers/IApplication.hpp"
#include "../headers/Shader.hpp"



#define UPDATE_CELLS_MPI_START 2
#define UPDATE_CELLS_MPI_END 4
#define UPDATE_CELLS_MPI_NEED_UPDATE 8



struct processInfo {
    int rank;
    int commsize;
    int start;
    int end;
};


class LifeGame : public GeneralApp::Application {

private:

    enum LIFEGAME_STATE : unsigned {
        STOP = 0,
        RUN,

        COUNT

    };


    processInfo renderProcessInfo;

    unsigned GameState_ = LIFEGAME_STATE::STOP;

    GeneralApp::Shader GridLinesShader_;

    GeneralApp::Shader CellShader_;
    
    unsigned VBO_;
    unsigned VAO_; 

    bool IsFullscreen_ = false;

    float mousePosX_;
    float mousePosY_;

    int gridWidth_;
    int gridHeight_;

    bool lButtonPressed = false;
    bool rButtonPressed = false;
    bool ctrlPressed = false;

    float prevMousePosXPan_;
    float prevMousePosYPan_;

    std::vector<float> gridLines_;
    std::vector<float> borderLines_;

    const float gridLinesThickness_ = 50.0f / static_cast<float>(std::min(gridWidth_, gridHeight_));
    const float borderLinesThickness_ = 4 * gridLinesThickness_;

    bool RecalculateCells_ = false;

    std::vector<int> totalCells_{};

    unsigned SSBO_;

    float currentScale_ = 1.0f;
    glm::mat4 currentTransform_ = glm::mat4(1.0f);
    glm::mat4 currentTransformInv_ = glm::mat4(1.0f);

    glm::ivec2 arrayIndexSpaceOffset_ = glm::ivec2(0);

    const VertexAttributeDesc<2> LineAttrDesc = {2, std::array<int, 2>{ {2, 3} }, VERTEX_TYPE::FLOAT_T};

    const VertexAttributeDesc<2> RectAttrDesc = {2, std::array<int, 2>{ {2, 2} }, VERTEX_TYPE::FLOAT_T};

    const std::vector<float> FullscreenTriangleVerts_ = {
        // Vertices         // UVs
        -1.0f, -1.0f,       0.0f, 0.0f,
        -1.0f,  1.0f,       0.0f, 1.0f,
         1.0f, -1.0f,       1.0f, 0.0f,

         1.0f, -1.0f,       1.0f, 0.0f,
        -1.0f,  1.0f,       0.0f, 1.0f,
         1.0f,  1.0f,       1.0f, 1.0f
    };

    const glm::vec2 ScaleRange_ = glm::vec2(0.9, static_cast<float>(std::min(gridWidth_, gridHeight_)));

    unsigned backgroundTextureHandle_;

    void (*updateCellsFunc)(std::vector<int>&, const int, const int, processInfo*, int&);

    int updateFlag_ = UPDATE_CELLS_MPI_START;
    
public:
    LifeGame();

    LifeGame(int gridW, int gridH, processInfo info);


    void renderToWindow() override;

    void mouseCallback(GLFWwindow* window, int button, int action, int mods) override;

    void cursorCallback(GLFWwindow* window, double xpos, double ypos) override;

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) override;

    void framebufferSizeCallback(GLFWwindow* window, int width, int height) override;

    void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    void initRender();

    template<typename T, int CountParam>
    void drawShapes(GLenum mode, const std::vector<T>& verts, const VertexAttributeDesc<CountParam>& VertAttrbs);

    void resetSSBO();
    
    void updateSSBO();
    
    void drawAliveCells();

    void drawGrid();

    void setCellUpdater(void (*newUpdateCellsFunc)(std::vector<int>&, const int, const int, processInfo*, int&));
};


#endif // LIFEGAME_HPP