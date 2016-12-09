#include "menger.h"
#include <glm/gtx/rotate_vector.hpp>

namespace {
    const int kMinLevel = 0;
    const int kMaxLevel = 4;
};

Menger::Menger()
{
    // Add additional initialization if you like
}

Menger::~Menger()
{
}

void
Menger::set_nesting_level(int level)
{
    nesting_level_ = level;
    dirty_ = true;
}

bool
Menger::is_dirty() const
{
    return dirty_;
}

void
Menger::set_clean()
{
    dirty_ = false;
}

// FIXME generate Menger sponge geometry
void
Menger::generate_geometry(std::vector<glm::vec4>& obj_vertices,
                          std::vector<glm::vec4>& vtx_normals,
                          std::vector<glm::uvec3>& obj_faces,
                          glm::vec3 origin) const
{
    // if (nesting_level_ == 0) {
    //    drawCube(obj_vertices, obj_faces, vtx_normals,
    //             -0.50f, -0.50f, -0.50f, 0.50f, 0.50f, 0.50f);
    // }
    // else {
        generate_cubes(obj_vertices, vtx_normals, obj_faces, origin.x - 5.00f, origin.y - 5.00f, origin.z - 5.00f, 
                                                            origin.x + 5.00f, origin.y + 5.00f, origin.z + 5.00f);
    // }
}

void Menger::generate_cubes(std::vector<glm::vec4>& obj_vertices,
                            std::vector<glm::vec4>& vtx_normals,
                            std::vector<glm::uvec3>& obj_faces,
                            double minx, double miny, double minz,
                            double maxx, double maxy, double maxz
                           ) const {
    // if (iterations == 0) {
        drawCube(obj_vertices, obj_faces, vtx_normals,
        minx, miny, minz, maxx, maxy, maxz);
    // }
    // else if (iterations > 0) {
    //     for (int z = 0; z < 3; z++) {
    //         for (int y = 0; y < 3; y++) {
    //             for (int x = 0; x < 3; x++) {
    //                 if ((z==1 && x==1 && y==1) || (z==1 && x==1) || (z==1 && y==1) || (x==1 && y==1)) {//z==1 || x == 1 || y == 1) {
    //                 } else {
    //                     generate_cubes(obj_vertices, vtx_normals, obj_faces,
    //                                    minx + x*(maxx-minx)/3,
    //                                    miny + y*(maxy-miny)/3,
    //                                    minz + z*(maxz-minz)/3,
    //                                    minx + (x+1)*(maxx-minx)/3,
    //                                    miny + (y+1)*(maxy-miny)/3,
    //                                    minz + (z+1)*(maxz-minz)/3,
    //                                    iterations-1);
    //                 }
    //             }
    //         }
    //     }
    // }
}
// counter clockwise vertices both when appending to obj_vertices
// and obj_faces triangle
void Menger::drawCube(std::vector<glm::vec4>& obj_vertices, 
                      std::vector<glm::uvec3>& obj_faces,
                      std::vector<glm::vec4>& vtx_normals,
                      double minx, double miny, double minz,
                      double maxx, double maxy, double maxz) const
{
    glm::uvec3 length(obj_faces.size(), obj_faces.size(), obj_faces.size());
    // front
    glm::vec4 front_norm(0.0f, 0.0f, 1.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, 0, 1, 2);

    obj_vertices.push_back(glm::vec4(maxx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));
    
    triangle(obj_faces, vtx_normals, front_norm, length, 3, 4, 5);

    // right
    glm::vec4 right_norm(1.0f, 0.0f, 0.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(maxx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f)); 

    triangle(obj_faces, vtx_normals, right_norm, length, 6, 7, 8);

    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f)); 
    obj_vertices.push_back(glm::vec4(maxx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, right_norm, length, 9, 10, 11);
    
    // back
    glm::vec4 back_norm(0.0f, 0.0f, -1.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f)); 

    triangle(obj_faces, vtx_normals, back_norm, length, 12, 13, 14);

    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f)); 
    obj_vertices.push_back(glm::vec4(minx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, minz, 1.0f));

    triangle(obj_faces, vtx_normals, back_norm, length, 15, 16, 17);

    // left
    glm::vec4 left_norm(-1.0f, 0.0f, 0.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, minz, 1.0f)); 

    triangle(obj_faces, vtx_normals, left_norm, length, 18, 19, 20);

    obj_vertices.push_back(glm::vec4(minx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, minz, 1.0f)); 
    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));
  
    triangle(obj_faces, vtx_normals, left_norm, length, 21, 22, 23);

    // top
    glm::vec4 top_norm(0.0f, 1.0f, 0.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f));

    triangle(obj_faces, vtx_normals, top_norm, length, 24, 25, 26);

    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, top_norm, length, 27, 28, 29);

    // bot
    glm::vec4 bot_norm(0.0f, -1.0f, 0.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, minz, 1.0f)); 

    triangle(obj_faces, vtx_normals, bot_norm, length, 30, 31, 32);

    obj_vertices.push_back(glm::vec4(maxx, miny, minz, 1.0f)); 
    obj_vertices.push_back(glm::vec4(minx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, bot_norm, length, 33, 34, 35);
}

void Menger::triangle(std::vector<glm::uvec3>& obj_faces,
                    std::vector<glm::vec4>& vtx_normals,
                    glm::vec4 normal, glm::uvec3 length, 
                    int a, int b, int c) const {

    obj_faces.push_back(glm::uvec3(a+length.x, b+length.y, c+length.z));// + length);
    vtx_normals.push_back(normal);
    vtx_normals.push_back(normal);
    vtx_normals.push_back(normal);
}

void Menger::rotate(float speed, glm::vec3 axis, std::vector<glm::uvec3>& obj_faces,std::vector<glm::vec4>& obj_vertices) {
    obj_faces.clear();
    glm::mat4 rotate_mat = glm::rotate(speed, axis);
    for (int n = 0; n < obj_vertices.size(); n++) {
	   obj_vertices[n] = rotate_mat * obj_vertices[n];
    	if (n%3 == 0) 
	        obj_faces.push_back(glm::uvec3(n-3,n-2,n-1));
    }
}
