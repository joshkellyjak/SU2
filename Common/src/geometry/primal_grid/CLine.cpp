/*!
 * \file CLine.cpp
 * \brief Main classes for defining the primal grid elements
 * \author F. Palacios
 * \version 7.2.0 "Blackbird"
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

#include "../../../include/geometry/primal_grid/CLine.hpp"
#include "../../../include/option_structure.hpp"

constexpr unsigned short CLineConnectivity::Faces[1][2];
constexpr unsigned short CLineConnectivity::Neighbor_Nodes[2][1];
constexpr unsigned short CLineConnectivity::nNodesFace[1];
constexpr unsigned short CLineConnectivity::nNeighbor_Nodes[2];
constexpr unsigned short CLineConnectivity::CLineConnectivity::nFaces;
constexpr unsigned short CLineConnectivity::nNodes;
constexpr unsigned short CLineConnectivity::nNeighbor_Elements;
constexpr unsigned short CLineConnectivity::VTK_Type;
constexpr unsigned short CLineConnectivity::maxNodesFace;


CLine::CLine(unsigned long val_point_0, unsigned long val_point_1) {

  /*--- Allocate and define face structure of the element ---*/

  Nodes = new unsigned long[GetnNodes()];
  Nodes[0] = val_point_0;
  Nodes[1] = val_point_1;

}

CLine::~CLine() {}
