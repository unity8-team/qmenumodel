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

#ifndef QDBUSACTION_H
#define QDBUSACTION_H

#include <QAction>
#include <QVariant>

class QStateAction : public QAction
{
    Q_OBJECT
    Q_PROPERTY(QVariant state READ state WRITE setState NOTIFY stateChanged)
public:
    QStateAction(const QString &text="", QObject *parent=0);

    QVariant state() const;
    void setState(const QVariant &state);

Q_SIGNALS:
    void stateChanged(QVariant state);

private:
    QVariant m_state;
};

#endif
