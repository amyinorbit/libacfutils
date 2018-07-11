/*
 * CDDL HEADER START
 *
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 *
 * CDDL HEADER END
*/
/*
 * Copyright 2017 Saso Kiselkov. All rights reserved.
 */

#include <XPLMPlugin.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <acfutils/assert.h>
#include <acfutils/dr.h>

#define	DRE_MSG_ADD_DATAREF	0x01000000

bool_t
dr_find(dr_t *dr, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(dr->name, sizeof (dr->name), fmt, ap);
	va_end(ap);

	dr->dr = XPLMFindDataRef(dr->name);
	if (dr->dr == NULL) {
		memset(dr, 0, sizeof (*dr));
		return (B_FALSE);
	}
	dr->type = XPLMGetDataRefTypes(dr->dr);
	VERIFY_MSG(dr->type & (xplmType_Int | xplmType_Float | xplmType_Double |
	    xplmType_IntArray | xplmType_FloatArray | xplmType_Data),
	    "dataref \"%s\" has bad type %x", dr->name, dr->type);
	dr->writable = XPLMCanWriteDataRef(dr->dr);
	dr->read_cb = NULL;
	dr->write_cb = NULL;
	dr->cb_userinfo = NULL;
	return (B_TRUE);
}

bool_t
dr_writable(dr_t *dr)
{
	return (dr->writable);
}

int
dr_geti(dr_t *dr)
{
	if (dr->type & xplmType_Int)
		return (XPLMGetDatai(dr->dr));
	if (dr->type & xplmType_Float)
		return (XPLMGetDataf(dr->dr));
	if (dr->type & xplmType_Double)
		return (XPLMGetDatad(dr->dr));
	if (dr->type & (xplmType_FloatArray | xplmType_IntArray |
	    xplmType_Data)) {
		int i;
		VERIFY3U(dr_getvi(dr, &i, 0, 1), ==, 1);
		return (i);
	}
	VERIFY_MSG(0, "dataref \"%s\" has bad type %x", dr->name, dr->type);
}

void
dr_seti(dr_t *dr, int i)
{
	ASSERT_MSG(dr->writable, "%s", dr->name);
	if (dr->type & xplmType_Int) {
		XPLMSetDatai(dr->dr, i);
	} else if (dr->type & xplmType_Float) {
		XPLMSetDataf(dr->dr, i);
	} else if (dr->type & xplmType_Double) {
		XPLMSetDatad(dr->dr, i);
	} else if (dr->type & (xplmType_FloatArray | xplmType_IntArray |
	    xplmType_Data)) {
		dr_setvi(dr, &i, 0, 1);
	} else {
		VERIFY_MSG(0, "dataref \"%s\" has bad type %x",
		    dr->name, dr->type);
	}
}

double
dr_getf(dr_t *dr)
{
	if (dr->type & xplmType_Int)
		return (XPLMGetDatai(dr->dr));
	if (dr->type & xplmType_Float)
		return (XPLMGetDataf(dr->dr));
	if (dr->type & xplmType_Double)
		return (XPLMGetDatad(dr->dr));
	if (dr->type & (xplmType_FloatArray | xplmType_IntArray |
	    xplmType_Data)) {
		double f;
		VERIFY3U(dr_getvf(dr, &f, 0, 1), ==, 1);
		return (f);
	}
	VERIFY_MSG(0, "dataref \"%s\" has bad type %x", dr->name, dr->type);
}

void
dr_setf(dr_t *dr, double f)
{
	ASSERT_MSG(dr->writable, "%s", dr->name);
	if (dr->type & xplmType_Int)
		XPLMSetDatai(dr->dr, f);
	else if (dr->type & xplmType_Float)
		XPLMSetDataf(dr->dr, f);
	else if (dr->type & xplmType_Double)
		XPLMSetDatad(dr->dr, f);
	else if (dr->type & (xplmType_FloatArray | xplmType_IntArray |
	    xplmType_Data))
		dr_setvf(dr, &f, 0, 1);
	else
		VERIFY_MSG(0, "dataref \"%s\" has bad type %x",
		    dr->name, dr->type);
}

