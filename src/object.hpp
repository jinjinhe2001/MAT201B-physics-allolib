#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
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
#include "math_helper.hpp"

using namespace al;
class Object
{
public:
    Nav nav;
    Vec3f scale;
    Mesh mesh;
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
        if (!strcmp(texPath.c_str(), "")) {
            // pass
        } else {
            loadTexture(texture, texPath);
        }
        // default material
        material.ambient(Color(1.0f, 1.0f, 1.0f, 1.0f));
        material.diffuse(Color(1.0f, 1.0f, 1.0f, 1.0f));
        material.specular(Color(1.0f, 1.0f, 1.0f, 1.0f));
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
    VAO vao;
    V1Object(const std::string meshPath = "", const std::string shaderPath = "./shaders/default", 
        const std::string texPath = "") 
        : Object(meshPath, shaderPath, texPath) {}

    void onCreate() override {
        Color lightColor(1.0f, 1.0f, 1.0f, 1.0f);
        singleLight.ambient(lightColor * 0.5f);
        singleLight.diffuse(lightColor * 0.7f);
        singleLight.specular(Color(1.0f, 1.0f, 1.0f, 1.0f));

        
        std::vector<int>bufferSize({3, 2, 3});
        vao.create();
        for (int i = 0; i < bufferSize.size(); i++) {
            bufferArray[i].bufferType(GL_ARRAY_BUFFER);
            bufferArray[i].usage(GL_STATIC_DRAW);
            bufferArray[i].create();
            bufferArray[i].bind();
            if (i == 0)
                bufferArray[i].data(mesh.vertices().size() * sizeof(float) * bufferSize[i], mesh.vertices().data());
            else if (i == 1) {
                bufferArray[i].data(mesh.texCoord2s().size() * sizeof(float) * bufferSize[i], mesh.texCoord2s().data());
            } else {
                bufferArray[i].data(mesh.normals().size() * sizeof(float) * bufferSize[i], mesh.normals().data());
            }
            vao.bind();
            vao.enableAttrib(i);
            vao.attribPointer(i, bufferArray[i], bufferSize[i], GL_FLOAT, 0, 0);
        }
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

        shader.use();

        g.translate(nav.pos());
        g.rotate(nav.quat());
        g.scale(scale);
        

        shader.uniform("model", g.modelMatrix());
        shader.uniform("view", g.viewMatrix());
        shader.uniform("projection", g.projMatrix());

        shader.uniform("viewPos", camera.pos());
        shader.uniform("material.ambient", Vec3f(material.ambient().r, 
        material.ambient().g, material.ambient().b));
        shader.uniform("material.diffuse", Vec3f(material.diffuse().r, 
        material.diffuse().g, material.diffuse().b));
        shader.uniform("material.specular", Vec3f(material.specular().r, 
        material.specular().g, material.specular().b));
        shader.uniform("material.shininess", material.shininess());

        shader.uniform("light.position", Vec3f(singleLight.pos()));
        shader.uniform("light.ambient", Vec3f(singleLight.ambient().r, 
        singleLight.ambient().g, singleLight.ambient().b));
        shader.uniform("light.diffuse", Vec3f(singleLight.diffuse().r, 
        singleLight.diffuse().g, singleLight.diffuse().b));
        shader.uniform("light.specular", Vec3f(singleLight.specular().r, 
        singleLight.specular().g, singleLight.specular().b));

        glUniform1i(glGetUniformLocation(shader.id(), "texture1"), 0);
        texture.bind(0);

        vao.bind();

        elementBuffer.bind();
        glDrawElements(
			GL_TRIANGLES,
			mesh.indices().size(),
			GL_UNSIGNED_INT,
			(void*)0
		);
    }

    void generateNormals() {
        mesh.generateNormals();
        bufferArray[2].bind();
        bufferArray[2].data(mesh.normals().size() * sizeof(float) * 3, mesh.normals().data());

        vao.bind();
        vao.enableAttrib(2);
        vao.attribPointer(2, bufferArray[2], 3, GL_FLOAT, 0, 0);
    }
};

class RigidObject : public V1Object
{
public:
    Vec3f v; // velocity;
    Vec3f w; // angular velocity
    float miu_t = 0.7;
    float mass;
    Mat4f I_ref; // inertia matrix

