//
// Created by hansljy on 22-5-12.
//

#ifndef FEM_OBJECT_H
#define FEM_OBJECT_H

#include "Util/EigenAll.h"
#include "Util/Pattern.h"
#include "BodyEnergy/ExternalForce.h"
#include <vector>

enum class OutputFormatType {
	kVtk,
	kObj
};

class Object {
public:
	Object() = default;

	//-> degree of freedom of the objects
	virtual int GetDOF() const = 0;

	//-> status vector of the object
	virtual const VectorXd& GetX() const = 0;

	//-> status vector of the object
	virtual VectorXd& GetX() = 0;

	//-> changing rate of the status vector (in a normal sense, velocity)
	virtual const VectorXd& GetV() const = 0;

	//-> changing rate of the status vector (in a normal sense, velocity)
	virtual VectorXd& GetV() = 0;

	//-> COO form of the mass for the objects
	// Technically speaking, an object of DOF n should possess a mass matrix of
	// size n by n DEFINED by F = Ma
	virtual const COO& GetM() const = 0;

	//-> All energy considered for this object (Both internal and external)
	// Note: collision force is not included
	double Energy() const;

	//-> Gradient of the energy against status vector, negate of the total force
	// (collision force not included)
	VectorXd EnergyGradient() const;

	virtual void
	InternalEnergyHessianCOO(COO &coo, int x_offset, int y_offset) const = 0;

	//-> HessianCOO of external energy
	void ExternalEnergyHessianCOO(COO &coo, int x_offset, int y_offset) const;

	// From DOF to shape
	virtual Matrix<int, Dynamic, 3> GetSurfaceTopo() const = 0;
	virtual MatrixXd GetSurfacePosition() const = 0;

	// From shape to DOF
	virtual SparseMatrixXd GetJ(int idx, const Vector3d &point) const = 0;

	virtual void Store(const std::string &filename, const OutputFormatType &format) const = 0;

	virtual double GetMu() const = 0;

	void AddExternalForce(const ExternalForce& external_force);
	virtual ~Object() noexcept;

	Object(const Object& obj);

	Object& operator=(const Object& rhs);
	BASE_DECLARE_CLONE(Object)
protected:
	// The internal energy of the object
	virtual double InternalEnergy() const = 0;

	virtual VectorXd InternalEnergyGradient() const = 0;
	// The external force of the object
	double ExternalEnergy() const;
	VectorXd ExternalEnergyGradient() const;

	std::vector<const ExternalForce*> _external_force;
};

#endif //FEM_OBJECT_H
