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
  void ClearOsg();
  void SetVerbose(bool value);
  void SetTexture(vtkTexture *vtkTex);
  OSG::NodePtr GetOsgRoot();

protected:
  void InitOpenSG(void);

private:
  vtkActor* _actor;
  vtkMapper* _mapper;
  
  vtkDataArray      *m_pvtkNormals;
  vtkDataArray      *m_pvtkTexCoords;
  vtkUnsignedCharArray  *m_pvtkColors;
  vtkTexture        *m_pvtkTexture;
  bool          m_bTextureHasChanged;
  vtkPolyData* _polyData;

  enum {NOT_GIVEN, PER_VERTEX, PER_CELL};
  int           m_iColorType;
  int           m_iNormalType;
  bool          m_bVerbose;

  int           m_iNumPoints;
  int           m_iNumNormals;
  int           m_iNumColors;
  int           m_iNumGLPoints;
  int           m_iNumGLLineStrips;
  int           m_iNumGLPolygons;
  int           m_iNumGLTriStrips;
  int           m_iNumGLPrimitives;

  //For the translation to OpenSG
  OSG::RefPtr<OSG::NodePtr> m_posgRoot;
  OSG::RefPtr<OSG::TransformPtr> m_posgTransform;
  OSG::RefPtr<OSG::NodePtr> m_posgGeomNode;
  OSG::RefPtr<OSG::GeometryPtr> m_posgGeometry;
  OSG::RefPtr<OSG::ChunkMaterialPtr> m_posgMaterial;
  OSG::RefPtr<OSG::MaterialChunkPtr> m_posgMaterialChunk;
  OSG::RefPtr<OSG::TextureChunkPtr> m_posgTextureChunk;
  OSG::RefPtr<OSG::ImagePtr> m_posgImage;

  OSG::RefPtr<OSG::GeoPTypesPtr> m_posgTypes;
  OSG::RefPtr<OSG::GeoPLengthsPtr> m_posgLengths;
  OSG::RefPtr<OSG::GeoIndicesUI32Ptr> m_posgIndices;
  OSG::RefPtr<OSG::GeoPositions3fPtr> m_posgPoints;
  OSG::RefPtr<OSG::GeoColors3fPtr> m_posgColors;
  OSG::RefPtr<OSG::GeoNormals3fPtr> m_posgNormals;
  OSG::RefPtr<OSG::GeoTexCoords2dPtr> m_posgTexCoords;
  

  void CreateTexture();
  OSG::ChunkMaterialPtr CreateMaterial();

  //Can use OpenSG simple indexed geometry
  OSG::NodePtr ProcessGeometryNormalsAndColorsPerVertex();

  //Can't use indexing and so requires a lot of storage space
  OSG::NodePtr ProcessGeometryNonIndexedCopyAttributes(int gl_primitive_type);
};

#endif // VTKOSGCONVERTER_H