    float linear_decay = 0.999f;
    float angular_decay = 0.98f;
    float restitution = 0.5f; // collision
    float g = 9.8;

public:
    RigidObject(const std::string meshPath = "", const std::string shaderPath = "./shaders/default", 
        const std::string texPath = "") 
        : V1Object(meshPath, shaderPath, texPath) {}

    void initIRef() {
        auto vertices = mesh.vertices();
        Mat4f S = ScaleMatrix(scale);
        float m = 0.001f;
        mass = 0;
        for (int i = 0; i < vertices.size(); i++) {
            vertices[i] = S * Vec4f(vertices[i], 1.0f);
            mass += m;
            float diag = m * vertices[i].magSqr();
            I_ref += Mat4f(diag - m * vertices[i][0]*vertices[i][0], -m*vertices[i][0]*vertices[i][1], -m*vertices[i][0]*vertices[i][2], 0,
                           -m*vertices[i][1]*vertices[i][0], diag - m*vertices[i][1]*vertices[i][1], -m*vertices[i][1]*vertices[i][2], 0,
                           -m*vertices[i][2]*vertices[i][0], -m*vertices[i][2]*vertices[i][1], diag - m*vertices[i][2]*vertices[i][2], 0,
                           0, 0, 0, 1);
        }
    }

    void addVelocity(Vec3f _v = Vec3f(0, 7.0f, 0)) {
        restitution = 0.5;
        v += _v;
    }
    
    void collisonImpulse_plane(Vec3f P, Vec3f N) {
        auto& vertices = mesh.vertices();
        Mat4f R; 
        Mat4f S = ScaleMatrix(scale);
        nav.quat().toMatrix(R.elems());
        R = S * R;
        Vec3f x = nav.pos();
        Vec3f collideL(0);
        Vec3f collideV(0);
        float count = 0;

        for (int i = 0; i < vertices.size(); i++) {
            Vec3f Rri = R * Vec4f(vertices[i], 1.0f);
            if ((x + Rri - P).dot(N) < 0) {
                Vec3f vi = v + w.cross(Rri);
                if (dot(vi, N) < 0) {
                    collideL += Rri;
                    collideV += vi;
                    count+=1;
                }
            }
        }
        if (count > 0) {
            collideL *= (1 / count);
            collideV *= (1 / count);
            Mat4f Rri_cross = CrossMatrix(collideL);
            Vec3f j(0.0f); // Impulse

            Vec3f v_ni = dot(collideV, N) * N;
            Vec3f v_ti = collideV - v_ni;
            
            float friction;
            if (v_ti.mag() < 1e-5) 
                friction = 0;
            else
                friction = max(1 - miu_t * (1 + restitution) * v_ni.mag() / v_ti.mag(), 0.0f);
            // 1 - μt(1 + μn)||Vni||/||Vti||

            v_ni *= -restitution;
            v_ti *= friction;
            //std::cout<<friction<<std::endl;

            Mat4f K = Mat4f::identity() * (1 / mass) - Rri_cross * I_ref.inversed() * Rri_cross;
            j = K.inversed() * Vec4f(v_ni + v_ti - collideV, 1.0f);

            /*std::cout<<"I_ref"<<std::endl;
            for(int i = 0; i < 16; i++){
                std::cout<<I_ref[i]<<std::endl;
            }
            std::cout<<"I_ref.inv"<<std::endl;
            for(int i = 0; i < 16; i++){
                std::cout<<I_ref.inversed()[i]<<std::endl;
            }
            std::cout<<"K"<<std::endl;
            for(int i = 0; i < 16; i++){
                std::cout<<K[i]<<std::endl;
            }
            std::cout<<"J"<<std::endl;
            std::cout<<j<<"\n";*/

            v += (1 / mass) * j;

            w += I_ref.inversed() * (Rri_cross * Vec4f(j, 1.0f));
            if (v_ni.mag() < 1) {
                restitution *= 0.9;
            }
        }
    }

    void onAnimate(double dt) {
        Vec3f Fg(0, -mass * g, 0);
        v *= linear_decay;
        v += (float)dt * Fg * (1 / mass);
        w *= angular_decay;

        collisonImpulse_plane(Vec3f(0, -1.5f, 0), Vec3f(0, 1, 0));

        auto& x = nav.pos();
        x += (float)dt * v;
        Quatf dq(0, w.x * dt * 0.5, w.y * dt * 0.5, w.z * dt * 0.5);

        auto& q = nav.quat();
        q += dq * q;
        q.normalize();
    }
};