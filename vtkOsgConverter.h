/**
 * \file vtkOsgConverter.h
 * 27/07/2011 LB Initial implementation
 * Derived from class vtkOsgActor from Bjoern Zehner
 */

#ifndef VTKOSGCONVERTER_H
#define VTKOSGCONVERTER_H

#include <OpenSG/OSGRefPtr.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGMatrix.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGMaterialChunk.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGGeoFunctions.h>
#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGTwoSidedLightingChunk.h>
#include <OpenSG/OSGGeoFunctions.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGImage.h>

class vtkActor;
class vtkMapper;
class vtkTexture;
class vtkDataArray;
class vtkUnsignedCharArray;
class vtkPolyData;

/// @brief
class vtkOsgConverter
{
public:
  vtkOsgConverter(vtkActor* actor);
  virtual ~vtkOsgConverter();
  
  bool WriteAnActor();
  void SetVerbose(bool value);
  OSG::NodePtr GetOsgRoot();

protected:

private:
  vtkActor* _actor;
  vtkMapper* _mapper;

  enum {NOT_GIVEN, PER_VERTEX, PER_CELL};
  bool          m_bVerbose;


  //For the translation to OpenSG
  OSG::RefPtr<OSG::NodePtr> m_posgRoot;
  OSG::RefPtr<OSG::TransformPtr> m_posgTransform;

  OSG::TextureChunkPtr CreateTexture(vtkTexture* vtkTexture);
  OSG::ChunkMaterialPtr CreateMaterial(bool lit, bool hasTexCoords);
};

#endif // VTKOSGCONVERTER_H
