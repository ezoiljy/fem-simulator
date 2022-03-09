//
// Created by hansljy on 2022/2/23.
//

#include "Target.h"
#include "Util/Factory.h"

TargetParameter::TargetParameter(const BodyEnergyParameter &body_para) : _body_para(body_para.Clone()){}

DEFINE_CLONE(TargetParameter, TargetParameter)
DEFINE_ACCESSIBLE_POINTER_MEMBER(TargetParameter, BodyEnergyParameter, BodyEnergyParameter, _body_para)
DEFINE_ACCESSIBLE_MEMBER(TargetParameter, MassModelType, MassModelType, _mass_type)
DEFINE_ACCESSIBLE_POINTER_MEMBER(TargetParameter, MassModelParameter, MassModelParameter, _mass_para)

void Target::Initialize(const TargetParameter &para) {
	_body_energy = new BodyEnergy;
	_body_energy->Initialize(*para.GetBodyEnergyParameter());
	_mass_model = MassModelFactory::GetInstance()->GetMassModel(para.GetMassModelType());
	_mass_model->Initialize(*para.GetMassModelParameter());
	ComputeVolumn();
	ComputeMass();
	ComputeInverse();
}

Target::~Target() {
	delete _body_energy;
	delete _mass_model;
};

Target::Target(const Target &target) {
	_reference = target._reference;
	_body_energy = target._body_energy->Clone();
	_mass_model = target._mass_model->Clone();
	_x = target._x;
	_v = target._v;
	_dt = target._dt;
}

void Target::SetX(const VectorXd &x) {
	_x = x;
}

void Target::SetV(const VectorXd &v) {
	_v = v;
}

void Target::SetDt(double dt) {
	_dt = dt;
}

void Target::SetMesh(const Mesh &mesh) {
	_reference = mesh;
}

void Target::ComputeInverse() {
	const auto& points = _reference.GetPoints();
	const auto& tets = _reference.GetTets();
	int num_of_tets = tets.size();
	_inv.resize(num_of_tets, 1);
	int index = 0;
	for (const auto& tet : tets) {
		Matrix3d D;
		for (int i = 0; i < 3; i++) {
			D.col(i) = points.block<3, 1>(3 * tet[i], 0) - points.block<3, 1>(3 * tet[3], 0);
		}
		_inv[index++] = D.inverse();
	}
}

void Target::ComputeMass() {
	_mass = _mass_model->GetMassDistribution(_reference);
}

void Target::ComputeVolumn() {
	const auto& points = _reference.GetPoints();
	const auto& tets = _reference.GetTets();
	int num_of_tets = tets.size();
	_volumn.resize(num_of_tets, 1);
	int index = 0;
	for (const auto& tet : tets) {
		Matrix3d D;
		for (int i = 0; i < 3; i++) {
			D.col(i) = points.block<3, 1>(3 * tet[i], 0) - points.block<3, 1>(3 * tet[3], 0);
		}
		_volumn[index++] = std::abs(D.determinant() / 6.0);
	}
}
