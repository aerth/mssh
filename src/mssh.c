#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <gtk/gtk.h>

#include "config.h"
#include "mssh-window.h"

#define PKGINFO		PACKAGE_NAME " " VERSION
#define COPYRIGHT	"Copyright (C) 2009 Bradley Smith <brad@brad-smith.co.uk>"

static void on_mssh_destroy(GtkWidget *widget, gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
}

void usage(const char *argv0)
{
	fprintf(stderr, "%s\n", PKGINFO);
	fprintf(stderr, "%s\n", COPYRIGHT);
	fprintf(stderr, "An ssh client to issue the same commands to multiple servers\n\n");
	fprintf(stderr, "Usage: %s [OPTION]... [HOSTS]\n\n", argv0);
	fprintf(stderr,
		"  -h, --help       Display this help and exit\n");
	fprintf(stderr,
		"  -V, --version    Output version information and exit\n");
	fprintf(stderr, "\nReport bugs to <%s>.\n", PACKAGE_BUGREPORT);
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[], char* env[])
{
	GtkWidget* window;
	int c, option_index = 0;
	int i, nhosts = 0;
	char **hosts = NULL;

	static struct option long_options[] =
	{
		{"help",	no_argument,	0, 'h'},
		{"version",	no_argument,	0, 'V'},
		{0, 0, 0, 0}
	};

	for(;;)
	{
		c = getopt_long(argc, argv, "hV", long_options, &option_index);

		if(c == -1)
			break;

		switch(c)
		{
		case 'h':
			usage(argv[0]);
			break;
		case 'V':
			printf("%s\n\n", PKGINFO);
			printf("%s\n\n", COPYRIGHT);
			printf("Redistribution and use in source and binary forms, with or without\n");
			printf("modification, are permitted provided that the following conditions are met:\n");
			printf("\n");
			printf("    1. Redistributions of source code must retain the copyright notice,\n");
			printf("       this list of conditions and the following disclaimer.\n");
			printf("    2. Redistributions in binary form must reproduce the copyright notice,\n");
			printf("       this list of conditions and the following disclaimer in the\n");
			printf("       documentation and/or other materials provided with the distribution.\n");
			printf("    3. The name of the author may not be used to endorse or promote\n");
			printf("       products derived from this software without specific prior written\n");
			printf("       permission.\n");
			printf("\n");
			printf("THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n");
			printf("IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES\n");
			printf("OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN\n");
			printf("NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n");
			printf("SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED\n");
			printf("TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n");
			printf("PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n");
			printf("LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n");
			printf("NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n");
			printf("SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n");

			exit(EXIT_SUCCESS);
			break;
		case '?':
			printf("\n");
			usage(argv[0]);
			exit(EXIT_FAILURE);
			break;
		default:
			abort();
		}
	}

	if (optind < argc)
	{
		hosts = malloc(sizeof(char*) * (argc - optind));
		while (optind < argc)
		{
			hosts[nhosts++] = strdup(argv[optind++]);
		}
	}
	else
	{
		fprintf(stderr, "No hosts specified\n\n");
		usage(argv[0]);
	}

	gtk_init(&argc, &argv);

	window = GTK_WIDGET(mssh_window_new());

	g_signal_connect(G_OBJECT(window), "destroy",
		G_CALLBACK(on_mssh_destroy), NULL);

	mssh_window_start_session(MSSH_WINDOW(window), env, nhosts, hosts);

	gtk_widget_show_all(window);
	gtk_main();

	for(i = 0; i < nhosts; i++)
		free(hosts[i]);

	free(hosts);

	return 0;
}
