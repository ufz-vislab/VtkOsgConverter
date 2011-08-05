/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenSGExporter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOpenSGExporter.h"

#include "vtkOsgConverter.h"
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGGroup.h>

#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCompositeDataGeometryFilter.h"
#include "vtkCompositeDataSet.h"
#include "vtkGeometryFilter.h"
#include "vtkImageData.h"
#include "vtkLightCollection.h"
#include "vtkLight.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkTexture.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkOpenSGExporter);

vtkOpenSGExporter::vtkOpenSGExporter()
{
  this->DebugOn();
  this->FileName = NULL;
  
  vtkDebugMacro(<< "OpenSG converter initing");
  OSG::osgInit(0, NULL);
}

vtkOpenSGExporter::~vtkOpenSGExporter()
{
  if ( this->FileName )
    {
    delete [] this->FileName;
    }
  
  vtkDebugMacro(<< "OpenSG converter exiting");
  OSG::osgExit();
}

void vtkOpenSGExporter::WriteData()
{
  vtkRenderer *ren;
  vtkActorCollection *ac;
  vtkActor *anActor, *aPart;
  vtkLightCollection *lc;
  vtkLight *aLight;
  vtkCamera *cam;
  double *tempd;
  
  vtkDebugMacro(<< "OpenSG converter executing");
  
  // make sure the user specified a FileName or FilePointer
  if (this->FileName == NULL)
    {
    vtkErrorMacro(<< "Please specify FileName to use");
    return;
    }

  // Always pick the first renderer
  // first make sure there is only one renderer in this rendering window
  //if (this->RenderWindow->GetRenderers()->GetNumberOfItems() > 1)
  //  {
  //  vtkErrorMacro(<< "VRML files only support one renderer per window.");
  //  return;
  //  }

  // get the renderer
  ren = this->RenderWindow->GetRenderers()->GetFirstRenderer();
  
  // make sure it has at least one actor
  if (ren->GetActors()->GetNumberOfItems() < 1)
    {
    vtkErrorMacro(<< "no actors found for writing OpenSG file.");
    return;
    }

  // do the actors now
  
  // create group node
  OSG::NodePtr rootNode = OSG::Node::create();
  beginEditCP(rootNode);
  rootNode->setCore(OSG::Group::create());
  endEditCP(rootNode);
  
  ac = ren->GetActors();
  vtkAssemblyPath *apath;
  vtkCollectionSimpleIterator ait;
  for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait)); )
    {
    for (anActor->InitPathTraversal(); (apath=anActor->GetNextPath()); )
      {
      aPart=static_cast<vtkActor *>(apath->GetLastNode()->GetViewProp());
      vtkOsgConverter osgConverter(aPart);
      osgConverter.UpdateOsg();
      OSG::NodePtr node = osgConverter.GetOsgRoot();
      beginEditCP(rootNode);
      rootNode->addChild(node);
      endEditCP(rootNode);
      }
    }
  
  OSG::SceneFileHandler::the().write(rootNode, this->FileName);
  
}

void vtkOpenSGExporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  if (this->FileName)
    {
    os << indent << "FileName: " << this->FileName << "\n";
    }
  else
    {
    os << indent << "FileName: (null)\n";
    }

}