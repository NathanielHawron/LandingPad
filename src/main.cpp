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
#include "filters/noise.h"

#include "controls.h"

struct rgba{
    uint8_t r, g, b, a;
    operator const uint32_t() const {return *(const uint32_t*)this;}
};

constexpr float RADIUS = 1.0f;
constexpr rgba PAD_COLOR = rgba{0,10,150,255};




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
    glfwSetKeyCallback(window,controls::key_callback);
    glfwSetCursorPosCallback(window,controls::cursor_position_callback);
    

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cout << "[Error]: Failed to initialize OpenGL context" << std::endl;
    }
    glEnable(GL_ALPHA);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    graphics::Shader shader("texture");
    graphics::Image image(2048,2048);
    filter::circle(image, PAD_COLOR, rgba{0,0,0,255}, 200, true, rgba{0,0,0,0});
    filter::noise(image, rgba{25,25,25,0});
    graphics::Texture texture(image);

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
        {{-1.0f,-1.0f,0.0f},       {0,0}},
        {{1.0f,1.0f,0.0f},   {65535,65535}},
        {{1.0f,-1.0f,0.0f},     {65535,0}},
        {{-1.0f,1.0f,0.0f},     {0,65535}}
    };
    GLuint indices[6] = {
        2,1,0,
        0,1,3
    };

    Mesh_t mesh = Mesh_t(sizeof(vertex)/4);
    mesh.add(positions,indices,4,6);

    renderable.loadVertexData(mesh.getVertices(), mesh.getVertexCount()*sizeof(vertex));
    renderable.loadIndexData(mesh.getIndices(), mesh.getIndexCount());

    glm::vec3 pos = glm::vec3(0,0,0);

    graphics::FBO frameBuffer = graphics::FBO(1000,1000);

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        glm::vec2 forwards{-std::sin(controls::mouseX),std::cos(controls::mouseX)};
        if(controls::controls & controls::FORWARDS){
            pos.x += forwards.x*controls::speed;
            pos.z += forwards.y*controls::speed;
        }
        if(controls::controls & controls::BACKWARDS){
            pos.x -= forwards.x*controls::speed;
            pos.z -= forwards.y*controls::speed;
        }
        if(controls::controls & controls::RIGHT){
            pos.z += forwards.x*controls::speed;
            pos.x -= forwards.y*controls::speed;
        }
        if(controls::controls & controls::LEFT){
            pos.z -= forwards.x*controls::speed;
            pos.x += forwards.y*controls::speed;
        }
        if(controls::controls & controls::UP){
            pos.y += controls::speed;
        }
        if(controls::controls & controls::DOWN){
            pos.y -= controls::speed;
        }


        {
            glViewport(0,0,2048,2048);

            frameBuffer.bind();
            shader.bind();
            texture.bind(0);
            shader.setUniform1i("u_Texture", 0);
            renderable.bindBuffers();

            glClearColor(0.0f,0.0f,0.0f,0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 modelMat = glm::translate(glm::mat4(1.0f),glm::vec3(0,-0.5f,0));
            glm::mat4 viewMat = glm::rotate(glm::mat4(1.0f),(float)controls::mouseX,glm::vec3(0,1,0))*glm::rotate(glm::mat4(1.0f),(float)controls::mouseY,glm::vec3(forwards.y,0,-forwards.x))*glm::translate(glm::mat4(1.0f),pos);
            glm::mat4 projMat = glm::perspective(3.1415926f*0.5f, (float)950 / (float)950, 0.1f, 300.0f);
            glm::mat4 vp = projMat*viewMat;
            shader.setUniformMat4f("u_M",modelMat);
            shader.setUniformMat4f("u_VP",vp);
            renderable.render();

            frameBuffer.unbind();
        }

        {
            glViewport(0,0,950,950);
            shader.bind();
            frameBuffer.bindTexture(0);
            shader.setUniform1i("u_Texture", 0);
            renderable.bindBuffers();

            glClearColor(0.0f,0.0f,0.0f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 modelMat = glm::translate(glm::mat4(1.0f),glm::vec3(0,0,0));
            glm::mat4 viewMat = glm::mat4(1.0f);
            glm::mat4 projMat = glm::ortho(-1.0f,1.0f,-1.0f,1.0f);
            glm::mat4 vp = projMat*viewMat;
            shader.setUniformMat4f("u_M",modelMat);
            shader.setUniformMat4f("u_VP",vp);
            renderable.render();
       }



        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
}