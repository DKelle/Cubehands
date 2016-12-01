#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>
#include "bone_geometry.h"
class LineMesh;

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces);
// FIXME: Add functions to generate the bone mesh.
void create_bones(std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_segments, Mesh* mesh);
void create_children(Joint *parent_joint, glm::vec4 parent_vert, glm::vec3 transform, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_segment, Mesh* mesh);
void create_cylinder(std::vector<glm::vec4>& cylinder_vertices, std::vector<glm::uvec2>& cylinder_faces, int cur_bone, Skeleton skelle);
void create_normals(std::vector<glm::vec4>& normal_vertices, std::vector<glm::uvec2>& normal_faces, int cur_bone, Skeleton skelle);
void create_binormals(std::vector<glm::vec4>& binormal_vertices, std::vector<glm::uvec2>& binormal_faces, int cur_bone, Skeleton skelle);

#endif
