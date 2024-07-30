#ifndef KOKKIDIO_EIGENDUALVIEW_HPP
#define KOKKIDIO_EIGENDUALVIEW_HPP

#ifndef KOKKIDIO_PUBLIC_HEADER
#error "Do not include this file directly. Include Kokkidio/Core.hpp instead."
#endif

#include "Kokkidio/EigenView.hpp"

namespace Kokkidio
{



enum DualViewCopyOnInit {
	DontCopyToTarget = 0,
	CopyToTarget = 1,
};


template<typename _PlainObjectType, Target targetArg = DefaultTarget>
class EigenDualView {
public:
	static constexpr Target target { ExecutionTarget<targetArg> };
	using PlainObjectType_host = _PlainObjectType;

	using ThisType = EigenDualView<PlainObjectType_host, target>;
	using EigenView_host   = EigenView<PlainObjectType_host, Target::host>;
	using EigenView_target = EigenView<PlainObjectType_host, target>;
	using PlainObjectType_target = typename EigenView_target::PlainObjectType_target;
	using Scalar = typename EigenView_target::Scalar;

	using ViewType_host   = typename EigenView_host  ::ViewType;
	using ViewType_target = typename EigenView_target::ViewType;
	using ExecutionSpace_target = typename EigenView_target::ExecutionSpace;

	using MapType_host   = typename EigenView_host  ::MapType;
	using MapType_target = typename EigenView_target::MapType;

	static_assert(
		is_owning_eigen_type<std::remove_const_t<PlainObjectType_target>>::value ||
		is_eigen_map        <std::remove_const_t<PlainObjectType_target>>::value
	);

protected:
	EigenView_host   m_host;
	EigenView_target m_target;

public:
	EigenDualView(
		PlainObjectType_host& hostObj,
		DualViewCopyOnInit copyToTarget = CopyToTarget
	) :
		m_host  (hostObj),
		m_target(hostObj)
	{
		if ( copyToTarget ){
			this->copyToTarget();
		}
	}

	EigenDualView(Index rows, Index cols)
	{
		m_host = {rows, cols};
		/* When target and host are identical, 
		 * then we can copy-initialise the target View with the host View
		 * (or vice versa), to make them point to the same data.
		 */
		if constexpr (target == Target::host){
			m_target = {m_host};
		} else {
			m_target = {rows, cols};
		}
	}

	/* For Eigen vector types,
	 * we allow a single size parameter, like in Eigen itself. */
	template<typename P = PlainObjectType_host,
		typename std::enable_if_t<P::IsVectorAtCompileTime, int> = 0>
	EigenDualView(Index size) :
		/* EigenDualView(Index, Index) overwrites rows/cols 
		 * if they're known at compile time,
		 * so we could pass any numbers. */
		EigenDualView(size, size)
	{
		static_assert( std::is_same_v<P, PlainObjectType_host> );
	}

	/* For fixed size Eigen types,
	 * the default constructor allocates memory,
	 * like in Eigen itself. */
	template<typename P = PlainObjectType_host,
		typename std::enable_if_t<P::IsFixedSizeAtCompileTime, int> = 0>
	EigenDualView() :
		/* EigenDualView(Index, Index) overwrites rows/cols 
		 * if they're known at compile time,
		 * so we could pass any numbers. */
		EigenDualView(P::RowsAtCompileTime, P::ColsAtCompileTime)
	{
		static_assert( std::is_same_v<P, PlainObjectType_host> );
	}

	/* For dynamically sized Eigen types, the default constructor does nothing */
	template<typename P = PlainObjectType_host,
		typename std::enable_if_t<!P::IsFixedSizeAtCompileTime, int> = 0>
	EigenDualView(){
		static_assert( std::is_same_v<P, PlainObjectType_host> );
	}

	void assign( PlainObjectType_host& hostObj ){
		this->m_host   = {hostObj};
		this->m_target = {hostObj};
	}

	void resize( Index rows, Index cols ){
		this->m_host  .resize(rows, cols);
		this->m_target.resize(rows, cols);
	}

	KOKKOS_FUNCTION
	bool isAlloc_host() const {
		return this->m_host.isAlloc();
	}

	KOKKOS_FUNCTION
	bool isAlloc_target() const {
		return this->m_target.isAlloc();
	}

	KOKKOS_FUNCTION
	auto get_host() const -> EigenView_host {
		return this->m_host;
	}

	KOKKOS_FUNCTION
	auto get_target() const -> EigenView_target {
		return this->m_target;
	}

	KOKKOS_FUNCTION
	auto view_host() const -> ViewType_host {
		assert( this->isAlloc_host() );
		return this->m_host.view();
	}

	KOKKOS_FUNCTION
	auto view_target() const -> ViewType_target {
		assert( this->isAlloc_target() );
		return this->m_target.view();
	}

	KOKKOS_FUNCTION
	auto map_host() const -> MapType_host {
		return this->m_host.map();
	}

	KOKKOS_FUNCTION
	auto map_target() const -> MapType_target {
		return this->m_target.map();
	}

	void copyToTarget(bool async = false){
		if constexpr ( target != Target::host ){
			printd( "Copying from host (n=%i) to target (n=%i)...\n"
				, static_cast<int>( this->view_host  ().size() )
				, static_cast<int>( this->view_target().size() )
			);
			if (async){
				Kokkos::deep_copy( ExecutionSpace_target{},
					this->view_target(), // dst
					this->view_host()    // src
				);
			} else {
				Kokkos::deep_copy(
					this->view_target(), // dst
					this->view_host()    // src
				);
			}
		} else {
			printd( "EigenDualView::copyToTarget, target==host, skipping...\n");
			assert( this->view_target().data() == this->view_host().data() );
			assert( this-> map_target().data() == this-> map_host().data() );
		}
	}

	void copyToHost(bool async = false){
		if constexpr ( target != Target::host ){
			printd( "Copying from target (n=%i) to host (n=%i)...\n"
				, static_cast<int>( this->view_target().size() )
				, static_cast<int>( this->view_host  ().size() )
			);
			if (async){
				Kokkos::deep_copy( ExecutionSpace_target{},
					this->view_host(),  // dst
					this->view_target() // src
				);
			} else {
				Kokkos::deep_copy(
					this->view_host(),  // dst
					this->view_target() // src
				);
			}
		} else {
			printd( "EigenDualView::copyToHost, target==host, skipping...\n");
			assert( this->view_target().data() == this->view_host().data() );
			assert( this-> map_target().data() == this-> map_host().data() );
		}
	}
};

template<Target targetArg, typename PlainObjectType_host>
EigenDualView<PlainObjectType_host, targetArg> make_EigenDualView(
	PlainObjectType_host& hostObj,
	DualViewCopyOnInit copyToTarget = CopyToTarget
){
	return {hostObj, copyToTarget};
}

template<typename T>
struct is_EigenDualView : std::false_type {};

template<typename PlainObjectType, Target targetArg>
struct is_EigenDualView<EigenDualView<PlainObjectType, targetArg>> : std::true_type {};

template<typename T>
inline constexpr bool is_EigenDualView_v = is_EigenDualView<T>::value;

} // namespace Kokkidio

#endif
