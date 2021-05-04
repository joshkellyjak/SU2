/*!
 * \file CFEAVariable.cpp
 * \brief Definition of the variables for FEM elastic structural problems.
 * \author R. Sanchez
 * \version 7.1.1 "Blackbird"
 *
 * SU2 Project Website: https://su2code.github.io
 *
 * The SU2 Project is maintained by the SU2 Foundation
 * (http://su2foundation.org)
 *
 * Copyright 2012-2021, SU2 Contributors (cf. AUTHORS.md)
 *
 * SU2 is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * SU2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SU2. If not, see <http://www.gnu.org/licenses/>.
 */


#include "../../include/variables/CFEAVariable.hpp"


CFEAVariable::CFEAVariable(const su2double *val_fea, unsigned long npoint, unsigned long ndim, unsigned long nvar, CConfig *config)
  : CVariable(npoint, ndim, nvar, config) {

  bool nonlinear_analysis = (config->GetGeometricConditions() == STRUCT_DEFORMATION::LARGE);
  bool body_forces        = config->GetDeadLoad();
  bool incremental_load   = config->GetIncrementalLoad();
  bool prestretch_fem     = config->GetPrestretch();  // Structure is prestretched
  bool discrete_adjoint   = config->GetDiscrete_Adjoint();
  bool refgeom            = config->GetRefGeom(); // Reference geometry needs to be stored
  bool dynamic_analysis   = config->GetTime_Domain();
  bool multizone          = config->GetMultizone_Problem();
  bool fsi_analysis       = config->GetFSI_Simulation() || multizone;

  VonMises_Stress.resize(nPoint) = su2double(0.0);

  if (nDim==2) Stress.resize(nPoint,3);
  else         Stress.resize(nPoint,6);

  // if (dynamic_analysis) Solution.resize(nPoint,3*nDim);
  /*--- Initialization of variables ---*/
  for (unsigned long iPoint = 0; iPoint < nPoint; ++iPoint)
    for (unsigned long iVar = 0; iVar < nVar; iVar++)
      Solution(iPoint,iVar) = val_fea[iVar];

  if (dynamic_analysis) {
    for (unsigned long iPoint = 0; iPoint < nPoint; ++iPoint) {
      for (unsigned long iVar = 0; iVar < nVar; iVar++) {
        Solution(iPoint,iVar+nDim) = val_fea[iVar+nVar];
        Solution(iPoint,iVar+2*nDim) = val_fea[iVar+2*nVar];
      }
    }

    Solution_time_n = Solution;
    if(config->GetMultizone_Problem() && config->GetAD_Mode()) {
      AD_InputIndex.resize(nPoint,3*nDim) = -1;
      AD_OutputIndex.resize(nPoint,3*nDim) = -1;
      if (config->GetTime_Domain()) {
        AD_Time_n_InputIndex.resize(nPoint,3*nDim) = -1;
        AD_Time_n_OutputIndex.resize(nPoint,3*nDim) = -1;
      }
    }
  }

  if (fsi_analysis) {
    Solution_Pred = Solution;
    Solution_Pred_Old = Solution;
    if (dynamic_analysis) {
      Solution_Vel_Pred.resize(nPoint,nVar);
      for (unsigned long iPoint = 0; iPoint < nPoint; ++iPoint) {
        for (unsigned long iVar = 0; iVar < nVar; iVar++) {
          Solution_Vel_Pred(iPoint,iVar) = Solution(iPoint,iVar+nDim);
        }
      }
    }
  }

  /*--- If we are going to use incremental analysis, we need a way to store the old solution ---*/

  if (incremental_load && nonlinear_analysis) Solution_Old.resize(nPoint,nVar) = su2double(0.0);

  /*--- If we are running a discrete adjoint iteration, we need this vector for cross-dependencies ---*/

  else if (discrete_adjoint && fsi_analysis) Solution_Old = Solution;

  /*--- Body residual ---*/
  if (body_forces) Residual_Ext_Body.resize(nPoint,nVar) = su2double(0.0);

  if (refgeom) Reference_Geometry.resize(nPoint,nVar);

  if (prestretch_fem) Prestretch.resize(nPoint,nVar);

  if (multizone) Set_BGSSolution_k();

  if (config->GetTopology_Optimization()) {
    nAuxVar = 1;
    AuxVar.resize(nPoint);
  }
}

void CFEAVariable::SetSolution_Vel_time_n() {
  for (unsigned long iPoint = 0; iPoint < nPoint; ++iPoint) {
    for (unsigned long iVar = 0; iVar < nVar; iVar++) {
      Solution_time_n(iPoint,iVar+nDim) = Solution(iPoint,iVar+nDim);
    }
  }
}

void CFEAVariable::SetSolution_Accel_time_n() {
  for (unsigned long iPoint = 0; iPoint < nPoint; ++iPoint) {
    for (unsigned long iVar = 0; iVar < nVar; iVar++) {
      Solution_time_n(iPoint,iVar+2*nDim) = Solution(iPoint,iVar+2*nDim);
    }
  }
}

void CFEAVariable::Register_femSolution(bool input, bool push_index) {
  for (unsigned long iPoint = 0; iPoint < nPoint; ++iPoint) {
    for(unsigned long iVar=0; iVar<int(Solution.size()/nPoint); ++iVar) {
      if(input) {
        if(push_index) {
          AD::RegisterInput(Solution(iPoint,iVar));
        }
        else {
          AD::RegisterInput(Solution(iPoint,iVar), false);
          AD::SetIndex(AD_InputIndex(iPoint,iVar), Solution(iPoint,iVar));
        }
      }
      else {
        AD::RegisterOutput(Solution(iPoint,iVar));
        if(!push_index)
          AD::SetIndex(AD_OutputIndex(iPoint,iVar), Solution(iPoint,iVar));
      }
    }
  }
}

void CFEAVariable::Register_femSolution_time_n(bool input, bool push_index) {
  for (unsigned long iPoint = 0; iPoint < nPoint; ++iPoint) {
    for(unsigned long iVar=0; iVar<int(Solution.size()/nPoint); ++iVar) {
      if(input) {
        if(push_index) {
          AD::RegisterInput(Solution_time_n(iPoint,iVar));
        }
        else {
          AD::RegisterInput(Solution_time_n(iPoint,iVar), false);
          AD::SetIndex(AD_Time_n_InputIndex(iPoint,iVar), Solution_time_n(iPoint,iVar));
        }
      }
      else {
        AD::RegisterOutput(Solution_time_n(iPoint,iVar));
        if(!push_index)
          AD::SetIndex(AD_Time_n_OutputIndex(iPoint,iVar), Solution_time_n(iPoint,iVar));
      }
    }
  }
}
