#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

#include "../headers/LifeGame.hpp"


void LifeGame::setCellUpdater(void (*newUpdateCellsFunc)(std::vector<int>&, const int, const int, processInfo*, int&)) {
    updateCellsFunc = newUpdateCellsFunc;
}



void LifeGame::renderToWindow() {
    
    std::cout << "State: STOP" << std::endl;
    initRender();

    while(!glfwWindowShouldClose(window_)) {

        showFPS();

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.1f, 0.3f, 1.0f);

        drawAliveCells();
        drawGrid();
        
        glfwSwapBuffers(window_);
        glfwPollEvents();    
    }

    glDeleteVertexArrays(1, &VAO_);
    glDeleteBuffers(1, &VBO_);
}


void LifeGame::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if(action == GLFW_PRESS) {
                lButtonPressed = true;

                if(ctrlPressed) {
                    prevMousePosXPan_ = mousePosX_;
                    prevMousePosYPan_ = mousePosY_;
                }
                
                if(GameState_ == LIFEGAME_STATE::STOP && !ctrlPressed) {

                    glm::vec2 tmpf = glm::vec2((currentTransformInv_ * glm::vec4(mousePosX_ / windowWidth_ * 2.0f - 1.0f, (1.0f - mousePosY_ / windowHeight_) * 2.0f - 1.0f, 0.0f, 1.0f))) / 2.0f + 0.5f;

                    glm::ivec2 tmpi = glm::ivec2((int)(gridWidth_ * tmpf.x), (int)(gridHeight_ * tmpf.y));

                    int linIndex = tmpi.x + tmpi.y * gridWidth_;
                    if(tmpi.x >= 0 && tmpi.x < gridWidth_ && tmpi.y >= 0 && tmpi.y < gridHeight_) {
                        totalCells_[linIndex] = 1;
                        updateSSBO();

                        updateFlag_ = UPDATE_CELLS_MPI_NEED_UPDATE;
                    }
                }
            }
            else {
                lButtonPressed = false;
            }

            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if(action == GLFW_PRESS) {
                rButtonPressed = true;
            
                if(GameState_ == LIFEGAME_STATE::STOP) {
                    glm::vec2 tmpf = glm::vec2((currentTransformInv_ * glm::vec4(mousePosX_ / windowWidth_ * 2.0f - 1.0f, (1.0f - mousePosY_ / windowHeight_) * 2.0f - 1.0f, 0.0f, 1.0f))) / 2.0f + 0.5f;

                    glm::ivec2 tmpi = glm::ivec2((int)(gridWidth_ * tmpf.x), (int)(gridHeight_ * tmpf.y));
                    
                    int linIndex = tmpi.x + tmpi.y * gridWidth_;
                    if(tmpi.x >= 0 && tmpi.x < gridWidth_ && tmpi.y >= 0 && tmpi.y < gridHeight_) {
                        totalCells_[linIndex] = 0;
                        updateSSBO();

                        updateFlag_ = UPDATE_CELLS_MPI_NEED_UPDATE;
                    }
                }
            }
            else
                rButtonPressed = false;

            break;
    }
}



void LifeGame::cursorCallback(GLFWwindow* window, double xpos, double ypos) {

    if(ctrlPressed && lButtonPressed) {
        float offsetX = (xpos - prevMousePosXPan_) / windowWidth_ * 2.0f / currentScale_;
        float offsetY = (ypos - prevMousePosYPan_) / windowHeight_ * 2.0f / currentScale_;

        prevMousePosXPan_ = xpos;
        prevMousePosYPan_ = ypos;

        currentTransform_ = glm::translate(currentTransform_, glm::vec3(offsetX,  -offsetY, 0.0f));
        currentTransformInv_ = glm::inverse(currentTransform_);
    }

    if(GameState_ == LIFEGAME_STATE::STOP) {
        if(lButtonPressed && !ctrlPressed) {
            glm::vec2 tmpf = glm::vec2((currentTransformInv_ * glm::vec4(xpos / windowWidth_ * 2.0f - 1.0f, (1.0f - ypos / windowHeight_) * 2.0f - 1.0f, 0.0f, 1.0f))) / 2.0f + 0.5f;

            glm::ivec2 tmpi = glm::ivec2((int)(gridWidth_ * tmpf.x), (int)(gridHeight_ * tmpf.y));

            int linIndex = tmpi.x + tmpi.y * gridWidth_;
            if(tmpi.x >= 0 && tmpi.x < gridWidth_ && tmpi.y >= 0 && tmpi.y < gridHeight_) {
                totalCells_[linIndex] = 1;
                updateSSBO();

                updateFlag_ = UPDATE_CELLS_MPI_NEED_UPDATE;
            }
        }

        if(rButtonPressed) {
            glm::vec2 tmpf = glm::vec2((currentTransformInv_ * glm::vec4(xpos / windowWidth_ * 2.0f - 1.0f, (1.0f - ypos / windowHeight_) * 2.0f - 1.0f, 0.0f, 1.0f))) / 2.0f + 0.5f;

            glm::ivec2 tmpi = glm::ivec2((int)(gridWidth_ * tmpf.x), (int)(gridHeight_ * tmpf.y));

            int linIndex = tmpi.x + tmpi.y * gridWidth_;
            if(tmpi.x >= 0 && tmpi.x < gridWidth_ && tmpi.y >= 0 && tmpi.y < gridHeight_) {
                totalCells_[linIndex] = 0;
                updateSSBO();

                updateFlag_ = UPDATE_CELLS_MPI_NEED_UPDATE;
            }
        }
    }

    mousePosX_ = xpos;
    mousePosY_ = ypos;
}


