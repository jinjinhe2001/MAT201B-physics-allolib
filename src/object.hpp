#pragma once
#include <iostream>
#include <fstream>
#include "al/io/al_ControlNav.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Shader.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_DefaultShaders.hpp"
#include "al/graphics/al_DefaultShaderString.hpp"
#include "al/graphics/al_BufferObject.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "loader.hpp"

using namespace al;
class Object
{
public:
    Nav nav;
    VAOMesh mesh;
    Texture texture;
    Material material;
    ShaderProgram shader;
public:
    Object(const std::string meshPath = "", const std::string shaderPath = "", 
        const std::string texPath = "") {
        if (!strcmp(meshPath.c_str(), "")) {
            addSphere(mesh);
        } else {
            std::vector<Vec3f> vertices;
            std::vector<Vec2f> uvs;
            std::vector<Vec3f> normals;
            loadOBJ(meshPath.c_str(), vertices, uvs, normals);
            indexVBO(vertices, uvs, normals, mesh.indices(), mesh.vertices(), mesh.texCoord2s(), mesh.normals());
            //for (auto i : mesh.vertices()) std::cout << i<<"\n";
        }

        if (!strcmp(shaderPath.c_str(), "")) {
            al::ShaderSources un = al::defaultShaderUniformColor(false, false, false);
            shader.compile(un.vert.c_str(), un.frag.c_str());
        } else {
            std::fstream vert(std::string(shaderPath + ".vert").c_str(), std::ios::in);
            std::fstream frag(std::string(shaderPath + ".frag").c_str(), std::ios::in);
            std::stringstream vertStr;
            std::stringstream fragStr;
            if (!vert.good() || !frag.good()) {
                std::cout<<"ERROR: loading obj:(" << shaderPath << ") file is not good.\n";
            }
            vertStr << vert.rdbuf();
            fragStr << frag.rdbuf();
            if (!shader.compile(vertStr.str().c_str(), fragStr.str().c_str())) {
                std::cout<<"ERROR: loading obj:(" << shaderPath << ") file is not good.\n";
            }
            vert.close();
            frag.close();
        }
        if (!strcmp(shaderPath.c_str(), "")) {
            // pass
        } else {
            loadTexture(texture, texPath);
        }
        // default material
        material.ambient(Color(1.0f, 0.5f, 0.31f, 1.0f));
        material.diffuse(Color(1.0f, 0.5f, 0.31f, 1.0f));
        material.specular(Color(1.0f, 0.5f, 0.31f, 1.0f));
        material.shininess(32.0f);
    }

    ~Object() {
        shader.destroy();
    }
    virtual void onCreate() = 0;
    virtual void onAnimate(double dt) = 0;
    virtual void onDraw(Graphics& g, Nav& camera) = 0;
};

class V1Object : public Object 
{
public:
    Light singleLight;
    BufferObject bufferArray[3];
    BufferObject elementBuffer;
    V1Object(const std::string meshPath = "", const std::string shaderPath = "./shaders/default", 
        const std::string texPath = "") 
        : Object(meshPath, shaderPath, texPath) {}

    void onCreate() override {
        std::vector<int>bufferSize({3, 2, 3});
        auto& vao = mesh.vao();
        vao.bind();
        for (int i = 0; i < bufferSize.size(); i++) {
            bufferArray[i].bufferType(GL_ARRAY_BUFFER);
            bufferArray[i].usage(GL_STATIC_DRAW);
            bufferArray[i].create();
            vao.enableAttrib(i);
            vao.attribPointer(i, bufferArray[i], bufferSize[i], GL_FLOAT, GL_FALSE, 0, 0);
            glVertexAttribDivisor(i, 1); 
        }
        bufferArray[0].bind();
        bufferArray[0].data(mesh.vertices().size() * sizeof(float) * 3, mesh.vertices().data());
        bufferArray[1].bind();
        bufferArray[1].data(mesh.texCoord2s().size() * sizeof(float) * 2, mesh.texCoord2s().data());
        bufferArray[2].bind();
        bufferArray[2].data(mesh.normals().size() * sizeof(float) * 3, mesh.normals().data());
        elementBuffer.bufferType(GL_ELEMENT_ARRAY_BUFFER);
        elementBuffer.usage(GL_STATIC_DRAW);
        elementBuffer.create();
        elementBuffer.bind();
        elementBuffer.data(mesh.indices().size() * sizeof(unsigned int), mesh.indices().data());
    }
    
    void onAnimate(double dt) override {
        // pass
    }

    void onDraw(Graphics& g, Nav& camera) override {
        g.shader(shader);
        
        g.shader().uniform("model", g.modelMatrix());

        g.shader().uniform("viewPos", camera.pos());
        g.shader().uniform("material.ambient", Vec3f(material.ambient().r, 
        material.ambient().g, material.ambient().b));
        g.shader().uniform("material.diffuse", Vec3f(material.diffuse().r, 
        material.diffuse().g, material.diffuse().b));
        g.shader().uniform("material.specular", Vec3f(material.specular().r, 
        material.specular().g, material.specular().b));
        g.shader().uniform("material.shininess", material.shininess());

        g.shader().uniform("light.position", Vec3f(singleLight.pos()));
        g.shader().uniform("light.ambient", Vec3f(singleLight.ambient().r, 
        singleLight.ambient().g, singleLight.ambient().b));
        g.shader().uniform("light.diffuse", Vec3f(singleLight.diffuse().r, 
        singleLight.diffuse().g, singleLight.diffuse().b));
        g.shader().uniform("light.specular", Vec3f(singleLight.specular().r, 
        singleLight.specular().g, singleLight.specular().b));

        mesh.vao().bind();

        elementBuffer.bind();
        glDrawElements(
			GL_TRIANGLES,
			mesh.indices().size(),
			GL_UNSIGNED_SHORT,
			(void*)0
		);
    }
};
