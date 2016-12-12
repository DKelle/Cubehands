

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

const char* joint_fragment_shader =
#include "shaders/joint.frag"
;

const char* hands_vertex_shader = 
#include "shaders/hands.vert"
;

const char* hands_fragment_shader = 
#include "shaders/hands.frag"
;


const char* line_vertex_shader =
#include "shaders/line.vert"
;

const char* line_fragment_shader =
#include "shaders/line.frag"
;


bool reset = false;

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


    float rIntensity = 0.0;
    float gIntensity = 0.0;
    glm::vec4 colorC = glm::vec4(rIntensity, gIntensity, 0.0f, 1.0f);

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
    auto color_data = [&colorC]() -> const void* {
        return &colorC;
    };
    
    float SIZE_CUBE = 5.00f;

    //Use these definitions to send matrices to renderpass
    ShaderUniform std_model = { "model", matrix_binder, std_model_data };
    ShaderUniform floor_model = { "model", matrix_binder, floor_model_data };
    ShaderUniform std_view = { "view", matrix_binder, std_view_data };
    ShaderUniform std_camera = { "camera_position", vector3_binder, std_camera_data };
    ShaderUniform std_proj = { "projection", matrix_binder, std_proj_data };
    ShaderUniform std_light = { "light_position", vector_binder, std_light_data };
    ShaderUniform object_alpha = { "alpha", float_binder, alpha_data };
    ShaderUniform color = { "colorC", vector_binder, color_data };

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
    glm::vec3 origin = glm::vec3(0,15,-100);
    glm::vec3 original_origin = origin;
    listener.g_menger->generate_geometry(cube_vertices, vtx_normals, cube_faces, origin, SIZE_CUBE);

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

    bool draw_floor = true;
    bool draw_skeleton = true;
    bool draw_object = true;
    bool draw_cylinder = true;
    bool draw_left;
    bool draw_right; 

    //Render the hands
    std::vector<glm::vec4> left_cylinder_vertices;
    std::vector<glm::uvec2> left_cylinder_indices;
    std::vector<glm::vec4> right_cylinder_vertices;
    std::vector<glm::uvec2> right_cylinder_indices;
    int counter = 0;
    for(int i = 0; i < 400; i ++)
    {
        right_cylinder_vertices.push_back(glm::vec4(0,0,-1,1));
        right_cylinder_vertices.push_back(glm::vec4(0,100,-1, 1));
        right_cylinder_indices.push_back(glm::uvec2(counter, counter+1));
        left_cylinder_vertices.push_back(glm::vec4(0,0,-1,1));
        left_cylinder_vertices.push_back(glm::vec4(0,100,-1, 1));
        left_cylinder_indices.push_back(glm::uvec2(counter, counter+1));
        counter += 2;
    }
    // listener.g_menger->generate_geometry(right_vertices, right_normals, right_faces, glm::vec3(0.0,15.0,0.0));

    //Create the center cube
    std::vector<glm::vec4> right_joint_vertices;
    std::vector<glm::vec4> right_joint_normals;
    std::vector<glm::uvec3> right_joint_faces;
    std::vector<glm::vec4> left_joint_vertices;
    std::vector<glm::vec4> left_joint_normals;
    std::vector<glm::uvec3> left_joint_faces;
    for(int i = 0; i < 200; i ++)
    {
        listener.g_menger->generate_geometry(right_joint_vertices, right_joint_normals, right_joint_faces, origin, 1.00f);
        listener.g_menger->generate_geometry(left_joint_vertices, left_joint_normals, left_joint_faces, origin, 1.00f);
    }

    //render joints
    RenderDataInput right_joint_pass_input;
    right_joint_pass_input.assign(0, "vertex_position", right_joint_vertices.data(), right_joint_vertices.size(), 4, GL_FLOAT);
    right_joint_pass_input.assign(1, "normal", right_joint_normals.data(), right_joint_normals.size(), 4, GL_FLOAT);
    right_joint_pass_input.assign_index(right_joint_faces.data(), right_joint_faces.size(), 3);
    RenderPass right_joint_pass(-1,
           right_joint_pass_input,
           { vertex_shader, geometry_shader, joint_fragment_shader},
           { std_model, std_view, std_proj, std_light },
           { "fragment_color" }
           );

    RenderDataInput left_joint_pass_input;
    left_joint_pass_input.assign(0, "vertex_position", left_joint_vertices.data(), left_joint_vertices.size(), 4, GL_FLOAT);
    left_joint_pass_input.assign(1, "normal", left_joint_normals.data(), left_joint_normals.size(), 4, GL_FLOAT);
    left_joint_pass_input.assign_index(left_joint_faces.data(), left_joint_faces.size(), 3);
    RenderPass left_joint_pass(-1,
           left_joint_pass_input,
           { vertex_shader, geometry_shader, joint_fragment_shader},
           { std_model, std_view, std_proj, std_light },
           { "fragment_color" }
           );

    //Create a cylinder 
    RenderDataInput left_cylinder_pass_input;
    left_cylinder_pass_input.assign(0, "vertex_position", left_cylinder_vertices.data(), left_cylinder_vertices.size(), 4, GL_FLOAT);
    left_cylinder_pass_input.assign_index(left_cylinder_indices.data(), left_cylinder_indices.size(), 2);
    RenderPass left_cylinder_pass(-1,
            left_cylinder_pass_input,
            { hands_vertex_shader, nullptr, hands_fragment_shader},
            { std_model, std_view, std_proj},
            { "fragment_color" }
            );

    RenderDataInput right_cylinder_pass_input;
    right_cylinder_pass_input.assign(0, "vertex_position", right_cylinder_vertices.data(), right_cylinder_vertices.size(), 4, GL_FLOAT);
    right_cylinder_pass_input.assign_index(right_cylinder_indices.data(), right_cylinder_indices.size(), 2);
    RenderPass right_cylinder_pass(-1,
            right_cylinder_pass_input,
            { hands_vertex_shader, nullptr, hands_fragment_shader},
            { std_model, std_view, std_proj},
            { "fragment_color" }
            );



    std::vector<glm::vec4> line_vertices;
    std::vector<glm::vec4> line_vtx_normals;
    std::vector<glm::uvec3> line_faces;
    listener.g_menger->generate_outer_geometry(line_vertices, line_vtx_normals, line_faces, origin, SIZE_CUBE);


    
    //listener.g_menger->create_cylinder(cylinder_vertices, cylinder_indices, glm::vec4(0,0,0,1), glm::vec4(0,1,0,1));


    

    // rIntensity += 0.01f;
    // gIntensity += 0.0055f;
    // intensity -= 0.01f;

    RenderDataInput line_pass_input;
    line_pass_input.assign(0, "vertex_position", line_vertices.data(), line_vertices.size(), 4, GL_FLOAT);
    line_pass_input.assign(1, "normal", line_vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);
    line_pass_input.assign_index(line_faces.data(), line_faces.size(), 3);
    RenderPass line_pass(-1,
            line_pass_input,
            { vertex_shader, geometry_shader, line_fragment_shader},
            { std_model, std_view, std_proj, std_light, color },
            { "fragment_color" }
            );


    


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

            line_pass.setup();
            CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, line_faces.size() * 3, GL_UNSIGNED_INT, 0));
        }


        
        //Get the current hand positions
        std::vector<glm::vec4> hand_pos = listener.get_hand_positions(100, 100);
        glm::vec4 left = hand_pos.at(0);
        glm::vec4 right = hand_pos.at(1);
        draw_left = left.w;
        draw_right = right.w;

        try {

            if(draw_left) {
                listener.drawHands(left_joint_vertices, left_joint_faces, left_joint_normals, left_cylinder_vertices, left_cylinder_indices, LEFT);
            }
            else
            {
                left_cylinder_vertices.clear();
                left_cylinder_indices.clear();
                left_joint_vertices.clear();
                left_joint_faces.clear();
            }
            if(draw_right)
            {
                listener.drawHands(right_joint_vertices, right_joint_faces, right_joint_normals, right_cylinder_vertices, right_cylinder_indices, RIGHT);

            }
            else
            {
                right_cylinder_vertices.clear();
                right_cylinder_indices.clear();
                right_joint_vertices.clear();
                right_joint_faces.clear();
            }

            //draw cubes at joint on left hands
            left_joint_pass.setup();
            left_joint_pass.updateVBO(0, left_joint_vertices.data(), left_joint_vertices.size());
            CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, left_joint_faces.size() * 3, GL_UNSIGNED_INT, 0));

            //draw cylinders for fingers on left hands
            left_cylinder_pass.setup();
            left_cylinder_pass.updateVBO(0, left_cylinder_vertices.data(), left_cylinder_vertices.size());
            CHECK_GL_ERROR(glDrawElements(GL_LINES, left_cylinder_indices.size()*2, GL_UNSIGNED_INT, 0));

    
            //draw cubes at joints on right hand
            right_joint_pass.setup();
            right_joint_pass.updateVBO(0, right_joint_vertices.data(), right_joint_vertices.size());
            CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, right_joint_faces.size() * 3, GL_UNSIGNED_INT, 0));

            //draw cylinders for fingers on left hands
            right_cylinder_pass.setup();
            right_cylinder_pass.updateVBO(0, right_cylinder_vertices.data(), right_cylinder_vertices.size());
            CHECK_GL_ERROR(glDrawElements(GL_LINES, right_cylinder_indices.size()*2, GL_UNSIGNED_INT, 0));
    
            int left_fingers = listener.digits.at(LEFT);
            int right_fingers = listener.digits.at(RIGHT);

            bool scale = (draw_left && left_fingers == 5) && (draw_right && listener.scale_prob.at(RIGHT) > 0.9);
            bool translate = draw_right && listener.pointable_list.at(RIGHT).count() == 1;
            bool rotate = (draw_left && left_fingers == 0);
            if(rotate || scale || translate) {
                if(rIntensity <= 1 && gIntensity <= 0.55f) {
                    rIntensity += 0.01f;
                    gIntensity += 0.0055f;
                    colorC = glm::vec4(rIntensity, gIntensity,0,1);
                }
            }
            else {
                if (rIntensity >= 0 && gIntensity >= 0) {
                    rIntensity -= 0.01f;
                    gIntensity -= 0.0055f;
                }
                colorC = glm::vec4(rIntensity, gIntensity,0,1);
            }

            if(rotate) {
                listener.g_menger->rotate(listener.rotation_matrices.at(LEFT), cube_faces, cube_vertices, origin);
                cube_pass.updateVBO(0, cube_vertices.data(), cube_vertices.size());
                cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);

                listener.g_menger->rotate(listener.rotation_matrices.at(LEFT), line_faces, line_vertices, origin);
                line_pass.updateVBO(0, line_vertices.data(), line_vertices.size());
                line_pass_input.assign(1, "normal", line_vtx_normals.data(), line_vtx_normals.size(), 4, GL_FLOAT);
            } 


            // use pointable??
            if(translate) {
                // NEED the order of the multiplication
                glm::vec3 translation = glm::vec3(listener.translation_vectors.at(RIGHT)) * 0.5f;
                origin = translation + origin;
                listener.g_menger->translate(cube_faces, cube_vertices, translation);
                cube_pass.updateVBO(0, cube_vertices.data(), cube_vertices.size());
                cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);

                listener.g_menger->translate(line_faces, line_vertices, translation);
                line_pass.updateVBO(0, line_vertices.data(), line_vertices.size());
                line_pass_input.assign(1, "normal", line_vtx_normals.data(), line_vtx_normals.size(), 4, GL_FLOAT);
            }

            if(scale) {
                // float temp_speed = (direction < 0) ? 1.1f : .9f;
                float temp_speed = listener.scale_factor.at(RIGHT);
                listener.g_menger->scale(cube_faces, cube_vertices, origin, temp_speed);
                cube_pass.updateVBO(0, cube_vertices.data(), cube_vertices.size());
                cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);


                listener.g_menger->scale(line_faces, line_vertices, origin, temp_speed);
                line_pass.updateVBO(0, line_vertices.data(), line_vertices.size());
                line_pass_input.assign(1, "normal", line_vtx_normals.data(), line_vtx_normals.size(), 4, GL_FLOAT);
            }

            if(gui.reset) {
                cube_vertices.clear();
                vtx_normals.clear();
                cube_faces.clear();

                line_vertices.clear();
                line_vtx_normals.clear();
                line_faces.clear();
                origin = original_origin;
                listener.g_menger->generate_geometry(cube_vertices, vtx_normals, cube_faces, origin, SIZE_CUBE);
                cube_pass.updateVBO(0, cube_vertices.data(), cube_vertices.size());
                cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);

                listener.g_menger->generate_outer_geometry(line_vertices, line_vtx_normals, line_faces, origin, SIZE_CUBE);
                line_pass.updateVBO(0, line_vertices.data(), line_vertices.size());
                line_pass_input.assign(1, "normal", line_vtx_normals.data(), line_vtx_normals.size(), 4, GL_FLOAT);

                gui.reset = false;
            }

        } catch(const std::out_of_range& e) {
            printf("error in main\n");
            exit(0);
        }

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
