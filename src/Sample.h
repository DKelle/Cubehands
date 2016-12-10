#include <iostream>
#include <cstring>
#include "Leap.h"

#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

using namespace Leap;

class SampleListener : public Listener {
  public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);
    std::vector<glm::vec4> get_hand_positions(int width, int height);
    std::vector<glm::vec4> transform_to_world(std::vector<glm::vec4> hand_positions, int width, int height);
    void drawBone(const Leap::Bone& bone, std::vector<glm::vec4>& bone_vertices, 
    std::vector<glm::uvec2>& bone_indices);
    void drawHands(std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_indices);
    glm::vec4 convertLeapToWorld(glm::vec4 vector, int width, int height);


    const float Z_DEPTH = 100;
    const float LEAP_MAX = 600;
    const float SCALE_WIDTH = 100;
    const float SCALE_HEIGHT = 100;



  private:
    std::vector<glm::vec4> hand_positions;
    bool print_leap_stats = false;
    std::vector<glm::vec4> bone_vertices;
    std::vector<glm::uvec2> bone_indices;


};
