#include "gui.h"
#include "config.h"
#include <jpegio.h>
#include "bone_geometry.h"
#include <iostream>
#include <debuggl.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>

GUI::GUI(GLFWwindow* window)
	:window_(window)
{
	glfwSetWindowUserPointer(window_, this);
	glfwSetKeyCallback(window_, KeyCallback);
	glfwSetCursorPosCallback(window_, MousePosCallback);
	glfwSetMouseButtonCallback(window_, MouseButtonCallback);

	glfwGetWindowSize(window_, &window_width_, &window_height_);
	float aspect_ = static_cast<float>(window_width_) / window_height_;
	projection_matrix_ = glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
}

GUI::~GUI()
{
}

void GUI::assignMesh(Mesh* mesh)
{
	mesh_ = mesh;
	center_ = mesh_->getCenter();
}

void GUI::keyCallback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window_, GL_TRUE);
		return ;
	}
	if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
		//FIXME save out a screenshot using SaveJPEG
		unsigned char*  data = new unsigned char[window_width_ * window_height_ * 3];
		glReadPixels(0,0,window_width_,window_height_,GL_RGB,GL_UNSIGNED_BYTE,data);
		SaveJPEG("../anime_girl_screenshot.jpg", window_width_,window_height_,data);
	}

	if (captureWASDUPDOWN(key, action))
		return ;
	if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
		float roll_speed;
		if (key == GLFW_KEY_RIGHT)
			roll_speed = -roll_speed_;
		else
			roll_speed = roll_speed_;
		if(current_bone_ != -1)
		{

			// FIXME: actually roll the bone here

			Bone* b = mesh_->skeleton.bones[current_bone_];
			b->A = b->A * glm::inverse(b->S);
			int jid_to = b->to_joint;
	
			Joint* to = mesh_->skeleton.joints[jid_to];

			glm::vec3 t_offset = to->offset;
			glm::vec3 tangent =  glm::vec3(b->S[2][0], b->S[2][1], b->S[2][2]);
			b->S = glm::rotate(roll_speed, tangent) * b->S;
			b->A = b->A * b->S;

			mesh_->recompose_matrices(b);
			//now that we have the offset delta, recursively apply it to all of our children
			
			//now that we have preformed the rotation, reclaculate the skelle
			mesh_->skeleton.dirty = true;
			pose_changed_= true;
			return ;
		}
	} else if (key == GLFW_KEY_C && action != GLFW_RELEASE) {
		fps_mode_ = !fps_mode_;
	} else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_RELEASE) {
		current_bone_--;
		current_bone_ += mesh_->getNumberOfBones();
		current_bone_ %= mesh_->getNumberOfBones();
	} else if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_RELEASE) {
		current_bone_++;
		current_bone_ += mesh_->getNumberOfBones();
		current_bone_ %= mesh_->getNumberOfBones();
	} else if (key == GLFW_KEY_T && action != GLFW_RELEASE) {
		transparent_ = !transparent_;
	}else if (key == GLFW_KEY_M && action != GLFW_RELEASE) {
		fps_mode_ = !fps_mode_;	
	}
}

