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
    bool compareWithSchema(const QVariant &qv, const QString strType)
    {
        GVariantType* expected_type;
        expected_type = g_variant_type_new(strType.toUtf8().data());

        bool result;
        GVariant *gv = Converter::toGVariantWithSchema(qv, strType.toUtf8().data());
        result = g_variant_type_equal(g_variant_get_type(gv), expected_type);
        if (!result) {
            qWarning() << "types are different: QVariant:" << qv.typeName()
                       << "Result:" << (const char*) g_variant_get_type(gv)
                       << "Expected:"<< (const char*) expected_type;
        }
        g_variant_unref(gv);
        return result;
    }

private Q_SLOTS:

    /*
     * Test converter QVariant to GVariant
     */

    void testConvertToGVariant_data()
    {
        QTest::addColumn<QVariant>("value");
        QTest::addColumn<QString>("expectedType");

        QTest::newRow("Boolean") << QVariant(true) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_BOOLEAN);
        QTest::newRow("Byte") << QVariant::fromValue<uchar>(42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_BYTE);
        QTest::newRow("Int16") << QVariant::fromValue<short>(-42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_INT16);
        QTest::newRow("UInt16") << QVariant::fromValue<ushort>(-42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_UINT16);
        QTest::newRow("Int32") << QVariant(-42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_INT32);
        QTest::newRow("UInt32") << QVariant((uint)42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_UINT32);
        QTest::newRow("Int64") << QVariant::fromValue<long>(-42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_INT64);
        QTest::newRow("UInt64") << QVariant::fromValue<ulong>(42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_UINT64);
        QTest::newRow("Int64") << QVariant::fromValue<qlonglong>(-42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_INT64);
        QTest::newRow("UInt64") << QVariant::fromValue<qulonglong>(42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_UINT64);
        QTest::newRow("Double") << QVariant((double)42.42) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_DOUBLE);
        QTest::newRow("String") << QVariant(QString("42")) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_STRING);
        QTest::newRow("ByteArray") << QVariant(QByteArray("42")) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_BYTESTRING);
        QTest::newRow("Map") << QVariant(QVariantMap()) << reinterpret_cast<const gchar*>(G_VARIANT_TYPE_VARDICT);
    }

    void testConvertToGVariant()
    {
        QFETCH(QVariant, value);
        QFETCH(QString, expectedType);

        QVERIFY(compare(value, g_variant_type_new(expectedType.toUtf8().data())));
    }

    void testTupleConversion()
    {
        QVariantList qTuple;
        qTuple << 1 << "2" << 3.3;

        GVariant *gTuple = Converter::toGVariant(qTuple);
        QVERIFY(g_variant_type_is_tuple(g_variant_get_type(gTuple)));
        QCOMPARE(g_variant_n_children(gTuple), (gsize)3);

        GVariant *v = g_variant_get_child_value(gTuple, 0);
        int v0 = g_variant_get_int32(v);
        QCOMPARE(v0, 1);
        g_variant_unref(v);

        v = g_variant_get_child_value(gTuple, 1);
        const gchar *v1 = g_variant_get_string(v, NULL);
        QCOMPARE(QString(v1), QString("2"));
        g_variant_unref(v);

        v = g_variant_get_child_value(gTuple, 2);
        gdouble v2 = g_variant_get_double(v);
        QCOMPARE(v2, 3.3);
        g_variant_unref(v);

        g_variant_unref(gTuple);
    }

    void testConvertToGVariantWithSchema_data()
    {
        QTest::addColumn<QVariant>("value");
        QTest::addColumn<QString>("schema");

        // convert to integer
        QTest::newRow("integer") << QVariant::fromValue<int>(1) << "i";
        QTest::newRow("integer from double") << QVariant::fromValue<double>(1.1) << "i";

        // convert to bool
        QTest::newRow("bool") << QVariant::fromValue<bool>(true) << "b";
        QTest::newRow("bool from int") << QVariant::fromValue<int>(1) << "b";

        // convert to double
        QTest::newRow("double") << QVariant::fromValue<double>(1.0) << "d";
        QTest::newRow("double from int") << QVariant::fromValue<int>(1) << "d";

        // convert to string
        QTest::newRow("string") << QVariant::fromValue<QString>("FoooBar") << "x";
        QTest::newRow("string from int") << QVariant::fromValue<int>(1) << "s";
        QTest::newRow("string from double") << QVariant::fromValue<double>(1.1) << "s";

        // convert to tuple
        auto list = QVariantList() << QVariant::fromValue<bool>(true) << QVariant::fromValue<int>(1) << QVariant::fromValue<int>(1) << QVariant::fromValue<QString>("test1");
        QTest::newRow("tuple") << QVariant(list) << "(bdis)";

        // convert to array
        list = QVariantList() << QVariant::fromValue<int>(1) << QVariant::fromValue<int>(1);
        QTest::newRow("int list") << QVariant(list) << "ad";
        list = QVariantList() << QVariant::fromValue<QString>("test1") << QVariant::fromValue<QString>("test2");
        QTest::newRow("string list") << QVariant(list) << "as";

        // convert to array of tuple
        QVariantList si1(QVariantList() << QVariant::fromValue<QString>("test1") << QVariant::fromValue<int>(1));
        QVariantList si2(QVariantList() << QVariant::fromValue<QString>("test1") << QVariant::fromValue<int>(1));
        list = QVariantList() << QVariant::fromValue(si1) << QVariant::fromValue(si2);
        QTest::newRow("array of tuple") << QVariant(list) << "a(sd)";

        // convert to vardict
        QVariantMap map;
        map["test1"] = QVariant::fromValue<int>(1);
        map["test2"] = QVariant::fromValue<double>(1);
        QTest::newRow("map") << QVariant(map) << "a{sv}";
    }

    void testConvertToGVariantWithSchema()
    {
        QFETCH(QVariant, value);
        QFETCH(QString, schema);

        compareWithSchema(value, schema);
    }

};

QTEST_MAIN(ConverterTest)

#include "convertertest.moc"
