#pragma once

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Filtered_kernel.h>

namespace CGAL {

	// The following is equivalent to Filtered_kernel< Simple_cartesian<double> >,
	// but it's shorter in terms of template name length (for error messages, mangling...).

	class Epickf
		: public Filtered_kernel_adaptor<
		Type_equality_wrapper< Simple_cartesian<float>::Base<Epickf>::Type, Epickf >,
#ifdef CGAL_NO_STATIC_FILTERS
		false >
#else
		true >
#endif
	{};

	typedef Simple_cartesian<float> Ipickf;

} //namespace CGAL

