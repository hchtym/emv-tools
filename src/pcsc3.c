#include <stdio.h>
#include <stdlib.h>

#include "scard.h"
#include "sc_helpers.h"
#include "tlv.h"
#include "tlvs.h"

#ifdef WIN32
static char *pcsc_stringify_error(LONG rv)
{

	static char out[20];
	sprintf_s(out, sizeof(out), "0x%08X", rv);

	return out;
}
#endif

#define CHECK(f, rv) \
	if (SCARD_S_SUCCESS != rv) \
	{ \
		printf(f ": %s\n", pcsc_stringify_error(rv)); \
		return -1; \
	}

static void dump(const unsigned char *ptr, size_t len)
{
	int i, j;

	for (i = 0; i < len; i += 16) {
		printf("\t%02x:", i);
		for (j = 0; j < 16; j++) {
			if (i + j < len)
				printf(" %02hhx", ptr[i + j]);
			else
				printf("   ");
		}
		printf(" |");
		for (j = 0; j < 16 && i + j < len; j++) {
			printf("%c", (ptr[i+j] >= 0x20 && ptr[i+j] < 0x7f) ? ptr[i+j] : '.' );
		}
		printf("\n");
	}
}

static bool print_cb(void *data, const struct tlv_elem_info *tei)
{
	if (!tei) {
		printf("NULL\n");
		return false;
	}

	if (tei->tag < 0x100)
		printf("Got tag %02hx len %02x:\n", tei->tag, tei->len);
	else
		printf("Got tag %04hx len %02x:\n", tei->tag, tei->len);

	dump(tei->ptr, tei->len);

	return true;
}

static struct tlv *docmd(struct sc *sc,
		unsigned char cla,
		unsigned char ins,
		unsigned char p1,
		unsigned char p2,
		size_t dlen,
		const unsigned char *data)
{
	unsigned short sw;
	size_t outlen;
	unsigned char *outbuf;
	struct tlv *tlv = NULL;

	printf("CMD: %02hhx %02hhx %02hhx %02hhx (%02zx)\n", cla, ins, p1, p2, dlen);
	outbuf = sc_command(sc, cla, ins, p1, p2,
			dlen, data, &sw, &outlen);
	if (scard_is_error(sc)) {
		printf(scard_error(sc));
		return NULL;
	}
	printf("response (%hx):\n", sw);
#if 0
	int i;
	for(i=0; i<outlen; i++)
		printf("%02X ", outbuf[i]);
	printf("\n");
#endif
	if (sw == 0x9000) {
		tlv = tlv_parse(outbuf, outlen);
	}

	if (!tlv)
		free(outbuf);

	printf("\n");

	return tlv;
}

int main(void)
{
	struct sc *sc;
#if 0
	unsigned char cmd1[] = {
		0x31, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31,
	};
#endif
	unsigned char cmd4[] = {
		0xa0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10,
	};
	unsigned char cmd5[] = {
		0x83, 0x00,
	};

	sc = scard_init();
	if (scard_is_error(sc)) {
		printf(scard_error(sc));
		return 1;
	}

	scard_connect(sc);
	if (scard_is_error(sc)) {
		printf(scard_error(sc));
		return 1;
	}

#if 0
	tlv_free(docmd(sc, 0x00, 0xa4, 0x04, 0x00, sizeof(cmd1), cmd1));
	tlv_free(docmd(sc, 0x00, 0xb2, 0x01, (0x01 << 3) | 0x04, 0, NULL));
	tlv_free(docmd(sc, 0x00, 0xb2, 0x02, (0x01 << 3) | 0x04, 0, NULL));
#endif

	struct tlvs *s = tlvs_new();
	struct tlv *t;
	const struct tlv_elem_info *e;
	t = docmd(sc, 0x00, 0xa4, 0x04, 0x00, sizeof(cmd4), cmd4);
	tlvs_add(s, t);
	t = docmd(sc, 0x80, 0xa8, 0x00, 0x00, sizeof(cmd5), cmd5);
	if ((e = tlv_get(t, 0x80)) != NULL) {
		struct tlv *t1, *t2;
		t1 = tlv_new_copy(0x82, e->ptr, 2);
		t2 = tlv_new_copy(0x94, e->ptr+2, e->len - 2);
		tlvs_add(s, t1);
		tlvs_add(s, t2);
		tlv_free(t);
	} else {
		tlvs_add(s, t);
	}

	e = tlvs_get(s, 0x94);
	int i;
	for (i = 0; i < e->len; i += 4) {
		unsigned char p2 = e->ptr[i + 0];
		unsigned char first = e->ptr[i + 1];
		unsigned char last = e->ptr[i + 2];
//		unsigned char sdarec = e->ptr[i + 3];

		if (p2 == 0 || p2 == (31 << 3) || first == 0 || first > last)
			break; /* error */

		for (; first <= last; first ++) {
			t = docmd(sc, 0x00, 0xb2, first, p2 | 0x04, 0, NULL);
			tlv_remove(t, 0x70);
			tlvs_add(s, t);
		}

	}

	tlvs_visit(s, print_cb, NULL);
	tlvs_free(s);

	scard_disconnect(sc);
	if (scard_is_error(sc)) {
		printf(scard_error(sc));
		return 1;
	}
	scard_shutdown(&sc);
	if (scard_is_error(sc)) {
		printf(scard_error(sc));
		return 1;
	}

	return 0;
}
