//
// Created by hansljy on 2022/2/22.
//

#ifndef FEM_VORONOIMODEL_H
#define FEM_VORONOIMODEL_H

#include "MassModel.h"

class VoronoiModel : public MassModel {
public:
	VectorXd GetMassDistribution(const Mesh &mesh) override;
};

#endif //FEM_VORONOIMODEL_H
