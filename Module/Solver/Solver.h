//
// Created by hansljy on 2022/4/9.
//

#ifndef FEM_SOLVER_H
#define FEM_SOLVER_H

#include "Solver/LCPSolver/LCPSolver.h"

enum class ProblemType {
	kLCP,
};

typedef union {
	LCPSolverType LCP;
} SolverType;

typedef union {
	const LCPSolver* LCP;
} Solver;

#endif //FEM_SOLVER_H
