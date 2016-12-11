#include "menger.h"
#include <glm/gtx/rotate_vector.hpp>
#include 	<glm/gtc/matrix_transform.hpp>

namespace {
    const int kMinLevel = 0;
    const int kMaxLevel = 4;
};

Menger::Menger()
{
    // Add additional initialization if you like
    // origin_ = glm::vec3(0,15, 0);
}

// void Menger::fill_origin() {
//     printf("filling origin\n");
//     origin_ = glm::vec3(0,15,0);
//         // origin_ = glm::vec3(0,15,0);
//     printf("post origin\n");

// }

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
                          glm::vec3 origin, float size) const
{

//    obj_vertices.clear();
//    vtx_normals.clear();
//    obj_faces.clear();
    // if (nesting_level_ == 0) {
    //    drawCube(obj_vertices, obj_faces, vtx_normals,
    //             -0.50f, -0.50f, -0.50f, 0.50f, 0.50f, 0.50f);
    // }
    // else {
    // printf("origin: %f, %f, %f", origin_.x, origin_.y, origin_.z);
        generate_cubes(obj_vertices, vtx_normals, obj_faces, origin.x - size, origin.y - size, origin.z - size,
                                                            origin.x + size, origin.y + size, origin.z + size);
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
    int count = obj_vertices.size();
    glm::uvec3 length(obj_faces.size(), obj_faces.size(), obj_faces.size());
    // front
    glm::vec4 front_norm(0.0f, 0.0f, 1.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    obj_vertices.push_back(glm::vec4(maxx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    // right
    glm::vec4 right_norm(1.0f, 0.0f, 0.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(maxx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    // back
    glm::vec4 back_norm(0.0f, 0.0f, -1.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, minz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    // left
    glm::vec4 left_norm(-1.0f, 0.0f, 0.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, minz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    obj_vertices.push_back(glm::vec4(minx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    // top
    glm::vec4 top_norm(0.0f, 1.0f, 0.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    obj_vertices.push_back(glm::vec4(maxx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, maxy, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    // bot
    glm::vec4 bot_norm(0.0f, -1.0f, 0.0f, 0.0f);

    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, maxz, 1.0f));
    obj_vertices.push_back(glm::vec4(maxx, miny, minz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;

    obj_vertices.push_back(glm::vec4(maxx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, minz, 1.0f));
    obj_vertices.push_back(glm::vec4(minx, miny, maxz, 1.0f));

    triangle(obj_faces, vtx_normals, front_norm, length, count+0, count+1, count+2);
    count += 3;
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

void Menger::rotate(float speed, glm::vec3 axis, std::vector<glm::uvec3>& obj_faces, std::vector<glm::vec4>& obj_vertices, glm::vec3 origin) {
    obj_faces.clear();
    glm::mat4 trans_mat_1 = glm::translate(-1.0f * origin);
    glm::mat4 rotate_mat = glm::rotate(speed, axis);
    glm::mat4 trans_mat_2 = glm::translate(origin);
    glm::vec3 cur_normal;

    for (int n = 0; n < obj_vertices.size(); n++) {
	    obj_vertices[n] = trans_mat_2 * rotate_mat * trans_mat_1 * obj_vertices[n];
    	if ((n+1)%3 == 0)
        {
            obj_faces.push_back(glm::uvec3(n-2,n-1,n));
        }
    }
}

void Menger::rotate(glm::mat4 rotation_matrix, std::vector<glm::uvec3>& obj_faces, std::vector<glm::vec4>& obj_vertices,
        glm::vec3 origin) {
    if(rotation_matrix == glm::mat4(0)) {
        return;
    }
    obj_faces.clear();

    glm::mat4 trans_mat_1 = glm::translate(-1.0f * origin);
    glm::mat4 trans_mat_2 = glm::translate(origin);
    glm::vec3 cur_normal;

    for (int n = 0; n < obj_vertices.size(); n++) {
        obj_vertices[n] = trans_mat_2 * rotation_matrix * trans_mat_1 * obj_vertices[n];
        if ((n+1)%3 == 0)
        {
            obj_faces.push_back(glm::uvec3(n-2,n-1,n));
        }
    }
}


void Menger::scale(std::vector<glm::uvec3>& obj_faces, std::vector<glm::vec4>& obj_vertices, glm::vec3 origin, float scale_factor) {
    if(scale_factor == 0) {
        return;
    }
    obj_faces.clear();

    glm::mat4 trans_mat_1 = glm::translate(-1.0f * origin);
    glm::mat4 scale_mat = glm::scale(glm::vec3(scale_factor));
    glm::mat4 trans_mat_2 = glm::translate(origin);

    for (int n = 0; n < obj_vertices.size(); n++) {
    	obj_vertices[n] = trans_mat_2 * scale_mat * trans_mat_1 * obj_vertices[n];
		if (n%3 == 0)
	    	obj_faces.push_back(glm::uvec3(n-3,n-2,n-1));
    }
}


void Menger::translate(std::vector<glm::uvec3>& obj_faces, std::vector<glm::vec4>& obj_vertices, glm::vec3 translated) {
    obj_faces.clear();

    glm::mat4 trans_mat_1 = glm::translate(translated);

    for (int n = 0; n < obj_vertices.size(); n++) {
        obj_vertices[n] = trans_mat_1 * obj_vertices[n];
        if (n%3 == 0)
            obj_faces.push_back(glm::uvec3(n-3,n-2,n-1));
    }
}

void Menger::create_cylinder(std::vector<glm::vec4>& cylinder_vertices, std::vector<glm::uvec2>& cylinder_faces, glm::vec4 start, glm::vec4 end)
{
    float magic = 1;
    //Create an n*n grid of lines
    for(float x = -magic; x <= magic; x += magic/5)
    {
        float xx =  cos(x*M_PI);
        float zz =  0;
        float y =   sin(x*M_PI);

        glm::vec4 z_start = start + glm::vec4(xx, y, 0,0);
        glm::vec4 z_end = z_start + (end - start);
        cylinder_faces.push_back(glm::uvec2(cylinder_vertices.size()+0,cylinder_vertices.size()+1));
        cylinder_vertices.push_back(z_start);
        cylinder_vertices.push_back(z_end);

    }

/*    std::vector<glm::vec4> points;
    for(float z = 0; z < len; z += len/10)
    {
        points.clear();
        for(float xy = -1; xy < 1; xy += .1)
        {
            float x = cos(xy*M_PI)/4;
            float y = sin(xy*M_PI)/4;

            glm::vec4 point = start + glm::vec4(x, y, z, 1);
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

    }*/
}
