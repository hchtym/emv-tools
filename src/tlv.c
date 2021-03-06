#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "tlv.h"

#define TLV_TAG_CLASS_MASK	0xc0
#define TLV_TAG_COMPLEX		0x20
#define TLV_TAG_VALUE_MASK	0x1f
#define TLV_TAG_VALUE_CONT	0x1f
#define TLV_TAG_INVALID		0

#define TLV_LEN_LONG		0x80
#define TLV_LEN_MASK		0x7f
#define TLV_LEN_INVALID		0

struct tlv_elem {
	struct tlv_elem_info info;

	struct tlv_elem *next;
};

struct tlv {
	unsigned char *buf;
	size_t len;

	struct tlv_elem  *e;
};

static uint16_t tlv_parse_tag(const unsigned char **buf, size_t *len)
{
	uint16_t tag;

	if (*len == 0)
		return TLV_TAG_INVALID;
	tag = **buf;
	--*len;
	++*buf;
	if ((tag & TLV_TAG_VALUE_MASK) != TLV_TAG_VALUE_CONT)
		return tag;

	if (*len == 0)
		return TLV_TAG_INVALID;

	tag |= **buf << 8;
	--*len;
	++*buf;

	return tag;
}

struct tlv *tlv_parse_copy(const unsigned char *buf, size_t len)
{
	if (!buf)
		return NULL;

	unsigned char *buf2 = malloc(len);
	memcpy(buf2, buf, len);

	struct tlv *r = tlv_parse(buf2, len);

	if (!r)
		free(buf2);

	return r;
}

static size_t tlv_parse_len(const unsigned char **buf, size_t *len)
{
	size_t l;

	if (*len == 0)
		return TLV_LEN_INVALID;

	l = **buf;
	--*len;
	++*buf;

	if (!(l & TLV_LEN_LONG))
		return l;

	size_t ll = l &~ TLV_LEN_LONG;
	if (*len < ll)
		return TLV_LEN_INVALID;

	/* FIXME */
	if (ll != 1)
		return TLV_LEN_INVALID;

	l = **buf;
	--*len;
	++*buf;

	return l;
}

static bool tlv_parse_one(struct tlv *tlv, const unsigned char **buf, size_t *len)
{
	struct tlv_elem *e = calloc(1, sizeof(*e));
	bool rc = false;
	e->info.tag = tlv_parse_tag(buf, len);
	e->info.len = tlv_parse_len(buf, len);
	e->info.ptr = *buf;

	if (*len < e->info.len || e->info.len == TLV_LEN_INVALID || e->info.tag == TLV_TAG_INVALID) {
		*len = 0;
		free(e);
		return false;
	}

	rc = true;

	if (e->info.tag & TLV_TAG_COMPLEX) {
		const unsigned char *b = e->info.ptr;
		size_t l = e->info.len;
		while (l > 0) {
			rc = tlv_parse_one(tlv, &b, &l);
		}
	}

	e->next = tlv->e;
	tlv->e = e;

	*buf += e->info.len;
	*len -= e->info.len;
	return rc;
}

void tlv_free(struct tlv *tlv)
{
	if (!tlv)
		return;

	while (tlv->e) {
		struct tlv_elem *e = tlv->e;
		tlv->e = e->next;
		free(e);
	}

	free(tlv->buf);
	free(tlv);
}

struct tlv *tlv_parse(unsigned char *buf, size_t len)
{
	const unsigned char *cbuf = buf;

	if (!buf)
		return NULL;

	struct tlv *r = calloc(1, sizeof(*r));
	if (!r)
		return NULL;

	r->buf = buf;
	r->len = len;

	if (tlv_parse_one(r, &cbuf, &len))
		return r;
	else {
		r->buf = NULL; /* Don't free it in tlv_free */
		tlv_free(r);
		return NULL;
	}
}

bool tlv_visit(struct tlv *tlv, bool (*cb)(void *data, const struct tlv_elem_info *tei), void *data)
{
	if (!tlv)
		return false;

	struct tlv_elem *e;
	bool rc = true;
	for (e = tlv->e; e && rc; e = e->next) {
		rc = cb(data, &e->info);
	}

	return rc;
}

const struct tlv_elem_info *tlv_get(struct tlv *tlv, uint16_t tag)
{
	if (!tlv)
		return NULL;

	struct tlv_elem *e;
	for (e = tlv->e; e; e = e->next) {
		if (e->info.tag == tag)
			return &e->info;
	}

	return NULL;
}

struct tlv *tlv_new(uint16_t tag, unsigned char *buf, size_t len)
{
	if (!buf)
		return NULL;

	struct tlv *r = calloc(1, sizeof(*r));
	if (!r)
		return NULL;

	r->buf = buf;
	r->len = len;

	struct tlv_elem *e = calloc(1, sizeof(*e));
	if (!e) {
		free(r);
		return NULL;
	}

	e->info.tag = tag;
	e->info.ptr = buf;
	e->info.len = len;

	e->next = r->e;
	r->e = e;

	return r;
}

struct tlv *tlv_new_copy(uint16_t tag, const unsigned char *buf, size_t len)
{
	if (!buf)
		return NULL;

	unsigned char *buf2 = malloc(len);
	memcpy(buf2, buf, len);

	struct tlv *r = tlv_new(tag, buf2, len);

	if (!r)
		free(buf2);

	return r;
}


bool tlv_remove(struct tlv *tlv, uint16_t tag)
{
	if (!tlv)
		return false;

	struct tlv_elem *e, **p;
	for (p = &tlv->e; *p; p = &((*p)->next)) {
		if ((*p)->info.tag == tag) {
			e = *p;
			*p = e->next;
			free(e);
			return true;
		}
	}

	return false;
}
