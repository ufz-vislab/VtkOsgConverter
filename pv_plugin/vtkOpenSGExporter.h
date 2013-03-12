/**
 * \file vtkOpenSGExporter.h
 * 18/08/2011 LB Initial implementation
 */

#ifndef __vtkOpenSGExporter_h
#define __vtkOpenSGExporter_h

#include "vtkExporter.h"

class vtkOpenSGExporter : public vtkExporter
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

  virtual void WriteData();
  char *FileName;

private:
  vtkOpenSGExporter(const vtkOpenSGExporter&);  // Not implemented.
  void operator=(const vtkOpenSGExporter&);  // Not implemented.
};

#endif
