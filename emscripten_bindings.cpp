
#include "./pxtone/pxtnService.h"
#include "./pxtone/pxtnError.h"
#include <string>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

std::string pxtnErrorToString(pxtnERR err) {
	return pxtnError_get_string(err);
}

EMSCRIPTEN_BINDINGS(pxtnError) {
	enum_<pxtnERR>("PxtnERR");

	function("pxtnErrorToString", &pxtnErrorToString);
}

static pxtnERR _load_ptcop(pxtnService* pxtn, std::string path)
{
	bool           b_ret = false;
	pxtnDescriptor desc;
	pxtnERR        pxtn_err = pxtnERR_VOID;
	FILE*          fp = NULL;

	if (!(fp = fopen(path.c_str(), "rb"))) goto term;
	if (!desc.set_file_r(fp)) goto term;

	pxtn_err = pxtn->read(&desc); if (pxtn_err != pxtnOK) goto term;
	pxtn_err = pxtn->tones_ready(); if (pxtn_err != pxtnOK) goto term;

	b_ret = true;

term:

	if (fp) {
		fclose(fp);
	}
	else {
		perror("Cannot open file: ");
	}

	if (!b_ret) {
		pxtn->evels->Release();
		return pxtnERR_FATAL;
	}

	return pxtn_err;
}

static bool pxtnServiceMooPreparation(pxtnService* pxtn, pxtnVOMITPREPARATION prep) {
	return pxtn->moo_preparation(&prep);
}

extern "C" {
	bool EMSCRIPTEN_KEEPALIVE pxtnServiceMoo(pxtnService* pxtn, void* buffer, int32_t buf_size) {
		return pxtn->Moo(buffer, buf_size);
	}
}

EMSCRIPTEN_BINDINGS(pxtnService) {
	value_object<pxtnVOMITPREPARATION>("PxtnPreparation")
		.field("flags", &pxtnVOMITPREPARATION::flags)
		.field("startPosition", &pxtnVOMITPREPARATION::start_pos_float)
		.field("masterVolume", &pxtnVOMITPREPARATION::master_volume)
		;

	class_<pxtnService>("PxtnService")
		.constructor<>()
		.function("init", &pxtnService::init)
		.function("setDestinationQuality", &pxtnService::set_destination_quality)
		.property("totalSample", &pxtnService::moo_get_total_sample)
		.function("preparation", &pxtnService::moo_preparation, allow_raw_pointers())
		;

	function("pxtnLoadPtcop", &_load_ptcop, allow_raw_pointers());
	function("pxtnServiceMooPreparation", &pxtnServiceMooPreparation, allow_raw_pointers());
}
