#include <Eigen/Dense>
#include "Debugger.h"
#include "Default3d.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include "level.h"
#include "player_camera.h"
#include "debug_camera.h"
#include "Header.h"

#include <chrono>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    
    //Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1000, 1000, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    //initialization is horrific rn needs a major improvement (ordering things is the main problem so perhaps it should be done in set phases)
    
    //HboxGraphics hboxGraphics;

    std::vector<InternalObject> bases;
    std::vector<Debugger> debugs;
    std::vector<InternalObject*> layout;//this being a vector which rearranges is horrible lol
    /*int n_debuggers = 10;
    for (int i = 0; i < n_debuggers; i++) {
        bases.emplace_back(Eigen::Matrix4f::Identity(), i + 1);
        bases[i].rotateY(2 * M_PI / n_debuggers * i);
        bases[i].update(window);
    }
    for (int i = 0; i < n_debuggers; i++) {
        debugs.emplace_back(Eigen::Matrix4f::Identity(), i + n_debuggers + 1, bases[i]);
        debugs[i].translate(Eigen::Vector3f(0, 0, 5));
    }
    for (auto& dbg : debugs) {
        layout.push_back(&dbg);
    }*/

    //Debugger right((Eigen::Matrix4f()<<1, 0, 0, .5, 0, 1, 0, 0, 0, 0, 1, .5, 0, 0, 0, 1).finished(), 2, default3d);
    ZMapper zmapper;
    Level room1(layout,window,Model("spiral_staircase_cult_exit.obj"),Texture("rocky.jpg"),"spiral staircase");
    Level room2(layout, window, Model("cult_exit_landing.obj"), Texture("rocky.jpg"), "spiral staircase");
    Level room3(layout, window, Model("cult_exit_hallway.obj"), Texture("rocky.jpg"), "spiral staircase");
    room1.addNeighbor(&room2);
    room1.addNeighbor(&room3);
    room2.addNeighbor(&room1);
    room3.addNeighbor(&room1);

    PlayerCamera camera(1.0,&room1.getZmap(),window,"player1cam");
    //Camera camera((Eigen::Matrix4f() << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1).finished(), -1);
    camera.activateKeyInput(window);
    Default3d default3d(camera, .1, 100, 90);
    for (auto& dbg : debugs) {
        default3d.add(dbg);
    }

    DebugCamera center(&room1,"obamna");
    room1.add(center);
    room1.add(camera); //shouldnt be part of the layout
    default3d.add(room1);
    default3d.add(room2);
    default3d.add(room3);

    default3d.add(center);
    room1.initZmap(zmapper,6);
    room2.initZmap(zmapper, 2);
    room3.initZmap(zmapper, 2);

    glfwSetWindowSize(window, 1600, 1200);

    //hboxGraphics.add(0,left.getHboxDbgGrobj());
    //hboxGraphics.add(1, right.getHboxDbgGrobj());
    camera.setPlayer(&center);
    //left.addCollidor(right);

    //need to add objects to the shaders manually

    std::vector<uint8_t> screenshot_buffer(1200*1600*3);
    std::fill(screenshot_buffer.begin(), screenshot_buffer.end(),255);
    int screenshot_width, screenshot_height;
    int screenshot_buffers = 3;
    std::string screenshot_fname;

    glfwGetWindowSize(window, &screenshot_width, &screenshot_height);


    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {

        room1.update();

        // Render here

        
        if (camera.getScreenshotFlag()) {
            default3d.startScreenshot(screenshot_width, screenshot_height);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        default3d.drawAll();

        if (camera.getScreenshotFlag()) {
            default3d.finishScreenshot<uint8_t, GL_UNSIGNED_BYTE>(&screenshot_buffer,"screenshot.png");
            camera.clearScreenshotFlag();
            //stbi_write_png("screenshot.png", screenshot_width, screenshot_height, screenshot_buffers, screenshot_buffer.data(), screenshot_width * screenshot_buffers);
        } 

        //hboxGraphics.drawAll();

        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

