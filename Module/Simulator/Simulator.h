//
// Created by hansljy on 2022/2/22.
//

#ifndef FEM_SIMULATOR_H
#define FEM_SIMULATOR_H

#include <vector>
#include <string>
#include "System/System.h"
#include "Integrator/Integrator.h"
#include "Mass/MassModel.h"
#include "NumericSolver/LCPSolver/LCPSolver.h"
#include "Output/IO.h"

using std::string;

class SimulatorParameter {
public:
	SimulatorParameter(
			double duration,
			double step,
			const SystemParameter& system_para,
			const IntegratorType& integrator_type,
			const IntegratorParameter& integrator_para,
			const ContactGeneratorType& contact_type,
			const ContactGeneratorParameter& contact_para,
			const FrictionModelType& friction_type,
			const FrictionModelParameter& friction_para,
			const SimulatorOutputType& output_type,
			const SimulatorOutputParameter& output_para
	) : _duration(duration),
		_step(step),
		_system_para(system_para.Clone()),
		_integrator_type(integrator_type),
		_integrator_para(integrator_para.Clone()),
		_contact_gen_type(contact_type),
		_contact_gen_para(contact_para.Clone()),
		_friction_type(friction_type),
		_friction_para(friction_para.Clone()),
		_output_type(output_type),
		_output_para(output_para.Clone())
	{}

	DECLARE_ACCESSIBLE_MEMBER(double, Duration, _duration)
	DECLARE_ACCESSIBLE_MEMBER(double, Step, _step)
	DECLARE_ACCESSIBLE_POINTER_MEMBER(SystemParameter, SystemPara, _system_para)
	DECLARE_ACCESSIBLE_MEMBER(IntegratorType, IntegratorType, _integrator_type)
	DECLARE_ACCESSIBLE_POINTER_MEMBER(IntegratorParameter, IntegratorPara, _integrator_para)
	DECLARE_ACCESSIBLE_MEMBER(ContactGeneratorType, ContactGenType, _contact_gen_type)
	DECLARE_ACCESSIBLE_POINTER_MEMBER(ContactGeneratorParameter, ContactGenPara, _contact_gen_para)
	DECLARE_ACCESSIBLE_MEMBER(FrictionModelType, FrictionModelType, _friction_type)
	DECLARE_ACCESSIBLE_POINTER_MEMBER(FrictionModelParameter, FrictionModelPara, _friction_para)
	DECLARE_ACCESSIBLE_MEMBER(SimulatorOutputType, SimulatorOutputType, _output_type)
	DECLARE_ACCESSIBLE_POINTER_MEMBER(SimulatorOutputParameter, SimulatorOutputPara, _output_para)

public:
	~SimulatorParameter() {
		delete _system_para;
		delete _integrator_para;
		delete _contact_gen_para;
		delete _friction_para;
		delete _output_para;
	}
};



class Simulator {
public:
	Simulator() = default;
	void Initialize(const SimulatorParameter& para);

	/**
	 * Add an object into the system
	 * @param obj the object to be added
	 * @return the id for the object in the system
	 * @note the returned ID can be used to remove objects
	 */
	int AddObject(const Object& obj) {
		return _system.AddObject(obj);
	}

	void RemoveObject(int obj_idx) {
		_system.RemoveObject(obj_idx);
	}
	const Object& GetObject(int obj_idx) {
		return _system.GetObject(obj_idx);
	}
	void Simulate();

	Simulator(const Simulator&) = delete;
	~Simulator() {
		delete _integrator;
		delete _contact;
	}

protected:
	double _duration;		// simulation duration
	double _step;			// time step

	System _system;			// the physical system

	Integrator* _integrator = nullptr;
	ContactGenerator* _contact = nullptr;
	FrictionModel* _friction = nullptr;
	SimulatorOutput* _output = nullptr;
};

#endif //FEM_SIMULATOR_H
