#include <GL/glew.h>
#include <dirent.h>

#include "bone_geometry.h"
#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
#include "gui.h"
#include "menger.h"

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

const char* line_vertex_shader =
#include "shaders/line.vert"
;

const char* line_fragment_shader =
#include "shaders/line.frag"
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

std::shared_ptr<Menger> g_menger;

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cerr << "Input model file is missing" << std::endl;
		std::cerr << "Usage: " << argv[0] << " <PMD file>" << std::endl;
		return -1;
	}
	GLFWwindow *window = init_glefw();
	GUI gui(window);

	std::vector<glm::vec4> floor_vertices;
	std::vector<glm::uvec3> floor_faces;
	create_floor(floor_vertices, floor_faces);

	// //add our leap motion code
 //    // Create a sample listener and controller
 //    SampleListener listener;
 //    Controller controller;

 //    // Have the sample listener receive events from the controller
 //    controller.addListener(listener);

 //    if (argc > 1 && strcmp(argv[1], "--bg") == 0)
 //      controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

 //    /*
 //    // Keep this process running until Enter is pressed
 //    std::cout << "Press Enter to quit..." << std::endl;
 //    std::cin.get();
 //    // Remove the sample listener when done
 //    controller.removeListener(listener);
 //    */

	Mesh mesh;
	mesh.loadpmd(argv[1]);
	std::cout << "argv[1]" << argv[1] << std::endl;
	std::cout << "Loaded object  with  " << mesh.vertices.size()
		<< " vertices and " << mesh.faces.size() << " faces.\n";

	glm::vec4 mesh_center = glm::vec4(0.0f);
	for (size_t i = 0; i < mesh.vertices.size(); ++i) {
		mesh_center += mesh.vertices[i];
	}
	mesh_center /= mesh.vertices.size();

	/*
	 * GUI object needs the mesh object for bone manipulation.
	 */
	gui.assignMesh(&mesh);

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
	auto bone_matrix_binder = [&mesh](int loc, const void* data) {
		auto nelem = mesh.getNumberOfBones();
		glUniformMatrix4fv(loc, nelem, GL_FALSE, (const GLfloat*)data);
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
	// FIXME: add more lambdas for data_source if you want to use RenderPass.
	//        Otherwise, do whatever you like here
	ShaderUniform std_model = { "model", matrix_binder, std_model_data };
	ShaderUniform floor_model = { "model", matrix_binder, floor_model_data };
	ShaderUniform std_view = { "view", matrix_binder, std_view_data };
	ShaderUniform std_camera = { "camera_position", vector3_binder, std_camera_data };
	ShaderUniform std_proj = { "projection", matrix_binder, std_proj_data };
	ShaderUniform std_light = { "light_position", vector_binder, std_light_data };
	ShaderUniform object_alpha = { "alpha", float_binder, alpha_data };
	// FIXME: define more ShaderUniforms for RenderPass if you want to use it.
	//        Otherwise, do whatever you like here

	std::vector<glm::vec2>& uv_coordinates = mesh.uv_coordinates;
	RenderDataInput object_pass_input;
	object_pass_input.assign(0, "vertex_position", nullptr, mesh.vertices.size(), 4, GL_FLOAT);
	object_pass_input.assign(1, "normal", mesh.vertex_normals.data(), mesh.vertex_normals.size(), 4, GL_FLOAT);
	object_pass_input.assign(2, "uv", uv_coordinates.data(), uv_coordinates.size(), 2, GL_FLOAT);
	object_pass_input.assign_index(mesh.faces.data(), mesh.faces.size(), 3);
	object_pass_input.useMaterials(mesh.materials);
	RenderPass object_pass(-1,
			object_pass_input,
			{
			  vertex_shader,
			  geometry_shader,
			  fragment_shader
			},
			{ std_model, std_view, std_proj,
			  std_light,
			  std_camera, object_alpha },
			{ "fragment_color" }
			);

	// FIXME: Create the RenderPass objects for bones here.
	//        Otherwise do whatever you like.

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
	std::cout << "center = " << mesh.getCenter() << "\n";

	std::vector<glm::vec4> cube_vertices;
	std::vector<glm::vec4> vtx_normals;
	std::vector<glm::uvec3> cube_faces;
	g_menger->generate_geometry(cube_vertices, vtx_normals, cube_faces, glm::vec3(0.0f, 10.0f, 0.0f));

	std::vector<glm::vec4> line_vertices;
	std::vector<glm::vec4> line_vtx_normals;
	std::vector<glm::uvec3> line_faces;
	g_menger->generate_outer_geometry(line_vertices, line_vtx_normals, line_faces, glm::vec3(0.0f, 10.0f, 0.0f));

	bool draw_floor = true;
	bool draw_skeleton = true;
	bool draw_object = true;
	bool draw_cylinder = true;

	float scale_factor = 1.0f;
	float rIntensity = 0.0;
	float gIntensity = 0.0;
	bool reverse = false;
	while (!glfwWindowShouldClose(window)) {
		glEnable(GL_CULL_FACE);
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
		}

		glm::vec4 colorC = glm::vec4(rIntensity, gIntensity, 0.0f, 1.0f);
		
		auto color_data = [&colorC]() -> const void* {
			return &colorC;
		};
		rIntensity += 0.01f;
		gIntensity += 0.0055f;
		// intensity -= 0.01f;
		ShaderUniform color = { "colorC", vector_binder, color_data };

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


		RenderDataInput cube_pass_input;
		cube_pass_input.assign(0, "vertex_position", cube_vertices.data(), cube_vertices.size(), 4, GL_FLOAT);
		cube_pass_input.assign(1, "normal", vtx_normals.data(), vtx_normals.size(), 4, GL_FLOAT);
		cube_pass_input.assign_index(cube_faces.data(), cube_faces.size(), 3);
		cube_pass_input.useMaterials(mesh.materials);
		glm::vec3 diffuse;
		glm::vec3 ambient;
		glm::vec3 specular;
		float shininess;
		for (int h = 3; h < 4; h++) {
			auto& ma = mesh.materials.at(h);
			
			diffuse += ma.diffuse[0];
			ambient += ma.ambient[0];
			specular += ma.specular[0];
			shininess += ma.shininess;
			std::cout << "ambient: " << ambient.x << " " << ambient.y << " " << ambient.z << std::endl;

		}
		std::vector<glm::vec4> components;
		components.push_back(glm::vec4(diffuse,0.0));
		components.push_back(glm::vec4(ambient,0.0));
		components.push_back(glm::vec4(specular,0.0));
		std::cout << "components[0]: " << components[0].x << " " << components[0].y << " " << components[0].z << std::endl;

		std::vector<float> shini;
		shini.push_back(shininess);
		auto diffuse_data = [&components]() -> const void* {
			return &components[0];
		};
		auto ambient_data = [&components]() -> const void* {
			return &components[1];
		};
		auto specular_data = [&components]() -> const void* {
			return &components[2];
		};	
		auto shininess_data = [&shini]() -> const void* {
			return &shini[0];
		};
		
		ShaderUniform std_diffuse = { "diffuse", vector_binder, diffuse_data };
		ShaderUniform std_ambience = { "ambient", vector_binder, ambient_data };
		ShaderUniform std_specular = { "specular", vector_binder, specular_data };
		ShaderUniform std_shininess = { "shininess", float_binder , shininess_data };

		RenderPass cube_pass(-1,
				cube_pass_input,
				{ vertex_shader, geometry_shader, cube_fragment_shader},
				{ std_model, std_view, std_proj, std_light, std_camera, object_alpha, std_diffuse, std_ambience, std_specular, std_shininess  },
				{ "fragment_color" }
				);
        g_menger->scale(cube_faces, cube_vertices, glm::vec3(0.0,10.0,0.0), scale_factor);
        scale_factor += 0.00001f;
        g_menger->rotate(0.05f, glm::vec3(0,1,1), cube_faces, cube_vertices, glm::vec3(0.0,10.0,0.0));
        g_menger->rotate_lines(0.05f, glm::vec3(0,1,1), line_faces, line_vertices, glm::vec3(0.0,10.0,0.0));

		cube_pass.setup();
		glDisable(GL_CULL_FACE);
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, cube_faces.size() * 3, GL_UNSIGNED_INT, 0));
	
	    line_pass.setup();
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, line_faces.size() * 3, GL_UNSIGNED_INT, 0));

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
