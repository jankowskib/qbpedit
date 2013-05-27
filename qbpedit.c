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
#if defined __linux__ || defined __linux 
#include <unistd.h>
#elif defined _WIN32 || defined _WIN64
char h[255] = "Windows";
#endif

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
	printf("Usage: %s options [file (default: build.prop)]\n"
		"\t-m,\t--model [text]\t\tchange model name\n"
		"\t-p,\t--producer [text]\tchange producer name\n"
		"\t-l,\t--language [lang_LANG]\tchange default language\n"
		"\t-u,\t--update-timestamp\tupdate timestamp to current one\n"
		"\t-d,\t--update-dev [user]\tupdate build maker to user and host to current one\n"
		"\t-v,\t--version [text]\tchange displayed version\n"
		"\t-h,\t--help\t\t\tdisplay this help and exit\n"
		, argv[0]
	);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	char * model = 0, *prod = 0, *dev = 0, *ll = 0, *lh = 0, *v = 0, *id = 0, *tags = 0;
	char ltmp[3] = {0};
	int c, i_opt = 0;
	time_t t = {0};
	FILE * f;
	char *fname = 0;
	const struct option s_opt[] = 
	{
		{"model",			required_argument,	0,	'm'	},
		{"producer",		required_argument,	0,	'p'	},
		{"language",		required_argument,	0,	'l' },
		{"update-timestamp",no_argument,		0,	'u'	},
		{"update-dev",		required_argument,	0,	'd' },
		{"version",			required_argument,	0,	'v'	},
		{"help",			no_argument,		0,	'h'	},
		{0,					0,					0,	0	}
	};
	#if defined __linux__ || defined __linux 
	char h[255] = {0};
	gethostname(h, 255);
	#endif
	
	while((c = getopt_long(argc, argv, "m:p:uhd:l:v:", s_opt, &i_opt)) != -1) 
	{
		switch(c) 
		{
		case 'm':
			model = optarg;
			break;
		case 'p':
			prod = optarg;
			break;
		case 'v':
			v = optarg;
			break;
		case 'l':
			if(strlen(optarg)!=5 || optarg[2] != '_') 
			{
			printf("Usage -l lang_LANG\n", argv[0]);
			exit(EXIT_FAILURE);	
			}
			strncpy(ltmp, optarg,2);
			ll = &ltmp[0];
			lh = &optarg[3];
			break;
		case 'u':
			time(&t);
			break;
		case 'd':
			dev = optarg;
			break;
		case 'h':
			usage(argv);
			break;
		default:
			printf("Unknown parameter. Type %s --help to show usage\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	
	if(v && !model) 
	{
		printf("--version requires to specify --model parameter\n");
		exit(EXIT_FAILURE);
	}
	
	if (optind < argc )
	{
		f = fopen(argv[optind], "r");
		if(!f)
		{
			printf("File %s doesn't exists!", argv[optind]);
			exit(EXIT_FAILURE);
		}
		fname = argv[optind];
	}
	else
	{
		fname = "build.prop";
		f = fopen(fname, "r");
		if(!f)
		{
			printf("Didn't find build.prop file in working directory!");
			exit(EXIT_FAILURE);
		}
		
//		printf("Specify input file.\n");
//		exit(EXIT_FAILURE);
	}
	printf("===========================================\n"
		   "/Q/uick /B/uild./P/rop Edit v. 0.2 by lolet\n"
		   "===========================================\n");
	printf ("Input file: %s\n", argv[optind] ? argv[optind] : "build.prop");
	if(model) printf("Model\t: %s\n", model);
	if(prod) printf("Producer\t: %s\n", prod);
	if(t) printf("Timestamp\t: %d\n", (int)t);
	if(v) printf("Version\t: %s\n", v);
	printf("===========================================\n");
	
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
				   !strcmp(key, "ro.product.device") || !strcmp(key, "ro.build.product")) && model)
				{
					fprintf(ft, "%s=%s\n", key, model);
				    printf("Changed: %s: %s -> %s \n", key, val, model);
				}
				else if(!strcmp(key, "ro.build.id"))
				{
					id = strdup(val);
					fprintf(ft, "%s=%s\n", key, val);
				}
				else if(!strcmp(key, "ro.build.tags"))
				{
					tags =strdup(val);
					fprintf(ft, "%s=%s\n", key, val);
				}
				else if((!strcmp(key, "ro.product.brand") || !strcmp(key, "ro.product.manufacturer") || 
						 !strcmp(key, "ro.product.usbfactory") || !strcmp(key, "ro.udisk.label")) && prod)
				{
					fprintf(ft, "%s=%s\n", key, prod);
					printf("Changed: %s: %s -> %s \n", key, val, prod);
				}
				else if(!strcmp(key,"ro.build.user") && dev)
				{
					fprintf(ft, "%s=%s\n", key, dev);
					printf("Changed: %s: %s -> %s \n", key, val, dev);
				}
				else if(!strcmp(key,"ro.build.host") && dev)
				{	
					fprintf(ft, "%s=%s\n", key, h);
					printf("Changed: %s: %s -> %s \n", key, val, h);
				}
				else if((!strcmp(key,"ro.product.locale.language") || !strcmp(key, "persist.sys.language")) && ll)
				{
					fprintf(ft, "%s=%s\n", key, ll);
					printf("Changed: %s: %s -> %s \n", key, val, ll);
				}
				else if((!strcmp(key,"ro.product.locale.region") || !strcmp(key, "persist.sys.country")) && lh)
				{
					fprintf(ft, "%s=%s\n", key, lh);
					printf("Changed: %s: %s -> %s \n", key, val, lh);					
				}
				// else if(!strcmp(key,"ro.build.fingerprint" && (model && v && prod &&))
				// {
					
				// }
				else if(!strcmp(key,"ro.build.display.id") && (model && v))
				{
					char d[255] = {0};
					sprintf(d, "%s-%s",model,v);
					fprintf(ft,"%s=%s\n", key, d);
					printf("Changed: %s: %s -> %s \n", key, val, d);
				}
				else if(!strcmp(key,"ro.build.description") && (model && v))
				{
					char d[255] = {0};		
					char dd[80] = {0};
					strftime(dd,80,"%Y%m%d.%H%M%S",localtime(&t));
					sprintf(d, "%s-%s %s %s %s",model, v, id, dd, tags);
					fprintf(ft,"%s=%s\n", key, d);
					printf("Changed: %s: %s -> %s \n", key, val, d);
				}
				else if(!strcmp(key,"ro.build.date.utc") && t)
				{
					fprintf(ft, "%s=%d\n", key,  (int)t);
					printf("Changed: %s: %s -> %d \n", key, val, (int)t);
				}
				else if(!strcmp(key,"ro.build.date") && t)
				{
					char d[80] = {0};
					strftime(d,80,"%Y %m %d %T %Z",localtime(&t));
					fprintf(ft,"%s=%s\n", key, d);
					printf("Changed: %s: %s -> %s \n", key, val, d);
				} 
				else if(!strcmp(key,"ro.build.version.incremental") && t)
				{
					char d[80] = {0};
					strftime(d,80,"%Y%m%d.%H%M%S",localtime(&t));
					fprintf(ft,"%s=%s\n", key, d);
					printf("Changed: %s: %s -> %s \n", key, val, d);
				}
				else
					fprintf(ft, "%s=%s\n", key, val);
			}
		}
		else fputs(l, ft);
		free(l);
		if(s == -1) break;
	}
	free(tags);
	free(id);
	fclose(ft);
	fclose(f);
	
	rename("build.prop.tmp", fname);
	return EXIT_SUCCESS;
}

