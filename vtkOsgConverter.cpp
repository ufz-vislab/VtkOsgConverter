/**
 * \file vtkOsgConverter.cpp
 * 27/07/2011 LB Initial implementation
 * 
 * Implementation of vtkOsgConverter class
 */

// ** INCLUDES **
#include "vtkOsgConverter.h"

#include <vtkActor.h>
#include <vtkTexture.h>
#include <vtkDataArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkImageData.h>

#include <vtkMapper.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkSmartPointer.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkGeometryFilter.h>

#include <OpenSG/OSGPolygonChunk.h>
#include <OpenSG/OSGPointChunk.h>
#include <OpenSG/OSGLineChunk.h>

OSG_USING_NAMESPACE

vtkOsgConverter::vtkOsgConverter(vtkActor* actor) :
  _actor(actor),
  m_pvtkNormals(NULL),
  m_pvtkTexCoords(NULL),
  m_pvtkColors(NULL),
  m_pvtkTexture(NULL),
  m_bTextureHasChanged(false),
  m_iColorType(NOT_GIVEN),
  m_iNormalType(NOT_GIVEN),
  m_bVerbose(false),
  m_iNumPoints(0),
  m_iNumNormals(0),
  m_iNumColors(0),
  m_iNumGLPoints(0),
  m_iNumGLLineStrips(0),
  m_iNumGLPolygons(0),
  m_iNumGLTriStrips(0),
  m_iNumGLPrimitives(0),
  m_posgRoot(NullFC),
  m_posgTransform(NullFC),
  m_posgGeomNode(NullFC),
  m_posgGeometry(NullFC),
  m_posgMaterial(NullFC),
  m_posgMaterialChunk(NullFC),
  m_posgTextureChunk(NullFC),
  m_posgImage(NullFC),
  m_posgTypes(NullFC),
  m_posgLengths(NullFC),
  m_posgIndices(NullFC),
  m_posgPoints(NullFC),
  m_posgColors(NullFC),
  m_posgNormals(NullFC),
  m_posgTexCoords(NullFC)
{
  TransformPtr tptr;
  m_posgRoot = makeCoredNode<osg::Transform>(&tptr);
  m_posgTransform = tptr;
  _mapper = _actor->GetMapper();
}

vtkOsgConverter::~vtkOsgConverter(void)
{
  m_posgRoot = NullFC;

  //Open SG Objects are deleted via the reference counting scheme
  ClearOsg();
}

void vtkOsgConverter::InitOpenSG()
{
  m_posgGeomNode = Node::create();
  m_posgGeometry = Geometry::create();
  m_posgMaterial = ChunkMaterial::create();
  m_posgTextureChunk = TextureChunk::create();
  m_posgImage = Image::create();
  beginEditCP(m_posgRoot);
  m_posgRoot->addChild(m_posgGeomNode);
  endEditCP(m_posgRoot);
  beginEditCP(m_posgGeomNode);
  m_posgGeomNode->setCore(m_posgGeometry);
  endEditCP(m_posgGeomNode);
  m_posgTypes = GeoPTypesUI8::create();
  m_posgLengths = GeoPLengthsUI32::create();
  m_posgIndices = GeoIndicesUI32::create();
  m_posgPoints = GeoPositions3f::create();
  m_posgColors = GeoColors3f::create();
  m_posgNormals = GeoNormals3f::create();
  m_posgTexCoords = GeoTexCoords2d::create();
}

