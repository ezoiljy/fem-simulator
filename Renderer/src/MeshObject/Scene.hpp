#ifndef SCENE_HPP
#define SCENE_HPP

#include "Util/EigenAll.h"
#include <vector>
#include "MeshObject/MeshObject.hpp"

class Scene {
public:
    /**
     * @brief Draw the whole scene
     */
    void Draw();

    /**
     * @brief Add a new slot for meshes into the scene
     * @return int The id for the new slot
     * @note This method will automatically select the
     *       newest mesh afterwards
     */
    int AddMesh();

    /**
     * @brief Select a specific mesh
     * @param idx The id for the meshes
     */
    void SelectData(int idx);

    /**
     * @brief Set the Mesh for the seleted mesh
     * 
     */
    void SetMesh(const MatrixXd& vertices, const MatrixXi& topo);

private:
    std::vector<MeshObject> _meshes;
    int _selected_data = -1;
};

#endif