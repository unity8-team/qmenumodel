#include "plugin.h"
#include "qmenumodel.h"
#include "qdbusmenumodel.h"
#include "qdbusactiongroup.h"

#include <QtDeclarative>


void QMenuModelQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterUncreatableType<QMenuModel>(uri, 0, 1, "QMenuModel",
                                           "QMenuModel is a interface");
    qmlRegisterType<QDBusMenuModel>(uri, 0, 1, "QDBusMenuModel");
    qmlRegisterType<QDBusActionGroup>(uri, 0, 1, "QDBusActionGroup");
}

Q_EXPORT_PLUGIN2(qmenumodel, QMenuModelQmlPlugin)
