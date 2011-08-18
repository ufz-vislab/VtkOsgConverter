/**
 * \file vtkOpenSGExporter.cxx
 * 18/08/2011 LB Initial implementation
 * 
 * Implementation of vtkOpenSGExporter class
 */

// ** INCLUDES **
#include "vtkOpenSGExporter.h"
#include "vtkOsgConverter.h"

#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGGroup.h>

#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkObjectFactory.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"

vtkStandardNewMacro(vtkOpenSGExporter);

bool vtkOpenSGExporter::osgInited = false;

vtkOpenSGExporter::vtkOpenSGExporter()
{
  this->DebugOn();
  this->FileName = NULL;
  
  if (!osgInited)
  {
    OSG::osgInit(0, NULL);
    osgInited = true;
  }
}

vtkOpenSGExporter::~vtkOpenSGExporter()
{
  if ( this->FileName )
    delete [] this->FileName;
  
  //OSG::osgExit();
}

void vtkOpenSGExporter::WriteData()
{ 
  // make sure the user specified a FileName or FilePointer
  if (this->FileName == NULL)
  {
    vtkErrorMacro(<< "Please specify FileName to use");
    return;
  }

  // get the renderer
  vtkRenderer *ren = this->RenderWindow->GetRenderers()->GetFirstRenderer();
  
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
  
  vtkActor *anActor, *aPart;
  vtkActorCollection *ac = ren->GetActors();
  ac->PrintSelf(std::cout, vtkIndent());
  vtkAssemblyPath *apath;
  vtkCollectionSimpleIterator ait;
  int count = 0;
  for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait)); )
  {
    for (anActor->InitPathTraversal(); (apath=anActor->GetNextPath()); )
    {
      aPart=static_cast<vtkActor *>(apath->GetLastNode()->GetViewProp());
      if (aPart->GetMapper() != NULL && aPart->GetVisibility() != 0)
      {
        vtkOsgConverter osgConverter(aPart);
        osgConverter.SetVerbose(true);
        if(osgConverter.WriteAnActor())
        {
          beginEditCP(rootNode);
          rootNode->addChild(osgConverter.GetOsgNode());
          endEditCP(rootNode);
          ++count;
        }
      }
    }
  } 
  std::cout << "OpenSG converter starts writing file with " << count << " objects." << std::endl;
  OSG::SceneFileHandler::the().write(rootNode, this->FileName);
  std::cout << "OpenSG converter finished." << std::endl;
}

void vtkOpenSGExporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  if (this->FileName)
    os << indent << "FileName: " << this->FileName << "\n";
  else
    os << indent << "FileName: (null)\n";
}