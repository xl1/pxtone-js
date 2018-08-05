
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <cstdint>

#include <tchar.h>

#include "./pxtone/pxtnService.h"
#include "./pxtone/pxtnError.h"

#define _CHANNEL_NUM           2
#define _SAMPLE_PER_SECOND 44100
#define _BUFFER_PER_SEC    (0.3f)

static bool _load_ptcop( pxtnService* pxtn, const TCHAR* path_src, pxtnERR* p_pxtn_err )
{
	bool           b_ret     = false;
	pxtnDescriptor desc;
	pxtnERR        pxtn_err  = pxtnERR_VOID;
	FILE*          fp        = NULL;
	int32_t        event_num =    0;

	if( !( fp = _tfopen( path_src, _T("rb") ) ) ) goto term;
	if( !desc.set_file_r( fp ) ) goto term;

	pxtn_err = pxtn->read       ( &desc ); if( pxtn_err != pxtnOK ) goto term;
	pxtn_err = pxtn->tones_ready(       ); if( pxtn_err != pxtnOK ) goto term;

	b_ret = true;
term:

	if( fp     ) fclose( fp );
	if( !b_ret ) pxtn->evels->Release();

	if( p_pxtn_err ) *p_pxtn_err = pxtn_err;

	return b_ret;
}

int main()
{
	bool           b_ret    = false;
	pxtnService*   pxtn     = NULL ;
	pxtnERR        pxtn_err = pxtnERR_VOID;

	// INIT PXTONE.
	pxtn = new pxtnService();
	pxtn_err = pxtn->init(); if( pxtn_err != pxtnOK ) goto term;
	if( !pxtn->set_destination_quality( _CHANNEL_NUM, _SAMPLE_PER_SECOND ) ) goto term;

	// SELECT MUSIC FILE.
	TCHAR* path_src = _T("sample data\\sample.ptcop");
	
	// LOAD MUSIC FILE.
	if( !_load_ptcop( pxtn, path_src, &pxtn_err ) ) goto term;

	// PREPARATION PLAYING MUSIC.
	{
		int32_t smp_total = pxtn->moo_get_total_sample();

		pxtnVOMITPREPARATION prep = {0};
		prep.flags          |= pxtnVOMITPREPFLAG_loop;
		prep.start_pos_float =     0;
		prep.master_volume   = 0.80f;

		if( !pxtn->moo_preparation( &prep ) ) goto term;
	}

	const int BUFFER_COUNT = 3;
	uint8_t* bufs[BUFFER_COUNT];
	int32_t buf_size = (int32_t)(_CHANNEL_NUM * _SAMPLE_PER_SECOND * _BUFFER_PER_SEC) * 2;
	bool is_quit = false;

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		if (!(bufs[i] = (uint8_t*)malloc(buf_size))) goto term;
		memset(bufs[i], 0, buf_size);
	}

	printf("enter to next, q to quit\n");
	do
	{
		for (int i = 0; i < BUFFER_COUNT; i++)
		{
			if (!pxtn->Moo(bufs[i], buf_size)) goto term;

			for (int j = 0; j < 8; j++)
				printf("%02X ", bufs[i][j]);
			printf("\n");
		}
		is_quit = getchar() == 'q';
	} while (!is_quit);

	b_ret = true;
term:

	if( !b_ret )
	{
		// ERROR MESSAGE.
		TCHAR err_msg[ 100 ] = {0};
		_stprintf_s( err_msg, 100, _T("ERROR: pxtnERR[ %s ]"), pxtnError_get_string( pxtn_err ) );
		printf(err_msg);
	}

	SAFE_DELETE( pxtn );
	return 1;
}