void LifeGame::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    float newScale = std::clamp<float>(std::pow(2.0f, 0.3 * yoffset), ScaleRange_.x / currentScale_, ScaleRange_.y / currentScale_);

    float xpos = mousePosX_ / windowWidth_ * 2.0f - 1.0f;
    float ypos = 1.0f - 2.0f * mousePosY_ / windowHeight_;

    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(xpos, ypos, 0.0f));
    glm::mat4 scale = glm::scale(translate, glm::vec3(newScale, newScale, 1.0f));
    currentTransform_ = glm::translate(scale, glm::vec3(-xpos, -ypos, 0.0f)) * currentTransform_;

    currentTransformInv_ = glm::inverse(currentTransform_);

    currentScale_ *= newScale;
}


void LifeGame::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth_  = width;
    windowHeight_ = height;
}


void LifeGame::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch(key) {
        case GLFW_KEY_ESCAPE: {
            updateFlag_ = UPDATE_CELLS_MPI_END;
            updateCellsFunc(totalCells_, gridWidth_, gridHeight_, &renderProcessInfo, updateFlag_);
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            
        }
        break;

        case GLFW_KEY_F11: {
            if(action == GLFW_PRESS) {
                IsFullscreen_ = !IsFullscreen_;

                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                if(IsFullscreen_) {
                    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                }
                else {
                    glfwSetWindowMonitor(window, nullptr, 0, 0, 1000, 1000, 0);
                }
            }
        }
        break;

        case GLFW_KEY_SPACE: {
            if(action == GLFW_PRESS){
                if(GameState_ == LIFEGAME_STATE::STOP) {
                    std::cout << "State: RUN" << std::endl;
                    GameState_ = LIFEGAME_STATE::RUN;
                }

                else if(GameState_ == LIFEGAME_STATE::RUN) {
                    std::cout << "State: STOP" << std::endl;
                    GameState_ = LIFEGAME_STATE::STOP;
                }

            }
        }
        break;

        case GLFW_KEY_RIGHT: {
            if(action == GLFW_PRESS && GameState_ == LIFEGAME_STATE::STOP){
                RecalculateCells_ = true;
            }
        }
        break;

        case GLFW_KEY_ENTER: {
            if(action == GLFW_PRESS && GameState_ == LIFEGAME_STATE::STOP){
                currentTransform_    = glm::mat4(1.0f);
                currentTransformInv_ = glm::mat4(1.0f);
                currentScale_ = 1.0f;

                arrayIndexSpaceOffset_ = glm::ivec2(0);

                std::fill(totalCells_.begin(), totalCells_.end(), 0);
                updateFlag_ = UPDATE_CELLS_MPI_NEED_UPDATE;
                updateSSBO();

                std::cout << "Reseting current cells..." << std::endl;
            }
        }
        break;

        case GLFW_KEY_LEFT_CONTROL: {
            if(action == GLFW_PRESS){
                ctrlPressed = true;
                prevMousePosXPan_ = mousePosX_;
                prevMousePosYPan_ = mousePosY_;
            }
            else {
                ctrlPressed = false;
            }
        }
        break;
    }

}


LifeGame::LifeGame() {
    m_Application = this;
}


LifeGame::LifeGame(int gridW, int gridH, processInfo info) : gridWidth_{gridW}, gridHeight_{gridH}, renderProcessInfo{info} {
    m_Application = this;
}



template<typename T, int CountParam>
void LifeGame::drawShapes(GLenum mode, const std::vector<T>& verts, const VertexAttributeDesc<CountParam>& VertAttrbs) {
    glBindVertexArray(VAO_);

    int totalStride = 0;
    for(int i = 0; i < CountParam; ++i)
        totalStride += VertAttrbs.ValuesPerVertex[i];

    int currentStride = 0;
    for(int i = 0; i < CountParam; ++i) {
        intptr_t firstPtr = currentStride;
        glVertexAttribPointer(i, VertAttrbs.ValuesPerVertex[i], VertAttrbs.VertexType, GL_FALSE, totalStride * sizeof(T), (void*)(firstPtr * sizeof(T)));
        glEnableVertexAttribArray(i);

        currentStride += VertAttrbs.ValuesPerVertex[i];
    }


    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(T), verts.data(), GL_STATIC_DRAW);

    glDrawArrays(mode, 0, verts.size() / totalStride);
    glBindVertexArray(0);
}


