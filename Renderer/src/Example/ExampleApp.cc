#include "ExampleApp.hpp"
#include "EigenAll.h"

void ExampleApp::InitializeScene(Scene &scene) {
    id1 = scene.AddMesh();
    id2 = scene.AddMesh();
}

void ExampleApp::Processing(Scene &scene) {
    MatrixXd vertices(8, 3);
    vertices << +0.5, +0.5, +0.5, 
                -0.5, +0.5, +0.5,
                +0.5, -0.5, +0.5,
                -0.5, -0.5, +0.5,
                +0.5, +0.5, -0.5, 
                -0.5, +0.5, -0.5,
                +0.5, -0.5, -0.5,
                -0.5, -0.5, -0.5;

    Eigen::MatrixXi topo(12, 3);
    topo << 0, 1, 3,
            0, 3, 2,
            0, 5, 1,
            0, 4, 5,
            0, 2, 4,
            2, 6, 4,
            2, 3, 7,
            2, 7, 6,
            4, 6, 7,
            4, 7, 5,
            1, 5, 3,
            3, 5, 7;

    MatrixXd offset(8, 3);
    offset.setConstant(1.2);

    scene.SelectData(id1);
    scene.SetMesh(vertices, topo);
    scene.SelectData(id2);
    scene.SetMesh(vertices + offset, topo);


}