bool vtkOsgConverter::WriteAnActor()
{
  vtkActor *anActor = _actor;
  vtkSmartPointer<vtkPolyData> pd;
  vtkPointData *pntData;
  vtkPoints *points;
  vtkDataArray *normals = NULL;
  vtkDataArray *tcoords = NULL;
  int i, i1, i2;
  double *tempd;
  vtkCellArray *cells;
  vtkIdType npts = 0;
  vtkIdType *indx = 0;
  int pointDataWritten = 0;
  vtkPolyDataMapper *pm;
  vtkUnsignedCharArray *colors;
  double *p;
  unsigned char *c;
  vtkTransform *trans;

  // see if the actor has a mapper. it could be an assembly
  if (anActor->GetMapper() == NULL)
    return false;
  // dont export when not visible
  if (anActor->GetVisibility() == 0)
    return false;

  vtkDataObject* inputDO = anActor->GetMapper()->GetInputDataObject(0, 0);
  if (inputDO == NULL)
    return false;

  // Convert if necessary becasue we only want polydata
  if(inputDO->IsA("vtkCompositeDataSet"))
  {
    vtkCompositeDataGeometryFilter* gf = vtkCompositeDataGeometryFilter::New();
    gf->SetInput(inputDO);
    gf->Update();
    pd = gf->GetOutput();
    gf->Delete();
  }
  else if(inputDO->GetDataObjectType() != VTK_POLY_DATA)
  {
    vtkGeometryFilter *gf = vtkGeometryFilter::New();
    gf->SetInput(inputDO);
    gf->Update();
    pd = gf->GetOutput();
    gf->Delete();
  }
  else
    pd = static_cast<vtkPolyData *>(inputDO);

  // Copy mapper to a new one
  pm = vtkPolyDataMapper::New();
  pm->SetInput(pd);
  pm->SetScalarRange(anActor->GetMapper()->GetScalarRange());
  pm->SetScalarVisibility(anActor->GetMapper()->GetScalarVisibility());
  pm->SetLookupTable(anActor->GetMapper()->GetLookupTable());
  pm->SetScalarMode(anActor->GetMapper()->GetScalarMode());

  if(pm->GetScalarMode() == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ||
     pm->GetScalarMode() == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA )
  {
    if(anActor->GetMapper()->GetArrayAccessMode() == VTK_GET_ARRAY_BY_ID )
      pm->ColorByArrayComponent(anActor->GetMapper()->GetArrayId(),
        anActor->GetMapper()->GetArrayComponent());
    else
      pm->ColorByArrayComponent(anActor->GetMapper()->GetArrayName(),
        anActor->GetMapper()->GetArrayComponent());
    }

  _mapper = pm;
  points = pd->GetPoints();
  pntData = pd->GetPointData();
  normals = pntData->GetNormals();
  m_pvtkTexCoords = pntData->GetTCoords();
  m_pvtkColors  = pm->MapScalars(1.0);
  
  // ARRAY SIZES
  m_iNumPoints = pd->GetNumberOfPoints();
  if (m_iNumPoints == 0)
    return false;
  m_iNumGLPoints = pd->GetVerts()->GetNumberOfCells();
  m_iNumGLLineStrips = pd->GetLines()->GetNumberOfCells();
  m_iNumGLPolygons = pd->GetPolys()->GetNumberOfCells();
  m_iNumGLTriStrips = pd->GetStrips()->GetNumberOfCells();
  m_iNumGLPrimitives = m_iNumGLPoints + m_iNumGLLineStrips + m_iNumGLPolygons + m_iNumGLTriStrips; 


  if (m_bVerbose)
  {
    std::cout << "Array sizes:" << std::endl;
    std::cout << "  number of vertices: " << m_iNumPoints << std::endl;
    std::cout << "  number of GL_POINTS: " << m_iNumGLPoints << std::endl;
    std::cout << "  number of GL_LINE_STRIPS: " << m_iNumGLLineStrips << std::endl;
    std::cout << "  number of GL_POLYGON's: " << m_iNumGLPolygons << std::endl;
    std::cout << "  number of GL_TRIANGLE_STRIPS: " << m_iNumGLTriStrips << std::endl;
    std::cout << "  number of primitives: " << m_iNumGLPrimitives << std::endl;
  }
    
  if (m_posgGeomNode == NullFC)
    InitOpenSG();
  if (m_bTextureHasChanged)
    CreateTexture();

  _mapper->Update();

  // NORMALS
  m_iNormalType = NOT_GIVEN;
  if (_actor->GetProperty()->GetInterpolation() == VTK_FLAT)
  {
    m_pvtkNormals = pd->GetCellData()->GetNormals();
    if (m_pvtkNormals != NULL) m_iNormalType = PER_CELL;
  }else
  {
    m_pvtkNormals = pd->GetPointData()->GetNormals();
    if (m_pvtkNormals != NULL) m_iNormalType = PER_VERTEX;
  }
  if (m_bVerbose)
  {
    std::cout << "Normals:" << std::endl;
    if (m_iNormalType != NOT_GIVEN)
    {
      std::cout << "  number of normals: " << m_pvtkNormals->GetNumberOfTuples() << std::endl;
      std::cout << "  normals are given: ";
      std::cout << ((m_iNormalType == PER_VERTEX) ? "per vertex" : "per cell") << std::endl;
    }
    else
      std::cout << "  no normals are given" << std::endl;
  }
  
  // COLORS
  m_iColorType = NOT_GIVEN;
  if(pm->GetScalarVisibility())
  {
    int iScalarMode = pm->GetScalarMode();
    if(m_pvtkColors == NULL)
    {
      m_iColorType = NOT_GIVEN;
      std::cout << "WARNING: MapScalars(1.0) did not return array!" << std::endl;
    }
    else if(iScalarMode == VTK_SCALAR_MODE_USE_CELL_DATA)
      m_iColorType = PER_CELL;
    else if(iScalarMode == VTK_SCALAR_MODE_USE_POINT_DATA)
      m_iColorType = PER_VERTEX;
    else if(iScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA)
    {
      std::cout << "WARNING TO BE REMOVED: Can not process colours with scalar mode using cell field data!" << std::endl;
      m_iColorType = PER_CELL;
    }
    else if(iScalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA)
    {
      std::cout << "WARNING TO BE REMOVED: Can not process colours with scalar mode using point field data!" << std::endl;
      m_iColorType = PER_VERTEX;
    }
    else if(iScalarMode == VTK_SCALAR_MODE_DEFAULT)
    {
      //Bummer, we do not know what it is. may be we can make a guess
      int numColors = m_pvtkColors->GetNumberOfTuples();
      if (numColors == 0)
      {
        m_iColorType = NOT_GIVEN;
        std::cout << "WARNING: No colors found!" << std::endl;
      }
      else if (numColors == m_iNumPoints)
        m_iColorType = PER_VERTEX;
      else if (numColors == m_iNumGLPrimitives)
        m_iColorType = PER_CELL;
      else
      {
        m_iColorType = NOT_GIVEN;
        std::cout << "WARNING: Number of colors do not match number of points / cells!" << std::endl;
      }
    }
  }
  if (m_bVerbose)
  {
    std::cout << "Colors:" << std::endl;
    if (m_iColorType != NOT_GIVEN){
      std::cout << "  number of colors: " << m_pvtkColors->GetNumberOfTuples() << std::endl;
      std::cout << "  colors are given: " << ((m_iColorType == PER_VERTEX) ? "per vertex" : "per cell") << std::endl;
    }else{
      std::cout << "  no colors are given" << std::endl;
    }
  }
  
  // TEXCOORDS
  if (m_bVerbose)
  {
    std::cout << "Tex-coords:" << std::endl;
    if (m_pvtkTexCoords)
      std::cout << "  Number of tex-coords: " << m_pvtkTexCoords->GetNumberOfTuples() << std::endl;
    else
      std::cout << "  No tex-coords where given" << std::endl;
  }
  
  // TEXTURE
  m_pvtkTexture = _actor->GetTexture();
  if (m_pvtkTexture)
    CreateTexture();

  // TRANSFORMATION
  double scaling[3];
  double translation[3];
  double rotation[3];

  _actor->GetPosition(translation);
  _actor->GetScale(scaling);
  //_actor->GetRotation(rotation[0], rotation[1], rotation[2]);

  if (m_bVerbose)
    std::cout << "set scaling: " << scaling[0] << " " << scaling[1] << " " << scaling[2] << std::endl;

  osg::Matrix m;
  m.setIdentity();
  m.setTranslate(translation[0], translation[1], translation[2]);
  m.setScale(scaling[0], scaling[1], scaling[2]);
  // TODO QUATERNION m.setRotate(rotation[0], rotation[1], rotation[2])
  beginEditCP(m_posgTransform);
  m_posgTransform->setMatrix(m);
  endEditCP(m_posgTransform);

  _mapper->Update();
    
  // Get the converted OpenSG node
  NodePtr newNodePtr;

  //Rendering with OpenSG simple indexed geometry
  if (((m_iNormalType == PER_VERTEX) || (m_iNormalType == NOT_GIVEN))  &&
    ((m_iColorType == PER_VERTEX) || (m_iColorType == NOT_GIVEN)))
  {
      newNodePtr = this->ProcessGeometryNormalsAndColorsPerVertex();
  }
  else
  {
    //Rendering with OpenSG non indexed geometry by copying a lot of attribute data
    if(m_iNumGLPolygons > 0)
    {
      if(m_iNumGLPolygons != m_iNumGLPrimitives)
        std::cout << "WARNING: vtkActor contains different kind of primitives" << std::endl;
      newNodePtr = this->ProcessGeometryNonIndexedCopyAttributes(GL_POLYGON);
    }
    else if(m_iNumGLLineStrips > 0)
    {
      if (m_iNumGLLineStrips != m_iNumGLPrimitives)
        std::cout << "WARNING: vtkActor contains different kind of primitives" << std::endl;
      newNodePtr = this->ProcessGeometryNonIndexedCopyAttributes(GL_LINE_STRIP);
    }
    else if(m_iNumGLTriStrips > 0)
    {
      if (m_iNumGLTriStrips != m_iNumGLPrimitives)
        std::cout << "WARNING: vtkActor contains different kind of primitives" << std::endl;
      newNodePtr = this->ProcessGeometryNonIndexedCopyAttributes(GL_TRIANGLE_STRIP);
    }
    else if (m_iNumGLPoints > 0)
    {
      if (m_iNumGLPoints != m_iNumGLPrimitives)
        std::cout << "WARNING: vtkActor contains different kind of primitives" << std::endl;
      newNodePtr = this->ProcessGeometryNonIndexedCopyAttributes(GL_POINTS);
    }
    else
      newNodePtr = NullFC;
  }
  
  if(newNodePtr == NullFC)
  {
    std::cout << "OpenSG converter was not able to convert this actor." << std::endl;
    return false;
  }

  if(m_iNormalType == NOT_GIVEN)
  {
    GeometryPtr newGeometryPtr = GeometryPtr::dcast(newNodePtr->getCore());
    if((newGeometryPtr != NullFC) && (m_iColorType == PER_VERTEX))
    {
      std::cout << "WARNING: Normals are missing in the vtk layer, calculating normals per vertex!" << std::endl;
      calcVertexNormals(newGeometryPtr);
    }
    else if ((newGeometryPtr != NullFC) && (m_iColorType == PER_CELL))
    {
      std::cout << "WARNING: Normals are missing in the vtk layer, calculating normals per face!" << std::endl;
      calcFaceNormals(newGeometryPtr);
    }
    else if (newGeometryPtr != NullFC)
    {
      std::cout << "WARNING: Normals are missing in the vtk layer, calculating normals per vertex!" << std::endl;
      calcVertexNormals(newGeometryPtr);
    }
  }

  std::cout << "Conversion finished." << std::endl;
  
  // Add node to root
  beginEditCP(m_posgRoot);
  m_posgRoot->addChild(newNodePtr);
  endEditCP(m_posgRoot);
  
  return true;
}

