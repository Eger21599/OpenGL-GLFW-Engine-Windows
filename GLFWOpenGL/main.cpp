#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Header files/stb_image.h"
#include "Header files/Shader.h"
#include "Header files/Model.h"

float windowWidth = 1280;
float windowHeight = 720;

glm::vec3 lightSourcePos(1.2f, 1.0f, -3.0f);

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = windowWidth / 2.0;
float lastY = windowHeight / 2.0;
float fov = 45.0f;

int Shader::numberOfSpotLights = 0;
int Shader::numberOfPointLights = 1;

enum class types
{
    PNG,
    JPG
};

unsigned int LoadTexture(const char* filePath, types typeOfFile)
{
    unsigned int texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    int width, height, nrChannels;
    unsigned char *data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (data)
    {
        if(typeOfFile == types::PNG)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        if(typeOfFile == types::JPG)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return texture;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 2.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.07f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void error(int error, const char* description)
{
    fputs(description, stderr);
}

int main()
{
    glfwSetErrorCallback(error);

    if (!glfwInit())
        exit(1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "GLFW Test", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window); //GLFW context

    glfwSwapInterval(0); //vsync
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glEnable(GL_DEPTH_TEST);

    if (glewInit() != GLEW_OK)
    {
        printf("Error: glew initialise error!\n");
        glfwTerminate();
        exit(1);
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    glewExperimental = GL_TRUE;
    stbi_set_flip_vertically_on_load(true);
    glGetError();

    Model backpack("../Models/Backpack/backpack.obj");

    Shader::ShaderProgramSource source = Shader::loadShaderFromFile("../Shaders/basic.shader");
    unsigned int basicShader = Shader::CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(basicShader);

    unsigned int numberOfSpotLightsLocation = glGetUniformLocation(basicShader, "numberOfSpotLights");
    glUniform1i(numberOfSpotLightsLocation, Shader::numberOfSpotLights);

    unsigned int numberOfPointLightsLocation = glGetUniformLocation(basicShader, "numberOfPointLights");
    glUniform1i(numberOfPointLightsLocation, Shader::numberOfPointLights);

    Shader::setFloat(basicShader, "shininess", 32.0f);

    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //lightSourcePos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
        //lightSourcePos.y = sin(glfwGetTime() / 2.0f) * 1.0f;
        //lightSourcePos.z = glfwGetTime() / 2.0f;

        glUseProgram(basicShader);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        unsigned int modelLoc = glGetUniformLocation(basicShader, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        unsigned int projLocation = glGetUniformLocation(basicShader, "projection");
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projection));

        unsigned int viewLoc = glGetUniformLocation(basicShader, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        Shader::setVec3(basicShader, "viewPos", cameraPos);

///////////////////////////////////////////////////////////// LIGHT SETTINGS //////////////////////////////////////////////////////////////////////////////////////////////////////////

        Shader::setDirLightIntensity(basicShader, lightSourcePos, glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f),
                                                 glm::vec3(1.0f, 1.0f, 1.0f));

        /*Shader::setSpotLightIntensity(basicShader, 0, cameraPos, cameraFront,
                                                   glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)),
                                                   1.0f, 0.22f, 0.20f,
                                                   glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
                                                   glm::vec3(1.0f, 1.0f, 1.0f));*/

        Shader::setPointLightIntensity(basicShader, 0, glm::vec3(0.7f,  0.2f,  2.0f), glm::vec3(0.05f, 0.05f, 0.05f),
                                                     glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f),
                                                     1.0f, 0.09f, 0.032f);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        backpack.Draw(basicShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(basicShader);

    glfwTerminate();
    return 0;
}