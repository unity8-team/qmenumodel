#include "plugin.h"
#include "qmenumodel.h"
#include "qdbusmenumodel.h"

#include <QtDeclarative>


void QMenuModelQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterUncreatableType<QMenuModel>(uri, 1, 0, "QMenuModel",
                                           "QMenuModel is a interface");
    qmlRegisterType<QDBusMenuModel>(uri, 1, 0, "QDBusMenuModel");
}

Q_EXPORT_PLUGIN2(qmenumodel, QMenuModelQmlPlugin)
