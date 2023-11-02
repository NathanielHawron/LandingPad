#include <iostream>
#include <cstdint>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#include "graphicsLibrary/include/shader.h"
#include "graphicsLibrary/include/texture.h"
#include "graphicsLibrary/include/renderable.h"
#include "graphicsLibrary/include/mesh.h"
#include "graphicsLibrary/include/frame_buffer_object.h"


#include "filters/circle.h"

struct rgba{
    uint8_t r, g, b, a;
    operator uint32_t(){return *(uint32_t*)this;}
};

uint8_t controls = 0;
double mouseX = 0, mouseY = 0;
double sensitivity = 0.001;
double speed = 0.01;
constexpr uint8_t RIGHT =       0b000001;
constexpr uint8_t LEFT =        0b000010;
constexpr uint8_t FORWARDS =    0b000100;
constexpr uint8_t BACKWARDS =   0b001000;
constexpr uint8_t UP =          0b010000;
constexpr uint8_t DOWN =        0b100000;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS){
        switch(key){
            case GLFW_KEY_D:controls |= RIGHT;      break;
            case GLFW_KEY_A:controls |= LEFT;       break;
            case GLFW_KEY_W:controls |= FORWARDS;   break;
            case GLFW_KEY_S:controls |= BACKWARDS;  break;
            case GLFW_KEY_SPACE:controls |= DOWN;   break;
            case GLFW_KEY_LEFT_SHIFT:controls |= UP;break;
        }
    }else if(action == GLFW_RELEASE){
        switch(key){
            case GLFW_KEY_D:controls &= ~RIGHT;      break;
            case GLFW_KEY_A:controls &= ~LEFT;       break;
            case GLFW_KEY_W:controls &= ~FORWARDS;   break;
            case GLFW_KEY_S:controls &= ~BACKWARDS;  break;
            case GLFW_KEY_SPACE:controls &= ~DOWN;   break;
            case GLFW_KEY_LEFT_SHIFT:controls &= ~UP;break;
        }
    }
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    mouseX = xpos * sensitivity;
    mouseY = std::max(std::min(3.0,ypos*sensitivity),-3.0);
}

int main(){
    if(!glfwInit()){
        throw std::runtime_error("Failed to initialize GLFW");
        return EXIT_FAILURE;
    }

    graphics::Shader::shaderFolder = new std::filesystem::path("res/shaders/");

    GLFWwindow *window = glfwCreateWindow(950,950,"Window",nullptr,nullptr);
    if(!window){
        throw std::runtime_error("Failed to create Window");
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(window, 0, 0);
    glfwSetKeyCallback(window,key_callback);
    glfwSetCursorPosCallback(window,cursor_position_callback);
    

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cout << "[Error]: Failed to initialize OpenGL context" << std::endl;
    }
    glEnable(GL_ALPHA);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.5f, 0.8f, 1.0f);

    graphics::Shader shader("texture");
    graphics::Image image(2048,2048);
    filter::circle(image, rgba{200,150,0,255}, rgba{0,0,0,255}, 100, 0, true, rgba{0,0,0,0});
    graphics::Texture texture(image);
    graphics::FBO frameBuffer = graphics::FBO(1000,1000);

    graphics::Renderable renderable = graphics::Renderable();
    renderable.init();
    graphics::VertexBufferLayout layout;

    layout.push(GL_FLOAT,3);
    layout.push(GL_UNSIGNED_SHORT,2,GL_TRUE);

    renderable.setVBOLayout(layout);

    typedef graphics::Mesh<GLuint> Mesh_t;
    struct vertex{
        GLfloat pos[3];
        GLushort texCoords[2];
    };

    vertex positions[4] = {
        {{-1.0f,0.0f,-1.0f},       {0,0}},
        {{1.0f,0.0f,1.0f},   {65535,65535}},
        {{1.0f,0.0f,-1.0f},     {65535,0}},
        {{-1.0f,0.0f,1.0f},     {0,65535}}
    };
    GLuint indices[6] = {
        2,1,0,
        0,1,3
    };

    Mesh_t mesh = Mesh_t(sizeof(vertex)/4);
    mesh.add(positions,indices,4,6);

    renderable.loadVertexData(mesh.getVertices(), mesh.getVertexCount()*sizeof(vertex));
    renderable.loadIndexData(mesh.getIndices(), mesh.getIndexCount());

    glClearColor(0.0f,0.5f,0.0f,1.0f);
    glViewport(0,0,950,950);

    glm::vec3 pos = glm::vec3(0,0,0);

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        glm::vec2 forwards{-std::sin(mouseX),std::cos(mouseX)};
        if(controls & FORWARDS){
            pos.x += forwards.x*speed;
            pos.z += forwards.y*speed;
        }
        if(controls & BACKWARDS){
            pos.x -= forwards.x*speed;
            pos.z -= forwards.y*speed;
        }
        if(controls & RIGHT){
            pos.z += forwards.x*speed;
            pos.x -= forwards.y*speed;
        }
        if(controls & LEFT){
            pos.z -= forwards.x*speed;
            pos.x += forwards.y*speed;
        }
        if(controls & UP){
            pos.y += speed;
        }
        if(controls & DOWN){
            pos.y -= speed;
        }


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.bind();
        texture.bind(0);
        shader.setUniform1i("u_Texture", 0);
        renderable.bindBuffers();


        glm::mat4 modelMat = glm::translate(glm::mat4(1.0f),glm::vec3(0,-0.5f,0));
        glm::mat4 viewMat = glm::rotate(glm::mat4(1.0f),(float)mouseX,glm::vec3(0,1,0))*glm::rotate(glm::mat4(1.0f),(float)mouseY,glm::vec3(forwards.y,0,-forwards.x))*glm::translate(glm::mat4(1.0f),pos);
        glm::mat4 projMat = glm::perspective(3.1415926f*0.5f, (float)950 / (float)950, 0.1f, 300.0f);
        glm::mat4 vp = projMat*viewMat;
        shader.setUniformMat4f("u_M",modelMat);
        shader.setUniformMat4f("u_VP",vp);
        renderable.render();

        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
}