void LifeGame::initRender() {
    CellShader_      = GeneralApp::Shader("../shaders/FullscreenTriangle.vert", "../shaders/FullscreenTriangle.frag");
    
    GridLinesShader_ = GeneralApp::Shader("../shaders/Grid.vert", "../shaders/Grid.frag");


    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);


    totalCells_.resize(gridWidth_ * gridHeight_);

    glGenBuffers(1, &SSBO_);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalCells_.size() * sizeof(totalCells_[0]), totalCells_.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO_);    
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



    glGenTextures(1, &backgroundTextureHandle_);
    glBindTexture(GL_TEXTURE_2D, backgroundTextureHandle_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("../textures/water.jpg", &width, &height, &nrChannels, 0);
    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Failed to create texture from image: " << "water.jpeg" << std::endl;
    }
    stbi_image_free(data);


    SoundEngine_->play2D("../audio/scott-buckley-jul.mp3", true);
}


void LifeGame::resetSSBO() {

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memset(p, 0, totalCells_.size() * sizeof(totalCells_[0]));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}



void LifeGame::updateSSBO() {

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p, totalCells_.data(), totalCells_.size() * sizeof(totalCells_[0]));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}



void LifeGame::drawAliveCells() {
    CellShader_.use();
    CellShader_.setIvec2("gridSize", glm::ivec2(gridWidth_, gridHeight_));
    CellShader_.setIvec2("winSize", glm::ivec2(windowWidth_, windowHeight_));


    CellShader_.setMat4("transform", currentTransform_);
    CellShader_.setMat4("transformInv", currentTransformInv_);
    
    if((GameState_ == LIFEGAME_STATE::RUN) || (GameState_ == LIFEGAME_STATE::STOP && RecalculateCells_)) {
        if(RecalculateCells_)
            RecalculateCells_ = false;

        updateCellsFunc(totalCells_, gridWidth_, gridHeight_, &renderProcessInfo, updateFlag_);
        updateSSBO();

        if(updateFlag_ != UPDATE_CELLS_MPI_END)
            updateFlag_ = UPDATE_CELLS_MPI_START;
    }


    glBindTexture(GL_TEXTURE_2D, backgroundTextureHandle_);
    drawShapes<float, 2>(GL_TRIANGLES, FullscreenTriangleVerts_, RectAttrDesc);
}


void LifeGame::drawGrid() {
    GridLinesShader_.use();

    GridLinesShader_.setMat4("transform", currentTransform_);

    gridLines_.clear();
    for(int i = 0; i < gridWidth_; ++i) {

        gridLines_.push_back(-1.0f + (2.0f * i) / gridWidth_);
        gridLines_.push_back(-1.0f);
        gridLines_.push_back(0.0f);
        gridLines_.push_back(0.0f);
        gridLines_.push_back(0.0f);

        gridLines_.push_back(-1.0f + (2.0f * i) / gridWidth_);
        gridLines_.push_back(1.0f);
        gridLines_.push_back(0.0f);
        gridLines_.push_back(0.0f);
        gridLines_.push_back(0.0f);
    }
    for(int i = 0; i < gridHeight_; ++i) {

        gridLines_.push_back(-1.0f);
        gridLines_.push_back(-1.0f + (2.0f * i) / gridHeight_);
        gridLines_.push_back(0.0f);
        gridLines_.push_back(0.0f);
        gridLines_.push_back(0.0f);

        gridLines_.push_back(1.0f);
        gridLines_.push_back(-1.0f + (2.0f * i) / gridHeight_);
        gridLines_.push_back(0.0f);
        gridLines_.push_back(0.0f);
        gridLines_.push_back(0.0f);

    }    
    glLineWidth(gridLinesThickness_);
    drawShapes<float, 2>(GL_LINES, gridLines_, LineAttrDesc);


    borderLines_.clear();

    borderLines_.push_back(-1.0f);
    borderLines_.push_back(-1.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);

    borderLines_.push_back(-1.0f);
    borderLines_.push_back(1.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);

    borderLines_.push_back(1.0f);
    borderLines_.push_back(1.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);

    borderLines_.push_back(1.0f);
    borderLines_.push_back(-1.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);

    borderLines_.push_back(-1.0f);
    borderLines_.push_back(-1.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);
    borderLines_.push_back(0.0f);


    glLineWidth(borderLinesThickness_);
    drawShapes<float, 2>(GL_LINE_STRIP, borderLines_, LineAttrDesc);
}
