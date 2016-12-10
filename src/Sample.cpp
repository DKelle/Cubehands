/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/
#include "Sample.h"
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

const int CYLINDER_RADIUS = 1;

void SampleListener::onInit(const Controller& controller) {
    std::cout << "Initialized" << std::endl;

    //push back two empty vectors for hand positions
    hand_positions.push_back(glm::vec4(0,0,0,0));
    hand_positions.push_back(glm::vec4(0,0,0,0));

    old_hand_positions.push_back(glm::vec4(0,0,0,0));
    old_hand_positions.push_back(glm::vec4(0,0,0,0));

    digits.push_back(0);
    digits.push_back(0);
}

void SampleListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
    controller.enableGesture(Gesture::TYPE_CIRCLE);
    controller.enableGesture(Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
    // Note: not dispatched when running in a debugger.
    if(print_leap_stats)
    {
        std::cout << "Disconnected" << std::endl;
    }
}

void SampleListener::onExit(const Controller& controller) {
    if(print_leap_stats) std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
    //Set the w coord of both hands to 0
    bone_vertices.clear();
    bone_indices.clear();
    old_hand_positions[RIGHT] = hand_positions[RIGHT];
    old_hand_positions[LEFT] = hand_positions[LEFT];
    hand_positions[LEFT].w = 0;
    hand_positions[RIGHT].w = 0;

    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();
    if(print_leap_stats) 
    {
        std::cout << "Frame id: " << frame.id()
                            << ", timestamp: " << frame.timestamp()
                            << ", hands: " << frame.hands().count()
                            << ", extended fingers: " << frame.fingers().extended().count()
                            << ", tools: " << frame.tools().count()
                            << ", gestures: " << frame.gestures().count() << std::endl;
    }

    HandList hands = frame.hands();
    for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
        // Get the first hand
        const Hand hand = *hl;
        std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
        if(print_leap_stats)
        {
            std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
                                << ", palm position: " << hand.palmPosition() << std::endl;
        }

        // Get the hand's normal vector and direction
        const Vector normal = hand.palmNormal();
        const Vector direction = hand.direction();

        //Put the palm position into the hand_positions vector
        Vector pos = hand.palmPosition();
        glm::vec4 glm_pos = glm::vec4(pos.x, pos.y, pos.z, 1.0f);

        //add this glm::vec4 to the correct position in hand_positions
        int index = (hand.isLeft()) ? LEFT : RIGHT;
        hand_positions[index] = glm_pos;

        if(print_leap_stats)
        {
            // Calculate the hand's pitch, roll, and yaw angles
            std::cout << std::string(2, ' ') <<  "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
                                << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
                                << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;            
        }

        // Get the Arm bone
        Arm arm = hand.arm();
        if(print_leap_stats)
        {
            std::cout << std::string(2, ' ') <<  "Arm direction: " << arm.direction()
                                << " wrist position: " << arm.wristPosition()
                                << " elbow position: " << arm.elbowPosition() << std::endl;
        }

        // Get fingers
        const FingerList fingers = hand.fingers();
        for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
            digits[index] = fingers.extended().count();
            const Finger finger = *fl;

            if(print_leap_stats)
            {
                std::cout << std::string(4, ' ') <<  fingerNames[finger.type()]
                                    << " finger, id: " << finger.id()
                                    << ", length: " << finger.length()
                                    << "mm, width: " << finger.width() << std::endl;
            }

            // Get finger bones
            for (int b = 0; b < 4; ++b) {
                Bone::Type boneType = static_cast<Bone::Type>(b);
                Bone bone = finger.bone(boneType);
                if(print_leap_stats)
                {
                    std::cout << std::string(6, ' ') <<  boneNames[boneType]
                                        << " bone, start: " << bone.prevJoint()
                                        << ", end: " << bone.nextJoint()
                                        << ", direction: " << bone.direction() << std::endl;
                }

                glm::vec4 start = bone.prevJoint().toVector4<glm::vec4>();
                start.w = 1;
                // std::cout << "pre Start bone leap: " << glm::to_string(start) << std::endl;
                // start = convertLeapToWorld(start, width, height);
                glm::vec4 end = bone.nextJoint().toVector4<glm::vec4>();
                end.w = 1;
                // end = convertLeapToWorld(end, width, height);
                // std::cout << "Start bone world: " << glm::to_string(start) << std::endl;
                // printf("hand positions: \n");
                // std::cout << glm::to_string(hand_positions[0]) << std::endl;

                bone_vertices.push_back(start);
                bone_vertices.push_back(end);
                bone_indices.push_back(glm::uvec2(bone_vertices.size() - 2, bone_vertices.size() - 1));
            }
        }
    }

    // Get tools
    const ToolList tools = frame.tools();
    for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
        const Tool tool = *tl;
        if(print_leap_stats)
        {
            std::cout << std::string(2, ' ') <<  "Tool, id: " << tool.id()
                                << ", position: " << tool.tipPosition()
                                << ", direction: " << tool.direction() << std::endl;
        }
    }

    // Get gestures
    const GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) {
        Gesture gesture = gestures[g];

        switch (gesture.type()) {
            case Gesture::TYPE_CIRCLE:
            {
                CircleGesture circle = gesture;
                std::string clockwiseness;

                if (circle.pointable().direction().angleTo(circle.normal()) <= PI/2) {
                    clockwiseness = "clockwise";
                } else {
                    clockwiseness = "counterclockwise";
                }

                // Calculate angle swept since last frame
                float sweptAngle = 0;
                if (circle.state() != Gesture::STATE_START) {
                    CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
                    sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
                }
                if(print_leap_stats)
                {
                    std::cout << std::string(2, ' ')
                                        << "Circle id: " << gesture.id()
                                        << ", state: " << stateNames[gesture.state()]
                                        << ", progress: " << circle.progress()
                                        << ", radius: " << circle.radius()
                                        << ", angle " << sweptAngle * RAD_TO_DEG
                                        <<  ", " << clockwiseness << std::endl;
                }
                break;
            }
            case Gesture::TYPE_SWIPE:
            {
                SwipeGesture swipe = gesture;
                if(print_leap_stats)
                {
                    std::cout << std::string(2, ' ')
                        << "Swipe id: " << gesture.id()
                        << ", state: " << stateNames[gesture.state()]
                        << ", direction: " << swipe.direction()
                        << ", speed: " << swipe.speed() << std::endl;
                }
                break;
            }
            case Gesture::TYPE_KEY_TAP:
            {
                KeyTapGesture tap = gesture;
                if(print_leap_stats)
                {
                    std::cout << std::string(2, ' ')
                        << "Key Tap id: " << gesture.id()
                        << ", state: " << stateNames[gesture.state()]
                        << ", position: " << tap.position()
                        << ", direction: " << tap.direction()<< std::endl;
                }
                break;
            }
            case Gesture::TYPE_SCREEN_TAP:
            {
                ScreenTapGesture screentap = gesture;
                if(print_leap_stats)
                {
                    std::cout << std::string(2, ' ')
                        << "Screen Tap id: " << gesture.id()
                        << ", state: " << stateNames[gesture.state()]
                        << ", position: " << screentap.position()
                        << ", direction: " << screentap.direction()<< std::endl;
                }
                break;
            }
            default:
            {
                if(print_leap_stats)
                {
                    std::cout << std::string(2, ' ')  << "Unknown gesture type." << std::endl;
                }
                break;
            }
        }
    }

    // this::drawHands(frame);

    if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
        std::cout << std::endl;
    }

}

