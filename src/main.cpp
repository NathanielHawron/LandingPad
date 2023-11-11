#include <iostream>
#include <cstdint>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

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
    bool equals(const rgba &col, uint8_t tr, uint8_t tg, uint8_t tb, uint8_t ta = 255) const {
        uint8_t dr = std::abs((int16_t)this->r - (int16_t)col.r), dg = std::abs((int16_t)this->g - (int16_t)col.g), db = std::abs((int16_t)this->b - (int16_t)col.b), da = std::abs((int16_t)this->a - (int16_t)col.a);
        return dr < tr && dg < tg && db < tb && da < ta;
    }
};

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

            int maxX = 0, minX = 950, maxY = 0, minY = 0;
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

            int dx = maxX - minX;
            int dy = maxY - minY;

            double calculatedDistance = 0.0;
            int accuracy = 0;

            if(dx > 0 && dy > 0){
                rgba *padSection = new rgba[dy*dx];
                // Copy subsection with just landing pad
                for(int y=0;y<dy;++y){
                    memcpy(padSection+y*dx,pixels+(y+minY)*950+minX,dx*4);
                }

                // Find diameters
                int cx = dx/2, cy = dy/2;
                int lx = cx, ly = dy-1;
                bool fail = false;

                // Find top of landing pad
                while(!padSection[lx + (--ly)*dx].equals(PAD_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                    padSection[lx + ly*dx] = {255,0,0,255};
                    if(ly<=1){
                        fail = true;
                        break;
                    }
                }
                padSection[lx + ly*dx] = {255,0,0,255};
                if(ly == 0){
                    fail = true;
                }
                if(!fail){
                    // Determine whether left is major or minor axis
                    bool lMajor;
                    fail = true;
                    while(--lx > 0){
                        if(!padSection[lx + ly*dx].equals(PAD_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                            lMajor = false;
                            break;
                        }
                        if(padSection[lx + (ly-1)*dx].equals(PAD_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                            lMajor = true;
                            break;
                        }
                        padSection[lx + ly*dx] = {255,255,0,255};
                    }
                    padSection[lx + ly*dx] = {255,255,0,255};
                    int rx = cx, ry = ly;
                    double longestSQ = 0.0, shortestSQ = 950.0*950.0;
                    int count = 0;

                    int &majorX = lMajor?lx:rx, &majorY = lMajor?ly:ry, &minorX = lMajor?rx:lx, &minorY = lMajor?ry:ly;

                    // Find major diameter
                    padSection[cx + cy*dx] = {255,255,255,255};
                    int h = 0;
                    while(majorX > 0 && majorX < dx-1 && count < 5){
                        majorX += lMajor?1:-1;
                        if(padSection[majorX + (majorY+h)*dx].equals(PAD_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                            while(majorY+h < dy-1 && padSection[majorX + (majorY+(++h))*dx].equals(PAD_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                                padSection[majorX + (majorY+h)*dx] = {255,0,255,255};
                                double dist = (majorX-cx)*(majorX-cx)+(majorY-cy)*(majorY-cy);
                                if(dist>longestSQ){
                                    longestSQ=dist;
                                    count = 0;
                                }else{
                                    if(++count > 5){
                                        break;
                                    }
                                }
                            }
                            --h;
                        }else{
                            while(majorY+h > 0 &&  !padSection[majorX + (majorY+(--h))*dx].equals(PAD_COLOR,TOLERANCE,TOLERANCE,TOLERANCE)){
                                padSection[majorX + (majorY+h)*dx] = {255,0,255,255};
                                double dist = (majorX-cx)*(majorX-cx)+(majorY-cy)*(majorY-cy);
                                if(dist>longestSQ){
                                    longestSQ=dist;
                                    count = 0;
                                }else{
                                    if(++count > 5){
                                        break;
                                    }
                                }
                            }
                            ++h;
                        }
                        padSection[majorX + (majorY+h)*dx] = {255,0,255,255};
                    }

                    if(longestSQ > 0){
                        accuracy = 2;
                        calculatedDistance = (1.1159)*475.0/std::sqrt(longestSQ);
                    }else{
                        accuracy = 1;
                        calculatedDistance = 475.0/(std::max(dx,dy));
                    }

                }

                //Save annotated screenshot of landing pad
                if(controls::controls & controls::SS){
                    stbi_write_png("landing_pad.png",dx,dy,4,padSection,4*dx);
                }
            }
            double lenActual = std::sqrt(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z);
            switch(accuracy){
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