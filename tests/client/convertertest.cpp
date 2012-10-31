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

extern "C" {
#include <glib.h>
}

#include "converter.h"

#include <QObject>
#include <QtTest>
#include <QDebug>

class ConverterTest : public QObject
{
    Q_OBJECT

private:
    bool compare(const QVariant &qv, const GVariantType *type)
    {
        bool result;
        GVariant *gv = Converter::toGVariant(qv);
        result = g_variant_type_equal(g_variant_get_type(gv), type);
        if (!result) {
            qWarning() << "types are different: QVariant:" << qv.typeName()
                       << "Result:" << (const char*) g_variant_get_type(gv)
                       << "Expected:"<< (const char*) type;
        }
        g_variant_unref(gv);
        return result;
    }

private Q_SLOTS:

    /*
     * Test converter QVariant to GVariant
     */
    void testToGVariant()
    {
        // Boolean
        QVERIFY(compare(QVariant(true), G_VARIANT_TYPE_BOOLEAN));

        // Byte
        QVERIFY(compare(QVariant::fromValue<uchar>(42), G_VARIANT_TYPE_BYTE));

        // Int16
        QVERIFY(compare(QVariant::fromValue<short>(-42), G_VARIANT_TYPE_INT16));

        // UInt16
        QVERIFY(compare(QVariant::fromValue<ushort>(-42), G_VARIANT_TYPE_UINT16));

        // Int32
        QVERIFY(compare(QVariant(-42), G_VARIANT_TYPE_INT32));

        // UInt32
        QVERIFY(compare(QVariant((uint)42), G_VARIANT_TYPE_UINT32));

        // Int64
        QVERIFY(compare(QVariant::fromValue<long>(-42), G_VARIANT_TYPE_INT64));

        // UInt64
        QVERIFY(compare(QVariant::fromValue<ulong>(42), G_VARIANT_TYPE_UINT64));

        // Double
        QVERIFY(compare(QVariant((double)42.42), G_VARIANT_TYPE_DOUBLE));

        // String
        QVERIFY(compare(QVariant(QString("42")), G_VARIANT_TYPE_STRING));

        // ByteArray
        QVERIFY(compare(QVariant(QByteArray("42")), G_VARIANT_TYPE_BYTESTRING));

        // Map
        QVERIFY(compare(QVariantMap(), G_VARIANT_TYPE_VARDICT));
    }

};

QTEST_MAIN(ConverterTest)

#include "convertertest.moc"