void SampleListener::onFocusGained(const Controller& controller) {
    if(print_leap_stats) std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
    if(print_leap_stats) std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
    if(print_leap_stats) std::cout << "Device Changed" << std::endl;
    const DeviceList devices = controller.devices();

    for (int i = 0; i < devices.count(); ++i) {
        if(print_leap_stats) std::cout << "id: " << devices[i].toString() << std::endl;
        if(print_leap_stats) std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
    }
}

void SampleListener::onServiceConnect(const Controller& controller) {
    if(print_leap_stats) std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
    if(print_leap_stats) std::cout << "Service Disconnected" << std::endl;
}

std::vector<glm::vec4> SampleListener::get_hand_positions(int width, int height)
{
    return transform_to_world(hand_positions, width, height);
}

std::vector<glm::vec4> SampleListener::get_old_hand_positions(int width, int height)
{
    return transform_to_world(old_hand_positions, width, height);
}

glm::vec4 SampleListener::convertLeapToWorld(glm::vec4 vector, int width, int height) {
    glm::vec4 world = glm::vec4(0);
    world.x = (vector.x / LEAP_MAX) * width;
    world.y = (vector.y / LEAP_MAX) * height;
    world.z = (vector.z / LEAP_MAX) * width - Z_DEPTH;
    world.w = vector.w;
    return world;
}


