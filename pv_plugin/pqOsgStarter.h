/**
 * \file pqOsgStarter.h
 * 18/08/2011 LB Initial implementation
 */

#ifndef PQOSGSTARTER_H
#define PQOSGSTARTER_H

#include <QObject>

/// @brief Inits and exits OpenSG on startup or shutdown of ParaView
class pqOsgStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqOsgStarter(QObject* parent = 0);
  ~pqOsgStarter();
  
  void onShutdown(); 
  void onStartup();

private:
  Q_DISABLE_COPY(pqOsgStarter);
};

#endif // PQOSGSTARTER_H
