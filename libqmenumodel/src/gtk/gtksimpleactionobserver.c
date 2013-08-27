/*
 * Copyright © 2013 Canonical Limited
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

#include "gtksimpleactionobserver.h"

typedef GObjectClass GtkSimpleActionObserverClass;

struct _GtkSimpleActionObserver
{
  GObject parent_instance;
  GtkActionObservable *observable;
  gchar* action_name;

  GtkActionAddedFunc action_added;
  GtkActionEnabledChangedFunc action_enabled_changed;
  GtkActionStateChangedFunc action_state_changed;
  GtkActionRemovedFunc action_removed;
};

static void gtk_simple_action_observer_init_observer_iface (GtkActionObserverInterface *iface);
G_DEFINE_TYPE_WITH_CODE (GtkSimpleActionObserver, gtk_simple_action_observer, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ACTION_OBSERVER, gtk_simple_action_observer_init_observer_iface))

static void
gtk_simple_action_observer_finalize (GObject *object)
{
  GtkSimpleActionObserver *self = GTK_SIMPLE_ACTION_OBSERVER (object);

  g_clear_object (&self->observable);

  g_free(self->action_name);

  G_OBJECT_CLASS (gtk_simple_action_observer_parent_class)->finalize (object);
}

static void
gtk_simple_action_observer_init (GtkSimpleActionObserver * self)
{
}

static void
gtk_simple_action_observer_class_init (GtkSimpleActionObserverClass *class)
{
  class->finalize = gtk_simple_action_observer_finalize;
}

static void
gtk_simple_action_observer_action_added (GtkActionObserver   *observer,
                                         GtkActionObservable *observable,
                                         const gchar         *action_name,
                                         const GVariantType  *parameter_type,
                                         gboolean             enabled,
                                         GVariant            *state)
{
  GtkSimpleActionObserver* self;
  self = GTK_SIMPLE_ACTION_OBSERVER (observer);
  self->action_added(self, action_name, enabled, state);
}

static void
gtk_simple_action_observer_action_enabled_changed (GtkActionObserver   *observer,
                                                   GtkActionObservable *observable,
                                                   const gchar         *action_name,
                                                   gboolean             enabled)
{
  GtkSimpleActionObserver* self;
  self = GTK_SIMPLE_ACTION_OBSERVER (observer);
  self->action_enabled_changed(self, action_name, enabled);
}

static void
gtk_simple_action_observer_action_state_changed (GtkActionObserver   *observer,
                                                 GtkActionObservable *observable,
                                                 const gchar         *action_name,
                                                 GVariant            *state)
{
  GtkSimpleActionObserver* self;
  self = GTK_SIMPLE_ACTION_OBSERVER (observer);
  self->action_state_changed(self, action_name, state);
}

static void
gtk_simple_action_observer_action_removed (GtkActionObserver   *observer,
                                           GtkActionObservable *observable,
                                           const gchar         *action_name)
{
  GtkSimpleActionObserver* self;
  self = GTK_SIMPLE_ACTION_OBSERVER (observer);
  self->action_removed(self, action_name);
}

static void
gtk_simple_action_observer_init_observer_iface (GtkActionObserverInterface *iface)
{
  iface->action_added = gtk_simple_action_observer_action_added;
  iface->action_enabled_changed = gtk_simple_action_observer_action_enabled_changed;
  iface->action_state_changed = gtk_simple_action_observer_action_state_changed;
  iface->action_removed = gtk_simple_action_observer_action_removed;
}

GtkSimpleActionObserver*
gtk_simple_action_observer_new (GtkActionObservable         *observable,
                                GtkActionAddedFunc           action_added,
                                GtkActionEnabledChangedFunc  action_enabled_changed,
                                GtkActionStateChangedFunc    action_state_changed,
                                GtkActionRemovedFunc         action_removed)
{
  GtkSimpleActionObserver* self;
  self = g_object_new (GTK_TYPE_SIMPLE_ACTION_OBSERVER, NULL);
  self->observable =  g_object_ref (observable);
  self->action_name = NULL;

  self->action_added = action_added;
  self->action_enabled_changed = action_enabled_changed;
  self->action_state_changed = action_state_changed;
  self->action_removed = action_removed;

  return self;
}

void
gtk_simple_action_observer_register_action (GtkSimpleActionObserver *self,
                                            const gchar             *action_name)
{
  gtk_simple_action_observer_unregister_action(self);

  if (action_name && g_strcmp0(action_name, "") != 0) {
    self->action_name = g_strdup (action_name);

    if (g_strcmp0(self->action_name, "") != 0) {
      gtk_action_observable_register_observer (self->observable, self->action_name, GTK_ACTION_OBSERVER (self));
    }
  }
}

void
gtk_simple_action_observer_unregister_action (GtkSimpleActionObserver *self)
{
  if (self->action_name) {
    gtk_action_observable_unregister_observer(self->observable, self->action_name, GTK_ACTION_OBSERVER (self));
    g_free(self->action_name);
    self->action_name = NULL;
  }
}
