/*
* Copyright (c) 2011-2013 by naehrwert
* This file is released under the GPLv2.
*/

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include "getopt.h"
#else
#include <unistd.h>
#include <getopt.h>
#endif

#include "types.h"
#include "config.h"
#include "aes.h"
#include "util.h"
#include "keys.h"
#include "sce.h"
#include "np.h"
#include "self.h"
#include "rvk.h"

#include "frontend.h"

/*! Shorter Versions of arg options. */
#define ARG_NULL no_argument
#define ARG_NONE no_argument
#define ARG_REQ required_argument
#define ARG_OPT optional_argument

/*! Verbose mode. */
BOOL _verbose = FALSE;
/*! Raw mode. */
BOOL _raw = FALSE;

/*! We got work. */
static BOOL _got_work = FALSE;
/*! List keys. */
static BOOL _list_keys = FALSE;
/*! Print infos on file. */
static BOOL _print_info = FALSE;
/*! Decrypt file. */
static BOOL _decrypt_file = FALSE;
/*! Encrypt file. */
static BOOL _encrypt_file = FALSE;

/*! Parameters. */
s8 *_template = NULL;
s8 *_file_type = NULL;
s8 *_compress_data = NULL;
s8 *_skip_sections = NULL;
s8 *_key_rev = NULL;
s8 *_meta_info = NULL;
s8 *_keyset = NULL;
s8 *_auth_id = NULL;
s8 *_vendor_id = NULL;
s8 *_self_type = NULL;
s8 *_app_version = NULL;
s8 *_fw_version = NULL;
s8 *_add_shdrs = NULL;
s8 *_ctrl_flags = NULL;
s8 *_cap_flags = NULL;
#ifdef CONFIG_CUSTOM_INDIV_SEED
s8 *_indiv_seed = NULL;
#endif
s8 *_license_type = NULL;
s8 *_app_type = NULL;
s8 *_content_id = NULL;
s8 *_klicensee = NULL;
s8 *_real_fname = NULL;
s8 *_add_sig = NULL;

/*! Input file. */
static s8 *_file_in = NULL;
/*! Ouput file. */
static s8 *_file_out = NULL;

/*! Long option values. */
#define VAL_TEMPLATE 't'
#define VAL_FILE_TYPE '0'
#define VAL_COMPRESS_DATA '1'
#define VAL_SKIP_SECTIONS 's'
#define VAL_KEY_REV '2'
#define VAL_META_INFO 'm'
#define VAL_KEYSET 'K'
#define VAL_AUTH_ID '3'
#define VAL_VENDOR_ID '4'
#define VAL_SELF_TYPE '5'
#define VAL_APP_VERSION 'A'
#define VAL_FW_VERSION '6'
#define VAL_ADD_SHDRS '7'
#define VAL_CTRL_FLAGS '8'
#define VAL_CAP_FLAGS '9'
#ifdef CONFIG_CUSTOM_INDIV_SEED
#define VAL_INDIV_SEED 'a'
#endif
#define VAL_LICENSE_TYPE 'b'
#define VAL_APP_TYPE 'c'
#define VAL_CONTENT_ID 'f'
#define VAL_KLICENSEE 'l'
#define VAL_REAL_FNAME 'g'
#define VAL_ADD_SIG 'j'

