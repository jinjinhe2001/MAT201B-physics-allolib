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
  std::shared_ptr<V1Object> bunny;
  Image texture;
  Mesh mesh;
  double phase = 0;

  void onCreate() override {
    bunny = std::make_shared<V1Object>("./assets/bunny/bunny.obj", "./shaders/default", "./assets/bunny/bunny-atlas.jpg");
    bunny->onCreate();
    bunny->singleLight.pos(5, 5, 5);

    nav().pos(0, 0, 4);
    nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);
  }

  void onAnimate(double dt) override {
    double period = 10;
    phase += dt / period;
    if (phase >= 1.) phase -= 1.;
    nav().faceToward(Vec3f(0, 0, 0));
  }

  void onDraw(Graphics& g) override {
    g.clear(0, 0, 0);
    g.depthTesting(true);
    g.pushMatrix();
    g.color(1);
    g.scale(0.005);
    bunny->texture.bind();
    g.texture();
    auto mesh = bunny->mesh;
    // strange thing: use g.draw(bunny->mesh); it draws nothing.
    g.draw(mesh);
    g.popMatrix();
  }
};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.start();
}
