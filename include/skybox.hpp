#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include "util.hpp"

class Skybox {
public:
    Skybox();
    ~Skybox();
    void render(const glm::mat4& view, const glm::mat4& projection);
    unsigned int loadCubemap(const std::vector<std::string>& faces);

private:
    unsigned int skyboxVAO, skyboxVBO, cubemapTexture;
    unsigned int shaderProgram;
    void setupSkybox();
};

#endif // SKYBOX_HPP