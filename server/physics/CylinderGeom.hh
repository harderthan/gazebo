/*
 *  Gazebo - Outdoor Multi-Robot Simulator
 *  Copyright (C) 2003  
 *     Nate Koenig & Andrew Howard
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/* Desc: Cylinder geometry
 * Author: Nate Keonig, Andrew Howard
 * Date: 8 May 2003
 * CVS: $Id: CylinderGeom.hh,v 1.1.2.1 2006/12/16 22:41:15 natepak Exp $
 */

#ifndef CYLINDERGEOM_HH
#define CYLINDERGEOM_HH

#include "Geom.hh"

namespace gazebo
{

class CylinderGeom : public Geom
{
  // Constructor
  public: CylinderGeom(Body *body, double radius, double length, double density, const std::string &meshName);

  // Destructor
  public: virtual ~CylinderGeom();
};

}
#endif
