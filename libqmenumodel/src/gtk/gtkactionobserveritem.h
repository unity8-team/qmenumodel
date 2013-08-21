/*
 * Copyright © 2011 Canonical Limited
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * licence or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Nick Dedekind <nick.dedekind@canonical.com
 */

#ifndef __GTK_ACTION_OBSERVER_ITEM_H__
#define __GTK_ACTION_OBSERVER_ITEM_H__

#include "gtkactionobserver.h"

G_BEGIN_DECLS

#define GTK_TYPE_ACTION_OBSERVER_ITEM                         (gtk_action_observer_item_get_type ())
#define GTK_ACTION_OBSERVER_ITEM(inst)                         (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                              GTK_TYPE_ACTION_OBSERVER_ITEM, GtkActionObserverItem))
#define GTK_IS_ACTION_OBSERVER_ITEM(inst)                      (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                              GTK_TYPE_ACTION_OBSERVER_ITEM))

typedef struct _GtkActionObserverItem GtkActionObserverItem;

typedef void (* GtkActionAddedFunc)             (GtkActionObserverItem    *observer_item,
                                                 const gchar          *action_name,
                                                 gboolean              enabled,
                                                 GVariant             *state);

typedef void (* GtkActionEnabledChangedFunc)    (GtkActionObserverItem    *observer_item,
                                                 const gchar          *action_name,
                                                 gboolean              enabled);

typedef void (* GtkActionStateChangedFunc)      (GtkActionObserverItem    *observer_item,
                                                 const gchar          *action_name,
                                                 GVariant             *state);

typedef void (* GtkActionRemovedFunc)           (GtkActionObserverItem    *observer_item,
                                                 const gchar          *action_name);

GType                     gtk_action_observer_item_get_type                   (void) G_GNUC_CONST;

GtkActionObserverItem*    gtk_action_observer_item_new                        (GtkActionObservable   *observable,
                                                                               GtkActionAddedFunc action_added,
                                                                               GtkActionEnabledChangedFunc action_enabled_changed,
                                                                               GtkActionStateChangedFunc action_state_changed,
                                                                               GtkActionRemovedFunc action_removed);


void                      gtk_action_observer_item_register_action            (GtkActionObserverItem *self,
                                                                               const gchar* action_name);

void                      gtk_action_observer_item_unregister_action          (GtkActionObserverItem *self);

G_END_DECLS

#endif // __GTK_ACTION_OBSERVER_ITEM_H__