int
dr_getvi(dr_t *dr, int *i, unsigned off, unsigned num)
{
	ASSERT(i != NULL || num == 0);

	if (dr->type & xplmType_IntArray)
		return (XPLMGetDatavi(dr->dr, i, off, num));
	if (dr->type & xplmType_FloatArray) {
		float f[num];
		int n = XPLMGetDatavf(dr->dr, num > 0 ? f : NULL, off, num);
		if (num != 0) {
			for (int x = 0; x < n; x++)
				i[x] = f[x];
		}
		return (n);
	}
	if (dr->type & xplmType_Data) {
		uint8_t u[num];
		int n = XPLMGetDatab(dr->dr, num > 0 ? u : NULL, off, num);
		if (num != 0) {
			for (int x = 0; x < n; x++)
				i[x] = u[x];
		}
		return (n);
	}
	VERIFY_MSG(0, "dataref \"%s\" has bad type %x", dr->name, dr->type);
}

void
dr_setvi(dr_t *dr, int *i, unsigned off, unsigned num)
{
	ASSERT_MSG(dr->writable, "%s", dr->name);
	if (dr->type & xplmType_IntArray) {
		XPLMSetDatavi(dr->dr, i, off, num);
	} else if (dr->type & xplmType_FloatArray) {
		float f[num];
		for (unsigned x = 0; x < num; x++)
			f[x] = i[x];
		XPLMSetDatavf(dr->dr, f, off, num);
	} else if (dr->type & xplmType_Data) {
		uint8_t u[num];
		for (unsigned x = 0; x < num; x++)
			u[x] = i[x];
		XPLMSetDatab(dr->dr, u, off, num);
	} else {
		VERIFY_MSG(0, "dataref \"%s\" has bad type %x",
		    dr->name, dr->type);
	}
}

int
dr_getvf(dr_t *dr, double *df, unsigned off, unsigned num)
{
	ASSERT(df != NULL || num == 0);

	if (dr->type & xplmType_IntArray) {
		int i[num];
		int n = XPLMGetDatavi(dr->dr, num > 0 ? i : NULL, off, num);
		if (num != 0) {
			for (int x = 0; x < n; x++)
				df[x] = i[x];
		}
		return (n);
	}
	if (dr->type & xplmType_FloatArray) {
		float f[num];
		int n = XPLMGetDatavf(dr->dr, num > 0 ? f : NULL, off, num);
		if (num != 0) {
			for (int x = 0; x < n; x++)
				df[x] = f[x];
		}
		return (n);
	}
	if (dr->type & xplmType_Data) {
		uint8_t u[num];
		int n = XPLMGetDatab(dr->dr, num > 0 ? u : NULL, off, num);
		if (num != 0) {
			for (int x = 0; x < n; x++)
				df[x] = u[x];
		}
		return (n);
	}
	VERIFY_MSG(0, "dataref \"%s\" has bad type %x", dr->name, dr->type);
}

int
dr_getvf32(dr_t *dr, float *ff, unsigned off, unsigned num)
{
	ASSERT(ff != NULL || num == 0);

	if (dr->type & xplmType_IntArray) {
		int i[num];
		int n = XPLMGetDatavi(dr->dr, num > 0 ? i : NULL, off, num);
		if (num != 0) {
			for (int x = 0; x < n; x++)
				ff[x] = i[x];
		}
		return (n);
	}
	if (dr->type & xplmType_FloatArray) {
		int n = XPLMGetDatavf(dr->dr, ff, off, num);
		return (n);
	}
	if (dr->type & xplmType_Data) {
		uint8_t u[num];
		int n = XPLMGetDatab(dr->dr, num > 0 ? u : NULL, off, num);
		if (num != 0) {
			for (int x = 0; x < n; x++)
				ff[x] = u[x];
		}
		return (n);
	}
	VERIFY_MSG(0, "dataref \"%s\" has bad type %x", dr->name, dr->type);
}

void
dr_setvf(dr_t *dr, double *df, unsigned off, unsigned num)
{
	ASSERT_MSG(dr->writable, "%s", dr->name);
	if (dr->type & xplmType_IntArray) {
		int i[num];
		for (unsigned x = 0; x < num; x++)
			i[x] = df[x];
		XPLMSetDatavi(dr->dr, i, off, num);
	} else if (dr->type & xplmType_FloatArray) {
		float f[num];
		for (unsigned x = 0; x < num; x++)
			f[x] = df[x];
		XPLMSetDatavf(dr->dr, f, off, num);
	} else if (dr->type & xplmType_Data) {
		uint8_t u[num];
		for (unsigned x = 0; x < num; x++)
			u[x] = df[x];
		XPLMSetDatab(dr->dr, u, off, num);
	} else {
		VERIFY_MSG(0, "dataref \"%s\" has bad type %x",
		    dr->name, dr->type);
	}
}

