#include "converter.h"

#include <QDebug>

/*! \internal */
QVariant Converter::parseGVariant(GVariant *value)
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
