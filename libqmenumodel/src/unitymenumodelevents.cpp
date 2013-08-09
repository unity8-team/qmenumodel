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
 *      Nicholas Dedekind <nick.dedekind@canonical.com
 */

extern "C" {
#include <glib-object.h>
#include <gio/gio.h>
}

#include "unitymenumodelevents.h"
#include "unitymenumodel.h"

const QEvent::Type UnityMenuModelClearEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuModelAddRowEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuModelRemoveRowEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuModelDataChangeEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

UnityMenuModelClearEvent::UnityMenuModelClearEvent(bool _reset)
    : QEvent(UnityMenuModelClearEvent::eventType),
      reset(_reset)
{}

UnityMenuModelAddRowEvent::UnityMenuModelAddRowEvent(GtkMenuTrackerItem *_item, int _position)
    : QEvent(UnityMenuModelAddRowEvent::eventType),
      item(_item),
      position(_position)
{
    if (item) {
        g_object_ref(item);
    }
}

UnityMenuModelAddRowEvent::~UnityMenuModelAddRowEvent()
{
    if (item) {
        g_object_unref(item);
    }
}

UnityMenuModelRemoveRowEvent::UnityMenuModelRemoveRowEvent(int _position)
    : QEvent(UnityMenuModelRemoveRowEvent::eventType),
      position(_position)
{}

UnityMenuModelDataChangeEvent::UnityMenuModelDataChangeEvent(int _position)
    : QEvent(UnityMenuModelDataChangeEvent::eventType),
      position(_position)
{}