void
dr_setvf32(dr_t *dr, float *ff, unsigned off, unsigned num)
{
	ASSERT_MSG(dr->writable, "%s", dr->name);
	if (dr->type & xplmType_IntArray) {
		int i[num];
		for (unsigned x = 0; x < num; x++)
			i[x] = ff[x];
		XPLMSetDatavi(dr->dr, i, off, num);
	} else if (dr->type & xplmType_FloatArray) {
		XPLMSetDatavf(dr->dr, ff, off, num);
	} else if (dr->type & xplmType_Data) {
		uint8_t u[num];
		for (unsigned x = 0; x < num; x++)
			u[x] = ff[x];
		XPLMSetDatab(dr->dr, u, off, num);
	} else {
		VERIFY_MSG(0, "dataref \"%s\" has bad type %x",
		    dr->name, dr->type);
	}
}

int
dr_gets(dr_t *dr, char *str, size_t cap)
{
	int n;

	ASSERT_MSG(dr->type & xplmType_Data, "%s", dr->name);
	n = XPLMGetDatab(dr->dr, str, 0, cap > 0 ? cap - 1 : 0);
	if (cap != 0)
		str[n] = '\0';	/* make sure it's properly terminated */

	return (n);
}

void
dr_sets(dr_t *dr, char *str)
{
	ASSERT_MSG(dr->type & xplmType_Data, "%s", dr->name);
	ASSERT_MSG(dr->writable, "%s", dr->name);
	XPLMSetDatab(dr->dr, str, 0, strlen(str));
}

static int
read_int_cb(void *refcon)
{
	dr_t *dr = refcon;
	ASSERT(dr != NULL);
	ASSERT_MSG(dr->type == xplmType_Int, "%s", dr->name);
	ASSERT_MSG(dr->value != NULL, "%s", dr->name);
	if (dr->read_cb != NULL)
		dr->read_cb(dr);
	return (*(int *)dr->value);
}

static void
write_int_cb(void *refcon, int value)
{
	dr_t *dr = refcon;
	ASSERT(dr != NULL);
	ASSERT_MSG(dr->type == xplmType_Int, "%s", dr->name);
	ASSERT_MSG(dr->value != NULL, "%s", dr->name);
	ASSERT_MSG(dr->writable, "%s", dr->name);
	if (dr->write_cb != NULL)
		dr->write_cb(dr, &value);
	*(int *)dr->value = value;
}

static float
read_float_cb(void *refcon)
{
	dr_t *dr = refcon;
	ASSERT(dr != NULL);
	ASSERT_MSG(dr->type == xplmType_Float, "%s", dr->name);
	ASSERT_MSG(dr->value != NULL, "%s", dr->name);
	if (dr->read_cb != NULL)
		dr->read_cb(dr);
	return (*(float *)dr->value);
}

static void
write_float_cb(void *refcon, float value)
{
	dr_t *dr = refcon;
	ASSERT(dr != NULL);
	ASSERT_MSG(dr->type == xplmType_Float, "%s", dr->name);
	ASSERT_MSG(dr->value != NULL, "%s", dr->name);
	ASSERT_MSG(dr->writable, "%s", dr->name);
	if (dr->write_cb != NULL)
		dr->write_cb(dr, &value);
	*(float *)dr->value = value;
}

