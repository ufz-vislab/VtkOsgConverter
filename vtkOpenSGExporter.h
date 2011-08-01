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

  // Description:
  // Specify the Speed of navigation. Default is 4.
  vtkSetMacro(Speed,double);
  vtkGetMacro(Speed,double);

  // Description:
  // Set the file pointer to write to. This will override
  // a FileName if specified.
  void SetFilePointer(FILE *);
  
protected:
  vtkOpenSGExporter();
  ~vtkOpenSGExporter();

  void WriteData();
  void WriteALight(vtkLight *aLight, FILE *fp);
  void WriteAnActor(vtkActor *anActor, FILE *fp);
  void WritePointData(vtkPoints *points, vtkDataArray *normals, 
                      vtkDataArray *tcoords, vtkUnsignedCharArray *colors, 
                      FILE *fp);
  void WriteShapeBegin(vtkActor* actor, FILE *fileP,
                       vtkPolyData *polyData,vtkPointData *pntData,
                       vtkUnsignedCharArray *color);
  void WriteShapeEnd( FILE *fileP );
  char *FileName;
  FILE *FilePointer;
  double Speed;
private:
  vtkOpenSGExporter(const vtkOpenSGExporter&);  // Not implemented.
  void operator=(const vtkOpenSGExporter&);  // Not implemented.
};

#endif
