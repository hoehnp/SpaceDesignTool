/*
 This program is free software; you can redistribute it and/or modify it under
 the terms of the European Union Public Licence - EUPL v.1.1 as published by
 the European Commission.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the European Union Public Licence - EUPL v.1.1
 for more details.

 You should have received a copy of the European Union Public Licence - EUPL v.1.1
 along with this program.

 Further information about the European Union Public Licence - EUPL v.1.1 can
 also be found on the world wide web at http://ec.europa.eu/idabc/eupl

*/

/*
 ------ Copyright (C) 2010 STA Steering Board (space.trajectory.analysis AT gmail.com) ----
*/



//
//------------------ Author:       Guillermo Ortega               -------------------
//------------------ Affiliation:  European Space Agency (ESA)    -------------------
//-----------------------------------------------------------------------------------

#ifndef _ASTROCORE_ORBITAL_TO_CARTESIAN_H_
#define _ASTROCORE_ORBITAL_TO_CARTESIAN_H_

#include "statevector.h"

using namespace sta;

sta::StateVector orbitalTOcartesian(double mu, double a, double ec, double i, double w0, double o0, double m0);

sta::StateVector orbitalTOcartesian(double mu, KeplerianElements elements);

#endif // _ASTROCORE_ORBITAL_TO_CARTESIAN_H_
