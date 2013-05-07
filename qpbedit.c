/*   qpbedit.c
 *      
 *   Copyright 2013 Bartosz Jankowski
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


static void usage(char **argv)
{
	printf("Usage: %s [options] file\n"
		"\t-m,\t--model [text]\t\tchange model name\n"
		"\t-p,\t--producer [text]\tchange producer name\n"
		"\t-u,\t--update-timestamp\tupdate timestamp to current one\n"
		"\t-h,\t--help\t\t\tdisplay this help and exit\n"
		, argv[0]
	);
}

int main(int argc, char *argv[])
{
	char * model = 0, *prod = 0;
	int c, t = 0, i_opt = 0;
	FILE * f = 0;
	const struct option s_opt[] = 
	{
		{"model",			required_argument,	0,	'm'	},
		{"producer",		required_argument,	0,	'p'	},
		{"update-timestamp",no_argument,		0,	'u'	},
		{"help",			no_argument,		0,	'h'	},
		{0,					0,					0,	0	}
	};
	
	while((c = getopt_long(argc, argv, "m:p:uh", s_opt, &i_opt)) != -1) 
	{
		switch(c) 
		{
		case 'm':
			model = optarg;
			break;
		case 'p':
			prod = optarg;
			break;
		case 'u':
			t = time(NULL);
			break;
		case 'h':
			usage(argv);
			break;
		default:
			printf("Unknown parameter. Type %s --help to show usage\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
		 
	if (optind < argc )
	{
		f = fopen(argv[optind], "r+");
		if(!f)
		{
			printf("File %s doesn't exists!", argv[optind]);
			exit(EXIT_FAILURE);
		}
		printf ("Input file: %s\n", argv[optind]);
	}
	else
	{
		printf("Specify input file.\n");
		exit(EXIT_FAILURE);
	}
	printf("=====================================\n"
		   "Quick Build.Prop Edit v. 0.1 by lolet\n"
		   "=====================================\n");
	if(model) printf("Model : %s\n", model);
	if(prod) printf("Producer : %s\n", prod);
	if(t) printf("Timestamp : %d\n", t);
	
	while(!feof(f))
	{
		char * l = 0;
		size_t len = 0;
		size_t s = getline(&l, &len, f);
		printf("%s",l);
		free(l);
		if(s == -1) break;
	}
	fclose(f);
	return EXIT_SUCCESS;
}

