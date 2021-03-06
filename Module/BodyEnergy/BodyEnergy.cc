//
// Created by hansljy on 2022/2/23.
//

#include "BodyEnergy.h"
#include "Object/SoftBody/SoftBody.h"
#include "Util/Factory.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <vector>
#include <omp.h>

typedef Eigen::Triplet<double> Triplet;

BodyEnergyParameter::BodyEnergyParameter(
		const ElasticEnergyModelType &elas_type,
		const ElasticEnergyModelParameter &elas_para,
		const DissipationEnergyModelType &diss_type,
		const DissipationEnergyModelParameter &diss_para,
		const ConstituteModelType &cons_type,
		const ConstituteModelParameter &cons_para) :
		_elas_type(elas_type), _elas_para(elas_para.Clone()),
		_diss_type(diss_type), _diss_para(diss_para.Clone()),
		_cons_type(cons_type), _cons_para(cons_para.Clone()) {}

DEFINE_CLONE(BodyEnergyParameter, BodyEnergyParameter)

DEFINE_ACCESSIBLE_MEMBER(BodyEnergyParameter, ElasticEnergyModelType, ElasticEnergyModelType, _elas_type)
DEFINE_ACCESSIBLE_POINTER_MEMBER(BodyEnergyParameter, ElasticEnergyModelParameter, ElasticEnergyModelParameter, _elas_para)
DEFINE_ACCESSIBLE_MEMBER(BodyEnergyParameter, DissipationEnergyModelType, DissipationEnergyModelType, _diss_type)
DEFINE_ACCESSIBLE_POINTER_MEMBER(BodyEnergyParameter, DissipationEnergyModelParameter, DissipationEnergyModelParameter, _diss_para)
DEFINE_ACCESSIBLE_MEMBER(BodyEnergyParameter, ConstituteModelType, ConstituteModelType, _cons_type)
DEFINE_ACCESSIBLE_POINTER_MEMBER(BodyEnergyParameter, ConstituteModelParameter, ConstituteModelParameter, _cons_para)

#include <Eigen/Eigenvalues>

void BodyEnergy::Initialize(const BodyEnergyParameter &para) {
	_cons_model = ConstituteModelFactory::GetInstance()->GetConstituteModel(para.GetConstituteModelType());
	_cons_model->Initialize(*para.GetConstituteModelParameter());
	_elas_model = ElasticEnergyModelFactory::GetInstance()->GetElasticEnergyModel(para.GetElasticEnergyModelType());
	_elas_model->Initialize(*para.GetElasticEnergyModelParameter());
	_diss_model = DissipationEnergyModelFactory::GetInstance()->GetDissipationEnergyModel(para.GetDissipationEnergyModelType());
	_diss_model->Initialize(*para.GetDissipationEnergyModelParameter());

	spdlog::info("BodyEnergy initialized");
}

inline Matrix3d GetDs(const VectorXd& X, const std::vector<int>& tet) {
	Vector3d Xi[4];
	for (int i = 0; i < 4; i++) {
		Xi[i] = X.block<3, 1>(3 * tet[i], 0);
	}
	Matrix3d D;
	for (int i = 0; i < 3; i++) {
		D.col(i) = Xi[i] - Xi[3];
	}
	return D;
}

double BodyEnergy::EEnergy(const SoftBody &soft_body) const {
	const auto& tets = soft_body._mesh.GetTets();
	const auto& X = soft_body._mesh.GetPoints();
	double energy = 0;
	int num_of_tets = tets.size();
	for (int i = 0; i < num_of_tets; i++) {
		auto D = GetDs(X, tets[i]);
		energy += _elas_model->Energy(*_cons_model, soft_body._volume[i], soft_body._inv[i], D);
	}
	return energy;
}

VectorXd
BodyEnergy::EGradient(const SoftBody &soft_body) const {
	const auto& tets = soft_body._mesh.GetTets();
	const auto& X = soft_body._mesh.GetPoints();
	VectorXd gradient(X.size());
	gradient.setZero();

	int num_of_tets = tets.size();
	for (int i = 0; i < num_of_tets; i++) {
		auto tet = tets[i];
		auto D = GetDs(X, tet);
		Vector12d local_gradient = _elas_model->Gradient(*_cons_model, soft_body._volume[i], soft_body._inv[i], D);
		for (int j = 0; j < 4; j++) {
			gradient.block<3, 1>(3 * tet[j], 0) += local_gradient.block<3, 1>(3 * j, 0);
		}
	}

	return gradient;
}

