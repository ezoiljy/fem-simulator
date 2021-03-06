//
// Created by hansljy on 2022/2/22.
//

#ifndef FEM_OPTIMIZER_H
#define FEM_OPTIMIZER_H

#include "Function/Function.h"
#include "Constraint/Constraint.h"
#include "Util/Pattern.h"

enum class OptimizerType {
	kInteriorPoint,
	kNewtonIterator
};

class OptimizerParameter {
public:
	OptimizerParameter(double max_error, int max_step);
	BASE_DECLARE_CLONE(OptimizerParameter)
	DECLARE_ACCESSIBLE_MEMBER(double, MaxError, _max_error)
	DECLARE_ACCESSIBLE_MEMBER(int, MaxStep, _max_step)
	DECLARE_VIRTUAL_ACCESSIBLE_MEMBER(double, Mu)
	DECLARE_VIRTUAL_ACCESSIBLE_MEMBER(double, Armijo)
	DECLARE_VIRTUAL_ACCESSIBLE_MEMBER(double, Curvature)
};

class Optimizer {
public:
	Optimizer() = default;
	virtual void Initialize(const OptimizerParameter& para);
	void SetTarget(const Function& func);
	void AddConstraint(const Function& cons);
	virtual VectorXd Optimize(const VectorXd& x0) const = 0;

	virtual ~Optimizer();
	Optimizer(const Optimizer& optimizer);

	BASE_DECLARE_CLONE(Optimizer)

protected:
	// Parameter
	double _max_error;
	double _max_step;

	Function* _target = nullptr;
	std::vector<const Function*> _constraints;

};

#endif //FEM_OPTIMIZER_H
