#pragma once
#include <glm/gtc/type_ptr.hpp>

class Material {

private:
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    float alpha;

public:

    Material(
            glm::vec3 ka = glm::vec3(0.18f, 0.1f, 0.1f),
            glm::vec3 kd = glm::vec3(0.9f, 0.9f, 0.9f),
            glm::vec3 ks = glm::vec3(0.9f, 0.9f, 0.9f),
            float alpha = 100.0){
        this->ka = ka;
        this->kd = kd;
        this->ks = ks;
        this->alpha = alpha;
    }

    // pass material properties to the shaders
    void Setup(GLuint program_id) {
        glUseProgram(program_id);

        GLuint ka_id = glGetUniformLocation(program_id, "ka");
        GLuint kd_id = glGetUniformLocation(program_id, "kd");
        GLuint ks_id = glGetUniformLocation(program_id, "ks");
        GLuint alpha_id = glGetUniformLocation(program_id, "alpha");

        glUniform3fv(ka_id, ONE, glm::value_ptr(ka));
        glUniform3fv(kd_id, ONE, glm::value_ptr(kd));
        glUniform3fv(ks_id, ONE, glm::value_ptr(ks));
        glUniform1f(alpha_id, alpha);
    }
};
