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
#include <string.h>
#include <getopt.h>
#include <time.h>

char *strstrip(char *s)
{
    size_t size;
    char *end;

    size = strlen(s);

    if (!size)
    	return s;

    end = s + size - 1;
    while (end >= s && isspace(*end))
    	end--;
    *(end + 1) = '\0';

    while (*s && isspace(*s))
    	s++;

    return s;
}

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
	int c, i_opt = 0;
	time_t t = {0};
	FILE * f;
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
			time(&t);
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
		f = fopen(argv[optind], "r");
		if(!f)
		{
			printf("File %s doesn't exists!", argv[optind]);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
	
		f = fopen("build.prop", "r");
		if(!f)
		{
			printf("Didn't find build.prop file!");
			exit(EXIT_FAILURE);
		}
//		printf("Specify input file.\n");
//		exit(EXIT_FAILURE);
	}
	printf("===========================================\n"
		   "/Q/uick /B/uild./P/rop Edit v. 0.2 by lolet\n"
		   "===========================================\n");
	printf ("Input file: %s\n", argv[optind] ? argv[optind] : "build.prop");
	if(model) printf("Model : %s\n", model);
	if(prod) printf("Producer : %s\n", prod);
	if(t) printf("Timestamp : %d\n", (int)t);
	
	FILE * ft = fopen("build.prop.tmp","w");
	if(!ft)
	{
		printf("Cannot create temporary file!");
		fclose(ft);
		exit(EXIT_FAILURE);
	}
	
	while(!feof(f))
	{
		char * l = 0;
		size_t len = 0;
		size_t s = getline(&l, &len, f);
		char * skip = strtok(l, "#");
		if(skip && skip == l && skip[0] != '\n')
		{
			char *key, *val = 0;
			key = strtok(l,"=");
			if(key)
			{
				val = strtok(NULL,"");
				strstrip(key);
				if(val)
					strstrip(val);
				if((!strcmp(key, "ro.product.model") || !strcmp(key, "ro.product.name") ||
				   !strcmp(key, "ro.product.device")) && model)
				{
					fprintf(ft, "%s=%s\n", key, model);
				}
				else if((!strcmp(key, "ro.product.brand") || !strcmp(key, "ro.product.manufacturer")) && prod)
				{
					fprintf(ft, "%s=%s\n", key, prod);
				}
			//	else if(!strcmp(key,"ro.build.fingerprint")
		//		{
					
			//	}
				else if(!strcmp(key,"ro.build.date.utc") && t)
				{
					fprintf(ft, "%s=%d\n", key,  (int)t);
				}
				else if(!strcmp(key,"ro.build.date") && t)
				{
					char d[80];
					strftime(d,80,"%Y %m %d %T %Z",localtime(&t));
					fprintf(ft,"%s=%s\n", key, d);
				} 
				else if(!strcmp(key,"ro.build.version.incremental") && t)
				{
					char d[80];
					strftime(d,80,"%Y%m%d.%H%M%S",localtime(&t));
					fprintf(ft,"%s=%s\n", key, d);
				}
				else
					fprintf(ft, "%s=%s\n", key, val);
			}
		}
		else fputs(l, ft);
		free(l);
		if(s == -1) break;
	}
	fclose(ft);
	fclose(f);
	
//	rename("build.prop.tmp","build.prop");
	return EXIT_SUCCESS;
}

