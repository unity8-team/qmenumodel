/*
 * Copyright 2013 Canonical Ltd.
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
 *      Nick Dedekind <nick.dedekind@canonical.com>
 */

#include "unitymenuaction.h"
#include "unitymenumodel.h"

UnityMenuAction::UnityMenuAction(QObject* parent)
    : QObject(parent),
      m_model(NULL),
      m_index(-1)
{
}

int UnityMenuAction::index() const
{
    return m_index;
}

void UnityMenuAction::setIndex(int index)
{
    if (m_index != index) {
        m_index = index;
        Q_EMIT indexChanged(index);
    }
}

UnityMenuModel* UnityMenuAction::model() const
{
    return m_model;
}

void UnityMenuAction::setModel(UnityMenuModel* model)
{
    if (m_model != model) {
        if (m_model) {
            disconnect(m_model);
        }
        m_model = model;

        connect(model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)), SIGNAL(stateChanged()));

        Q_EMIT modelChanged(model);
    }
}
