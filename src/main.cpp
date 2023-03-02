#include <iostream>
#include <memory>

// for master branch
// #include "al/core.hpp"

// for devel branch
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_Image.hpp"
#include "object.hpp"

using namespace al;

struct MyApp : App {
  std::unique_ptr<RigidObject> bunny;
  std::unique_ptr<V1Object> plane;

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
  }

  void createPlane() {
    plane = std::make_unique<V1Object>("./assets/plane/plane.obj", 
                                       "./shaders/default", 
                                       "./assets/plane/uvmap.jpeg");
    plane->onCreate();
    plane->scale = Vec3f(1);
    plane->nav.pos(0, -1.5, 0);
    plane->nav.quat().fromAxisAngle(-0.25 * M_2PI, 1, 0, 0);
    plane->material.shininess(2);
    bunny->singleLight.pos(5, 10, -5);
  }

  void onCreate() override {
    createBunny();
    createPlane();
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
  }
  void onAnimate(double dt) override {
    bunny->onAnimate(dt);
  }

  void onDraw(Graphics& g) override {
    g.depthTesting(true);
    g.clear(0.2);

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
