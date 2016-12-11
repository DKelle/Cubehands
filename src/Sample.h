#define LEFT 0
#define RIGHT 1


#include <iostream>
#include <cstring>
#include "Leap.h"
#include "menger.h"
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
    std::vector<glm::vec4> get_old_hand_positions(int width, int height);
    std::vector<glm::vec4> transform_to_world(std::vector<glm::vec4> hand_positions, int width, int height);
    void drawBone(const Leap::Bone& bone, std::vector<glm::vec4>& bone_vertices,std::vector<glm::uvec2>& bone_indices);
    void drawHands(std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_indices, std::vector<glm::vec4>& joint_vertices, std::vector<glm::uvec3>& joint_indices, std::vector<glm::vec4>& joint_normals);
    glm::vec4 convertLeapToWorld(glm::vec4 vector, int width, int height);
    std::vector<int> digits;
    std::shared_ptr<Menger> g_menger;


    const float Z_DEPTH = 100;
    const float LEAP_MAX = 600;
    const float SCALE_WIDTH = 100;
    const float SCALE_HEIGHT = 100;
    std::vector<glm::mat4> rotation_matrices;
    std::vector<glm::vec4> translation_vectors;
    std::vector<Leap::PointableList> pointable_list;
    std::vector<float> scale_prob;
    std::vector<float> scale_factor;


  private:
    std::vector<glm::vec4> hand_positions;
    bool print_leap_stats = false;
    std::vector<glm::vec4> bone_vertices;
    std::vector<glm::uvec2> bone_indices;
    std::vector<glm::vec4> old_hand_positions;


};
