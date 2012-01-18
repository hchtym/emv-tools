#include <stdio.h>
#define GCRYPT_NO_DEPRECATED
#define GCRYPT_NO_MPI_MACROS
#include <gcrypt.h>

#define HASH_SHA_1	1
#define PK_RSA		1

struct capk {
	unsigned char rid[5];
	unsigned char index;
	unsigned char hash_algo;
	unsigned char pk_algo;
	unsigned char hash[20];
	unsigned char exp[3];
	size_t elen;
	size_t mlen;
	unsigned char modulus[];
};

struct capk vsdc_01 = {
	.rid = { 0xa0, 0x00, 0x00, 0x00, 0x03, },
	.index = 1,
	.hash_algo = HASH_SHA_1,
	.pk_algo = PK_RSA,
	.hash = {
		0xd3, 0x4a, 0x6a, 0x77,
		0x60, 0x11, 0xc7, 0xe7,
		0xce, 0x3a, 0xec, 0x5f,
		0x03, 0xad, 0x2f, 0x8c,
		0xfc, 0x55, 0x03, 0xcc, },
	.exp = { 0x03, },
	.elen = 1,
	.mlen = 1024 / 8,
	.modulus = {
		0xc6, 0x96, 0x03, 0x42, 0x13, 0xd7, 0xd8, 0x54, 0x69, 0x84, 0x57, 0x9d, 0x1d, 0x0f, 0x0e, 0xa5,
		0x19, 0xcf, 0xf8, 0xde, 0xff, 0xc4, 0x29, 0x35, 0x4c, 0xf3, 0xa8, 0x71, 0xa6, 0xf7, 0x18, 0x3f,
		0x12, 0x28, 0xda, 0x5c, 0x74, 0x70, 0xc0, 0x55, 0x38, 0x71, 0x00, 0xcb, 0x93, 0x5a, 0x71, 0x2c,
		0x4e, 0x28, 0x64, 0xdf, 0x5d, 0x64, 0xba, 0x93, 0xfe, 0x7e, 0x63, 0xe7, 0x1f, 0x25, 0xb1, 0xe5,
		0xf5, 0x29, 0x85, 0x75, 0xeb, 0xe1, 0xc6, 0x3a, 0xa6, 0x17, 0x70, 0x69, 0x17, 0x91, 0x1d, 0xc2,
		0xa7, 0x5a, 0xc2, 0x8b, 0x25, 0x1c, 0x7e, 0xf4, 0x0f, 0x23, 0x65, 0x91, 0x24, 0x90, 0xb9, 0x39,
		0xbc, 0xa2, 0x12, 0x4a, 0x30, 0xa2, 0x8f, 0x54, 0x40, 0x2c, 0x34, 0xae, 0xca, 0x33, 0x1a, 0xb6,
		0x7e, 0x1e, 0x79, 0xb2, 0x85, 0xdd, 0x57, 0x71, 0xb5, 0xd9, 0xff, 0x79, 0xea, 0x63, 0x0b, 0x75,
	},
};

const unsigned char cert[] = {
		0x3c, 0x5f, 0xea, 0xd4, 0xdd, 0x7b, 0xca, 0x44, 0xf9, 0x3e, 0x90, 0xc4, 0x4f, 0x76, 0xed, 0xe5,
	       	0x4a, 0x32, 0x88, 0xec, 0xdc, 0x78, 0x46, 0x9f, 0xcb, 0x12, 0x25, 0xc0, 0x3b, 0x2c, 0x04, 0xf2,
	       	0xc2, 0xf4, 0x12, 0x28, 0x1a, 0x08, 0x22, 0xdf, 0x14, 0x64, 0x92, 0x30, 0x98, 0x9f, 0xb1, 0x49,
	       	0x40, 0x70, 0xda, 0xf8, 0xc9, 0x53, 0x4a, 0x78, 0x81, 0x96, 0x01, 0x48, 0x61, 0x6a, 0xce, 0x58,
	       	0x17, 0x88, 0x12, 0x0d, 0x35, 0x06, 0xac, 0xe4, 0xce, 0xe5, 0x64, 0xfb, 0x27, 0xee, 0x53, 0x34,
	       	0x1c, 0x22, 0xf0, 0xb4, 0x5b, 0x31, 0x87, 0x3d, 0x05, 0xde, 0x54, 0x5e, 0xfe, 0x33, 0xbc, 0xd2,
	       	0x9b, 0x21, 0x85, 0xd0, 0x35, 0xa8, 0x06, 0xad, 0x08, 0xc6, 0x97, 0x6f, 0x35, 0x05, 0xa1, 0x99,
	       	0x99, 0x93, 0x0c, 0xa8, 0xa0, 0x3e, 0xfa, 0x32, 0x1c, 0x48, 0x60, 0x61, 0xf7, 0xdc, 0xec, 0x9f,
};

int main(void) {
	/* Version check should be the very first call because it
	 * makes sure that important subsystems are intialized. */
	if (!gcry_check_version (GCRYPT_VERSION)) {
		fputs ("libgcrypt version mismatch\n", stderr);
		exit (2);
	}

	/* We don't want to see any warnings, e.g. because we have not yet
	 * parsed program options which might be used to suppress such
	 * warnings. */
	gcry_control (GCRYCTL_SUSPEND_SECMEM_WARN);

	/* ... If required, other initialization goes here.  Note that the
	 * process might still be running with increased privileges and that
	 * the secure memory has not been intialized.  */

	/* Allocate a pool of 16k secure memory.  This make the secure memory
	 * available and also drops privileges where needed.  */
	gcry_control (GCRYCTL_INIT_SECMEM, 16384, 0);

	/* It is now okay to let Libgcrypt complain when there was/is
	 * a problem with the secure memory. */
	gcry_control (GCRYCTL_RESUME_SECMEM_WARN);

	/* ... If required, other initialization goes here.  */

	/* Tell Libgcrypt that initialization has completed. */
	gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
	    gcry_control (GCRYCTL_SET_DEBUG_FLAGS, 1u , 0);

	const struct capk *pk = &vsdc_01;

	gcry_error_t err;
#if 0
	gcry_md_hd_t mdh;
	err = gcry_md_open(&mdh, GCRY_MD_SHA1, 0);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		exit(1);
	}

	gcry_md_write(mdh, pk->rid, sizeof(pk->rid));
	gcry_md_putc(mdh, pk->index);
	gcry_md_write(mdh, pk->modulus, pk->mlen);
	gcry_md_write(mdh, pk->exp, pk->elen);

	gcry_md_final(mdh);

	unsigned char *h = gcry_md_read(mdh, GCRY_MD_SHA1);
	int i;

	for (i = 0; i < 20; i++) {
		printf("%s%02hhx", i ? ":" : "", h[i]);
	}	
	printf("\n");
#endif

	gcry_sexp_t ksexp;
	gcry_sexp_t csexp;
	gcry_sexp_t usexp;

	err = gcry_sexp_build(&ksexp, NULL, "(public-key (rsa (n %b) (e %b)))",
			pk->mlen, pk->modulus, pk->elen, pk->exp);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		exit(1);
	}

	err = gcry_sexp_build(&csexp, NULL, "(data (flags raw) (value %b))",
			sizeof(cert), cert);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		exit(1);
	}

	err = gcry_pk_encrypt(&usexp, csexp, ksexp);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		exit(1);
	}

	char buf[4096] = {[0 ... 4095] = '.'};
	size_t l = gcry_sexp_sprint(usexp, GCRYSEXP_FMT_DEFAULT, buf, sizeof(buf));
	printf("%d '%s'\n", l, buf);


	return 0;
}