static struct option options[] = 
{
	{"help", ARG_NONE, NULL, 'h'},
	{"print-keys", ARG_NONE, NULL, 'k'},
	{"print-infos", ARG_REQ, NULL, 'i'},
	{"decrypt", ARG_REQ, NULL, 'd'},
	{"encrypt", ARG_REQ, NULL, 'e'},
	{"verbose", ARG_NONE, NULL, 'v'},
	{"raw", ARG_NONE, NULL, 'r'},
	{"template", ARG_REQ, NULL, VAL_TEMPLATE},
	{"sce-type", ARG_REQ, NULL, VAL_FILE_TYPE},
	{"compress-data", ARG_REQ, NULL, VAL_COMPRESS_DATA},
	{"skip-sections", ARG_REQ, NULL, VAL_SKIP_SECTIONS},
	{"key-revision", ARG_REQ, NULL, VAL_KEY_REV},
	{"meta-info", ARG_REQ, NULL, VAL_META_INFO},
	{"keyset", ARG_REQ, NULL, VAL_KEYSET},
	{"self-auth-id", ARG_REQ, NULL, VAL_AUTH_ID},
	{"self-vendor-id", ARG_REQ, NULL, VAL_VENDOR_ID},
	{"self-type", ARG_REQ, NULL, VAL_SELF_TYPE},
	{"self-app-version", ARG_REQ, NULL, VAL_APP_VERSION},
	{"self-fw-version", ARG_REQ, NULL, VAL_FW_VERSION},
	{"self-add-shdrs", ARG_REQ, NULL, VAL_ADD_SHDRS},
	{"self-ctrl-flags", ARG_REQ, NULL, VAL_CTRL_FLAGS},
	{"self-cap-flags", ARG_REQ, NULL, VAL_CAP_FLAGS},
#ifdef CONFIG_CUSTOM_INDIV_SEED
	{"self-indiv-seed", ARG_REQ, NULL, VAL_INDIV_SEED},
#endif
	{"np-license-type", ARG_REQ, NULL, VAL_LICENSE_TYPE},
	{"np-app-type", ARG_REQ, NULL, VAL_APP_TYPE},
	{"np-content-id", ARG_REQ, NULL, VAL_CONTENT_ID},
	{"np-klicensee", ARG_REQ, NULL, VAL_KLICENSEE},
	{"np-real-fname", ARG_REQ, NULL, VAL_REAL_FNAME},
	{"np-add-sig", ARG_REQ, NULL, VAL_ADD_SIG},
	{NULL, ARG_NULL, NULL, 0}
};

static void print_version()
{
	printf("\nscetool " SCETOOL_VERSION CONFIG_PRIVATE_BUILD " (C) 2011-2013 by naehrwert\n");
	printf("NP local license handling (C) 2012 by flatz\n");
	printf("[Build Date/Time: %s/%s]\n\n", __DATE__, __TIME__);
}

#include <time.h>

static void print_usage()
{
	print_version();

	printf("�ᯮ�짮�����: scetool [��樨] �������\n\n");
	printf("�������                 ��ࠬ����                         �ਬ�砭��\n\n");
	printf(" -h, --help                                               �뢥�� ��� ������.\n");
	printf(" -k, --print-keys                                         ���᮪ ���祩.\n");
	printf(" -i, --print-infos      File-in                           �뢥�� ���ଠ�� SCE 䠩��.\n");
	printf(" -d, --decrypt          File-in File-out                  �����஢��/���� SCE 䠩��.\n");
	printf(" -e, --encrypt          File-in File-out                  ���஢���/ᮧ���� SCE 䠩�.\n\n");
	printf("�����                   �������� ���祭��                �ਬ�砭��\n\n");
	printf(" -v, --verbose                                            ������� ���஡�� �뢮�.\n");
	printf(" -r, --raw                                                ������� �뢮� ��室��� ���祭��.\n");
	printf(" -t, --template         File-in                           ������ 䠩�� (SELF ⮫쪮)\n");
	printf(" -0, --sce-type         SELF/RVK/PKG/SPP                  SCE ⨯ 䠩��\n");
	printf(" -1, --compress-data    TRUE/FALSE(default)               ������ᨬ�, ᦠ�� ����� ��� ���.\n");
	printf(" -s, --skip-sections    TRUE(default)/FALSE               ������ᨬ�, �ய�饭� ࠧ���� ��� ���.\n");
	printf(" -2, --key-revision     e.g. 00,01,...,0A,...             ������� ����\n");
	printf(" -m, --meta-info        64 bytes                          �ᯮ�짮���� ��⠤���� ��� ����஢��.\n");
	printf(" -K, --keyset           32(Key)16(IV)40(Pub)21(Priv)1(CT) ������� ����� ���祩.\n");
	printf(" -3, --self-auth-id     e.g. 1010000001000003             ID ��⥭�䨪�樨\n");
	printf(" -4, --self-vendor-id   e.g. 01000002                     ID �த���\n");
	printf(" -5, --self-type        LV0/LV1/LV2/APP/ISO/LDR/NPDRM     SELF ⨯\n");
	printf(" -A, --self-app-version e.g. 0001000000000000             ����� �ਫ������\n");
	printf(" -6, --self-fw-version  e.g. 0003004100000000             ����� ��訢��\n");
	printf(" -7, --self-add-shdrs   TRUE(default)/FALSE               ������ᨬ�, �������� ELF 襩���� ��� ���.\n");
	printf(" -8, --self-ctrl-flags  32 bytes                          ������� 䫠�� ����஫�.\n");
	printf(" -9, --self-cap-flags   32 bytes                          ������� 䫠�� �㭪権.\n");
#ifdef CONFIG_CUSTOM_INDIV_SEED
	printf(" -a, --self-indiv-seed  256 bytes                         �������㠫�� ᨤ (⮫쪮 ISO)\n");
#endif
	printf(" -b, --np-license-type  LOCAL/FREE                        ��� ��業���\n");
	printf(" -c, --np-app-type      SPRX/EXEC/USPRX/UEXEC             ��� �ਫ������ (U* ��� ����������)\n");
	printf(" -f, --np-content-id                                      ID ���⥭�\n");
	printf(" -l, --np-klicensee     16 bytes                          ������� klicensee.\n");
	printf(" -g, --np-real-fname    e.g. EBOOT.BIN                    ����쭮� ��� 䠩��\n");
	printf(" -j, --np-add-sig       TRUE/FALSE(default)               ������ᨬ�, ��������� ������� NP ��� ���.\n");

	//getchar();

	exit(1);
}

