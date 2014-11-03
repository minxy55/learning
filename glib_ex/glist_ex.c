#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>

void func_each(gpointer data, gpointer user_data)
{
	printf("data: %s\n", (char *)data);
}

int main(int argc, char *argv[])
{
	GList *list = NULL;
	GTimer *timer = g_timer_new();
	gulong ms;

	list = g_list_append(list, "first");
	list = g_list_append(list, "second");
	list = g_list_append(list, "three");

	g_timer_start(timer);

	g_list_foreach(list, func_each, NULL);

	g_timer_stop(timer);
	g_timer_elapsed(timer, &ms);
	printf("elapsed time:%ld ms\n", ms);

	list = g_list_remove(list, "second");

	g_list_foreach(list, func_each, NULL);

	g_timer_destroy(timer);
	g_list_free(list);
}

