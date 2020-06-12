#include "fay/render/render_graph.h"

namespace fay
{

inline namespace type
{

/*
	setup pass:    placeholders for resources, does not hold actual resources
	complier pass: crop off unnecessary (virtual) resources
	execure pass:  Link them to actual resources
*/
class virtual_resource;

}



} // namespace fay