static void parse_args(int argc, char **argv)
{
	char c;

#ifdef CONFIG_CUSTOM_INDIV_SEED
	while((c = getopt_long(argc, argv, "hki:d:e:vrt:0:1:s:2:m:K:3:4:5:A:6:7:8:9:a:b:c:f:l:g:j:", options, NULL)) != -1)
#else
	while((c = getopt_long(argc, argv, "hki:d:e:vrt:0:1:s:2:m:K:3:4:5:A:6:7:8:9:b:c:f:l:g:j:", options, NULL)) != -1)
#endif
	{
		switch(c)
		{
		case 'h':
			print_usage();
			break;
		case 'k':
			_got_work = TRUE;
			_list_keys = TRUE;
			//Got all args.
			return;
			break;
		case 'i':
			_got_work = TRUE;
			_print_info = TRUE;
			_file_in = optarg;
			//Got all args.
			return;
			break;
		case 'd':
			_got_work = TRUE;
			_decrypt_file = TRUE;
			_file_in = optarg;
			//Need more args.
			goto get_args;
			break;
		case 'e':
			_got_work = TRUE;
			_encrypt_file = TRUE;
			_file_in = optarg;
			//Need more args.
			goto get_args;
			break;
		case 'v':
			_verbose = TRUE;
			break;
		case 'r':
			_raw = TRUE;
			break;
		case VAL_TEMPLATE:
			_template = optarg;
			break;
		case VAL_FILE_TYPE:
			_file_type = optarg;
			break;
		case VAL_COMPRESS_DATA:
			_compress_data = optarg;
			break;
		case VAL_SKIP_SECTIONS:
			_skip_sections = optarg;
			break;
		case VAL_KEY_REV:
			_key_rev = optarg;
			break;
		case VAL_META_INFO:
			_meta_info = optarg;
			break;
		case VAL_KEYSET:
			_keyset = optarg;
			break;
		case VAL_AUTH_ID:
			_auth_id = optarg;
			break;
		case VAL_VENDOR_ID:
			_vendor_id = optarg;
			break;
		case VAL_SELF_TYPE:
			_self_type = optarg;
			break;
		case VAL_APP_VERSION:
			_app_version = optarg;
			break;
		case VAL_FW_VERSION:
			_fw_version = optarg;
			break;
		case VAL_ADD_SHDRS:
			_add_shdrs = optarg;
			break;
		case VAL_CTRL_FLAGS:
			_ctrl_flags = optarg;
			break;
		case VAL_CAP_FLAGS:
			_cap_flags = optarg;
			break;
#ifdef CONFIG_CUSTOM_INDIV_SEED
		case VAL_INDIV_SEED:
			_indiv_seed = optarg;
			break;
#endif
		case VAL_LICENSE_TYPE:
			_license_type = optarg;
			break;
		case VAL_APP_TYPE:
			_app_type = optarg;
			break;
		case VAL_CONTENT_ID:
			_content_id = optarg;
			break;
		case VAL_KLICENSEE:
			_klicensee = optarg;
			break;
		case VAL_REAL_FNAME:
			_real_fname = optarg;
			break;
		case VAL_ADD_SIG:
			_add_sig = optarg;
			break;
		case '?':
			print_usage();
			break;
		}
	}

	get_args:;

	//Additional decrypt args.
	if(_decrypt_file)
	{
		if(argc - optind < 1)
		{
			printf("[*] �訡��: ����室��� ����஢��� ��室��� 䠩�!\n");
			print_usage();
		}

		_file_out = argv[optind];
	}

	//Additional encrypt args.
	if(_encrypt_file)
	{
		if(argc - optind < 1)
		{
			printf("[*] �訡��: ����室��� ����஢��� �室��� � ��室��� 䠩�!\n");
			print_usage();
		}

		_file_out = argv[optind];
	}
}

