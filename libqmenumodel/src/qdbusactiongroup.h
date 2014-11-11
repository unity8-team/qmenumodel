/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Renato Araujo Oliveira Filho <renato@canonical.com>
 */

#ifndef QDBUSACTIONGROUP_H
#define QDBUSACTIONGROUP_H

#include "qdbusobject.h"

#include <QObject>
#include <QVariant>

class QStateAction;
class ActionStateParser;

typedef char gchar;
typedef void* gpointer;
typedef struct _GVariant GVariant;
typedef struct _GActionGroup GActionGroup;
typedef struct _GDBusActionGroup GDBusActionGroup;

class QDBusActionGroup : public QObject, public QDBusObject
{
    Q_OBJECT
    Q_PROPERTY(int busType READ busType WRITE setIntBusType NOTIFY busTypeChanged)
    Q_PROPERTY(QString busName READ busName WRITE setBusName NOTIFY busNameChanged)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath NOTIFY objectPathChanged)
    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(ActionStateParser* actionStateParser READ actionStateParser WRITE setActionStateParser NOTIFY actionStateParserChanged)

public:
    QDBusActionGroup(QObject *parent=0);
    ~QDBusActionGroup();

    void updateActionState(const QString &name, const QVariant &state);
    void activateAction(const QString &name, const QVariant &parameter);
    bool hasAction(const QString &name);

    Q_INVOKABLE QStateAction *action(const QString &name);
    Q_INVOKABLE QVariant actionState(const QString &name);

    ActionStateParser* actionStateParser() const;
    void setActionStateParser(ActionStateParser* actionStateParser);

Q_SIGNALS:
    void busTypeChanged(DBusEnums::BusType type);
    void busNameChanged(const QString &busNameChanged);
    void objectPathChanged(const QString &objectPath);
    void statusChanged(DBusEnums::ConnectionStatus status);
    void actionAppear(const QString &name);
    void actionVanish(const QString &name);
    void actionStateChanged(const QString &name, QVariant state);
    void actionStateParserChanged(ActionStateParser* parser);

public Q_SLOTS:
    void start();
    void stop();

protected:
    virtual void serviceAppear(GDBusConnection *connection);
    virtual void serviceVanish(GDBusConnection *connection);

    virtual bool event(QEvent* e);

private:
    GActionGroup *m_actionGroup;
    int m_signalActionAddId;
    int m_signalActionRemovedId;
    int m_signalStateChangedId;

    ActionStateParser* m_actionStateParser;

    // workaround to support int as busType
    void setIntBusType(int busType);

    void setActionGroup(GDBusActionGroup *ag);
    QStateAction *actionImpl(const QString &name);

    void clear();

    // glib slots
    static void onActionAdded(GDBusActionGroup *ag, gchar *name, gpointer data);
    static void onActionRemoved(GDBusActionGroup *ag, gchar *name, gpointer data);
    static void onActionStateChanged(GDBusActionGroup *ag, gchar *name, GVariant *value, gpointer data);
};

#endif // QDBUSACTIONGROUP_H
