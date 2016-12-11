

#include <GL/glew.h>
#include <dirent.h>

#include "render_pass.h"
#include "config.h"
#include "gui.h"
#include "menger.h"
#include "Sample.h"
#include "procedure_geometry.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>

int window_width = 800, window_height = 600;
const std::string window_title = "Skinning";

const char* vertex_shader =
#include "shaders/default.vert"
;

const char* geometry_shader =
#include "shaders/default.geom"
;

const char* fragment_shader =
#include "shaders/default.frag"
;

const char* floor_fragment_shader =
#include "shaders/floor.frag"
;

const char* cube_fragment_shader =
#include "shaders/cube.frag"
;

const char* hands_vertex_shader = 
#include "shaders/hands.vert"
;

const char* hands_fragment_shader = 
#include "shaders/hands.frag"
;

// FIXME: Add more shaders here.

void ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << "\n";
}

GLFWwindow* init_glefw()
{
    if (!glfwInit())
        exit(EXIT_FAILURE);
    glfwSetErrorCallback(ErrorCallback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    auto ret = glfwCreateWindow(window_width, window_height, window_title.data(), nullptr, nullptr);
    CHECK_SUCCESS(ret != nullptr);
    glfwMakeContextCurrent(ret);
    glewExperimental = GL_TRUE;
    CHECK_SUCCESS(glewInit() == GLEW_OK);
    glGetError();  // clear GLEW's error for it
    glfwSwapInterval(1);
    const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
    const GLubyte* version = glGetString(GL_VERSION);    // version as a string
    std::cout << "Renderer: " << renderer << "\n";
    std::cout << "OpenGL version supported:" << version << "\n";

    return ret;
}

//std::shared_ptr<Menger> listener.g_menger;

int main(int argc, char* argv[])
{
    GLFWwindow *window = init_glefw();
    SampleListener listener;
    Controller controller;

    // Have the sample listener receive events from the controller
    controller.addListener(listener);
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
    GUI gui(window);

    std::vector<glm::vec4> floor_vertices;
    std::vector<glm::uvec3> floor_faces;
    create_floor(floor_vertices, floor_faces);

    // listener.g_menger->fill_origin();

    glm::vec4 light_position = glm::vec4(0.0f, 100.0f, 0.0f, 1.0f);
    MatrixPointers mats; // Define MatrixPointers here for lambda to capture
    /*
     * In the following we are going to define several lambda functions to bind Uniforms.
     * 
     * Introduction about lambda functions:
     *      http://en.cppreference.com/w/cpp/language/lambda
     *      http://www.stroustrup.com/C++11FAQ.html#lambda
     */
    auto matrix_binder = [](int loc, const void* data) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)data);
    };
    auto vector_binder = [](int loc, const void* data) {
        glUniform4fv(loc, 1, (const GLfloat*)data);
    };
    auto vector3_binder = [](int loc, const void* data) {
        glUniform3fv(loc, 1, (const GLfloat*)data);
    };
    auto float_binder = [](int loc, const void* data) {
        glUniform1fv(loc, 1, (const GLfloat*)data);
    };
    /*
     * These lambda functions below are used to retrieve data
     */
    auto std_model_data = [&mats]() -> const void* {
        return mats.model;
    }; // This returns point to model matrix
    glm::mat4 floor_model_matrix = glm::mat4(1.0f);
    auto floor_model_data = [&floor_model_matrix]() -> const void* {
        return &floor_model_matrix[0][0];
    }; // This return model matrix for the floor.
    auto std_view_data = [&mats]() -> const void* {
        return mats.view;
    };
    auto std_camera_data  = [&gui]() -> const void* {
        return &gui.getCamera()[0];
    };
    auto std_proj_data = [&mats]() -> const void* {
        return mats.projection;
    };
    auto std_light_data = [&light_position]() -> const void* {
        return &light_position[0];
    };
    auto alpha_data  = [&gui]() -> const void* {
        static const float transparet = 0.5; // Alpha constant goes here
        static const float non_transparet = 1.0;
        if (gui.isTransparent())
            return &transparet;
        else
            return &non_transparet;
    };
    

    //Use these definitions to send matrices to renderpass
    ShaderUniform std_model = { "model", matrix_binder, std_model_data };
    ShaderUniform floor_model = { "model", matrix_binder, floor_model_data };
    ShaderUniform std_view = { "view", matrix_binder, std_view_data };
    ShaderUniform std_camera = { "camera_position", vector3_binder, std_camera_data };
    ShaderUniform std_proj = { "projection", matrix_binder, std_proj_data };
    ShaderUniform std_light = { "light_position", vector_binder, std_light_data };
    ShaderUniform object_alpha = { "alpha", float_binder, alpha_data };

    //Create the floor
    RenderDataInput floor_pass_input;
    floor_pass_input.assign(0, "vertex_position", floor_vertices.data(), floor_vertices.size(), 4, GL_FLOAT);
    floor_pass_input.assign_index(floor_faces.data(), floor_faces.size(), 3);
    RenderPass floor_pass(-1,
            floor_pass_input,
            { vertex_shader, geometry_shader, floor_fragment_shader},
            { floor_model, std_view, std_proj, std_light },
            { "fragment_color" }
            );
    float aspect = 0.0f;

    //Create the center cube
    std::vector<glm::vec4> cube_vertices;
    std::vector<glm::vec4> vtx_normals;
    std::vector<glm::uvec3> cube_faces;
    glm::vec3 origin = glm::vec3(0,15,0);
    listener.g_menger->generate_geometry(cube_vertices, vtx_normals, cube_faces, origin, 5.00f);

    RenderDataInput cube_pass_input;
    cube_pass_input.assign(0, "vertex_position", cube_vertices.data(), cube_vertices.size(), 4, GL_FLOAT);
    cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);
    cube_pass_input.assign_index(cube_faces.data(), cube_faces.size(), 3);
    RenderPass cube_pass(-1,
           cube_pass_input,
           { vertex_shader, geometry_shader, cube_fragment_shader},
           { std_model, std_view, std_proj, std_light },
           { "fragment_color" }
           );

    // //Create the left hand 
    // std::vector<glm::vec4> left_vertices;
    // std::vector<glm::vec4> left_normals;
    // std::vector<glm::uvec3> left_faces;
    // listener.g_menger->generate_geometry(left_vertices, left_normals, left_faces, glm::vec3(0.0,15.0,0.0));

    // RenderDataInput left_pass_input;
    // left_pass_input.assign(0, "vertex_position", left_vertices.data(), left_vertices.size(), 4, GL_FLOAT);
    // left_pass_input.assign(1, "normal", left_normals.data(), left_normals.size(), 4, GL_FLOAT);
    // left_pass_input.assign_index(left_faces.data(), left_faces.size(), 3);
    // RenderPass left_pass(-1,
    //         left_pass_input,
    //         { vertex_shader, geometry_shader, cube_fragment_shader},
    //         { std_model, std_view, std_proj, std_light },
    //         { "fragment_color" }
    //         );

    // //Create the right hande
    // std::vector<glm::vec4> right_vertices;
    // std::vector<glm::vec4> right_normals;
    // std::vector<glm::uvec3> right_faces;
    // listener.g_menger->generate_geometry(right_vertices, right_normals, right_faces, glm::vec3(0.0,15.0,0.0));

    // RenderDataInput right_pass_input;
    // right_pass_input.assign(0, "vertex_position", right_vertices.data(), right_vertices.size(), 4, GL_FLOAT);
    // right_pass_input.assign(1, "normal", right_normals.data(), right_normals.size(), 4, GL_FLOAT);
    // right_pass_input.assign_index(right_faces.data(), right_faces.size(), 3);
    // RenderPass right_pass(-1,
    //         right_pass_input,
    //         { vertex_shader, geometry_shader, cube_fragment_shader},
    //         { std_model, std_view, std_proj, std_light },
    //         { "fragment_color" }
    //         );

    bool draw_floor = true;
    bool draw_skeleton = true;
    bool draw_object = true;
    bool draw_cylinder = true;
    bool draw_left;
    bool draw_right; 


    //Render the hands
    std::vector<glm::vec4> bone_vertices;
    std::vector<glm::uvec2> bone_indices;
    int counter = 0;
    for(int i = 0; i < 400; i ++)
    {
        bone_vertices.push_back(glm::vec4(0,0,-1,1));
        bone_vertices.push_back(glm::vec4(0,100,-1, 1));
        bone_indices.push_back(glm::uvec2(counter, counter+1));
        counter += 2;
    }
    // listener.g_menger->generate_geometry(right_vertices, right_normals, right_faces, glm::vec3(0.0,15.0,0.0));

    //Create the center cube
    std::vector<glm::vec4> joint_vertices;
    std::vector<glm::vec4> joint_normals;
    std::vector<glm::uvec3> joint_faces;
    origin = glm::vec3(0,20,0);
    listener.g_menger->generate_geometry(joint_vertices, joint_normals, joint_faces, origin, 1.00f);
    origin = glm::vec3(3,20,0);
    for(int i = 0; i < 200; i ++)
    {
        listener.g_menger->generate_geometry(joint_vertices, joint_normals, joint_faces, origin, 1.00f);
    }

    RenderDataInput joint_pass_input;
    joint_pass_input.assign(0, "vertex_position", joint_vertices.data(), joint_vertices.size(), 4, GL_FLOAT);
    joint_pass_input.assign(1, "normal", joint_normals.data(), joint_normals.size(), 4, GL_FLOAT);
    joint_pass_input.assign_index(joint_faces.data(), joint_faces.size(), 3);
    RenderPass joint_pass(-1,
           joint_pass_input,
           { vertex_shader, geometry_shader, cube_fragment_shader},
           { std_model, std_view, std_proj, std_light },
           { "fragment_color" }
           );


    RenderDataInput bone_pass_input;
    bone_pass_input.assign(0, "vertex_position", bone_vertices.data(), bone_vertices.size(), 4, GL_FLOAT);
    bone_pass_input.assign_index(bone_indices.data(), bone_indices.size(), 2);
    RenderPass bone_pass(-1,
            bone_pass_input,
            { hands_vertex_shader, nullptr, hands_fragment_shader},
            { std_model, std_view, std_proj},
            { "fragment_color" }
            );

    //Create the center cube

    glm::vec3 rot;
    glm::vec3 prev_rot;
    while (!glfwWindowShouldClose(window)) {
        // Setup some basic window stuff.
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glCullFace(GL_BACK);

        gui.updateMatrices();
        mats = gui.getMatrixPointers();

        int current_bone = gui.getCurrentBone();

#if 1
        draw_cylinder = (current_bone != -1 && gui.isTransparent());
#else
        draw_cylinder = true;
#endif
        // FIXME: Draw bones first.
        // Then draw floor.
        if (draw_floor) {
            floor_pass.setup();
            // Draw our triangles.
            CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, floor_faces.size() * 3, GL_UNSIGNED_INT, 0));

            //also draw the centered cube
            cube_pass.setup();
            glDisable(GL_CULL_FACE);
            CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, cube_faces.size() * 3, GL_UNSIGNED_INT, 0));

        }
        
        //Get the current hand positions
        std::vector<glm::vec4> hand_pos = listener.get_hand_positions(100, 100);
        glm::vec4 left = hand_pos[0];
        glm::vec4 right = hand_pos[1];
        draw_left = left.w;
        draw_right = right.w;

        if(draw_left || draw_right) {
            // Leap::Frame frame = controller.frame();         
            listener.drawHands(bone_vertices, bone_indices, joint_vertices, joint_faces, joint_normals);
            printf("joints has %d vertices \n", joint_vertices.size());
            bone_pass.setup();
            bone_pass.updateVBO(0, bone_vertices.data(), bone_vertices.size());
            //joint_pass.updateVBO(0, joint_vertices.data(), joint_vertices.size());

            CHECK_GL_ERROR(glDrawElements(GL_LINES, bone_indices.size()*2, GL_UNSIGNED_INT, 0));

            //draw cubes at joints
            joint_pass.setup();
            joint_pass.updateVBO(0, joint_vertices.data(), joint_vertices.size());
            CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, joint_faces.size() * 3, GL_UNSIGNED_INT, 0));
        }

        //Render the left hand
        // if(draw_left)
        // {
        //     left_pass.setup();
        //     listener.g_menger->generate_geometry(left_vertices, left_normals, left_faces, glm::vec3(left));
        //     left_pass.updateVBO(0, left_vertices.data(), left_vertices.size());	
        //     //CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, left_faces.size() * 3, GL_UNSIGNED_INT, 0));
            
        // }
        // //Render the right hand
        // if(draw_right)
        // {
        //     right_pass.setup();
        //     listener.g_menger->generate_geometry(right_vertices, right_normals, right_faces, glm::vec3(right));
        //     right_pass.nupdateVBO(0, right_vertices.data(), right_vertices.size());	
        //     //CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, right_faces.size() * 3, GL_UNSIGNED_INT, 0));
            
        // }

        //calculate the delta hand positions, and the axis of rotation
        std::vector<glm::vec4> old_hand_pos = listener.get_old_hand_positions(100, 100);
        glm::vec4 old_left = old_hand_pos[0];
        glm::vec4 old_right = old_hand_pos[1];
        glm::vec4 delta_left = old_left - left;
        glm::vec4 delta_right = old_right - right;
        glm::vec3 delta_left_3 = glm::vec3(delta_left);
        glm::vec3 delta_right_3 = glm::vec3(delta_right);

        float direction = glm::dot(delta_left_3, glm::vec3(0,1,0));
        prev_rot = rot;
        rot = glm::normalize(glm::cross(delta_right_3, delta_left_3));
        glm::vec3 avg_rot = (rot + prev_rot) / 2.0f;
        //check that our hands were visible this fram and last frame
        bool draw_old_left = old_left.w;
        bool draw_old_right = old_right.w;

        int left_fingers = listener.digits[LEFT];
        int right_fingers = listener.digits[RIGHT];

        float speed = (glm::length(delta_left_3) + glm::length(delta_right_3))/2;
        bool rotate = speed > .5 && draw_old_left && draw_old_right && draw_right && draw_left && left_fingers == 0 && right_fingers == 0;
        // bool scale = speed > .6 && left_fingers == 5 && right_fingers == 5 && draw_old_left && draw_old_right && draw_left && draw_right;
        // printf("scaled: %f, %f\n", listener.scale_prob[LEFT], listener.scale_prob[RIGHT]);
        // bool scale = (draw_left && listener.scale_prob[LEFT] > 0.9) || (draw_right && listener.scale_prob[RIGHT] > 0.9);
        bool scale = (draw_right && listener.scale_prob[RIGHT] > 0.9);


        //rotate
        if(draw_left && left_fingers == 0) {
            listener.g_menger->rotate(listener.rotation_matrices[LEFT], cube_faces, cube_vertices, origin);
            cube_pass.updateVBO(0, cube_vertices.data(), cube_vertices.size());
            cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);
        } 
        // else if(scale) {
        //     // float temp_speed = (direction < 0) ? 1.1f : .9f;
        //     float temp_speed = listener.scale_factor[RIGHT];
        //     listener.g_menger->scale(cube_faces, cube_vertices, origin, temp_speed);
        //     cube_pass.updateVBO(0, cube_vertices.data(), cube_vertices.size());
        // }

        //translate 
        if(draw_right && listener.pointable_list[RIGHT].count() == 1) {
            //NEED the order of the multiplication
            glm::vec3 translation = glm::vec3(listener.translation_vectors[RIGHT]) * 0.1f;
            origin = translation + origin;
            listener.g_menger->translate(cube_faces, cube_vertices, translation);
            cube_pass.updateVBO(0, cube_vertices.data(), cube_vertices.size());
            cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);
        }

        // if(draw_left && draw_right && )

        // if(rotate)
        // {
        //     float temp_speed = (direction < 0) ? -.1f : .1f;
        //     // listener.g_menger->rotate(temp_speed, avg_rot, cube_faces, cube_vertices, glm::vec3(0,15,0));
        //     // cube_pass.updateVBO(0, cube_vertices.data(), cube_vertices.size());
        //     // cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);
        // } else if(scale)
        // {

        // }

        // Poll and swap.
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
#if 0
    for (size_t i = 0; i < images.size(); ++i)
        delete [] images[i].bytes;
#endif
    exit(EXIT_SUCCESS);
}