void GUI::mousePosCallback(double mouse_x, double mouse_y)
{
	last_x_ = current_x_;
	last_y_ = current_y_;
	current_x_ = mouse_x;
	current_y_ = window_height_ - mouse_y;
	mouse_pos = glm::vec2(current_x_, current_y_);
	float delta_x = current_x_ - last_x_;
	float delta_y = current_y_ - last_y_;
	if (sqrt(delta_x * delta_x + delta_y * delta_y) < 1e-15)
		return;
	glm::vec3 mouse_direction = glm::normalize(glm::vec3(delta_x, delta_y, 0.0f));
	glm::vec2 mouse_start = glm::vec2(last_x_, last_y_);
	glm::vec2 mouse_end = glm::vec2(current_x_, current_y_);
	glm::uvec4 viewport = glm::uvec4(0, 0, window_width_, window_height_);


	glm::vec3 near = glm::unProject(glm::vec3(mouse_pos.x, mouse_pos.y,0), view_matrix_,projection_matrix_, viewport);
	glm::vec3 far = glm::unProject(glm::vec3(mouse_pos.x, mouse_pos.y,1), view_matrix_,projection_matrix_, viewport);
	mouse_dir = glm::normalize(far-near);

	bool drag_camera = drag_state_ && current_button_ == GLFW_MOUSE_BUTTON_RIGHT;
	bool drag_bone = drag_state_ && current_button_ == GLFW_MOUSE_BUTTON_LEFT;

	if (drag_camera || drag_bone && fps_mode_) {
		glm::vec3 axis = glm::normalize(
				orientation_ *
				glm::vec3(mouse_direction.y, -mouse_direction.x, 0.0f)
				);
		orientation_ =
			glm::mat3(glm::rotate(rotation_speed_, axis) * glm::mat4(orientation_));
		tangent_ = glm::column(orientation_, 0);
		up_ = glm::column(orientation_, 1);
		look_ = glm::column(orientation_, 2);
	} else if (drag_bone && current_bone_ != -1) {
		// FIXME: Handle bone rotation
		glm::vec3 delta_dir = glm::normalize(glm::vec3(delta_x, delta_y, 0));
		glm::vec3 rot = glm::cross(look_, delta_dir); 
	
		Bone* b = mesh_->skeleton.bones[current_bone_];
		b->A = b->A * glm::inverse(b->S);
		b->S = glm::rotate(rotation_speed_, rot) * b->S;
		b->A = b->A * b->S;

		mesh_->recompose_matrices(b);
		//now that we have the offset delta, recursively apply it to all of our children
		
		//now that we have preformed the rotation, reclaculate the skelle
		mesh_->skeleton.dirty = true;
		pose_changed_= true;
		return ;
	}
	// FIXME: highlight bones that have been moused over

	//printf("inside mouse calback\n");

	float t;
	float min_t = 100000;
	current_bone_ = -1;
	for(size_t i = 0; i < mesh_->skeleton.bones.size(); i++)
	{
		//printf("checking bone %d\n", i);
		Bone *bone = mesh_->skeleton.bones[i];
		
		glm::vec4 transformed_dir = glm::inverse(bone->A) * glm::vec4(mouse_dir, 0);
		glm::vec3 t_dir = glm::normalize(glm::vec3(transformed_dir));

		// printf("About to use inverse matrix ");
		// std::cout << glm::to_string(glm::inverse(bone->A)) << std::endl;

		glm::vec4 transformed_center = glm::inverse(bone->A) * glm::vec4(eye_, 1);
		glm::vec3 t_center = glm::vec3(transformed_center);

		if(bone->IntersectCylinder(t_center, t_dir, .5f, bone->length, &t))
		{
			//printf("found an intersection with bone %d\n",i);
			if (t < min_t)
			{
				min_t = t;
				current_bone_ = i;
				//printf("setting cur bone to %d\n", i);
			}
		}	
	}
}

void GUI::mouseButtonCallback(int button, int action, int mods)
{
	drag_state_ = (action == GLFW_PRESS);
	current_button_ = button;
}

void GUI::updateMatrices()
{
	// Compute our view, and projection matrices.
	if (fps_mode_)
		center_ = eye_ + camera_distance_ * look_;
	else
		eye_ = center_ - camera_distance_ * look_;

	view_matrix_ = glm::lookAt(eye_, center_, up_);
	light_position_ = glm::vec4(eye_, 1.0f);

	aspect_ = static_cast<float>(window_width_) / window_height_;
	projection_matrix_ =
		glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
	model_matrix_ = glm::mat4(1.0f);
}

MatrixPointers GUI::getMatrixPointers() const
{
	MatrixPointers ret;
	ret.projection = &projection_matrix_[0][0];
	ret.model= &model_matrix_[0][0];
	ret.view = &view_matrix_[0][0];
	return ret;
}

bool GUI::setCurrentBone(int i)
{
	if (i < 0 || i >= mesh_->getNumberOfBones())
		return false;
	current_bone_ = i;
	return true;
}

bool GUI::captureWASDUPDOWN(int key, int action)
{
	if (key == GLFW_KEY_W) {
		if (fps_mode_)
			eye_ += zoom_speed_ * look_;
		else
			camera_distance_ -= zoom_speed_;
		return true;
	} else if (key == GLFW_KEY_S) {
		if (fps_mode_)
			eye_ -= zoom_speed_ * look_;
		else
			camera_distance_ += zoom_speed_;
		return true;
	} else if (key == GLFW_KEY_A) {
		if (fps_mode_)
			eye_ -= pan_speed_ * tangent_;
		else
			center_ -= pan_speed_ * tangent_;
		return true;
	} else if (key == GLFW_KEY_D) {
		if (fps_mode_)
			eye_ += pan_speed_ * tangent_;
		else
			center_ += pan_speed_ * tangent_;
		return true;
	} else if (key == GLFW_KEY_DOWN) {
		if (fps_mode_)
			eye_ -= pan_speed_ * up_;
		else
			center_ -= pan_speed_ * up_;
		return true;
	} else if (key == GLFW_KEY_UP) {
		if (fps_mode_)
			eye_ += pan_speed_ * up_;
		else
			center_ += pan_speed_ * up_;
		return true;
	}
	return false;
}


// Delegrate to the actual GUI object.
void GUI::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->keyCallback(key, scancode, action, mods);
}

void GUI::MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->mousePosCallback(mouse_x, mouse_y);
}

void GUI::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->mouseButtonCallback(button, action, mods);
}
