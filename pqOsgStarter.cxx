/**
 * \file pqOsgStarter.cxx
 * 18/08/2011 LB Initial implementation
 * 
 * Implementation of pqOsgStarter class
 */

// ** INCLUDES **
#include "pqOsgStarter.h"

#include <OpenSG/OSGSceneFileHandler.h>
#include <QDebug>

pqOsgStarter::pqOsgStarter(QObject* parent)
  : QObject(parent)
{
}

pqOsgStarter::~pqOsgStarter()
{
}

void pqOsgStarter::onStartup()
{
  OSG::osgInit(0, NULL);
  // qWarning() << "OpenSG inited.";
}

void pqOsgStarter::onShutdown()
{
  OSG::osgExit();
  // qWarning() << "OpenSG exited.";
}