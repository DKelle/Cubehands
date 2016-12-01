#include "procedure_geometry.h"
#include "config.h"

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>
#include <iostream>

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces)
{
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	floor_faces.push_back(glm::uvec3(0, 1, 2));
	floor_faces.push_back(glm::uvec3(2, 3, 0));
}

void create_bones(std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_segments, Mesh* mesh)
{
	Skeleton skelle = mesh->skeleton;
	bone_segments.clear();
	bone_vertices.clear();
	int root = skelle.root;
	Joint* root_joint = skelle.joints[root];
	//glm::vec3 offset = glm::vec3(0.0,0.80,-01.0);
	glm::vec3 offset = root_joint->offset;
	glm::vec4 vert = glm::vec4(offset.x, offset.y, offset.z, 1);

	create_children(root_joint, vert, offset, bone_vertices, bone_segments, mesh);
}

void create_cylinder(std::vector<glm::vec4>& cylinder_vertices, std::vector<glm::uvec2>& cylinder_faces, int cur_bone, Skeleton skelle)
{
        cylinder_faces.clear();                                                                                            
        cylinder_vertices.clear();          
	glm::mat4 transform = skelle.bones[cur_bone]->A;
	float len = skelle.bones[cur_bone]->length;
                                                                                                                           
        float magic = 1;                                                                                                
        //Create an n*n grid of lines                                                                                      
        for(float x = -magic; x <= magic; x += magic/10)                                                                   
        {                                                                                                                  
                for(float z = -len/2; z <= len/2; z += len/10 )                                                          
                {                                                                                                          
			float xx = cos(x*M_PI)/4;                                                                                 
                        float zz =  z;              
			float y = sin(x*M_PI)/4;
                                          
                        glm::vec4 z_start = transform * glm::vec4(xx, y, 0, 1);                                                        
                        glm::vec4 z_end =  transform * glm::vec4(xx, y, len, 1);                                                   
                        cylinder_faces.push_back(glm::uvec2(cylinder_vertices.size()+0,cylinder_vertices.size()+1));       
                        cylinder_vertices.push_back(z_start);                                                              
                        cylinder_vertices.push_back(z_end);                                                                

                }                                                                                                          
        } 

	std::vector<glm::vec4> points;
	for(float z = 0; z < len; z += len/10)
	{
		points.clear();
		for(float xy = -1; xy < 1; xy += .1)
		{
			float x = cos(xy*M_PI)/4;
			float y = sin(xy*M_PI)/4;
			
			glm::vec4 point = transform * glm::vec4(x, y, z, 1);
			points.push_back(point);
		}

		//push on all points, creating a single cirlce around the cylinder
		for(int i = 0; i < points.size(); i++)
		{
			glm::vec4 start = points[i];
			glm::vec4 end = (i == points.size()-1) ? points[0] : points[i+1];
                        cylinder_faces.push_back(glm::uvec2(cylinder_vertices.size()+0,cylinder_vertices.size()+1));       
                        cylinder_vertices.push_back(start);                                                              
                        cylinder_vertices.push_back(end);                                                                
		}

	}

                                                                                                                 
        return;                                                                                                            
}

void create_normals(std::vector<glm::vec4>& normal_vertices, std::vector<glm::uvec2>& normal_faces, int cur_bone, Skeleton skelle)
{

        normal_vertices.clear();                                                                                            
        normal_vertices.clear();          


        Bone* b = skelle.bones[cur_bone];
	glm::mat4 transform = b->A;
        glm::vec4 start = transform * glm::vec4(0,0,0,1);
        glm::vec4 normal = transform * glm::normalize(glm::vec4(b->S[1][0], b->S[1][1], b->S[1][2], 1));
        
        normal_faces.push_back(glm::uvec2(normal_vertices.size()+0,normal_vertices.size()+1));
        normal_vertices.push_back(start);
        normal_vertices.push_back(normal);

	return;
}

void create_binormals(std::vector<glm::vec4>& binormal_vertices, std::vector<glm::uvec2>& binormal_faces, int cur_bone, Skeleton skelle)
{

        binormal_vertices.clear();                                                                                            
        binormal_vertices.clear();          

        Bone* b = skelle.bones[cur_bone];
	glm::mat4 transform = b->A;
        glm::vec4 start = transform * glm::vec4(0,0,0,1);
        glm::vec4 binormal = transform * glm::normalize(glm::vec4(b->S[0][0], b->S[0][1], b->S[0][2], 1));
        
        binormal_faces.push_back(glm::uvec2(binormal_vertices.size()+0,binormal_vertices.size()+1));
        binormal_vertices.push_back(start);
        binormal_vertices.push_back(binormal);

	return;
}

void create_children(Joint* parent_joint, glm::vec4 parent_vert, glm::vec3 parent_pos,
			std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_segments, Mesh* mesh)
{
	Skeleton skelle = mesh->skeleton;
	for(std::vector<int>::size_type i = 0; i < parent_joint->children.size(); i++) {
		int child_id = parent_joint->children[i];
		Joint* child = skelle.joints[child_id];
		glm::vec3 child_offset = child->offset;
		child_offset += parent_pos;
		glm::vec4 child_vert = glm::vec4(child_offset.x, child_offset.y, child_offset.z, 1);

		int x = 0+bone_vertices.size();
		int y = 1+bone_vertices.size();
		//printf("pushing back %d, %d\n", x,y);
		glm::uvec2 segment = glm::uvec2(0+bone_vertices.size(), 1+bone_vertices.size());

		//printf("start: %f %f %f\n", parent_vert.x, parent_vert.y, parent_vert.z);
		//printf("end: %f %f %f\n", child_vert.x, child_vert.y, child_vert.z);
		//printf("parent_joint id is %d \n", parent_joint.id);	


		glm::vec4 first_point = child->in_bone->A * glm::inverse(child->in_bone->S) * glm::vec4(0,0,0,1);
		glm::vec4 second_point = child->in_bone->A * glm::vec4(0,0,glm::length(child->offset),1);
		glm::vec3 first = glm::vec3(first_point.x, first_point.y, first_point.z);
		glm::vec3 second = glm::vec3(second_point.x, second_point.y, second_point.z);

		//printf("first point is");
		//std::cout << glm::to_string(first_point) << std::endl;

		//printf("bone.a is\n");
		//std::cout << glm::to_string(child.in_bone.A) << std::endl;

		bone_vertices.push_back(first_point);
		//printf("first point (%f %f %f %f)\n", first_point.x, first_point.y, first_point.z, first_point.w);
		//printf("parent_vert (%f %f %f %f)\n", parent_vert.x, parent_vert.y, parent_vert.z, parent_vert.w);
		//bone_vertices.push_back(parent_vert);
		//bone_vertices.push_back(child_vert);
		bone_vertices.push_back(second_point);
		bone_segments.push_back(segment);

		create_children(child, child_vert, child_offset, bone_vertices, bone_segments, mesh);
    /* std::cout << someVector[i]; ... */
	}
}



// FIXME: create cylinders and lines for the bones
// Hints: Generate a lattice in [-0.5, 0, 0] x [0.5, 1, 0] We wrap this
// around in the vertex shader to produce a very smooth cylinder.  We only
// need to send a small number of points.  Controlling the grid size gives a
// nice wireframe.
