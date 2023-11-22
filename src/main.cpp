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
#include "image.h"



constexpr float RADIUS = 1.0f;
constexpr rgba PAD_COLOR = rgba{0,10,150,255};
constexpr rgba BORDER_COLOR = rgba{0,0,0,255};
constexpr int TOLERANCE = 55;
constexpr float FOV = 3.1415926f*0.5f;


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
    filter::circle(image, PAD_COLOR, rgba{0,0,0,255}, 475, true, rgba{0,0,0,0});
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
        {{-RADIUS,-RADIUS,0.0f},       {0,0}},
        {{RADIUS,RADIUS,0.0f},   {65535,65535}},
        {{RADIUS,-RADIUS,0.0f},     {65535,0}},
        {{-RADIUS,RADIUS,0.0f},     {0,65535}}
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

    graphics::FBO frameBuffer = graphics::FBO(950,950);

    rgba *pixels = new rgba[950*950*4];

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
            glViewport(0,0,950,950);

            frameBuffer.bind();
            shader.bind();
            texture.bind(0);
            shader.setUniform1i("u_Texture", 0);
            renderable.bindBuffers();

            glClearColor(0.0f,0.0f,0.0f,0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 modelMat = glm::rotate(glm::mat4(1.0f),1.57079633f,glm::vec3(1,0,0));
            glm::mat4 viewMat = glm::rotate(glm::mat4(1.0f),(float)controls::mouseX,glm::vec3(0,1,0))*glm::rotate(glm::mat4(1.0f),(float)controls::mouseY,glm::vec3(forwards.y,0,-forwards.x))*glm::translate(glm::mat4(1.0f),pos);
            glm::mat4 projMat = glm::perspective(FOV, (float)950 / (float)950, 0.1f, 300.0f);
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

            glClearColor(0.0f,0.5f,0.0f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 modelMat = glm::mat4(1.0f);
            glm::mat4 viewMat = glm::mat4(1.0f);
            glm::mat4 projMat = glm::ortho(-1.0f,1.0f,-1.0f,1.0f);
            glm::mat4 vp = projMat*viewMat;
            shader.setUniformMat4f("u_M",modelMat);
            shader.setUniformMat4f("u_VP",vp);
            renderable.render();
        }

        {
            glReadPixels(0,0,950,950,GL_RGBA,GL_UNSIGNED_BYTE,pixels);

            uint32_t maxX = 0, minX = 950, maxY = 0, minY = 0;
            bool b = false;
            for(int x=0;x<950 && !b;++x){
                for(int y=0;y<950 && !b;++y){
                    rgba col = pixels[x + y*950];
                    if(col.equals(BORDER_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                        minX = x;
                        b = true;
                    }
                }
            }
            b = false;
            for(int x=949;x>=0 && !b;--x){
                for(int y=0;y<950 && !b;++y){
                    rgba col = pixels[x + y*950];
                    if(col.equals(BORDER_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                        maxX = x;
                        b = true;
                    }
                }
            }
            b = false;
            for(int y=0;y<950 && !b;++y){
                for(int x=0;x<950 && !b;++x){
                    rgba col = pixels[x + y*950];
                    if(col.equals(BORDER_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                        minY = y;
                        b = true;
                    }
                }
            }
            b = false;
            for(int y=949;y>=0 && !b;--y){
                for(int x=0;x<950 && !b;++x){
                    rgba col = pixels[x + y*950];
                    if(col.equals(BORDER_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                        maxY = y;
                        b = true;
                    }
                }
            }

            uint32_t dx = maxX - minX;
            uint32_t dy = maxY - minY;

            double calculatedDistance = 0.0;
            int accuracy = 0;

            while(dx > 0 && dy > 0){
                accuracy = 1;
                Image padSection = Image(pixels,{minX,minY},{maxX,maxY}, 950);

                //Find top of landing pad
                point centerTop;
                if(!padSection.linearSearch({dx/2,0},{0,1},centerTop,PAD_COLOR,TOLERANCE,{0,0,0,0})){break;}

                //Find direction of major
                bool rMajor;
                if(!padSection.xDirOfMax(centerTop,rMajor,PAD_COLOR,BORDER_COLOR,TOLERANCE,{0,0,0,0})){break;}
                    
                //Approximate first major radius
                point center = {dx/2,dy/2};
                float radiusSquared;
                point major1;
                int checkCount = (padSection.width+padSection.height)/20;
                if(!padSection.traceBorder(centerTop,center,rMajor,false,checkCount,major1,radiusSquared,BORDER_COLOR,TOLERANCE,{0,0,0,0})){break;}

                //Approximate second major radius
                point major2Guess = {dx-major1.x,dy-major1.y};

                //Move second major radius guess onto pad
                direction c_m1 = major1-center;
                point major2Guess2;
                if(!padSection.linearSearch(major2Guess,{c_m1.dx>0?1:-1,c_m1.dy>0?1:-1},major2Guess2,PAD_COLOR,TOLERANCE,{0,0,0,0})){break;}

                //Find better approximation of second major radius
                point major2;
                float r2Squared;
                if(!padSection.traceBorder(major2Guess2,major1,!rMajor,false,checkCount,major2,r2Squared,BORDER_COLOR,TOLERANCE,{0,0,0,0})){break;}

                //Find better approximation of first major radius
                if(!padSection.traceBorder(major1,major2,rMajor,false,checkCount,major1,radiusSquared,BORDER_COLOR,TOLERANCE,{0,0,0,0})){break;}

                //Calculate major diameter
                calculatedDistance = (950.0/std::sqrt((major2-major1).magSq())) / std::tan(0.5*FOV);
                accuracy = 2;
                
                if(controls::controls & controls::SS){
                    padSection.save("landing_pad");
                }
                break;
            }
            if(accuracy == 1){
                calculatedDistance = (950.0/(std::max(dx,dy))) / std::tan(0.5*FOV);
            }
            double lenActual = std::sqrt(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z);
            switch(accuracy){
                case -1:break;
                case 0:{
                    std::cout << "Distance (calculated/actual): NA / " << lenActual << std::endl;
                }break;
                case 1:{
                    std::cout << "Distance (calculated/actual): " << calculatedDistance << " / " << lenActual << " +-100%" << std::endl;
                }break;
                case 2:{
                    std::cout << "Distance (calculated/actual): " << calculatedDistance << " / " << lenActual << std::endl;
                }
            }
        }

        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
}