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
 * Authors: Lars Uebernickel <lars.uebernickel@canonical.com>
 */

#include "unitythemediconprovider.h"

#include <QIcon>

UnityThemedIconProvider::UnityThemedIconProvider():
  QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

#include <QtDebug>

QPixmap UnityThemedIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
  QPixmap pixmap = QIcon::fromTheme(id).pixmap(requestedSize.isValid() ? requestedSize : QSize(32, 32));
  *size = pixmap.size();
  return pixmap;
}
