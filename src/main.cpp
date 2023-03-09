#include <iostream>
#include <memory>

// for master branch
// #include "al/core.hpp"

// for devel branch
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_Image.hpp"
#include "object.hpp"
#include "physicsObject.hpp"
#include "skybox.hpp"

using namespace al;

struct MyApp : App {
  std::unique_ptr<RigidObject> bunny;
  std::unique_ptr<V1Object> plane;
  std::unique_ptr<Skybox> skybox;

  void createBunny() {
    bunny = std::make_unique<RigidObject>("./assets/bunny/bunny.obj", 
                                       "./shaders/default", 
                                       "./assets/bunny/bunny-atlas.jpg");
    bunny->onCreate();
    bunny->generateNormals();
    bunny->scale = Vec3f(0.005);
    bunny->nav.pos(1, 4, 1);
    bunny->nav.quat().fromAxisAngle(-0.5 * M_2PI, 1, 0, 0);
    bunny->material.shininess(8.0f);
    bunny->singleLight.pos(5, 10, -5);
    bunny->initIRef();
    bunny->createAABBAndOctree();
  }

  void createPlane() {
    plane = std::make_unique<V1Object>("./assets/plane/plane.obj", 
                                       "./shaders/default", 
                                       "./assets/plane/uvmap.jpeg");
    plane->onCreate();
    plane->scale = Vec3f(1);
    plane->nav.pos(0, -1.5, 0);
    plane->nav.quat().fromAxisAngle(-0.25 * M_2PI, 1, 0, 0);
    plane->material.shininess(32);
    plane->singleLight.pos(5, 10, -5);
  }

  void createSkybox() {
    std::vector<std::string> faces
    {
        "./assets/skybox/right.jpg",
        "./assets/skybox/left.jpg",
        "./assets/skybox/top.jpg",
        "./assets/skybox/bottom.jpg",
        "./assets/skybox/front.jpg",
        "./assets/skybox/back.jpg",
    };
    skybox = std::make_unique<Skybox>(faces);
  }

  void onCreate() override {
    createBunny();
    createPlane();
    createSkybox();
    nav().pos(0, 10, 20);
    nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);
    nav().faceToward(Vec3f(0));
  }

  bool onKeyDown(Keyboard const& k) override {
    switch (k.key()) {
      case ' ': {
        bunny->addVelocity();
      } break;
      case '1': {
        bunny->addVelocity(Vec3f(5.0f, 0, 0));
      } break;
      case '2': {
        bunny->addVelocity(Vec3f(-5.0f, 0, 0));
      } break;
      case '3': {
        bunny->addVelocity(Vec3f(0, 0, 5.0f));
      } break;
      case '4': {
        bunny->addVelocity(Vec3f(0, 0, -5.0f));
      } break;
    }
    return true;
  }
  void onAnimate(double dt) override {
    bunny->onAnimate(dt);
  }

  void onDraw(Graphics& g) override {

    g.depthTesting(true);
    g.clear(0.2);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    g.pushMatrix();
    g.pushCamera(view());
    skybox->onDraw(g, nav());
    g.popMatrix();

    g.pushMatrix();
    g.pushCamera(view());
    bunny->drawAABB(g, nav());
    g.popMatrix();

    g.pushMatrix();
    g.pushCamera(view());
    bunny->onDraw(g, nav());
    g.popMatrix();

    g.pushMatrix();
    g.pushCamera(view());
    plane->onDraw(g, nav());
    g.popMatrix();

  }
};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.start();
}