void vtkOsgConverter::ClearOsg(){
  //This also decrements the reference count, possibly deleting the objects
  m_posgGeomNode = NullFC;
  m_posgGeometry = NullFC;
  m_posgMaterial = NullFC;
  m_posgTextureChunk = NullFC;
  m_posgImage = NullFC;
  m_posgTypes = NullFC;
  m_posgLengths = NullFC;
  m_posgIndices = NullFC;
  m_posgPoints = NullFC;
  m_posgColors = NullFC;
  m_posgNormals = NullFC;
  m_posgTexCoords = NullFC;
  m_bTextureHasChanged = true;
}

void vtkOsgConverter::SetVerbose(bool value)
{
  m_bVerbose = value;
}

// void vtkOsgConverter::SetTexture(vtkTexture *vtkTex){
//  m_pvtkTexture = vtkTex;
//  m_bTextureHasChanged = true;
//  vtkOpenGLActor::SetTexture(vtkTex);
// }

NodePtr vtkOsgConverter::GetOsgRoot()
{
  return m_posgRoot;
}

void vtkOsgConverter::CreateTexture()
{
  if(m_bVerbose)
    std::cout << "Calling CreateTexture()" << std::endl;
    
  // if(! m_bTextureHasChanged)
  // {
  //   if (m_bVerbose)
  //   {
  //     std::cout << "    ... nothing to do" << std::endl;
  //     std::cout << "    End CreateTexture()" << std::endl;
  //   }
  //   //can we check if the actual image has been updated, even if the texture is the same?
  //   return;
  // }
  // else if(m_pvtkTexture == NULL)
  // {
  //   if (m_bVerbose)
  //   {
  //     std::cout << "    ... texture is (still ?) NULL" << std::endl;
  //     std::cout << "    EndCreateTexture()" << std::endl;
  //   }
  //   //the texture has changed but it is NULL now. We should remove the texture from the material
  //   return;
  // }
  // m_bTextureHasChanged = false;

  vtkImageData *imgData = m_pvtkTexture->GetInput();
  int iImgDims[3];
  imgData->GetDimensions(iImgDims);

  vtkPointData *imgPointData = imgData->GetPointData();
  vtkCellData *imgCellData = imgData->GetCellData();

  vtkDataArray *data = NULL;

  if (imgPointData != NULL)
  {
    if (NULL != imgPointData->GetScalars())
    {
      data = imgPointData->GetScalars();
      if (m_bVerbose) std::cout << "    found texture data in point data" << std::endl;
    }
  }

  if (imgCellData != NULL)
  {
    if (NULL != imgCellData->GetScalars())
    {
      data = imgCellData->GetScalars();
      if (m_bVerbose) std::cout << "    found texture data in cell data" << std::endl;
    }
  }

  if (data == NULL)
  {
    std::cout << "    could not load texture data" << std::endl;
    return;
  }
  
  int iImgComps = data->GetNumberOfComponents();
  int iImgPixels = data->GetNumberOfTuples();
  if (iImgPixels != (iImgDims[0] * iImgDims[1] * iImgDims[2]))
  {
    std::cout << "Number of pixels in data array seems to be wrong!" << std::endl;
    return;
  }

  UInt8 *newImageData = new UInt8[iImgDims[0] * iImgDims[1] * iImgDims[2] * iImgComps];
  vtkUnsignedCharArray *oldImageUChar = NULL;
  oldImageUChar = dynamic_cast<vtkUnsignedCharArray*>(data);
  int ucharCounter = 0;
  if (oldImageUChar != NULL)
  {
    for (int i=0; i<iImgPixels; i++)
    {
      unsigned char pixel[4];
      oldImageUChar->GetTupleValue(i, pixel);
      for (int j=0; j<iImgComps; j++)
        newImageData[ucharCounter + j] = pixel[j];
      ucharCounter += iImgComps;
    }
  }
  else
    std::cout << "Pixel data come in unsupported vtk type" << std::endl;
  
  beginEditCP(m_posgImage);{
    m_posgImage->setWidth(iImgDims[0]);
    m_posgImage->setHeight(iImgDims[1]);
    m_posgImage->setDepth(iImgDims[2]);
    m_posgImage->setDataType(Image::OSG_UINT8_IMAGEDATA);
    if (iImgComps == 1)
      m_posgImage->setPixelFormat(Image::OSG_L_PF);
    else if (iImgComps == 3)
      m_posgImage->setPixelFormat(Image::OSG_RGB_PF);
    else if (iImgComps == 4)
      m_posgImage->setPixelFormat(Image::OSG_RGBA_PF);
    else
    {
      std::cout << "Unsupported image type!" << std::endl;
      delete [] newImageData;
      return;
    }
    m_posgImage->setData(newImageData);
  };endEditCP(m_posgImage);

  beginEditCP(m_posgTextureChunk);{
    m_posgTextureChunk->setImage(m_posgImage.get());
  };endEditCP(m_posgTextureChunk);

  if (m_bVerbose)
  {
    std::cout << "    Loading image with " << iImgDims[0] << " x " << iImgDims[1] << " x " << iImgDims[2] << "pixels." << std::endl;
    std::cout << "    components: " << iImgComps << std::endl;
    std::cout << "End CreateTexture()" << std::endl;
  }
}

