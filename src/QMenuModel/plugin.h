#ifndef QMENUMODELQMLPLUGIN_H
#define QMENUMODELQMLPLUGIN_H

#include <QDeclarativeExtensionPlugin>


class QMenuModelQmlPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
public:
    void registerTypes(const char *uri);
};

#endif
