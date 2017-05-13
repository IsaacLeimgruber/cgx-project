#pragma once
#include <map>
#include <glm/gtc/type_ptr.hpp>

struct LightProgramIds{
    GLuint La_id, Ld_id, Ls_id, lightPos_id, lightPosCameraTranslated_id;
};

class Light{
private:
    glm::vec3 defaultLa = glm::vec3(0.35, 0.35, 0.35);
    glm::vec3 defaultLd = glm::vec3(1.1, 1.1, 1.1);
    glm::vec3 defaultLs = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 La = defaultLa;
    glm::vec3 Ld = defaultLd;
    glm::vec3 Ls = defaultLs;
    glm::vec3 lightPos;
    glm::vec3 lightPosCameraTranslated;

    std::map<GLuint, LightProgramIds> programToIds;

public:

    Light(glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f))
    {
        this->lightPos = lightPos;
        this->lightPosCameraTranslated = lightPos;
    }

    glm::vec3 getPos(){
        return this->lightPos;
    }

    void setPos(glm::vec3 pos){
        this->lightPos = pos;
    }

    void setLightPosCameraTranslated(glm::vec3 pos){
        this->lightPosCameraTranslated = pos;
    }

    void setAmbientIntensity(glm::vec3 c){
        La = c;
    }

    void setDiffuseIntensity(glm::vec3 c){
        Ld = c;
    }

    void setSpecularIntensity(glm::vec3 c){
        Ls = c;
    }

    void resetAmbientIntensity(){
        La = defaultLa;
    }

    void resetDiffuseIntensity(){
        Ld = defaultLd;
    }

    void resetSpecularIntensity(){
        Ls = defaultLs;
    }

    glm::vec3 getDefaultAmbientIntensity(){
        return defaultLa;
    }

    glm::vec3 getDefaultDiffuseIntensity(){
        return defaultLd;
    }

    glm::vec3 getDefaultSpecularIntensity(){
        return defaultLs;
    }

    // pass light properties to the shader
    void registerProgram(GLuint program_id) {

        LightProgramIds registeredIds;

        glUseProgram(program_id);

        registeredIds.lightPosCameraTranslated_id = glGetUniformLocation(program_id, "lightPos");

        registeredIds.La_id = glGetUniformLocation(program_id, "La");
        registeredIds.Ld_id = glGetUniformLocation(program_id, "Ld");
        registeredIds.Ls_id = glGetUniformLocation(program_id, "Ls");

        glUniform3fv(registeredIds.lightPosCameraTranslated_id, ONE, glm::value_ptr(lightPosCameraTranslated));
        glUniform3fv(registeredIds.La_id, ONE, glm::value_ptr(La));
        glUniform3fv(registeredIds.Ld_id, ONE, glm::value_ptr(Ld));
        glUniform3fv(registeredIds.Ls_id, ONE, glm::value_ptr(Ls));

        programToIds[program_id] = registeredIds;
    }

    void updatePosUniform(GLuint programId){
        LightProgramIds pids = programToIds[programId];
        glUniform3fv(pids.lightPosCameraTranslated_id, ONE, glm::value_ptr(lightPosCameraTranslated));
    }

    void updateProgram(GLuint programId){
        LightProgramIds pids = programToIds[programId];
        glUniform3fv(pids.lightPosCameraTranslated_id, ONE, glm::value_ptr(lightPosCameraTranslated));
        glUniform3fv(pids.La_id, ONE, glm::value_ptr(La));
        glUniform3fv(pids.Ld_id, ONE, glm::value_ptr(Ld));
        glUniform3fv(pids.Ls_id, ONE, glm::value_ptr(Ls));
    }
};
