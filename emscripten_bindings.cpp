
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

static bool pxtnServiceMooPreparation(pxtnService* pxtn, pxtnVOMITPREPARATION prep) {
	return pxtn->moo_preparation(&prep);
}

extern "C" {
	pxtnERR EMSCRIPTEN_KEEPALIVE pxtnServiceLoad(pxtnService* pxtn, void* p_mem, int size) {
		pxtnDescriptor desc;
		pxtnERR pxtn_err = pxtnERR_VOID;

		if (desc.set_memory_r(p_mem, size)) {
			pxtn_err = pxtn->read(&desc);
			if (pxtn_err == pxtnOK) {
				pxtn_err = pxtn->tones_ready();
				if (pxtn_err == pxtnOK) {
					return pxtnOK;
				}
			}
		}

		pxtn->evels->Release();
		return pxtn_err;
	}

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

	function("pxtnServiceMooPreparation", &pxtnServiceMooPreparation, allow_raw_pointers());
}
