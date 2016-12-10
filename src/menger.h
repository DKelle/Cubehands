#ifndef MENGER_H
#define MENGER_H

#include <glm/glm.hpp>
#include <vector>

class Menger {
public:
    Menger();
    ~Menger();
    void set_nesting_level(int);
    bool is_dirty() const;
    void set_clean();
    void generate_geometry(std::vector<glm::vec4>& obj_vertices,
                   std::vector<glm::vec4>& vtx_normals,
                           std::vector<glm::uvec3>& obj_faces,
                           glm::vec3 origin) const;
    void generate_cubes(std::vector<glm::vec4>& obj_vertices,
                            std::vector<glm::vec4>& vtx_normals,
                            std::vector<glm::uvec3>& obj_faces,
                            double minx, double miny, double minz,
                            double maxx, double maxy, double maxz) const;
    void drawCube(std::vector<glm::vec4>& obj_vertices, std::vector<glm::uvec3>& obj_faces,
                      std::vector<glm::vec4>& vtx_normals,
                       double minx, double miny, double minz,
                      double maxx, double maxy, double maxz) const;
    void triangle(std::vector<glm::uvec3>& obj_faces,
                    std::vector<glm::vec4>& vtx_normals,
                    glm::vec4 normal, glm::uvec3 length,
                    int a, int b, int c) const;
  void rotate(float speed, glm::vec3 axis, std::vector<glm::uvec3>& obj_faces, std::vector<glm::vec4>& obj_vertices, glm::vec3 origin); 
  void scale(std::vector<glm::uvec3>& obj_faces, std::vector<glm::vec4>& obj_vertices, glm::vec3 origin, float scale_factor);
private:
        int index = 0;
        int nesting_level_ = 0;
    bool dirty_ = false;
};

#endif