std::vector<glm::vec4> SampleListener::transform_to_world(std::vector<glm::vec4> hand_positions, int width, int height)
{

    // float leap_max = 600;

    // //Transform X. X ranges from -300 to 300 in leap coordinates
    // float x_left = hand_positions[0].x;
    // float x_right = hand_positions[1].x;

    // float world_x_left = (x_left / leap_max) * width;
    // float world_x_right = (x_right / leap_max) * width;

    // //transform Y. Y ranges from 0 to 600 in leap coordinates
    // float y_left = hand_positions[0].y;
    // float y_right = hand_positions[1].y;

    // float world_y_left = (y_left / leap_max) * height;
    // float world_y_right = (y_right / leap_max) * height;

    // //transform Z. Z ranges from -300 to 300 in leap coordinates
    // float z_left = hand_positions[0].z - 100;
    // float z_right = hand_positions[1].z - 100;
    // std::cout << z_right << std::endl;

    // float world_z_left = (z_left / leap_max) * width;
    // float world_z_right = (z_right / leap_max) * width;

    glm::vec4 left= convertLeapToWorld(hand_positions[0], width, height);
    glm::vec4 right = convertLeapToWorld(hand_positions[1], width, height);

    //Create the new hand position vector
    // glm::vec4 left = glm::vec4(world_x_left, world_y_left, world_z_left, hand_positions[0].w);
    // glm::vec4 right = glm::vec4(world_x_right, world_y_right, world_z_right, hand_positions[1].w);

    std::vector<glm::vec4> world_positions;
    world_positions.push_back(left);
    world_positions.push_back(right);
    return world_positions;
}

// Draws a cylinder for a bone.
// void SampleListener::drawBone(const Leap::Bone& bone, std::vector<glm::vec4>& bone_vertices, 
//     std::vector<glm::uvec2>& bone_indices) {
  
//     glm::mat4 bone_basis_values = bone.basis().toMatrix4x4<glm::mat4>();
//   // glm::mat4 bone_basis_values;
//   // bone.basis().toArray4x4(bone_basis_values);
//     std::cout << glm::to_string(bone_basis_values) << std::endl;
//   // glPushMatrix();
//   glm::vec4 second_point = glm::vec4(0,0,bone.length(), 1);
//   second_point = bone_basis_values * second_point;

//   glm::vec4 first_point = bone_basis_values * glm::vec4(0,0,0,1);
//   // printf("first_point: %f, %f, %f\n", first_point.x, first_point.y, first_point.z);
//   bone_vertices.push_back(first_point);
//   bone_vertices.push_back(second_point);
//   bone_indices.push_back(glm::uvec2(bone_vertices.size() - 2, bone_vertices.size() - 1));



// }

// void SampleListener::drawHands(std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_indices) {
void SampleListener::drawHands(std::vector<glm::vec4>& hand_vertices, 
    std::vector<glm::uvec2>& hand_indices) {

    hand_vertices.clear();
    hand_indices.clear();


    int counter = 0;
    for(int i = 0; i < bone_indices.size(); i++) {

        if(bone_indices[i][0] >= bone_vertices.size() || bone_indices[i][1] >= bone_vertices.size())
            return;
        glm::vec4 first_point = bone_vertices.at(bone_indices[i][0]);
        glm::vec4 second_point = bone_vertices.at(bone_indices[i][1]);

        first_point = convertLeapToWorld(first_point, SCALE_WIDTH, SCALE_HEIGHT);
        second_point = convertLeapToWorld(second_point, SCALE_WIDTH, SCALE_HEIGHT);
        hand_vertices.push_back(first_point);
        hand_vertices.push_back(second_point);

        hand_indices.push_back(glm::uvec2(counter, counter+1));
        counter += 2;

    }
    // glm::vec4 left= convertLeapToWorld(hand_positions[0], SCALE_WIDTH, SCALE_HEIGHT);
    // glm::vec4 right = convertLeapToWorld(hand_positions[1], SCALE_WIDTH, SCALE_HEIGHT);

    //Create the new hand position vector
    // glm::vec4 left = glm::vec4(world_x_left, world_y_left, world_z_left, hand_positions[0].w);
    // glm::vec4 right = glm::vec4(world_x_right, world_y_right, world_z_right, hand_positions[1].w);

    // hand_vertices.push_back(glm::vec4(0,100,0,1));
    // hand_vertices.push_back(glm::vec4(0,0,0,1));
    // hand_indices.push_back(glm::uvec2(0,1));


    // hand_vertices.push_back(left);
    // hand_vertices.push_back(glm::vec4(0,0,0,1));
    // hand_indices.push_back(glm::uvec2(0,1));
  // Leap::Frame frame = controller_.frame();

  // // Draw all the bones in the hands!
  // for (int h = 0; h < frame.hands().count(); ++h) {
  //   Leap::Hand hand = frame.hands()[h];

  //   for (int f = 0; f < hand.fingers().count(); ++f) {
  //     Leap::Finger finger = hand.fingers()[f];

  //     for (int b = 0; b < 4; ++b)
  //       drawBone(finger.bone(static_cast<Leap::Bone::Type>(b)), bone_vertices, bone_indices);
  //   }
  // }

}


/*
int main(int argc, char** argv) {
    // Create a sample listener and controller
    SampleListener listener;
    Controller controller;

    // Have the sample listener receive events from the controller
    controller.addListener(listener);

    if (argc > 1 && strcmp(argv[1], "--bg") == 0)
        controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

    // Keep this process running until Enter is pressed
    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();

    // Remove the sample listener when done
    controller.removeListener(listener);

    return 0;
}*/
