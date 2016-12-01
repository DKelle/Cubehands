#ifndef BONE_GEOMETRY_H
#define BONE_GEOMETRY_H

#include <ostream>
#include <vector>
#include <map>
#include <limits>
#include <glm/glm.hpp>
#include <mmdadapter.h>

struct BoundingBox {
	BoundingBox()
		: min(glm::vec3(-std::numeric_limits<float>::max())),
		max(glm::vec3(std::numeric_limits<float>::max())) {}
	glm::vec3 min;
	glm::vec3 max;
};

struct Bone {
	int to_joint;
	int from_joint;
	float length;
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 S;
	glm::mat4 A;
	glm::mat4 original_A;
	glm::mat4 o_A_inverse;

	bool IntersectCylinder(const glm::vec3 origin, const glm::vec3 direction,float radius, float height, float* t);
};
struct Joint {
	int id;
	glm::vec3 offset;
	int parent_id;

	std::vector<int> children;
	Bone* in_bone;
	std::vector<Bone*> out_bones;

	glm::vec3 tangent;
	glm::vec3 normal;
	glm::vec3 binormal;
	// FIXME: Implement your Joint data structure.
	// Note: PMD represents weights on joints, but you need weights on
	//       bones to calculate the actual animation.
};

struct Skeleton {
	// FIXME: create skeleton and bone data structures
	std::vector<Joint*> joints;
	std::vector<Bone*> bones;
	int root;
	bool dirty = false;
};

struct Mesh {
	Mesh();
	~Mesh();
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec4> animated_vertices;
	std::vector<glm::uvec3> faces;
	std::vector<glm::vec4> vertex_normals;
	std::vector<glm::vec4> face_normals;
	std::vector<glm::vec2> uv_coordinates;
	std::vector<Material> materials;
	void createMatrices(Joint* parent_joint, glm::vec3 parent_pos);
	void createMatrices();
	void recompose_matrices(Bone* b);
	BoundingBox bounds;
	Skeleton skeleton;
	int numbones;
	void loadpmd(const std::string& fn);
	void loadpmd_skinning_weights(std::vector<SparseTuple> tup);
	void updateAnimation();
	int getNumberOfBones() const 
	{ 
		return numbones;
		// FIXME: return number of bones in skeleton
	}
	glm::vec3 getCenter() const { return 0.5f * glm::vec3(bounds.min + bounds.max); }
	std::map<int, std::map<int, float>> weights;

private:
	void computeBounds();
	void computeNormals();
};

#endif