ChunkMaterialPtr vtkOsgConverter::CreateMaterial()
{
  if (m_bVerbose)
    std::cout << "Start CreateMaterial()" << std::endl;

  vtkProperty *prop = _actor->GetProperty();
  double *diffuseColor = prop->GetDiffuseColor();
  double *ambientColor = prop->GetAmbientColor();
  double *specularColor = prop->GetSpecularColor();
  double specularPower = prop->GetSpecularPower();

  double diffuse = prop->GetDiffuse();
  double ambient = prop->GetAmbient();
  double specular = prop->GetSpecular();
  double opacity = prop->GetOpacity();
  
  float pointSize = prop->GetPointSize();
  float lineWidth = prop->GetLineWidth();
  // int lineStipplePattern = prop->GetLineStipplePattern();

  int representation = prop->GetRepresentation();

  if (m_bVerbose)
  {
    std::cout << "    Colors:" << std::endl;
    std::cout << "    diffuse " << diffuse << " * " << diffuseColor[0] << " " << diffuseColor[1] << " " << diffuseColor[2] << std::endl;
    std::cout << "    ambient " << ambient << " * " << ambientColor[0] << " " << ambientColor[1] << " " << ambientColor[2] << std::endl;
    std::cout << "    specular " << specular << " * " << specularColor[0] << " " << specularColor[1] << " " << specularColor[2] << std::endl;
  }

  
  PolygonChunkPtr polygonChunk = PolygonChunk::create();
  beginEditCP(polygonChunk);{
    if (representation == VTK_SURFACE)
    {
      polygonChunk->setFrontMode(GL_FILL);
      polygonChunk->setBackMode(GL_FILL);
    }
    else if (representation == VTK_WIREFRAME)
    {
      polygonChunk->setFrontMode(GL_LINE);
      polygonChunk->setBackMode(GL_LINE);
    }
    else
    {
      polygonChunk->setFrontMode(GL_POINT);
      polygonChunk->setBackMode(GL_POINT);
    }
  };endEditCP(polygonChunk);

  m_posgMaterialChunk = MaterialChunk::create();
  beginEditCP(m_posgMaterialChunk);{
    m_posgMaterialChunk->setDiffuse(Color4f(diffuseColor[0]*diffuse, diffuseColor[1]*diffuse, diffuseColor[2]*diffuse, opacity));
    m_posgMaterialChunk->setSpecular(Color4f(specularColor[0]*specular, specularColor[1]*specular, specularColor[2]*specular, 1.0));
    m_posgMaterialChunk->setAmbient(Color4f(ambientColor[0]*ambient, ambientColor[1]*ambient, ambientColor[2]*ambient, opacity));
    m_posgMaterialChunk->setShininess(specularPower);
    
    //if(opacity < 1.0)
    //{
      // m_posgMaterialChunk->setColorMaterial(GL_AMBIENT); // HACK: Opacity does not work with GL_AMBIENT_AND_DIFFUSE
      //m_posgMaterialChunk->setTransparency(1.0f - opacity);
    //}
    //else
      m_posgMaterialChunk->setColorMaterial(GL_AMBIENT_AND_DIFFUSE);
    
    // On objects consisting only of points or lines, dont lit
    if(m_iNumGLPolygons == 0 && m_iNumGLTriStrips == 0)
      m_posgMaterialChunk->setLit(false);
      
  };endEditCP(m_posgMaterialChunk);

  beginEditCP(m_posgMaterial);{
    m_posgMaterial->addChunk(m_posgMaterialChunk);
    m_posgMaterial->addChunk(TwoSidedLightingChunk::create());
    m_posgMaterial->addChunk(polygonChunk);
    
    if(pointSize > 1.0f)
    {
      PointChunkPtr pointChunk = PointChunk::create();
      pointChunk->setSize(pointSize);
      m_posgMaterial->addChunk(pointChunk);
    }
    
    if(lineWidth > 1.0f)
    {
      LineChunkPtr lineChunk = LineChunk::create();
      lineChunk->setWidth(lineWidth);
      m_posgMaterial->addChunk(lineChunk);
    }

    if (m_pvtkTexCoords != NULL)
    {
      if (m_bVerbose)
        std::cout << "    Add TextureChunk" << std::endl;
      m_posgMaterial->addChunk(m_posgTextureChunk);
    } 
    else
    {
      if (m_bVerbose)
        std::cout << "    Not adding TextureChunk" << std::endl;
    }
  }endEditCP(m_posgMaterial);
  
  if (m_bVerbose)
    std::cout << "    End CreateMaterial()" << std::endl;
  
  return m_posgMaterial;
}

