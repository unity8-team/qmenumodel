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

#include "menunode.h"

#include <QMetaMethod>
#include <QDebug>

MenuNode::MenuNode(const QString &linkType, GMenuModel *model, MenuNode *parent, int pos, QObject *listener)
    : m_model(model),
      m_parent(parent),
      m_signalChangedId(0),
      m_linkType(linkType),
      m_currentOpPosition(-1),
      m_currentOpAdded(0),
      m_currentOpRemoved(0)
{
    g_object_ref(model);

    if (m_parent) {
        m_parent->insertChild(this, pos);
    }

    m_size = g_menu_model_get_n_items(model);
    for(int i=0; i < m_size; i++) {
        MenuNode::create(model, i, this, listener);
    }

    connect(listener);
}

MenuNode::~MenuNode()
{
    disconnect();
    Q_FOREACH(MenuNode *child, m_children) {
        delete child;
    }
    m_children.clear();
    if (m_model) {
        g_object_unref(m_model);
    }
}

void MenuNode::connect(QObject *listener)
{
    m_listener = listener;
    if (m_model && (m_signalChangedId == 0)) {
        m_signalChangedId = g_signal_connect(m_model,
                                             "items-changed",
                                             G_CALLBACK(MenuNode::onItemsChanged),
                                             this);
    }
}

void MenuNode::disconnect()
{
    if (m_signalChangedId != 0) {
        g_signal_handler_disconnect(m_model, m_signalChangedId);
    }
}

int MenuNode::position() const
{
    if (m_parent) {
        return m_parent->childPosition(this);
    } else {
        return 0;
    }
}

MenuNode *MenuNode::parent() const
{
    return m_parent;
}

GMenuModel *MenuNode::model() const
{
    return m_model;
}

QString MenuNode::linkType() const
{
    return m_linkType;
}

MenuNode *MenuNode::child(int pos) const
{
    if (m_children.contains(pos)) {
        return m_children.value(pos);
    }
    return 0;
}

int MenuNode::childPosition(GMenuModel *item) const
{
    QMap<int, MenuNode*>::const_iterator i = m_children.constBegin();
    while (i != m_children.constEnd()) {
        if (i.value()->m_model == item) {
            return i.key();
        }
        ++i;
    }
    return 0;
}
int MenuNode::childPosition(const MenuNode *item) const
{
    return childPosition(item->m_model);
}

int MenuNode::size() const
{
    return m_size;
}

int MenuNode::depth() const
{
    int depth = 0;
    const MenuNode *child = this;
    while(child->parent()) {
        depth++;
        child = child->parent();
    }
    return depth;
}

int MenuNode::realPosition(int row) const
{
    int result = row;
    if ((row >= 0) && (row < m_size)) {
        if (row >= m_currentOpPosition) {
            if ((m_currentOpRemoved > 0) && (row < (m_currentOpPosition + m_currentOpRemoved))) {
                result = -1;
            } else {
                result += (m_currentOpAdded - m_currentOpRemoved);
            }
        }
        return result;
    } else {
        return -1;
    }
}

void MenuNode::change(int start, int added, int removed)
{
    if (added > 0) {
        for (int i=(m_size - 1 + added), iMin=start; i >= iMin; i--) {
            if (m_children.contains(i)) {
                m_children.insert(i + added, m_children.take(i));
            }
        }

        m_size += added;

        for (int i = start; i < (start + added); i++) {
            MenuNode::create(m_model, i, this, m_listener);
        }
    }

    if (removed > 0) {
        int removedEnd = start + removed;
        for (int i=start, iMax=m_size; i < iMax; i++) {
            if (i <= removedEnd) {
                delete m_children.take(i);
            } else if (m_children.contains(i)) {
                m_children.insert(i - removed, m_children.take(i));
            }
        }
        m_size -= removed;
    }
}

void MenuNode::insertChild(MenuNode *child, int pos)
{
    if (m_children.contains(pos)) {
        qWarning() << "Section conflic: parent" << this << "child" << child << "pos" << pos;
        return;
    }

    child->m_parent = this;
    m_children.insert(pos, child);
}


MenuNode *MenuNode::find(GMenuModel *item)
{
    if (m_model == item) {
        return this;
    }

    Q_FOREACH(MenuNode *child, m_children) {
        MenuNode *found = child->find(item);
        if (found) {
            return found;
        }
    }
    return 0;
}

MenuNode *MenuNode::create(GMenuModel *model, int pos, MenuNode *parent, QObject *listener)
{
    QString linkType(G_MENU_LINK_SUBMENU);
    GMenuModel *link = g_menu_model_get_item_link(model, pos, G_MENU_LINK_SUBMENU);
    if (link == NULL) {
        linkType = G_MENU_LINK_SECTION;
        link = g_menu_model_get_item_link(model, pos, G_MENU_LINK_SECTION);
    }

    if (link) {
        return new MenuNode(linkType, link, parent, pos, listener);
    }
    return 0;
}

void MenuNode::commitOperation()
{
    change(m_currentOpPosition, m_currentOpAdded, m_currentOpRemoved);

    m_currentOpPosition = -1;
    m_currentOpAdded = m_currentOpRemoved = 0;
}

void MenuNode::onItemsChanged(GMenuModel *model, gint position, gint removed, gint added, gpointer data)
{
    MenuNode *self = reinterpret_cast<MenuNode*>(data);
    self->m_currentOpPosition = position;
    self->m_currentOpAdded = added;
    self->m_currentOpRemoved = removed;

    const QMetaObject *mobj = self->m_listener->metaObject();
    if (!mobj->invokeMethod(self->m_listener,
                           "onItemsChanged",
                           Q_ARG(MenuNode*, self),
                           Q_ARG(int, position),
                           Q_ARG(int, removed),
                           Q_ARG(int, added)))
    {
        qWarning() << "Slot 'onItemsChanged(MenuNode*, int, int, int)' not found in" << self->m_listener;
    }
    self->commitOperation();
}
