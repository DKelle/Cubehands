#include "config.h"
#include "bone_geometry.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <cmath>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>
/*
 * For debugging purpose.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
	size_t count = std::min(v.size(), static_cast<size_t>(10));
	for (size_t i = 0; i < count; ++i) os << i << " " << v[i] << "\n";
	os << "size = " << v.size() << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const BoundingBox& bounds)
{
	os << "min = " << bounds.min << " max = " << bounds.max;
	return os;
}



// FIXME: Implement bone animation.


Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Bone::IntersectCylinder(glm::vec3 origin, glm::vec3 direction, float radius, float height, float* t)
{

	float xp = origin.x;
	float yp = origin.y;
	float x = direction.x;
	float y = direction.y;
	float a = pow(x,2)+pow(y,2);
	float b = 2 * (xp*x+yp*y);
	float c = pow(xp,2)+pow(yp,2)-pow(radius,2);

	//printf("a: %f, b: %f, c: %f\n",a,b,c);

	float discrim = pow(b,2)-4*a*c;
	if(discrim >= 0)
	{
		float pos = sqrt(discrim);
		float neg = -1*pos;
		float t1 = (-b+pos)/(2*a);
		float t2 = (-b+neg)/(2*a);
		//printf("intersection points:\n\tt1: %f\n\tt2: %f\n",t1,t2);

		float z;

		if(t1 < 0 && t2 < 0)
		{
			return false;
		}
		else if(t1 > 0 && t2 > 0)
		{
			*t = std::min(t1, t2);
		}
		else
		{
			*t = std::max(t1, t2);
		}

		z = origin.z + *t * direction.z;
		if(z >= 0 && z <= length)
		{
			return true;
		}
	}
	return false;
}

void Mesh::loadpmd(const std::string& fn)
{
	//initialize the skeleton
	//skeleton = {};
	//skeleton.joints = std::vector<Joint>();
	//skeleton.bones = std::vector<Bone>();

	std::vector<SparseTuple> tups;
	int parent;
	glm::vec3 offset;
	MMDReader mr;
	mr.open(fn);
	mr.getMesh(vertices, faces, vertex_normals, uv_coordinates);
	computeBounds();
	mr.getMaterial(materials);
    int i = 0;
	while(mr.getJoint(i, offset, parent))
	{
		tups.clear();
		Joint *j = new Joint;
		j->id = i;
		j->offset = offset;
		j->parent_id = parent;
		if(parent >= 0)
		{

			//We have a parent, so we can create a bone out of this join
			Bone* b = new Bone;
			b->to_joint = i;
			b->from_joint = parent;
			b->length = glm::length(offset);
			j->in_bone = b;

			Joint *parent_joint = skeleton.joints[parent];
			parent_joint->out_bones.push_back(b);
			parent_joint->children.push_back(i);
			skeleton.joints[parent] = parent_joint;

			skeleton.bones.push_back(b);	
		}
		else
		{
			skeleton.root = i;
		}
		skeleton.joints.push_back(j);
		i += 1;
	}

	//for every bone, create a weights vector of size |vertices|
	mr.getJointWeights(tups);	
	loadpmd_skinning_weights(tups);
	createMatrices();
}

void Mesh::loadpmd_skinning_weights(std::vector<SparseTuple> tups)
{

	for(int i = 0; i < tups.size(); i++)
	{
		SparseTuple t = tups[i];
		int jid = t.jid;
		int vid = t.vid;
		float weight = t.weight;

		Joint *j = skeleton.joints[jid];
		for(int k = 0; k < j->out_bones.size(); k++)
		{
			Bone* b = j->out_bones[k];
			int bid = b->to_joint;
			weights[vid][bid] = weight;
		}
	}


//	for(auto const& ent1 : skeleton.weights) {
	  // ent1.first is the first key
//	  for(auto const& ent2 : ent1.second) {
//		printf("map[%d][%d] = %f\n",ent1.first,ent2.first,ent2.second);
	    // ent2.first is the second key
	    // ent2.second is the data
//	  }
//	}
}


void Mesh::recompose_matrices(Bone* b)
{
	int joint_id = b->to_joint;
	Joint* j = skeleton.joints[joint_id];
	
	for(int i = 0; i < j->out_bones.size(); i++)
	{
		Bone* child_bone = j->out_bones[i];
		child_bone->A = b->A * child_bone->translation * child_bone->S;

	//	child_bone->A * glm::inverse(child_bone->S);
		

		recompose_matrices(child_bone);
	}
}

void Mesh::createMatrices()
{
	Joint* r = skeleton.joints[skeleton.root];
	createMatrices(r, r->offset);
}

void Mesh::createMatrices(Joint* parent_joint, glm::vec3 parent_pos)
{
	for(std::vector<int>::size_type i = 0; i < parent_joint->children.size(); i++) 
	{

		int child_id = parent_joint->children[i];
		Joint* child = skeleton.joints[child_id];
		glm::vec3 child_world_point = child->offset + parent_pos;
		if(parent_joint->id == 0)
		{
			//The first bones T and R matrices are going to be the identity
			child->in_bone->translation = glm::mat4(1);
			child->in_bone->rotation =    glm::mat4(1);
			child->in_bone->S =    	      glm::mat4(1);
			child->in_bone->A =           glm::mat4(1);
			child->in_bone->original_A =  glm::mat4(1);
			child->in_bone->o_A_inverse =  glm::inverse(child->in_bone->o_A_inverse);

			
		}
		else
		{
			//Calculate the translation matrix
			glm::vec4 parent_world_point = glm::vec4(parent_pos, 1);
			Bone* in_bone = parent_joint->in_bone;
			glm::mat4 A = in_bone->A;
			glm::mat4 a = glm::inverse(A);
			glm::vec4 q = glm::inverse(A) * parent_world_point;
			glm::mat4 my_t = glm::transpose(glm::mat4(1.0f, 0.0f, 0.0f, q.x,
						0.0f, 1.0f, 0.0f, q.y,
						0.0f, 0.0f, 1.0f, q.z,
						0.0f, 0.0f, 0.0f, 1.0f));
			child->in_bone->translation = my_t;

			//calculate the rotation matrix

			glm::vec4 world_point = glm::vec4(child_world_point, 1);
			float bone_length = glm::length(child->offset);
			A *= my_t;

			glm::vec4 t4 = (glm::inverse(A) * world_point);
			glm::vec3 t3 = glm::normalize(glm::vec3(t4.x, t4.y, t4.z));
			glm::vec3 v = t3;
			if(std::abs(v.x) <= std::abs(v.y) && std::abs(v.x) <= std::abs(v.z)) v = glm::vec3(1,0,0);
			else if(std::abs(v.y) <= std::abs(v.x) && std::abs(v.y) <= std::abs(v.z)) v = glm::vec3(0,1,0);
			else if(std::abs(v.z) <= std::abs(v.x) && std::abs(v.z) <= std::abs(v.y)) v = glm::vec3(0,0,1);
			glm::vec3 n = glm::cross(t3, v) / glm::length(glm::cross(t3, v));
			glm::vec3 b = glm::cross(t3, n);

			child->tangent = t3;
			child->normal = n;
			child->binormal = b;

			glm::mat4 my_r = glm::mat4(b.x,  b.y,  b.z, 0,
						   n.x,  n.y,  n.z, 0,
						   t3.x, t3.y, t3.z, 0,
						   0, 	 0,   0,   1);
			child->in_bone->rotation = my_r;
			child->in_bone->S = my_r; 
			child->in_bone->A = A*my_r;
			child->in_bone->original_A = A*my_r;
			child->in_bone->o_A_inverse = glm::inverse(child->in_bone->original_A);
			A*=my_r;

		}
		createMatrices(child, child_world_point);
	}

}

void Mesh::updateAnimation()
{
	animated_vertices = vertices;
	for(int i = 0; i < animated_vertices.size(); i ++)
	{
		glm::mat4 total_translation = glm::mat4(0);
		glm::vec4 vert = animated_vertices[i];

		
		for(auto const& entry : weights[i]) {
		    // ent2.first is the second key
		    // ent2.second is the data
			int bid = entry.first;
			int weight = entry.second;
			Bone* b = skeleton.joints[bid]->in_bone;
			glm::mat4 W = b->A * b->o_A_inverse;
			glm::mat4 v = weights[i][bid] * W ;

			total_translation += v;
		}
		animated_vertices[i] = total_translation * vert;
	}
}


void Mesh::computeBounds()
{
	bounds.min = glm::vec3(std::numeric_limits<float>::max());
	bounds.max = glm::vec3(-std::numeric_limits<float>::max());
	for (const auto& vert : vertices) {
		bounds.min = glm::min(glm::vec3(vert), bounds.min);
		bounds.max = glm::max(glm::vec3(vert), bounds.max);
	}
}

