#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>

gint key_compare_fn(gconstpointer key1, gconstpointer key2)
{
	gint k1 = (gint)key1;
	gint k2 = (gint)key2;

	if (k1 == k2)
		return 0;
	else if (k1 > k2)
		return 1;
	else if (k1 < k2)
		return -1;
}

gboolean traverse_fn(gpointer key, gpointer val, gpointer data)
{
	printf("key, val = %d %d\n", (gint)key, (gint)val);

	return 0;
}

int main(int argc, char *argv[])
{
	GTree *tree = NULL;
	GTimer *timer = g_timer_new();
	gulong ms;

	tree = g_tree_new(key_compare_fn);

	g_timer_start(timer);

	g_tree_insert(tree, (gpointer)1, (gpointer)1);
	g_tree_insert(tree, (gpointer)3, (gpointer)3);
	g_tree_insert(tree, (gpointer)4, (gpointer)4);
	g_tree_insert(tree, (gpointer)2, (gpointer)2);

	printf("nnodes: %d\n", g_tree_nnodes(tree));
	g_tree_foreach(tree, traverse_fn, NULL);

	gint v = (gint)g_tree_lookup(tree, (gconstpointer)4);
	printf("lookup 4, result = %d\n", v);

	g_timer_stop(timer);
	g_timer_elapsed(timer, &ms);
	printf("elapsed time:%ld ms\n", ms);

	g_timer_destroy(timer);
	g_tree_destroy(tree);
}

