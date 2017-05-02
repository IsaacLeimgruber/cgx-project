#pragma once
#include <map>
#include <glm/gtc/type_ptr.hpp>

struct LightProgramIds{
    GLuint La_id, Ld_id, Ls_id, lightPos_id;
};

class Light{
private:
    glm::vec3 La;
    glm::vec3 Ld;
    glm::vec3 Ls;
    glm::vec3 lightPos;

    std::map<GLuint, LightProgramIds> programToIds;

public:

    Light(glm::vec3 lightPos =  glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3 La =        glm::vec3(0.35, 0.35, 0.35),
          glm::vec3 Ld =        glm::vec3(1.1, 1.1, 1.1),
          glm::vec3 Ls =        glm::vec3(1.0f, 1.0f, 1.0f))
    {
        this->lightPos = lightPos;
        this->La = La;
        this->Ld = Ld;
        this->Ls = Ls;
    }

    glm::vec3 getPos(){
        return this->lightPos;
    }

    void setPos(glm::vec3 pos){
        this->lightPos = pos;
    }

    // pass light properties to the shader
    void registerProgram(GLuint program_id) {

        LightProgramIds registeredIds;

        glUseProgram(program_id);

        registeredIds.lightPos_id = glGetUniformLocation(program_id, "lightPos");

        registeredIds.La_id = glGetUniformLocation(program_id, "La");
        registeredIds.Ld_id = glGetUniformLocation(program_id, "Ld");
        registeredIds.Ls_id = glGetUniformLocation(program_id, "Ls");

        glUniform3fv(registeredIds.lightPos_id, ONE, glm::value_ptr(lightPos));
        glUniform3fv(registeredIds.La_id, ONE, glm::value_ptr(La));
        glUniform3fv(registeredIds.Ld_id, ONE, glm::value_ptr(Ld));
        glUniform3fv(registeredIds.Ls_id, ONE, glm::value_ptr(Ls));

        programToIds[program_id] = registeredIds;
    }

    void updatePosUniform(GLuint programId){
        LightProgramIds pids = programToIds[programId];
        glUniform3fv(pids.lightPos_id, ONE, glm::value_ptr(lightPos));
    }
};