NodePtr vtkOsgConverter::ProcessGeometryNormalsAndColorsPerVertex()
{
  if (m_bVerbose)
    std::cout << "Start ProcessGeometryNormalsAndColorsPerVertex()" << std::endl;

  beginEditCP(m_posgTypes);{
    m_posgTypes->clear();
  };endEditCP(m_posgTypes);

  beginEditCP(m_posgLengths);{
    m_posgLengths->clear();
  };endEditCP(m_posgLengths);

  beginEditCP(m_posgIndices);{
    m_posgIndices->clear();
  };endEditCP(m_posgIndices);

  beginEditCP(m_posgPoints);{
    m_posgPoints->clear();
  };endEditCP(m_posgPoints);

  beginEditCP(m_posgColors);{
    m_posgColors->clear();
  };endEditCP(m_posgColors);

  beginEditCP(m_posgNormals);{
    m_posgNormals->clear();
  };endEditCP(m_posgNormals);

  beginEditCP(m_posgTexCoords);{
    m_posgTexCoords->clear();
  };endEditCP(m_posgTexCoords);

  int iNumPoints = 0;
  int iNumNormals = 0;
  int iNumColors = 0;
  int i;

  vtkPolyData *pPolyData = NULL;
  if (dynamic_cast<vtkPolyDataMapper*>(_mapper))
  {
    pPolyData = (vtkPolyData*) _mapper->GetInput();
    if (m_bVerbose){
      std::cout << "    Using vtkPolyDataMapper directly" << std::endl;
    }
  } else if (dynamic_cast<vtkDataSetMapper*>(_mapper)){
    vtkDataSetMapper *dataSetMapper = (vtkDataSetMapper*) _mapper;
    pPolyData = (vtkPolyData*) dataSetMapper->GetPolyDataMapper()->GetInput();
    if (m_bVerbose){
      std::cout << "    Using vtkPolyDataMapper via the vtkDataSetMapper" << std::endl;
    }
  }
  if (pPolyData == NULL) return NullFC;

  //getting the vertices:
  beginEditCP(m_posgPoints);{
    iNumPoints = pPolyData->GetNumberOfPoints();
    for (i=0; i<iNumPoints; i++)
    {
      double *aVertex = pPolyData->GetPoint(i);
      m_posgPoints->addValue(Vec3f(aVertex[0], aVertex[1], aVertex[2]));
    }
  }endEditCP(m_posgPoints);
  
  //possibly getting the normals
  if (m_iNormalType == PER_VERTEX)
  {
    iNumNormals = m_pvtkNormals->GetNumberOfTuples();
    beginEditCP(m_posgNormals);{
      double *aNormal;
      for (i=0; i<iNumNormals; i++)
      {
        aNormal = m_pvtkNormals->GetTuple(i);
        m_posgNormals->addValue(Vec3f(aNormal[0], aNormal[1], aNormal[2]));
      }
    }endEditCP(m_posgNormals);
    if (iNumNormals != iNumPoints)
    {
      std::cout << "WARNING: CVtkActorToOpenSG::ProcessGeometryNormalsAndColorsPerVertex() number of normals" << std::endl;
      std::cout << "should equal the number of vertices (points)!" << std::endl << std::endl;
    }
  }
  //possibly getting the colors
  if (m_iColorType == PER_VERTEX)
  {
    iNumColors = m_pvtkColors->GetNumberOfTuples();
    beginEditCP(m_posgColors);{
      unsigned char aColor[4];
      for (i=0; i<iNumColors; i++)
      {
        m_pvtkColors->GetTupleValue(i, aColor);
        float r = ((float) aColor[0]) / 255.0f;
        float g = ((float) aColor[1]) / 255.0f;
        float b = ((float) aColor[2]) / 255.0f;
        m_posgColors->addValue(Color3f(r, g, b));
      }
    }endEditCP(m_posgColors);
    if (iNumColors != iNumPoints)
    {
      std::cout << "WARNING: CVtkActorToOpenSG::ProcessGeometryNormalsAndColorsPerVertex() number of colors" << std::endl;
      std::cout << "should equal the number of vertices (points)!" << std::endl << std::endl;
    }
  }

  //possibly getting the texture coordinates. These are alwary per vertex
  if (m_pvtkTexCoords != NULL)
  {
    int numTuples = m_pvtkTexCoords->GetNumberOfTuples();
    for (i=0; i<numTuples; i++)
    {
      double texCoords[3];
      m_pvtkTexCoords->GetTuple(i, texCoords);
      m_posgTexCoords->addValue(Vec2f(texCoords[0], texCoords[1]));
    }
  }

  //getting the cells
  beginEditCP(m_posgTypes);
  beginEditCP(m_posgLengths);
  beginEditCP(m_posgIndices);{
    vtkCellArray *pCells;
    vtkIdType npts, *pts;
    int prim;

    prim = 0;
    pCells = pPolyData->GetVerts();
    if (pCells->GetNumberOfCells() > 0)
    {
      for (pCells->InitTraversal(); pCells->GetNextCell(npts, pts); prim++)
      {
        m_posgLengths->addValue(npts);
        m_posgTypes->addValue(GL_POINTS);
        for (i=0; i<npts; i++)
          m_posgIndices->addValue(pts[i]);
      }
    }

    prim = 0;
    pCells = pPolyData->GetLines();
    if (pCells->GetNumberOfCells() > 0)
    {
      for (pCells->InitTraversal(); pCells->GetNextCell(npts, pts); prim++)
      {
        m_posgLengths->addValue(npts);
        m_posgTypes->addValue(GL_LINE_STRIP);
        for (i=0; i<npts; i++)
          m_posgIndices->addValue(pts[i]);
      }
    }

    prim = 0;
    pCells = pPolyData->GetPolys();
    if (pCells->GetNumberOfCells() > 0)
    {
      for (pCells->InitTraversal(); pCells->GetNextCell(npts, pts); prim++)
      {
        m_posgLengths->addValue(npts);
        m_posgTypes->addValue(GL_POLYGON);
        for (i=0; i<npts; i++)
          m_posgIndices->addValue(pts[i]);
      }
    }

    prim = 0;
    pCells = pPolyData->GetStrips();
    if (pCells->GetNumberOfCells() > 0)
    {
      for (pCells->InitTraversal(); pCells->GetNextCell(npts, pts); prim++)
      {
        m_posgLengths->addValue(npts);
        m_posgTypes->addValue(GL_TRIANGLE_STRIP);
        for (i=0; i<npts; i++)
          m_posgIndices->addValue(pts[i]);
      }
    }
  }endEditCP(m_posgIndices);
  endEditCP(m_posgLengths);
  endEditCP(m_posgTypes);

  ChunkMaterialPtr material = CreateMaterial();
  beginEditCP(m_posgGeometry);{
    m_posgGeometry->setPositions(m_posgPoints);
    m_posgGeometry->setTypes(m_posgTypes);
    m_posgGeometry->setLengths(m_posgLengths);
    m_posgGeometry->setIndices(m_posgIndices);
    m_posgGeometry->setMaterial(material);

    if (m_iNormalType == PER_VERTEX) m_posgGeometry->setNormals(m_posgNormals);
    if (m_iColorType == PER_VERTEX) m_posgGeometry->setColors(m_posgColors);
    if (m_posgTexCoords->getSize() > 0) m_posgGeometry->setTexCoords(m_posgTexCoords);
  };endEditCP(m_posgGeometry);
  
  if (m_bVerbose)
    std::cout << "    End ProcessGeometryNormalsAndColorsPerVertex()" << std::endl;

  return m_posgGeomNode;
}

