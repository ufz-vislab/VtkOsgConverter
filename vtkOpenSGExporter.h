/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenSGExporter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOpenSGExporter - export a scene into VRML 2.0 format.
// .SECTION Description
// vtkOpenSGExporter is a concrete subclass of vtkExporter that writes VRML 2.0
// files. This is based on the VRML 2.0 draft #3 but it should be pretty
// stable since we aren't using any of the newer features.
//
// .SECTION See Also
// vtkExporter


#ifndef __vtkOpenSGExporter_h
#define __vtkOpenSGExporter_h

#include "vtkExporter.h"

class vtkLight;
class vtkActor;
class vtkPoints;
class vtkDataArray;
class vtkUnsignedCharArray;
class vtkPolyData;
class vtkPointData;

class VTK_RENDERING_EXPORT vtkOpenSGExporter : public vtkExporter
{
public:
  static vtkOpenSGExporter *New();
  vtkTypeMacro(vtkOpenSGExporter,vtkExporter);
  
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the name of the VRML file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  
protected:
  vtkOpenSGExporter();
  ~vtkOpenSGExporter();

  void WriteData();
  char *FileName;
private:
  vtkOpenSGExporter(const vtkOpenSGExporter&);  // Not implemented.
  void operator=(const vtkOpenSGExporter&);  // Not implemented.
  
  static bool osgInited;
};

#endif
