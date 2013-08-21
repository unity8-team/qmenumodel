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
 * Authors: Nick Dedekind <nick.dedekind@canonical.com>
 */

#include "unitymenuaction.h"
#include "unitymenumodel.h"

#include <QDebug>

UnityMenuAction::UnityMenuAction(QObject* parent)
    :   QObject(parent),
        m_valid(false),
        m_enabled(false),
        m_model(NULL)
{
}

UnityMenuAction::~UnityMenuAction()
{
    if (m_model) {
        m_model->unregisterAction(this);
    }
}

QString UnityMenuAction::name() const
{
    return m_name;
}

void UnityMenuAction::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged(m_name);
    }
}

UnityMenuModel* UnityMenuAction::model() const
{
    return m_model;
}

void UnityMenuAction::setModel(UnityMenuModel* model)
{
    if (m_model != model) {
        if (!model) {
            unregisterAction();
        }
        m_model = model;
        registerAction();
        Q_EMIT modelChanged(model);
    }
}

QVariant UnityMenuAction::state() const
{
    return m_state;
}

bool UnityMenuAction::isEnabled() const
{
    return m_enabled;
}

bool UnityMenuAction::isValid() const
{
    return m_valid;
}

void UnityMenuAction::onAdded(bool enabled, const QVariant &state)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged(m_enabled);
    }
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
    if (m_valid != true) {
        m_valid = true;
        Q_EMIT validChanged(m_valid);
    }
}

void UnityMenuAction::onRemoved()
{
    if (m_valid != false) {
        m_valid = false;
        Q_EMIT validChanged(m_valid);
    }
}

void UnityMenuAction::onEnabledChanged(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged(m_enabled);
    }
}

void UnityMenuAction::onStateChanged(const QVariant &state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}

void UnityMenuAction::registerAction()
{
    if (m_model) {
        m_model->registerAction(this);
    }
}

void UnityMenuAction::unregisterAction()
{
    if (m_model) {
        m_model->unregisterAction(this);
    }
}
