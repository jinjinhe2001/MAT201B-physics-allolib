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
#include "al/app/al_DistributedApp.hpp"
#include "al/io/al_Imgui.hpp"


using namespace al;

struct MyApp : DistributedApp {
  std::vector<std::shared_ptr<RigidObject>> bunnys;
  std::unique_ptr<V1Object> plane;
  std::unique_ptr<Skybox> skybox;
  std::unique_ptr<MassSpring> cloth;

  bool showOctree = true;

  void createCloth() {
    cloth = std::make_unique<MassSpring>("", 
                                         "./shaders/cloth",
                                         "./assets/cloth/cloth.jpeg");
    cloth->onCreate();
    cloth->scale = Vec3f(1);
    cloth->reCalculateL();
    cloth->nav.pos(0, 8, 0);
    cloth->material.shininess(128);
    cloth->singleLight.pos(5, 10, -5);
  }

  void createBunny() {
    std::shared_ptr<RigidObject> bunny = std::make_shared<RigidObject>(
                      "./assets/bunny/bunny.obj", 
                      "./shaders/default", 
                      "./assets/bunny/bunny-atlas.jpg");

    bunny->onCreate();
    bunny->generateNormals();
    bunny->scale = Vec3f(0.005);
    bunny->nav.pos(1, 4, 1);
    bunny->nav.quat().fromAxisAngle(-0.5 * M_2PI, 1, 0, 0);
    bunny->material.shininess(8.0f);
    bunny->singleLight.pos(5, 10, -5);
    bunny->createAABBAndOctree();
    bunny->initIRef();
    bunnys.push_back(bunny);
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
    createCloth();
    createPlane();
    createSkybox();
    nav().pos(0, 20, 30);
    nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);
    nav().faceToward(Vec3f(0));
  }

  bool onKeyDown(Keyboard const& k) override {
    switch (k.key()) {
      case ' ': {
        bunnys[0]->addVelocity();
      } break;
      case '1': {
        bunnys[0]->addVelocity(Vec3f(5.0f, 0, 0));
      } break;
      case '2': {
        bunnys[0]->addVelocity(Vec3f(-5.0f, 0, 0));
      } break;
      case '3': {
        bunnys[0]->addVelocity(Vec3f(0, 0, 5.0f));
      } break;
      case '4': {
        bunnys[0]->addVelocity(Vec3f(0, 0, -5.0f));
      } break;
    }
    return true;
  }
  void onAnimate(double dt) override {
    if (dt < 1e-6) return;
    for (int i = 0; i < bunnys.size(); i++) {
      for (int j = 0; j < bunnys.size(); j++) {
        if (i == j) continue;
        bunnys[i]->rigidBodyCollision(*bunnys[j]);
      }
    }

    for (int i = 0; i < bunnys.size(); i++) {
      bunnys[i]->onAnimate(dt);
    }
    cloth->onAnimate(dt);
  }

  void onDraw(Graphics& g) override {

    g.depthTesting(true);
    g.clear(0.2);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g.pushMatrix();
    g.pushCamera(view());
    skybox->onDraw(g, nav());
    g.popMatrix();

    for (int i = 0; i < bunnys.size(); i++) {
      g.pushMatrix();
      g.pushCamera(view());
      bunnys[i]->onDraw(g, nav());
      g.popMatrix();
      if (showOctree) {
        g.pushMatrix();
        g.pushCamera(view());
        bunnys[i]->drawAABB(g, nav());
        g.popMatrix();
      }
    }

    g.pushMatrix();
    g.pushCamera(view());
    cloth->onDraw(g, nav());
    g.popMatrix();

    g.pushMatrix();
    g.pushCamera(view());
    plane->onDraw(g, nav());
    g.popMatrix();

    drawImGUI(g);
  }

  void drawImGUI(Graphics& g) {
    imguiBeginFrame();

    ImGui::Begin("GUI");
    static bool _showOctree = true;
    ImGui::Checkbox("Show Octree", &_showOctree);
    showOctree = _showOctree;

    
    if (ImGui::Button("Add Bunny")) {
      createBunny();
    }
    ImGui::End();
    imguiEndFrame();
    imguiDraw();
  }

  void onInit() override {
    imguiInit();
  }
  void onExit() override { imguiShutdown(); }
};

int main() {
  MyApp app;
  app.dimensions(1080, 720);
  app.start();
}
