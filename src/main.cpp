#include <iostream>

// for master branch
// #include "al/core.hpp"

// for devel branch
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_Image.hpp"
#include "helper.hpp"

using namespace al;
using namespace helper;

struct MyApp : App {
  Object bunny;
  Image texture;
  double phase = 0;

  void onCreate() override {
    helper::loadObjFile(bunny.fMesh, "./assets/bunny/bunny.obj");
    bunny.loadTexture("./assets/bunny/bunny-atlas.jpg");

    lens().near(0.1).far(25).fovy(45);
    nav().pos(0, 0, 4);
    nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);
  }

  void onAnimate(double dt) override {
    double period = 10;
    phase += dt / period;
    if (phase >= 1.) phase -= 1.;
  }

  void onDraw(Graphics& g) override {
    g.depthTesting(true);
    g.clear(0, 0, 0);
    g.pushMatrix();
    g.color(1);
    g.scale(0.005);
    bunny.texture.bind();
    g.texture();
    g.draw(bunny.fMesh());
    g.popMatrix();
  }
};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.start();
}
