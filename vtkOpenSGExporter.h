/**
 * \file vtkOpenSGExporter.h
 * 18/08/2011 LB Initial implementation
 */

#ifndef __vtkOpenSGExporter_h
#define __vtkOpenSGExporter_h

#include "vtkExporter.h"

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
