/*
 * Copyright 2011 Nate Koenig & Andrew Howard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
/* Desc: Heightmap geometry
 * Author: Nate Keonig, Andrew Howard
 * Date: 8 May 2003
 * CVS: $Id: BulletHeightmapGeom.cc 7640 2009-05-13 02:06:08Z natepak $
 */

#include <iostream>
#include <string.h>
#include <math.h>

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include "common/Image.hh"
#include "common/Global.hh"
#include "rendering/OgreHeightmap.hh"
#include "common/GazeboError.hh"
#include "Body.hh"
#include "BulletHeightmapGeom.hh"

using namespace gazebo;
using namespace physics;

using namespace physics;

using namespace physics;


//////////////////////////////////////////////////////////////////////////////
// Constructor
BulletHeightmapGeom::BulletHeightmapGeom(Body *body)
    : BulletGeom(body)
{
  Param::Begin(&this->parameters);
  this->imageFilenameP = new ParamT<std::string>("image","",1);
  this->worldTextureP = new ParamT<std::string>("worldTexture","",0);
  this->detailTextureP = new ParamT<std::string>("detailTexture","",0);
  this->sizeP = new ParamT<Vector3>("size",Vector3(10,10,10), 0);
  this->offsetP = new ParamT<Vector3>("offset",Vector3(0,0,0), 0);
  Param::End();

  this->ogreHeightmap = new OgreHeightmap();
}


//////////////////////////////////////////////////////////////////////////////
// Destructor
BulletHeightmapGeom::~BulletHeightmapGeom()
{
  delete this->imageFilenameP;
  delete this->worldTextureP;
  delete this->detailTextureP;
  delete this->sizeP;
  delete this->offsetP;

  delete this->ogreHeightmap;
}

//////////////////////////////////////////////////////////////////////////////
/// Update function.
void BulletHeightmapGeom::Update()
{
  BulletGeom::Update();
}

////////////////////////////////////////////////////////////////////////////////
// Create a lookup table of the terrain's height
void BulletHeightmapGeom::FillHeightMap()
{
  unsigned int x,y;
  float *heights = new float[this->width * this->height];
  float maxHeight = -FLT_MAX;

  Vector3 scale = this->terrainSize / this->width;

  // Iterate over all the verices
  for (y=0; y<this->height; y++)
  {
    for (x=0; x<this->width; x++)
    {
      // Find the height at a vertex
      heights[y*this->width + x] = this->ogreHeightmap->GetHeightAt(
          Vector2<float>(x*scale.x, y*scale.y));

      if (heights[y*this->width + x] > maxHeight)
        maxHeight = heights[y*this->width + x];
    }
  }

  if (this->collisionShape)
    delete this->collisionShape;

  // This will force the Z-axis to be up
  int upIndex = 2;
  int forwardIndex = 1;
  int rightIndex = 0;

  btVector3 localScaling(this->terrainSize.x, this->terrainSize.y, 
                         this->terrainSize.z );

  this->heightFieldShape  = new btHeightfieldTerrainShape( this->width, 
      this->height, heights, maxHeight, upIndex, true, false); 

  this->heightFieldShape->setUseDiamondSubdivision(true);
  this->heightFieldShape->setLocalScaling(localScaling);

  this->collisionShape = this->heightFieldShape;
}

////////////////////////////////////////////////////////////////////////////////
/// Load the heightmap
void BulletHeightmapGeom::Load(XMLConfigNode *node)
{

  Image tmpImage;

  this->imageFilenameP->Load(node);
  this->worldTextureP->Load(node);
  this->detailTextureP->Load(node);
  this->sizeP->Load(node);
  this->offsetP->Load(node);

  // Use the image to get the size of the heightmap
  tmpImage.Load( (**this->imageFilenameP) );

  // Width and height must be the same
  if (tmpImage.GetWidth() != tmpImage.GetHeight())
  {
    gzthrow("Heightmap image must be square\n");
  }

  this->width = this->height = tmpImage.GetWidth();

  this->terrainSize = (**this->sizeP);

  // Step 1: Create the Ogre height map: Performs a ray scene query
  this->ogreHeightmap->Load( (**this->imageFilenameP), (**this->worldTextureP),
      (**this->detailTextureP), this->terrainSize );

  // Step 2: Fill the bullet heightmap 
  this->FillHeightMap();

  BulletGeom::Load(node);
}

////////////////////////////////////////////////////////////////////////////////
/// Save child parameters
void BulletHeightmapGeom::Save(std::string &prefix, std::ostream &stream)
{
  BulletGeom::Save(prefix, stream);
  stream << prefix << *(this->imageFilenameP) << "\n";
  stream << prefix << *(this->worldTextureP) << "\n";
  stream << prefix << *(this->detailTextureP) << "\n";
  stream << prefix << *(this->sizeP) << "\n";
  stream << prefix << *(this->offsetP) << "\n";
}