NodePtr vtkOsgConverter::ProcessGeometryNonIndexedCopyAttributes(int gl_primitive_type)
{
  if (m_bVerbose)
    std::cout << "Start ProcessGeometryNonIndexedCopyAttributes(int gl_primitive_type)" << std::endl;


  beginEditCP(m_posgTypes);{
    m_posgTypes->clear();
  };endEditCP(m_posgTypes);

  beginEditCP(m_posgLengths);{
    m_posgLengths->clear();
  };endEditCP(m_posgLengths);

  beginEditCP(m_posgIndices);{
    m_posgIndices->clear();
  };endEditCP(m_posgIndices);

  beginEditCP(m_posgPoints);{
    m_posgPoints->clear();
  };endEditCP(m_posgPoints);

  beginEditCP(m_posgColors);{
    m_posgColors->clear();
  };endEditCP(m_posgColors);

  beginEditCP(m_posgNormals);{
    m_posgNormals->clear();
  };endEditCP(m_posgNormals);

  beginEditCP(m_posgTexCoords);{
    m_posgTexCoords->clear();
  };endEditCP(m_posgTexCoords);

  vtkPolyData *pPolyData = NULL;
  if (dynamic_cast<vtkPolyDataMapper*>(_mapper)){
    pPolyData = (vtkPolyData*) _mapper->GetInput();
    if (m_bVerbose)
      std::cout << "    Using vtkPolyDataMapper directly" << std::endl;   
  } else if (dynamic_cast<vtkDataSetMapper*>(_mapper)){
    vtkDataSetMapper *dataSetMapper = (vtkDataSetMapper*) _mapper;
    pPolyData = (vtkPolyData*) dataSetMapper->GetPolyDataMapper()->GetInput();
    if (m_bVerbose)
      std::cout << "    Using vtkPolyDataMapper via the vtkDataSetMapper" << std::endl;
  }
  if (pPolyData == NULL) return NullFC;

  vtkCellArray *pCells;
  if (gl_primitive_type == GL_POINTS)
    pCells = pPolyData->GetVerts();
  else if (gl_primitive_type == GL_LINE_STRIP)
    pCells = pPolyData->GetLines();
  else if (gl_primitive_type == GL_POLYGON)
    pCells = pPolyData->GetPolys();
  else if (gl_primitive_type == GL_TRIANGLE_STRIP)
    pCells = pPolyData->GetStrips();
  else
  {
    std::cout << "CVtkActorToOpenSG::ProcessGeometryNonIndexedCopyAttributes(int gl_primitive_type)" << std::endl;
    std::cout << "  was called with non implemented gl_primitive_type!" << std::endl;
    return NullFC;
  }

  beginEditCP(m_posgTypes);
  beginEditCP(m_posgLengths);
  beginEditCP(m_posgPoints);
  beginEditCP(m_posgColors);
  beginEditCP(m_posgNormals);{
    int prim = 0;
    if (pCells->GetNumberOfCells() > 0)
    {
      vtkIdType npts, *pts;
      for (pCells->InitTraversal(); pCells->GetNextCell(npts, pts); prim++)
      {
        m_posgLengths->addValue(npts);
        m_posgTypes->addValue(GL_POLYGON);
        for (int i=0; i<npts; i++)
        {
          double *aVertex;
          double *aNormal;
          unsigned char aColor[4];

          aVertex = pPolyData->GetPoint(pts[i]);
          m_posgPoints->addValue(Vec3f(aVertex[0], aVertex[1], aVertex[2]));

          if (m_iNormalType == PER_VERTEX)
          {
            aNormal = m_pvtkNormals->GetTuple(pts[i]);
            m_posgNormals->addValue(Vec3f(aNormal[0], aNormal[1], aNormal[2]));
          }
          else if (m_iNormalType == PER_CELL)
          {
            aNormal = m_pvtkNormals->GetTuple(prim);
            m_posgNormals->addValue(Vec3f(aNormal[0], aNormal[1], aNormal[2]));
          }

          if (m_iColorType == PER_VERTEX)
          {
            m_pvtkColors->GetTupleValue(pts[i], aColor);
            float r = ((float) aColor[0]) / 255.0f;
            float g = ((float) aColor[1]) / 255.0f;
            float b = ((float) aColor[2]) / 255.0f;
            m_posgColors->addValue(Color3f(r, g, b));
          }
          else if (m_iColorType == PER_CELL)
          {
            m_pvtkColors->GetTupleValue(prim, aColor);
            float r = ((float) aColor[0]) / 255.0f;
            float g = ((float) aColor[1]) / 255.0f;
            float b = ((float) aColor[2]) / 255.0f;
            m_posgColors->addValue(Color3f(r, g, b));
          }
        }
      }
    }
  };endEditCP(m_posgTypes);
  endEditCP(m_posgLengths);
  endEditCP(m_posgPoints);
  endEditCP(m_posgColors);
  endEditCP(m_posgNormals);

  //possibly getting the texture coordinates. These are always per vertex
  vtkPoints *points = pPolyData->GetPoints();
  if ((m_pvtkTexCoords != NULL) && (points != NULL))
  {
    int numPoints = points->GetNumberOfPoints();
    int numTexCoords = m_pvtkTexCoords->GetNumberOfTuples();
    if (numPoints == numTexCoords){
      beginEditCP(m_posgTexCoords);{
        int numTuples = m_pvtkTexCoords->GetNumberOfTuples();
        for (int i=0; i<numTuples; i++)
        {
          double texCoords[3];
          m_pvtkTexCoords->GetTuple(i, texCoords);
          m_posgTexCoords->addValue(Vec2f(texCoords[0], texCoords[1]));
        }
      };endEditCP(m_posgTexCoords);
    }
  }

  ChunkMaterialPtr material = CreateMaterial();
  //GeometryPtr geo = Geometry::create();
  beginEditCP(m_posgGeometry);{
    m_posgGeometry->setPositions(m_posgPoints);
    m_posgGeometry->setTypes(m_posgTypes);
    m_posgGeometry->setLengths(m_posgLengths);
    m_posgGeometry->setMaterial(material);

    if (m_iNormalType != NOT_GIVEN) m_posgGeometry->setNormals(m_posgNormals);
    if (m_iColorType != NOT_GIVEN) m_posgGeometry->setColors(m_posgColors);
    if (m_posgTexCoords->getSize() > 0) m_posgGeometry->setTexCoords(m_posgTexCoords);
    //geo->setMaterial(getDefaultMaterial());
  }endEditCP(m_posgGeometry);

  if (m_bVerbose)
    std::cout << "    End ProcessGeometryNonIndexedCopyAttributes(int gl_primitive_type)" << std::endl;
  return m_posgGeomNode;
}
