
#include "util.hpp"
#include "sphere.hpp"
#include "skybox.hpp"

using std::vector;


void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);


unsigned int VBO, VAO, EBO, normalVBO, uvVBO, textureID;

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

float worldAmbient = 0.1f;
float lightSourceAmbient = 0.95f;

glm::vec3 cameraPos(0.0f, 20.0f, 0.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 80.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;


struct Planet {
    glm::vec3 coordinates;
    int relativeTo;
    float scale;
    float rotationSpeed;
    float selfRotationSpeed;
    char* texturePath;
    int textureId;
    float ambientLight;
};

struct PlanetSystem {
    glm::vec3 centerOffset;
    float centerRotation;
};

glm::mat4 getPlanetModel(Planet p, PlanetSystem ps);
glm::mat4 getPlanetTransform(Planet p);

//    coordinates, relativeTo, scale, rotationSpeed, selfRotationSpeed, texturePath, textureId
vector<Planet> planets = {
    { glm::vec3(0.0f, 0.0f, 0.0f),  0, 2.0f, 50.0f, 20.0f, (char*)"resources/sun.jpg",         0, lightSourceAmbient },
    { glm::vec3(6.0f, 0.0f, 0.0f),  0, 0.6f, 70.0f, 10.0f, (char*)"resources/ceres_fictional.jpg", 0, worldAmbient },
    { glm::vec3(11.0f, 0.0f, 0.0f),  0, 0.8f, 20.0f, 30.0f, (char*)"resources/earth.jpg",      0, worldAmbient },
    { glm::vec3(2.0f, 0.0f, 0.0f),  2, 0.3f, 50.0f, 60.0f, (char*)"resources/moon.png",        0, worldAmbient },
    { glm::vec3(18.0f, 0.0f, 0.0f),  0, 1.3f, 60.0f, 15.0f, (char*)"resources/venus.jpg",      0, worldAmbient },
    { glm::vec3(24.0f, 0.0f, 0.0f), 0, 1.7f, 25.0f, 60.0f, (char*)"resources/jupiter.jpg",     0, worldAmbient },
};

vector<std::string> faces = {
    "resources/skybox/right.jpg",
    "resources/skybox/left.jpg",
    "resources/skybox/top.jpg",
    "resources/skybox/bottom.jpg",
    "resources/skybox/front.jpg",
    "resources/skybox/back.jpg"
};

float sOff = 60.0f;

vector<PlanetSystem> planetSystems = {
    { glm::vec3(0.0f, 0.0f, 0.0f), 0.0f },
    // { glm::vec3(sOff, 0.0f, 0.0f), 10.0f },
    // { glm::vec3(-sOff, 0.0f, 0.0f), 20.0f },
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Space", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int shaderProgram = loadShader("shader/shader_vert.glsl", "shader/shader_frag.glsl");

    // init planet textures
    Sphere sphere(1.0f, 60, 30, true, 2);
    for (auto &planet : planets) {
        planet.textureId = loadTexture(planet.texturePath);
    }


    // init skybox
    Skybox skybox;
    skybox.loadCubemap(faces);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &uvVBO);
    glGenBuffers(1, &EBO);


    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.getVertexSize(), sphere.getVertices(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.getNormalSize(), sphere.getNormals(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.getTexCoordSize(), sphere.getTexCoords(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.getIndexSize(), sphere.getIndices(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    unsigned int lightCountLoc = glGetUniformLocation(shaderProgram, "lightCount");
    unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "lights[0].pos");
    unsigned int lightColorLoc = glGetUniformLocation(shaderProgram, "lights[0].color");
    unsigned int ambientLightLoc = glGetUniformLocation(shaderProgram, "lights[0].ambient");

    unsigned int lightDistConstantLoc = glGetUniformLocation(shaderProgram, "lights[0].constant");
    unsigned int lightDistLinearLoc = glGetUniformLocation(shaderProgram, "lights[0].linear");
    unsigned int lightDistQuadraticLoc = glGetUniformLocation(shaderProgram, "lights[0].quadratic");

    unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    unsigned int textureLoc = glGetUniformLocation(shaderProgram, "texture1");

    // render loop
    while (!glfwWindowShouldClose(window)) {

        // precalc
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // light and camera
        glUniform1i(lightCountLoc, 1);

        glUniform3fv(lightPosLoc, 1, &lightPos[0]);
        glUniform3fv(lightColorLoc, 1, &lightColor[0]);
        glUniform3fv(viewPosLoc, 1, &cameraPos[0]);

        glUniform1f(lightDistConstantLoc, 1.0f);
        glUniform1f(lightDistLinearLoc, 0.03f);
        glUniform1f(lightDistQuadraticLoc, 0.003f);

        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f
        );

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);

        // print galaxies
        for (auto planetSystem : planetSystems) {
            for (auto planet : planets) {
                glm::mat4 model = getPlanetModel(planet, planetSystem);
                glUniform1f(ambientLightLoc, planet.ambientLight);
                glBindTexture(GL_TEXTURE_2D, planet.textureId);
                glUniform1i(textureLoc, 0);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, sphere.getIndexCount(), GL_UNSIGNED_INT, 0);
            }
        }

        // render skybox with negative depth test
        skybox.render(view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// recursive planet model
glm::mat4 getPlanetModel(Planet p, PlanetSystem ps) {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), ps.centerOffset);
    translation = glm::rotate(translation, glm::radians(ps.centerRotation), glm::vec3(1.0f, 0.0f, 0.0f));
    translation = glm::translate(translation, glm::vec3(getPlanetTransform(p)[3]));

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(p.selfRotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(p.scale));

    return translation * rotation * scale;
}

// recursive planet transform for chiuld planets
glm::mat4 getPlanetTransform(Planet p) {
    glm::mat4 translation = glm::mat4(1.0f);
    if (p.relativeTo != 0) {
        translation = getPlanetTransform(planets[p.relativeTo]);
    }
    translation = glm::rotate(translation, (float)glfwGetTime() * glm::radians(p.rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
    translation = glm::translate(translation, p.coordinates);
    translation = glm::rotate(translation, (float)glfwGetTime() * glm::radians(-p.rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
    return translation;
}

// process all input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 moveVec(0.0f, 0.0f, 0.0f);

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraSpeed = 2*cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveVec += cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveVec -= cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveVec -= glm::normalize(glm::cross(cameraFront, cameraUp));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveVec += glm::normalize(glm::cross(cameraFront, cameraUp));
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        moveVec -= cameraUp;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        moveVec += cameraUp;

    cameraPos += moveVec * cameraSpeed;
}

// process mouse movement
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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

    float sensitivity = 0.1f;
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

// process scroll for zoom
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)static_cast<float>(yoffset);
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 80.0f)
        fov = 80.0f;
}

