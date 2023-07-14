#include "timetron_core/proc_util.h"


using namespace timetron::core;


char const * proc_util::replace_if_null(char const * str)
{
	return str ? str : "";
}