//#ifndef _DEBUG
int main(int argc, char **argv)
{
	s8 *ps3 = NULL, path[256];

	//Check for args.
	if(argc <= 1)
		print_usage();

	//Parse them.
	parse_args(argc, argv);
	
	//Only options won't suffice.
	if(_got_work == FALSE)
		print_usage();

	print_version();
	printf("\n");

	//Try to get path from env:PS3.
	if((ps3 = getenv(CONFIG_ENV_PS3)) != NULL)
		if(access(ps3, 0) != 0)
			ps3 = NULL;

	//Load keysets.
	if(ps3 != NULL)
	{
		sprintf(path, "%s/%s", ps3, CONFIG_KEYS_FILE);
		if(access(path, 0) != 0)
			sprintf(path, "%s/%s", CONFIG_KEYS_PATH, CONFIG_KEYS_FILE);
	}
	else
		sprintf(path, "%s/%s", CONFIG_KEYS_PATH, CONFIG_KEYS_FILE);
	if(keys_load(path) == TRUE)
		_LOG_VERBOSE("����㦥� ����� ���祩.\n");
	else
	{
		if(_list_keys == TRUE)
		{
			printf("[*] �訡��: �� 㤠���� ����㧨�� ����.\n");
			return 0;
		}
		else
			printf("[*] ��������: �� 㤠���� ����㧨�� ����.\n");
	}

	//Load curves.
	if(ps3 != NULL)
	{
		sprintf(path, "%s/%s", ps3, CONFIG_CURVES_FILE);
		if(access(path, 0) != 0)
			sprintf(path, "%s/%s", CONFIG_CURVES_PATH, CONFIG_CURVES_FILE);
	}
	else
		sprintf(path, "%s/%s", CONFIG_CURVES_PATH, CONFIG_CURVES_FILE);
	if(curves_load(path) == TRUE)
		_LOG_VERBOSE("����㦥�� �ࠪ���⨪� �����稪�.\n");
	else
		printf("[*] ��������: �� 㤠���� ����㧨�� �ࠪ���⨪� �����稪�.\n");

	//Load curves.
	if(ps3 != NULL)
	{
		sprintf(path, "%s/%s", ps3, CONFIG_VSH_CURVES_FILE);
		if(access(path, 0) != 0)
			sprintf(path, "%s/%s", CONFIG_VSH_CURVES_PATH, CONFIG_VSH_CURVES_FILE);
	}
	else
		sprintf(path, "%s/%s", CONFIG_VSH_CURVES_PATH, CONFIG_VSH_CURVES_FILE);
	if(vsh_curves_load(path) == TRUE)
		_LOG_VERBOSE("����㦥�� �ࠪ���⨪� vsh.\n");
	else
		printf("[*] ��������: �� 㤠���� ����㧨�� �ࠪ���⨪� vsh.\n");

	//Set klicensee.
	if(_klicensee != NULL)
	{
		if(strlen(_klicensee) != 0x10*2)
		{
			printf("[*] �訡��: klicensee ������ ������ �� 16 ����.\n");
			return FALSE;
		}
		np_set_klicensee(_x_to_u8_buffer(_klicensee));
	}

	if(_list_keys == TRUE)
	{
		printf("[*] ����㦥� ����� ���祩:\n");
		_print_key_list(stdout);
	}
	else if(_print_info)
		frontend_print_infos(_file_in);
	else if(_decrypt_file)
		frontend_decrypt(_file_in, _file_out);
	else if(_encrypt_file)
		frontend_encrypt(_file_in, _file_out);

	return 0;
}
//#endif
