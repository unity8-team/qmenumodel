#include "converter.h"

#include <QDebug>

/*! \internal */
QVariant Converter::toQVariant(GVariant *value)
{
    QVariant result;
    if (value == NULL) {
        return result;
    }

    const GVariantType *type = g_variant_get_type(value);
    if (g_variant_type_equal(type, G_VARIANT_TYPE_BOOLEAN)) {
        result.setValue((bool)g_variant_get_boolean(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTE)) {
        result.setValue(g_variant_get_byte(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT16)) {
        result.setValue(g_variant_get_int16(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT16)) {
        result.setValue(g_variant_get_uint16(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT32)) {
        result.setValue(g_variant_get_int32(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT32)) {
        result.setValue(g_variant_get_uint32(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT64)) {
        result.setValue(g_variant_get_int64(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT64)) {
        result.setValue(g_variant_get_uint64(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_DOUBLE)) {
        result.setValue(g_variant_get_double(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_STRING)) {
        gsize size = 0;
        const gchar *v = g_variant_get_string(value, &size);
        result.setValue(QString::fromLatin1(v, size));
    } else {
        qWarning() << "Unsupported GVariant value";
    }

    /* TODO: implement convertions to others types
     * G_VARIANT_TYPE_HANDLE
     * G_VARIANT_TYPE_OBJECT_PATH
     * G_VARIANT_TYPE_SIGNATURE
     * G_VARIANT_TYPE_VARIANT
     * G_VARIANT_TYPE_ANY
     * G_VARIANT_TYPE_BASIC
     * G_VARIANT_TYPE_MAYBE
     * G_VARIANT_TYPE_ARRAY
     * G_VARIANT_TYPE_TUPLE
     * G_VARIANT_TYPE_UNIT
     * G_VARIANT_TYPE_DICT_ENTRY
     * G_VARIANT_TYPE_DICTIONARY
     * G_VARIANT_TYPE_STRING_ARRAY
     * G_VARIANT_TYPE_BYTESTRING
     * G_VARIANT_TYPE_OBJECT_PATH_ARRAY
     * G_VARIANT_TYPE_BYTESTRING_ARRAY
     * G_VARIANT_TYPE_VARDICT
     */

    return result;
}

GVariant* Converter::toGVariant(const QString &typeName, const QVariant &value)
{
    if (typeName == "uchar") {
        return g_variant_new_byte(value.value<uchar>());
    } else if (typeName == "short") {
        return g_variant_new_int16(value.value<short>());
    } else if (typeName == "ushort") {
        return g_variant_new_uint16(value.value<ushort>());
    } else if (typeName == "long") {
        return g_variant_new_int64(value.value<long>());
    } else if (typeName == "ulong") {
        return g_variant_new_uint64(value.value<ulong>());
    } else {
        qWarning() << "QVariant type not supported:" << typeName;
    }

    return NULL;
}

GVariant* Converter::toGVariant(const QVariant &value)
{
    GVariant *result = NULL;
    if (value.isNull() || !value.isValid())
        return result;

    switch(value.type()) {
    case QVariant::Bool:
        result = g_variant_new_boolean(value.toBool());
        break;
    case QVariant::ByteArray:
        result = g_variant_new_bytestring(value.toByteArray());
        break;
    case QVariant::Double:
        result = g_variant_new_double(value.toDouble());
        break;
    case QVariant::Int:
        result = g_variant_new_int32(value.toInt());
        break;
    case QVariant::String:
        result = g_variant_new_string(value.toString().toLatin1());
        break;
    case QVariant::UInt:
        result = g_variant_new_uint32(value.toUInt());
        break;
    default:
        result = toGVariant(value.typeName(), value);
    }

    return result;
}