#define	DEF_READ_ARRAY_CB(typename, xp_typename, type_sz) \
static int \
read_ ## typename ## _array_cb(void *refcon, typename *out_values, int off, \
    int count) \
{ \
	dr_t *dr = refcon; \
 \
	ASSERT(dr != NULL); \
	ASSERT_MSG(dr->type == xplmType_ ## xp_typename, "%s", dr->name); \
	ASSERT_MSG(dr->value != NULL, "%s", dr->name); \
 \
	if (dr->read_array_cb != NULL) \
		dr->read_array_cb(dr, out_values, off, count); \
	if (out_values == NULL) \
		return (dr->count); \
	if (off < dr->count) { \
		count = MIN(count, dr->count - off); \
		memcpy(out_values, dr->value + (off * type_sz), \
		    type_sz * count); \
	} \
 \
	return (count); \
}

#define	DEF_WRITE_ARRAY_CB(typename, xp_typename, type_sz) \
static void \
write_ ## typename ## _array_cb(void *refcon, typename *in_values, int off, \
    int count) \
{ \
	dr_t *dr = refcon; \
 \
	ASSERT(dr != NULL); \
	ASSERT_MSG(dr->type == xplmType_ ## xp_typename, "%s", dr->name); \
	ASSERT_MSG(dr->value != NULL, "%s", dr->name); \
	ASSERT_MSG(dr->writable, "%s", dr->name); \
 \
	if (dr->write_array_cb != NULL) \
		dr->write_array_cb(dr, in_values, off, count); \
	if (off < dr->count) { \
		count = MIN(count, dr->count - off); \
		memcpy(dr->value + (off * type_sz), in_values, \
		    type_sz * count); \
	} \
}

DEF_READ_ARRAY_CB(int, IntArray, sizeof (int))
DEF_WRITE_ARRAY_CB(int, IntArray, sizeof (int))
DEF_READ_ARRAY_CB(float, FloatArray, sizeof (int))
DEF_WRITE_ARRAY_CB(float, FloatArray, sizeof (int))
DEF_READ_ARRAY_CB(void, Data, sizeof (uint8_t))
DEF_WRITE_ARRAY_CB(void, Data, sizeof (uint8_t))

#undef	DEF_READ_ARRAY_CB
#undef	DEF_WRITE_ARRAY_CB

static void
dr_create_common(dr_t *dr, XPLMDataTypeID type, void *value, size_t count,
    bool_t writable, const char *fmt, va_list ap)
{
	XPLMPluginID dre_plug;

	vsnprintf(dr->name, sizeof (dr->name), fmt, ap);

	dr->dr = XPLMRegisterDataAccessor(dr->name, type, writable,
	    read_int_cb, write_int_cb, read_float_cb, write_float_cb,
	    NULL, NULL, read_int_array_cb, write_int_array_cb,
	    read_float_array_cb, write_float_array_cb,
	    read_void_array_cb, write_void_array_cb, dr, dr);

	VERIFY(dr->dr != NULL);
	dr->type = type;
	dr->writable = writable;
	dr->value = value;
	dr->count = count;
	dr->read_cb = NULL;
	dr->write_cb = NULL;

	dre_plug = XPLMFindPluginBySignature(
	    "xplanesdk.examples.DataRefEditor");
	if (dre_plug != XPLM_NO_PLUGIN_ID) {
		XPLMSendMessageToPlugin(dre_plug, DRE_MSG_ADD_DATAREF,
		    (void*)dr->name);
	}
}

/*
 * Sets up an integer dataref that will read and optionally write to
 * an int*.
 */
void
dr_create_i(dr_t *dr, int *value, bool_t writable, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dr_create_common(dr, xplmType_Int, value, 1, writable, fmt, ap);
	va_end(ap);
}

/*
 * Sets up a float dataref that will read and optionally write to
 * an float*.
 */
void
dr_create_f(dr_t *dr, float *value, bool_t writable, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dr_create_common(dr, xplmType_Float, value, 1, writable, fmt, ap);
	va_end(ap);
}

void
dr_create_vi(dr_t *dr, int *value, size_t n, bool_t writable,
    const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dr_create_common(dr, xplmType_IntArray, value, n, writable, fmt, ap);
	va_end(ap);
}

void
dr_create_vf(dr_t *dr, float *value, size_t n, bool_t writable,
    const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dr_create_common(dr, xplmType_FloatArray, value, n, writable, fmt, ap);
	va_end(ap);
}

void
dr_create_b(dr_t *dr, void *value, size_t n, bool_t writable,
    const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dr_create_common(dr, xplmType_Data, value, n, writable, fmt, ap);
	va_end(ap);
}

/*
 * Destroys a dataref previously set up using dr_intf_add_{i,f}.
 */
void
dr_delete(dr_t *dr)
{
	VERIFY_MSG(dr->value != NULL, "%s", dr->name);
	XPLMUnregisterDataAccessor(dr->dr);
	memset(dr, 0, sizeof (*dr));
}
