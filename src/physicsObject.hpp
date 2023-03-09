#pragma once

#include "object.hpp"
#include "mesh_helper.hpp"
#include "octree.hpp"

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

    Vec3f AABBmin;
    Vec3f AABBmax;
    Mesh AABB; // Actually, it's octree's mesh
    ShaderProgram AABBshader;
    BufferObject AABBbuffer;
    VAO AABBVao;

    OctreeNode *root;
    int octreeDepth = 5;

public:
    RigidObject(const std::string meshPath = "", const std::string shaderPath = "./shaders/default",
                const std::string texPath = "")
        : V1Object(meshPath, shaderPath, texPath) {}

    ~RigidObject()
    {
        if (root != nullptr)
            deleteTree(root);
    }

    void Octree2Mesh(OctreeNode *node)
    {
        if (node != nullptr)
        {
            if (node->depth == octreeDepth)
            {
                addAABB(AABB, Vec3f(node->xmin, node->ymin, node->zmin),
                        Vec3f(node->xmax, node->ymax, node->zmax));
            }

            for (int i = 0; i < 8; i++)
            {
                Octree2Mesh(node->children[i]);
            }
        }
    }

    void createAABBAndOctree()
    {
        AABBmin = Vec3f(9999, 9999, 9999);
        AABBmax = Vec3f(-9999, -9999, -9999);
        for (auto vert : mesh.vertices())
        {
            for (int i = 0; i < 3; i++)
            {
                if (vert[i] < AABBmin[i])
                    AABBmin[i] = vert[i];
                if (vert[i] > AABBmax[i])
                    AABBmax[i] = vert[i];
            }
        }
        createOctree();
        // addAABB(AABB, AABBmin, AABBmax);
        Octree2Mesh(root);

        AABBVao.create();
        AABBVao.bind();
        AABBbuffer.bufferType(GL_ARRAY_BUFFER);
        AABBbuffer.usage(GL_STATIC_DRAW);
        AABBbuffer.create();
        AABBbuffer.bind();
        AABBbuffer.data(AABB.vertices().size() * sizeof(float) * 3,
                        AABB.vertices().data());

        AABBVao.enableAttrib(0);
        AABBVao.attribPointer(0, AABBbuffer, 3, GL_FLOAT, 0, 0);

        std::string shaderPath("./shaders/line");
        std::fstream vert(std::string(shaderPath + ".vert").c_str(), std::ios::in);
        std::fstream frag(std::string(shaderPath + ".frag").c_str(), std::ios::in);
        std::stringstream vertStr;
        std::stringstream fragStr;
        if (!vert.good() || !frag.good())
        {
            std::cout << "ERROR: loading obj:(" << shaderPath << ") file is not good.\n";
        }
        vertStr << vert.rdbuf();
        fragStr << frag.rdbuf();
        if (!AABBshader.compile(vertStr.str().c_str(), fragStr.str().c_str()))
        {
            std::cout << "ERROR: loading obj:(" << shaderPath << ") file is not good.\n";
        }
        vert.close();
        frag.close();
    }

    void createOctree()
    {
        // create AABB first
        root = new OctreeNode();
        root->depth = 1;
        createMeshOctree(root, mesh, AABBmin.x, AABBmax.x,
                         AABBmin.y, AABBmax.y,
                         AABBmin.z, AABBmax.z, octreeDepth);

        preOrder(root);
        std::cout << "octree node num:" << nodeNum(root) << std::endl;
    }

    void drawAABB(Graphics &g, Nav &camera)
    {
        AABBshader.use();

        g.translate(nav.pos());
        g.rotate(nav.quat());
        g.scale(scale);

        AABBshader.uniform("model", g.modelMatrix());
        AABBshader.uniform("view", g.viewMatrix());
        AABBshader.uniform("projection", g.projMatrix());
        AABBshader.uniform("color", Vec3f(1.0f, 0.5f, 0.7f));

        AABBVao.bind();
        glDrawArrays(GL_LINES, 0, AABB.vertices().size());
    }

    void initIRef()
    {
        auto vertices = mesh.vertices();
        Mat4f S = ScaleMatrix(scale);
        float m = 0.001f;
        mass = 0;
        for (int i = 0; i < vertices.size(); i++)
        {
            vertices[i] = S * Vec4f(vertices[i], 1.0f);
            mass += m;
            float diag = m * vertices[i].magSqr();
            I_ref += Mat4f(diag - m * vertices[i][0] * vertices[i][0], -m * vertices[i][0] * vertices[i][1], -m * vertices[i][0] * vertices[i][2], 0,
                           -m * vertices[i][1] * vertices[i][0], diag - m * vertices[i][1] * vertices[i][1], -m * vertices[i][1] * vertices[i][2], 0,
                           -m * vertices[i][2] * vertices[i][0], -m * vertices[i][2] * vertices[i][1], diag - m * vertices[i][2] * vertices[i][2], 0,
                           0, 0, 0, 1);
        }
    }

    void addVelocity(Vec3f _v = Vec3f(0, 7.0f, 0))
    {
        restitution = 0.5;
        v += _v;
    }

    void collisonImpulse_plane(Vec3f P, Vec3f N)
    {
        auto &vertices = mesh.vertices();
        Mat4f R;
        Mat4f S = ScaleMatrix(scale);
        nav.quat().toMatrix(R.elems());
        R = S * R;
        Vec3f x = nav.pos();
        Vec3f collideL(0);
        Vec3f collideV(0);
        float count = 0;

        for (int i = 0; i < vertices.size(); i++)
        {
            Vec3f Rri = R * Vec4f(vertices[i], 1.0f);
            if ((x + Rri - P).dot(N) < 0)
            {
                Vec3f vi = v + w.cross(Rri);
                if (dot(vi, N) < 0)
                {
                    collideL += Rri;
                    collideV += vi;
                    count += 1;
                }
            }
        }
        if (count > 0)
        {
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
            // std::cout<<friction<<std::endl;

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
            if (v_ni.mag() < 1)
            {
                restitution *= 0.9;
            }
        }
    }

    void onAnimate(double dt)
    {
        Vec3f Fg(0, -mass * g, 0);
        v *= linear_decay;
        v += (float)dt * Fg * (1 / mass);
        w *= angular_decay;

        collisonImpulse_plane(Vec3f(0, -1.5f, 0), Vec3f(0, 1, 0));
        collisonImpulse_plane(Vec3f(15.0f, 0, 0), Vec3f(-1, 0, 0));
        collisonImpulse_plane(Vec3f(-15.0f, 0, 0), Vec3f(1, 0, 0));
        collisonImpulse_plane(Vec3f(0, 0, 15.0f), Vec3f(0, 0, -1));
        collisonImpulse_plane(Vec3f(0, 0, -15.0f), Vec3f(0, 0, 1));

        auto &x = nav.pos();
        x += (float)dt * v;
        Quatf dq(0, w.x * dt * 0.5, w.y * dt * 0.5, w.z * dt * 0.5);

        auto &q = nav.quat();
        q += dq * q;
        q.normalize();
    }
};