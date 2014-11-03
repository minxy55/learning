
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>

struct chinfo_t
{
	guint64 key;

	guint16 degree;
	guint16 freq;
	guint16 sid;

	guint8 cw[16];
};

struct fdinfo_t 
{
	gint fd;
	struct chinfo_t *chi;
};

static inline void fill_key(struct chinfo_t *chi)
{
	chi->key = chi->degree; 
	chi->key <<= 32;
	chi->key += (chi->freq << 16) + chi->sid;
}

static void display_ch_data(gpointer key, gpointer val, gpointer ctx)
{
	struct chinfo_t *chi = (struct chinfo_t *)val;	

	printf("ch key & val : %llx %llx %02x %02x\n", *(guint64*)key, chi->key, chi->cw[0], chi->cw[8]);
}


static void display_fd_data(gpointer key, gpointer val, gpointer ctx)
{
	struct fdinfo_t *fdi = (struct fdinfo_t *)val;
	struct chinfo_t *chi = (struct chinfo_t *)fdi->chi;

	printf("fd key & val : %x %x %llx, %02x %02x\n", *(guint*)key, (guint)fdi->fd, chi->key, chi->cw[0], chi->cw[8]);
}

int main()
{
	int i;
	GHashTable *hash_ch;
	GHashTable *hash_fd;
	hash_ch = g_hash_table_new(g_int64_hash, g_int64_equal);
	printf("create a hash table for ch %p. %m \n", hash_ch);
	printf("ch hash size:%d\n", g_hash_table_size(hash_ch));
	hash_fd = g_hash_table_new(g_int_hash, g_int_equal);
	printf("create a hash table for fd %p. %m \n", hash_fd);
	printf("fd hash size:%d\n", g_hash_table_size(hash_fd));

	printf("Insert (key & value)s in ch hash\n");
	struct chinfo_t ch1 = {0, 133, 27500, 100, "\x11\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"};
	struct chinfo_t ch2 = {0, 133, 27500, 101, "\x22\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"};
	struct chinfo_t ch3 = {0, 133, 27500, 102, "\x33\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"};
	struct chinfo_t ch4 = {0, 133, 27500, 303, "\x44\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"};
	struct chinfo_t ch5 = {0, 135, 27500, 100, "\x55\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"};
	fill_key(&ch1);
	fill_key(&ch2);
	fill_key(&ch3);
	fill_key(&ch4);
	fill_key(&ch5);
	g_hash_table_insert(hash_ch, (gpointer)&ch1.key, (gpointer)&ch1);
	g_hash_table_insert(hash_ch, (gpointer)&ch2.key, (gpointer)&ch2);
	g_hash_table_insert(hash_ch, (gpointer)&ch3.key, (gpointer)&ch3);
	g_hash_table_insert(hash_ch, (gpointer)&ch4.key, (gpointer)&ch4);
	g_hash_table_insert(hash_ch, (gpointer)&ch5.key, (gpointer)&ch5);

	printf("ch hash size = %d \n", g_hash_table_size(hash_ch));
	printf("foreach:\n");
	g_hash_table_foreach(hash_ch, display_ch_data, NULL);

	printf("add in ch hash\n");
	struct chinfo_t ch6 = {0, 133, 27500, 102, "\x66\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff"};
	fill_key(&ch6);
	g_hash_table_insert(hash_ch, (gpointer)&ch6.key, (gpointer)&ch6);

	printf("ch hash size = %d \n", g_hash_table_size(hash_ch));
	printf("ch hash foreach:\n");
	g_hash_table_foreach(hash_ch, display_ch_data, NULL);

	struct chinfo_t ch7 = ch2;
	struct chinfo_t *chi = (struct chinfo_t *)g_hash_table_lookup(hash_ch, (gpointer)&ch7.key);
	printf("lookup in ch hast %llx\n", ch7.key);
	printf("%llx => %llx %02x %02x \n", ch7.key, chi->key, chi->cw[0], chi->cw[8]);

	
	printf("Insert (key & value)s in fd hash\n");
	struct fdinfo_t fd1 = {3, (gpointer)&ch1};
	struct fdinfo_t fd2 = {4, (gpointer)&ch2};
	struct fdinfo_t fd3 = {5, (gpointer)&ch3};
	struct fdinfo_t fd4 = {6, (gpointer)&ch4};
	struct fdinfo_t fd5 = {7, (gpointer)&ch5};
	struct fdinfo_t fd6 = {8, (gpointer)&ch6};
	g_hash_table_insert(hash_fd, (gpointer)&fd1.fd, (gpointer)&fd1);
	g_hash_table_insert(hash_fd, (gpointer)&fd2.fd, (gpointer)&fd2);
	g_hash_table_insert(hash_fd, (gpointer)&fd3.fd, (gpointer)&fd3);
	g_hash_table_insert(hash_fd, (gpointer)&fd4.fd, (gpointer)&fd4);
	g_hash_table_insert(hash_fd, (gpointer)&fd5.fd, (gpointer)&fd5);
	g_hash_table_insert(hash_fd, (gpointer)&fd6.fd, (gpointer)&fd6);

	printf("fd hash size = %d \n", g_hash_table_size(hash_fd));
	g_hash_table_foreach(hash_fd, display_fd_data, NULL);

	struct fdinfo_t fd7 = fd2;
	struct fdinfo_t *fdi = (struct fdinfo_t *)g_hash_table_lookup(hash_fd, (gpointer)&fd7.fd);
	struct chinfo_t *chi2 = (struct chinfo_t *)g_hash_table_lookup(hash_ch, (gpointer)&(fdi->chi->key));
	printf("chi2->key = %llx\n", chi2->key);

	if (g_hash_table_remove(hash_fd, (gpointer)&fdi->fd)) {
		printf("removed with fd %x\n", fdi->fd);
	}
	if (g_hash_table_remove(hash_ch, (gpointer)&chi2->key)) {
		printf("removed with key %llx\n", chi2->key);
	}

	printf("ch hash foreach\n");
	g_hash_table_foreach(hash_ch, display_ch_data, NULL);

	printf("fd hash foreach\n");
	g_hash_table_foreach(hash_fd, display_fd_data, NULL);

	return 0;
}