void BodyEnergy::EHessianCOO(const SoftBody &soft_body, COO &coo, int x_offset,
							 int y_offset) const {
	const auto& tets = soft_body._mesh.GetTets();
	const auto& X = soft_body._mesh.GetPoints();
//	auto t = clock();

	int num_of_tets = tets.size();

//	auto start_allocation = clock();
	std::vector<Matrix12d> local_hessian(num_of_tets);
//	spdlog::info("Time spent in allocation: {} s", (double)(clock() - start_allocation) / CLOCKS_PER_SEC);

//	auto start_computing = clock();
	#pragma omp parallel for
	for (int i = 0; i < num_of_tets; i++) {
		auto& tet = tets[i];
		auto D = GetDs(X, tet);
		local_hessian[i] = _elas_model->Hessian(*_cons_model, soft_body._volume[i], soft_body._inv[i], D, soft_body._pFpX[i]);
	}

//	auto start_assembly = clock();
	for (int i = 0; i < num_of_tets; i++) {
		auto& tet = tets[i];
		auto& local = local_hessian[i];
//		#pragma omp parallel for default(none) shared(triplets, tet, local_hessian, i)
		for (int j = 0; j < 4; j++) {
			const int base_row = 3 * tet[j];
			const int base_row_local = 3 * j;
			for (int k = 0; k < 4; k++) {
				const int base_col = 3 * tet[k];
				const int base_col_local = 3 * k;
				for (int  row = 0; row < 3; row++) {
					for (int col = 0; col < 3; col++) {
						coo.push_back(Triplet(base_row + row + x_offset, base_col + col + y_offset, local(base_row_local + row, base_col_local + col)));
					}
				}
			}
		}
	}
}

double
BodyEnergy::DEnergy(const SoftBody &soft_body) const {
	const auto& tets = soft_body._mesh.GetTets();
	const auto& X = soft_body._mesh.GetPoints();
	double energy = 0;
	int num_of_tets = tets.size();
	for (int i = 0; i < num_of_tets; i++) {
		auto tet = tets[i];
		Vector12d v;
		Vector4d m;
		for (int j = 0; j < 4; j++) {
			v.block<3, 1>(3 * j, 0) = soft_body._v.block<3, 1>(3 * tet[j], 0);
			m(j) = soft_body._mass(tet[j]);
		}
		auto D = GetDs(X, tet);
		energy += _diss_model->Energy(*_cons_model, *_elas_model, soft_body._volume[i], soft_body._inv[i], m, v, D, soft_body._pFpX[i]);
	}
	return energy;
}

VectorXd BodyEnergy::DGradient(const SoftBody &soft_body) const {
	const auto& tets = soft_body._mesh.GetTets();
	const auto& X = soft_body._mesh.GetPoints();
	VectorXd gradient(X.size());
	gradient.setZero();

	int num_of_tets = tets.size();
	for (int i = 0; i < num_of_tets; i++) {
		auto tet = tets[i];
		Vector12d v;
		Vector4d m;
		for (int j = 0; j < 4; j++) {
			v.block<3, 1>(3 * j, 0) = soft_body._v.block<3, 1>(3 * tet[j], 0);
			m(j) = soft_body._mass(tet[j]);
		}
		auto D = GetDs(X, tet);
		Vector12d local_gradient = _diss_model->Gradient(*_cons_model,
														 *_elas_model, soft_body._volume[i],
														 soft_body._inv[i],
														 m, v, D,
														 soft_body._pFpX[i]);
		for (int j = 0; j < 4; j++) {
			gradient.block<3, 1>(3 * tet[j], 0) += local_gradient.block<3, 1>(3 * j, 0);
		}
	}

	return gradient;
}

void BodyEnergy::DHessianCOO(const SoftBody &soft_body, COO &coo, int x_offset,
							 int y_offset) const {
	const auto& tets = soft_body._mesh.GetTets();
	const auto& X = soft_body._mesh.GetPoints();
	int num_of_tets = tets.size();
	std::vector<Matrix12d> local_hessian(num_of_tets);

//	#pragma omp parallel for default(none) shared(num_of_tets, tets, mass, points, local_hessian, W, inv, V) num_threads(4)
	for (int i = 0; i < num_of_tets; i++) {
		auto& tet = tets[i];
		Vector12d v;
		Vector4d m;
		for (int j = 0; j < 4; j++) {
			v.block<3, 1>(3 * j, 0) = soft_body._v.block<3, 1>(3 * tet[j], 0);
			m(j) = soft_body._mass(tet[j]);
		}
		auto D = GetDs(X, tet);
		local_hessian[i] = _diss_model->Hessian(*_cons_model, *_elas_model,
												soft_body._volume[i], soft_body._inv[i], m, v, D, soft_body._pFpX[i]);
	}

	for (int i = 0; i < num_of_tets; i++) {
		auto& tet = tets[i];
//		#pragma omp parallel for default(none) shared(triplets, tet, local_hessian, i)
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				const int base_row = 3 * tet[j];
				const int base_col = 3 * tet[k];
				const int base_row_local = 3 * j;
				const int base_col_local = 3 * k;
				for (int  row = 0; row < 3; row++) {
					for (int col = 0; col < 3; col++) {
						coo.push_back(Triplet(base_row + row + x_offset, base_col + col + y_offset, local_hessian[i](base_row_local + row, base_col_local + col)));
					}
				}
			}
		}
	}
}



BodyEnergy::~BodyEnergy() {
	delete _elas_model;
	delete _diss_model;
	delete _cons_model;
}

BodyEnergy::BodyEnergy(const BodyEnergy &body_energy) {
	_elas_model = body_energy._elas_model->Clone();
	_diss_model = body_energy._diss_model->Clone();
	_cons_model = body_energy._cons_model->Clone();
}

DEFINE_CLONE(BodyEnergy, BodyEnergy)