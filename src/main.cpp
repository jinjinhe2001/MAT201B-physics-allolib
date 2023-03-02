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
  std::unique_ptr<V1Object> bunny;
  std::unique_ptr<V1Object> plane;
  Image texture;
  Mesh mesh;
  double phase = 0;

  void createBunny() {
    bunny = std::make_unique<V1Object>("./assets/bunny/bunny.obj", 
                                       "./shaders/default", 
                                       "./assets/bunny/bunny-atlas.jpg");
    bunny->onCreate();
    bunny->generateNormals();
    
    bunny->scale = Vec3f(0.005);
    bunny->nav.pos(1, 1, 1);
    bunny->nav.quat().fromAxisAngle(-0.5 * M_2PI, 1, 0, 0);
    bunny->material.shininess(8.0f);
    bunny->singleLight.pos(5, 10, -5);
  }

  void createPlane() {
    plane = std::make_unique<V1Object>("./assets/plane/plane.obj", 
                                       "./shaders/default", 
                                       "./assets/plane/uvmap.jpeg");
    plane->onCreate();
    plane->scale = Vec3f(1);
    plane->nav.pos(0, -3, 0);
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

  void onAnimate(double dt) override {
    double period = 10;
    phase += dt / period;
    if (phase >= 1.) phase -= 1.;
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
