#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>
#include "../light/light.h"
#include "../light/lightable.h"
#include "../utils.h"

#include "Mesh.h"

GLint TextureFromFile(const char* path, string directory);

class Model: public ILightable
{
public:
    /*  Functions   */
    // Constructor, expects a filepath to a 3D model.
    Model(GLchar* path)
    {
        this->modelPath = path;
    }

    void Init(GLuint shaderProgram, GLuint shadowTexture, int fogStop, int fogLength){
        this->shaderProgram = shaderProgram;
        this->loadModel();

        glUseProgram(shaderProgram);

        MVP_id = glGetUniformLocation(shaderProgram, "MVP");
        MV_id = glGetUniformLocation(shaderProgram, "MV");
        NORMALM_id = glGetUniformLocation(shaderProgram, "NORMALM");
        SHADOWMVP_id = glGetUniformLocation(shaderProgram, "SHADOWMVP");
        mirrorPass_id = glGetUniformLocation(shaderProgram, "mirror_pass");
        shadowTexture_id = glGetUniformLocation(shaderProgram, "shadowMap");
        translationToSceneCenter_id = glGetUniformLocation(shaderProgram, "translationToSceneCenter");
        glUniform1f(glGetUniformLocation(shaderProgram, "threshold_vpoint_World_F"), fogStop - fogLength);
        glUniform1f(glGetUniformLocation(shaderProgram, "max_vpoint_World_F"), fogStop);

        this->shadowTexture_id = shadowTexture;
        GLuint shadowMapLocation = glGetUniformLocation(shaderProgram, "shadowMap");
        glUniform1i(shadowMapLocation, 7);
    }

    // Draws the model, and thus all its meshes
    void Draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
              const glm::mat4 &MV = IDENTITY_MATRIX,
              const glm::mat4 &NORMALM = IDENTITY_MATRIX,
              const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
              const glm::vec2 &translationToSceneCenter = glm::vec2(0.0, 0.0),
              bool mirrorPass = false)
    {
        glUseProgram(this->shaderProgram);

        glUniformMatrix4fv(MVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(MV_id, ONE, DONT_TRANSPOSE, glm::value_ptr(MV));
        glUniformMatrix4fv(NORMALM_id, ONE, DONT_TRANSPOSE, glm::value_ptr(NORMALM));
        glUniformMatrix4fv(SHADOWMVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(SHADOWMVP));
        glUniform2fv(translationToSceneCenter_id, 1, glm::value_ptr(translationToSceneCenter));
        glUniform1i(mirrorPass_id, mirrorPass);

        light->updateProgram(this->shaderProgram);

        glActiveTexture(GL_TEXTURE0 + 7);
        glBindTexture(GL_TEXTURE_2D, this->shadowTexture_id);

        for(GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Draw(this->shaderProgram);

        glActiveTexture(GL_TEXTURE0 + 7);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void useLight(Light* l){
        this->light = l;
        light->registerProgram(shaderProgram);
    }
    
private:
    /*  Model Data  */
    Light* light;
    GLuint shaderProgram;
    GLuint MVP_id, MV_id, NORMALM_id, SHADOWMVP_id, mirrorPass_id, translationToSceneCenter_id;
    GLuint shadowTexture_id;
    GLchar* modelPath;
    vector<Mesh> meshes;
    string directory;
    vector<Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

    /*  Functions   */
    // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel()
    {
        string path = this->modelPath;
        // Read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        // Check for errors
        if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // Retrieve the directory path of the filepath
        this->directory = path.substr(0, path.find_last_of('/'));

        // Process ASSIMP's root node recursively
        this->processNode(scene->mRootNode, scene);
        int breakpoint = 0;
    }

    // Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // Process each mesh located at the current node
        for(GLuint i = 0; i < node->mNumMeshes; i++)
        {
            // The node object only contains indices to index the actual objects in the scene. 
            // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
            this->meshes.push_back(this->processMesh(mesh, scene));			
        }
        // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(GLuint i = 0; i < node->mNumChildren; i++)
        {
            this->processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // Data to fill
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Texture> textures;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        bool textured = false;

        // Walk through each of the mesh's vertices
        for(GLuint i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // Positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // Normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // Texture Coordinates
            if(mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // Retrieve all indices of the face and store them in the indices vector
            for(GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // Process materials
        if(mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
            // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
            // Same applies to other texture as the following list summarizes:
            // Diffuse: texture_diffuseN
            // Specular: texture_specularN
            // Normal: texture_normalN

            aiColor3D aiDiffuseColor (0.f,0.f,0.f);
            material->Get(AI_MATKEY_COLOR_DIFFUSE,aiDiffuseColor);
            aiColor3D aiSpecularColor (0.f,0.f,0.f);
            material->Get(AI_MATKEY_COLOR_SPECULAR,aiSpecularColor);

            diffuseColor = glm::vec3(aiDiffuseColor.r, aiDiffuseColor.g, aiDiffuseColor.b);
            specularColor = glm::vec3(aiSpecularColor.r, aiSpecularColor.g, aiSpecularColor.b);

            // 1. Diffuse maps
            vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            // 2. Specular maps
            vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

            if(diffuseMaps.size() > 0 || specularMaps.size() > 0){
                textured = true;
            }
        }
        
        // Return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures, diffuseColor, specularColor, textured);
    }

    // Checks all material textures of a given type and loads the textures if they're not loaded yet.
    // The required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            GLboolean skip = false;
            for(GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // If texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }
};




GLint TextureFromFile(const char* path, string directory)
{
     //Generate texture ID and load texture data 
    string filename = string(path);
    size_t dotPos = filename.find_first_of(".");
    filename = filename.substr(0, dotPos);

    // Due to bugg and lack of time we force tga textures
    if(dotPos != string::npos){
        filename.append(".tga");
    }

    // We don't use directories in output folder
    //filename = directory + '/' + filename;
    return Utils::loadImage(filename.c_str());
}

