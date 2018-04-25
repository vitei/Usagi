/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// tuple.h
// Defines a tuple type for C++
// Based on the technique used by boost, but without pulling in its dependencies.
// Tuples are standard in C++ 11 so hopefully we'll be able to get rid of this
// once compilers are brought up-to-date.

// Supports tuples with up to 100 elements.
// This limit, too, will go away in C++ 11 (which supports variable length template parameters)

#ifndef _USG_CORE_TUPLE_H_
#define _USG_CORE_TUPLE_H_

#include "Engine/Common/Common.h"

namespace usg {

struct unit_t {};
inline const unit_t cunit() { return unit_t(); }
template <typename CAR, typename CDR> struct cons;

template<int I>
struct tuple_getter
{
	template<typename RET, typename CAR, typename CDR>
	inline static RET element(const cons<CAR, CDR>& t)
	{
		return tuple_getter<I-1>::template element<RET>(t.cdr);
	}

	template<typename RET, typename CAR, typename CDR>
	inline static RET element(cons<CAR, CDR>& t)
	{
		return tuple_getter<I-1>::template element<RET>(t.cdr);
	}
};

template<>
struct tuple_getter<0>
{
	template<typename RET, typename CAR, typename CDR>
	inline static RET element(const cons<CAR, CDR>& t)
	{
		return t.car;
	}

	template<typename RET, typename CAR, typename CDR>
	inline static RET element(cons<CAR, CDR>& t)
	{
		return t.car;
	}
};

template<int I, typename T>
struct element_type
{
	private:
		typedef typename T::tail_t Next;
	public:
		typedef typename element_type<I-1, Next>::type type;
};

template<typename T>
struct element_type<0, T>
{
	typedef typename T::head_t type;
};

template<int I, typename CAR, typename CDR>
inline const typename element_type< I, cons<CAR, CDR> >::type
tuple_element(const cons<CAR, CDR>& c)
{
	typedef typename element_type< I, cons<CAR, CDR> >::type RET;
	return tuple_getter<I>::template element<RET>(c);
}

template<int I, typename CAR, typename CDR>
inline typename element_type< I, cons<CAR, CDR> >::type
tuple_element(cons <CAR, CDR>& c)
{
	typedef typename element_type< I, cons<CAR, CDR> >::type RET;
	return tuple_getter<I>::template element<RET>(c);
}

template<typename CAR, typename CDR>
struct cons
{
	CAR car; CDR cdr;

	typedef CAR head_t;
	typedef CDR tail_t;

	cons() : car(), cdr() {}
	template<typename T00, typename T01, typename T02, typename T03, typename T04,
	         typename T05, typename T06, typename T07, typename T08, typename T09,
	         typename T10, typename T11, typename T12, typename T13, typename T14,
	         typename T15, typename T16, typename T17, typename T18, typename T19,
	         typename T20, typename T21, typename T22, typename T23, typename T24,
	         typename T25, typename T26, typename T27, typename T28, typename T29,
	         typename T30, typename T31, typename T32, typename T33, typename T34,
	         typename T35, typename T36, typename T37, typename T38, typename T39,
	         typename T40, typename T41, typename T42, typename T43, typename T44,
	         typename T45, typename T46, typename T47, typename T48, typename T49,
	         typename T50, typename T51, typename T52, typename T53, typename T54,
	         typename T55, typename T56, typename T57, typename T58, typename T59,
	         typename T60, typename T61, typename T62, typename T63, typename T64,
	         typename T65, typename T66, typename T67, typename T68, typename T69,
	         typename T70, typename T71, typename T72, typename T73, typename T74,
	         typename T75, typename T76, typename T77, typename T78, typename T79,
	         typename T80, typename T81, typename T82, typename T83, typename T84,
	         typename T85, typename T86, typename T87, typename T88, typename T89,
	         typename T90, typename T91, typename T92, typename T93, typename T94,
	         typename T95, typename T96, typename T97, typename T98, typename T99>
	cons(T00& t00, T01& t01, T02& t02, T03& t03, T04& t04, T05& t05, T06& t06, T07& t07, T08& t08, T09& t09,
	     T10& t10, T11& t11, T12& t12, T13& t13, T14& t14, T15& t15, T16& t16, T17& t17, T18& t18, T19& t19,
	     T20& t20, T21& t21, T22& t22, T23& t23, T24& t24, T25& t25, T26& t26, T27& t27, T28& t28, T29& t29,
	     T30& t30, T31& t31, T32& t32, T33& t33, T34& t34, T35& t35, T36& t36, T37& t37, T38& t38, T39& t39,
	     T40& t40, T41& t41, T42& t42, T43& t43, T44& t44, T45& t45, T46& t46, T47& t47, T48& t48, T49& t49,
	     T50& t50, T51& t51, T52& t52, T53& t53, T54& t54, T55& t55, T56& t56, T57& t57, T58& t58, T59& t59,
	     T60& t60, T61& t61, T62& t62, T63& t63, T64& t64, T65& t65, T66& t66, T67& t67, T68& t68, T69& t69,
	     T70& t70, T71& t71, T72& t72, T73& t73, T74& t74, T75& t75, T76& t76, T77& t77, T78& t78, T79& t79,
	     T80& t80, T81& t81, T82& t82, T83& t83, T84& t84, T85& t85, T86& t86, T87& t87, T88& t88, T89& t89,
	     T90& t90, T91& t91, T92& t92, T93& t93, T94& t94, T95& t95, T96& t96, T97& t97, T98& t98, T99& t99)
	: car(t00)
	, cdr(     t01, t02, t03, t04, t05, t06, t07, t08, t09,
	      t10, t11, t12, t13, t14, t15, t16, t17, t18, t19,
	      t20, t21, t22, t23, t24, t25, t26, t27, t28, t29,
	      t30, t31, t32, t33, t34, t35, t36, t37, t38, t39,
	      t40, t41, t42, t43, t44, t45, t46, t47, t48, t49,
	      t50, t51, t52, t53, t54, t55, t56, t57, t58, t59,
	      t60, t61, t62, t63, t64, t65, t66, t67, t68, t69,
	      t70, t71, t72, t73, t74, t75, t76, t77, t78, t79,
	      t80, t81, t82, t83, t84, t85, t86, t87, t88, t89,
	      t90, t91, t92, t93, t94, t95, t96, t97, t98, t99, cunit())
	{}

	template<              typename T01, typename T02, typename T03, typename T04,
	         typename T05, typename T06, typename T07, typename T08, typename T09,
	         typename T10, typename T11, typename T12, typename T13, typename T14,
	         typename T15, typename T16, typename T17, typename T18, typename T19,
	         typename T20, typename T21, typename T22, typename T23, typename T24,
	         typename T25, typename T26, typename T27, typename T28, typename T29,
	         typename T30, typename T31, typename T32, typename T33, typename T34,
	         typename T35, typename T36, typename T37, typename T38, typename T39,
	         typename T40, typename T41, typename T42, typename T43, typename T44,
	         typename T45, typename T46, typename T47, typename T48, typename T49,
	         typename T50, typename T51, typename T52, typename T53, typename T54,
	         typename T55, typename T56, typename T57, typename T58, typename T59,
	         typename T60, typename T61, typename T62, typename T63, typename T64,
	         typename T65, typename T66, typename T67, typename T68, typename T69,
	         typename T70, typename T71, typename T72, typename T73, typename T74,
	         typename T75, typename T76, typename T77, typename T78, typename T79,
	         typename T80, typename T81, typename T82, typename T83, typename T84,
	         typename T85, typename T86, typename T87, typename T88, typename T89,
	         typename T90, typename T91, typename T92, typename T93, typename T94,
	         typename T95, typename T96, typename T97, typename T98, typename T99>
	cons(const unit_t&, T01& t01, T02& t02, T03& t03, T04& t04, T05& t05, T06& t06, T07& t07, T08& t08, T09& t09,
	     T10& t10, T11& t11, T12& t12, T13& t13, T14& t14, T15& t15, T16& t16, T17& t17, T18& t18, T19& t19,
	     T20& t20, T21& t21, T22& t22, T23& t23, T24& t24, T25& t25, T26& t26, T27& t27, T28& t28, T29& t29,
	     T30& t30, T31& t31, T32& t32, T33& t33, T34& t34, T35& t35, T36& t36, T37& t37, T38& t38, T39& t39,
	     T40& t40, T41& t41, T42& t42, T43& t43, T44& t44, T45& t45, T46& t46, T47& t47, T48& t48, T49& t49,
	     T50& t50, T51& t51, T52& t52, T53& t53, T54& t54, T55& t55, T56& t56, T57& t57, T58& t58, T59& t59,
	     T60& t60, T61& t61, T62& t62, T63& t63, T64& t64, T65& t65, T66& t66, T67& t67, T68& t68, T69& t69,
	     T70& t70, T71& t71, T72& t72, T73& t73, T74& t74, T75& t75, T76& t76, T77& t77, T78& t78, T79& t79,
	     T80& t80, T81& t81, T82& t82, T83& t83, T84& t84, T85& t85, T86& t86, T87& t87, T88& t88, T89& t89,
	     T90& t90, T91& t91, T92& t92, T93& t93, T94& t94, T95& t95, T96& t96, T97& t97, T98& t98, T99& t99)
	: car()
	, cdr(     t01, t02, t03, t04, t05, t06, t07, t08, t09,
	      t10, t11, t12, t13, t14, t15, t16, t17, t18, t19,
	      t20, t21, t22, t23, t24, t25, t26, t27, t28, t29,
	      t30, t31, t32, t33, t34, t35, t36, t37, t38, t39,
	      t40, t41, t42, t43, t44, t45, t46, t47, t48, t49,
	      t50, t51, t52, t53, t54, t55, t56, t57, t58, t59,
	      t60, t61, t62, t63, t64, t65, t66, t67, t68, t69,
	      t70, t71, t72, t73, t74, t75, t76, t77, t78, t79,
	      t80, t81, t82, t83, t84, t85, t86, t87, t88, t89,
	      t90, t91, t92, t93, t94, t95, t96, t97, t98, t99, cunit())
	{}

	template<int I>
	inline typename element_type< I, cons<CAR, CDR> >::type element()
	{
		return ::usg::tuple_element<I>(*this);
	}

	template<int I>
	inline const typename element_type< I, cons<CAR, CDR> >::type element() const
	{
		return ::usg::tuple_element<I>(*this);
	}
};

template<typename CAR>
struct cons<CAR, unit_t>
{
	CAR    car;
	unit_t cdr;

	typedef CAR head_t;
	typedef unit_t tail_t;

	cons() : car() {}
	template<typename T0>
	cons(T0& t0, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&)
	: car(t0) {}
	cons(const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t&,
	     const unit_t&, const unit_t&, const unit_t&, const unit_t&, const unit_t& )
	: car() {}

	template<int I>
	inline typename element_type< I, cons<CAR, unit_t> >::type element()
	{
		::usg::tuple_element<I>(*this);
	}

	template<int I>
	inline const typename element_type< I, cons<CAR, unit_t> >::type element() const
	{
		::usg::tuple_element<I>(*this);
	}
};

template<typename T00, typename T01, typename T02, typename T03, typename T04,
         typename T05, typename T06, typename T07, typename T08, typename T09,
         typename T10, typename T11, typename T12, typename T13, typename T14,
         typename T15, typename T16, typename T17, typename T18, typename T19,
         typename T20, typename T21, typename T22, typename T23, typename T24,
         typename T25, typename T26, typename T27, typename T28, typename T29,
         typename T30, typename T31, typename T32, typename T33, typename T34,
         typename T35, typename T36, typename T37, typename T38, typename T39,
         typename T40, typename T41, typename T42, typename T43, typename T44,
         typename T45, typename T46, typename T47, typename T48, typename T49,
         typename T50, typename T51, typename T52, typename T53, typename T54,
         typename T55, typename T56, typename T57, typename T58, typename T59,
         typename T60, typename T61, typename T62, typename T63, typename T64,
         typename T65, typename T66, typename T67, typename T68, typename T69,
         typename T70, typename T71, typename T72, typename T73, typename T74,
         typename T75, typename T76, typename T77, typename T78, typename T79,
         typename T80, typename T81, typename T82, typename T83, typename T84,
         typename T85, typename T86, typename T87, typename T88, typename T89,
         typename T90, typename T91, typename T92, typename T93, typename T94,
         typename T95, typename T96, typename T97, typename T98, typename T99>
struct tuple_to_cons
{
	typedef cons<T00, typename tuple_to_cons<     T01, T02, T03, T04, T05, T06, T07, T08, T09,
	                                         T10, T11, T12, T13, T14, T15, T16, T17, T18, T19,
	                                         T20, T21, T22, T23, T24, T25, T26, T27, T28, T29,
	                                         T30, T31, T32, T33, T34, T35, T36, T37, T38, T39,
	                                         T40, T41, T42, T43, T44, T45, T46, T47, T48, T49,
	                                         T50, T51, T52, T53, T54, T55, T56, T57, T58, T59,
	                                         T60, T61, T62, T63, T64, T65, T66, T67, T68, T69,
	                                         T70, T71, T72, T73, T74, T75, T76, T77, T78, T79,
	                                         T80, T81, T82, T83, T84, T85, T86, T87, T88, T89,
	                                         T90, T91, T92, T93, T94, T95, T96, T97, T98, T99, unit_t>::type> type;
};

template <>
struct tuple_to_cons<unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t,
                     unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t, unit_t>
{
	typedef unit_t type;
};

template<typename T00 = unit_t, typename T01 = unit_t, typename T02 = unit_t, typename T03 = unit_t, typename T04 = unit_t,
         typename T05 = unit_t, typename T06 = unit_t, typename T07 = unit_t, typename T08 = unit_t, typename T09 = unit_t,
         typename T10 = unit_t, typename T11 = unit_t, typename T12 = unit_t, typename T13 = unit_t, typename T14 = unit_t,
         typename T15 = unit_t, typename T16 = unit_t, typename T17 = unit_t, typename T18 = unit_t, typename T19 = unit_t,
         typename T20 = unit_t, typename T21 = unit_t, typename T22 = unit_t, typename T23 = unit_t, typename T24 = unit_t,
         typename T25 = unit_t, typename T26 = unit_t, typename T27 = unit_t, typename T28 = unit_t, typename T29 = unit_t,
         typename T30 = unit_t, typename T31 = unit_t, typename T32 = unit_t, typename T33 = unit_t, typename T34 = unit_t,
         typename T35 = unit_t, typename T36 = unit_t, typename T37 = unit_t, typename T38 = unit_t, typename T39 = unit_t,
         typename T40 = unit_t, typename T41 = unit_t, typename T42 = unit_t, typename T43 = unit_t, typename T44 = unit_t,
         typename T45 = unit_t, typename T46 = unit_t, typename T47 = unit_t, typename T48 = unit_t, typename T49 = unit_t,
         typename T50 = unit_t, typename T51 = unit_t, typename T52 = unit_t, typename T53 = unit_t, typename T54 = unit_t,
         typename T55 = unit_t, typename T56 = unit_t, typename T57 = unit_t, typename T58 = unit_t, typename T59 = unit_t,
         typename T60 = unit_t, typename T61 = unit_t, typename T62 = unit_t, typename T63 = unit_t, typename T64 = unit_t,
         typename T65 = unit_t, typename T66 = unit_t, typename T67 = unit_t, typename T68 = unit_t, typename T69 = unit_t,
         typename T70 = unit_t, typename T71 = unit_t, typename T72 = unit_t, typename T73 = unit_t, typename T74 = unit_t,
         typename T75 = unit_t, typename T76 = unit_t, typename T77 = unit_t, typename T78 = unit_t, typename T79 = unit_t,
         typename T80 = unit_t, typename T81 = unit_t, typename T82 = unit_t, typename T83 = unit_t, typename T84 = unit_t,
         typename T85 = unit_t, typename T86 = unit_t, typename T87 = unit_t, typename T88 = unit_t, typename T89 = unit_t,
         typename T90 = unit_t, typename T91 = unit_t, typename T92 = unit_t, typename T93 = unit_t, typename T94 = unit_t,
         typename T95 = unit_t, typename T96 = unit_t, typename T97 = unit_t, typename T98 = unit_t, typename T99 = unit_t>
class tuple : public tuple_to_cons<T00, T01, T02, T03, T04, T05, T06, T07, T08, T09,
                                   T10, T11, T12, T13, T14, T15, T16, T17, T18, T19,
                                   T20, T21, T22, T23, T24, T25, T26, T27, T28, T29,
                                   T30, T31, T32, T33, T34, T35, T36, T37, T38, T39,
                                   T40, T41, T42, T43, T44, T45, T46, T47, T48, T49,
                                   T50, T51, T52, T53, T54, T55, T56, T57, T58, T59,
                                   T60, T61, T62, T63, T64, T65, T66, T67, T68, T69,
                                   T70, T71, T72, T73, T74, T75, T76, T77, T78, T79,
                                   T80, T81, T82, T83, T84, T85, T86, T87, T88, T89,
                                   T90, T91, T92, T93, T94, T95, T96, T97, T98, T99>::type
{
	public:
		typedef typename tuple_to_cons<T00, T01, T02, T03, T04, T05, T06, T07, T08, T09,
		                               T10, T11, T12, T13, T14, T15, T16, T17, T18, T19,
		                               T20, T21, T22, T23, T24, T25, T26, T27, T28, T29,
		                               T30, T31, T32, T33, T34, T35, T36, T37, T38, T39,
		                               T40, T41, T42, T43, T44, T45, T46, T47, T48, T49,
		                               T50, T51, T52, T53, T54, T55, T56, T57, T58, T59,
		                               T60, T61, T62, T63, T64, T65, T66, T67, T68, T69,
		                               T70, T71, T72, T73, T74, T75, T76, T77, T78, T79,
		                               T80, T81, T82, T83, T84, T85, T86, T87, T88, T89,
		                               T90, T91, T92, T93, T94, T95, T96, T97, T98, T99>::type inherited;

		//We only support constructors with up to ten members for now...
		tuple() {}
		tuple(T00 t00)
			: inherited(t00,     cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01)
			: inherited(t00,     t01,     cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01, T02 t02)
			: inherited(t00,     t01,     t02,     cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01, T02 t02, T03 t03)
			: inherited(t00,     t01,     t02,     t03    , cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01, T02 t02, T03 t03, T04 t04)
			: inherited(t00,     t01,     t02,     t03    , t04    , cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01, T02 t02, T03 t03, T04 t04, T05 t05)
			: inherited(t00,     t01,     t02,     t03,     t04,     t05,     cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01, T02 t02, T03 t03, T04 t04, T05 t05, T06 t06)
			: inherited(t00,     t01,     t02,     t03    , t04    , t05,     t06,     cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01, T02 t02, T03 t03, T04 t04, T05 t05, T06 t06, T07 t07)
			: inherited(t00,     t01,     t02,     t03    , t04    , t05,     t06,     t07,     cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01, T02 t02, T03 t03, T04 t04, T05 t05, T06 t06, T07 t07, T08 t08)
			: inherited(t00,     t01,     t02,     t03    , t04    , t05,     t06,     t07,     t08,     cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
		tuple(T00 t00, T01 t01, T02 t02, T03 t03, T04 t04, T05 t05, T06 t06, T07 t07, T08 t08, T09 t09)
			: inherited(t00,     t01,     t02,     t03    , t04    , t05,     t06,     t07,     t08,     t09,
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(),
			            cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit(), cunit()) {}
};

template<typename Functor, typename CAR, typename CDR>
struct ConsIterator
{
	inline static void TypeAction(Functor& f)
	{
		f. DEPENDENT_TEMPLATE operator()<CAR>();
		ConsIterator<Functor, typename CDR::head_t, typename CDR::tail_t>::TypeAction(f);
	}

	inline static void ValueAction(Functor& f, CAR& head, CDR& tail)
	{
		f(head);
		ConsIterator<Functor, typename CDR::head_t, typename CDR::tail_t>::ValueAction(f, tail.car, tail.cdr);
	}
};

template<typename Functor, typename CAR>
struct ConsIterator< Functor, CAR, unit_t >
{
	inline static void TypeAction(Functor& f)
	{
		f. DEPENDENT_TEMPLATE operator()<CAR>();
	}

	inline static void ValueAction(Functor& f, CAR& head, unit_t&)
	{
		f(head);
	}
};

template<typename Functor, typename CAR, typename CDR>
struct ConsFolder
{
	typedef typename Functor::Accumulator Accumulator;
	inline static Accumulator TypeFold(Functor& f, Accumulator acc)
	{
		Accumulator next = f. DEPENDENT_TEMPLATE operator()<CAR>(acc);
		return ConsFolder<Functor, typename CDR::head_t, typename CDR::tail_t>::TypeFold(f, next);
	}
};

template<typename Functor, typename CAR>
struct ConsFolder< Functor, CAR, unit_t >
{
	typedef typename Functor::Accumulator Accumulator;
	inline static Accumulator TypeFold(Functor& f, Accumulator acc)
	{
		return f. DEPENDENT_TEMPLATE operator()<CAR>(acc);
	}
};

template<typename Functor>
struct ConsFolder< Functor, unit_t, unit_t >
{
	typedef typename Functor::Accumulator Accumulator;
	inline static Accumulator TypeFold(Functor& f, Accumulator acc)
	{
		return acc;
	}
};

template<typename CAR, typename CDR, typename RHS>
struct ConsAppend
	: public cons< CAR, ConsAppend<typename CDR::head_t, typename CDR::tail_t, RHS> >
{
};

template<typename CAR, typename RHS>
struct ConsAppend<CAR, unit_t, RHS>
	: public cons< CAR, RHS >
{
};

template<template<typename> class Functor, typename CAR, typename CDR>
struct ConsMap
	: public cons< typename Functor<CAR>::rtn_type, typename ConsMap<Functor, typename CDR::head_t, typename CDR::tail_t>::inherited >
{
	typedef cons< typename Functor<CAR>::rtn_type, ConsMap<Functor, typename CDR::head_t, typename CDR::tail_t> > inherited;
	inline static void map(cons< typename inherited::head_t, typename inherited::tail_t >& out,
	                       const cons<CAR, CDR>& in)
	{
		out.car = Functor<CAR>::map(in.car);
		ConsMap<Functor, typename CDR::head_t, typename CDR::tail_t>::map(out.cdr, in.cdr);
	}
};

template<template<typename> class Functor, typename CAR>
struct ConsMap<Functor, CAR, unit_t>
	: public cons< typename Functor<CAR>::rtn_type, unit_t >
{
	typedef cons< typename Functor<CAR>::rtn_type, unit_t > inherited;
	inline static void map(cons< typename inherited::head_t, unit_t >& out,
	                       const cons<CAR, unit_t>& in)
	{
		out.car = Functor<CAR>::map(in.car);
	}
};

// ConsFind returns the index of the type TYPE in a tuple in the value "value".
template<typename TYPE, typename CAR, typename CDR, int IDX = 0>
struct ConsFind
{
	static const unsigned int value = ConsFind<TYPE, typename CDR::head_t, typename CDR::tail_t, IDX + 1>::value;
};

template<typename TYPE, typename CDR, int IDX>
struct ConsFind<TYPE, TYPE, CDR, IDX>
{
	static const unsigned int value = IDX;
};

template<typename CAR, typename CDR>
struct ConsLength
{
	static const unsigned int value = 1 + ConsLength<typename CDR::head_t, typename CDR::tail_t>::value;
};

template<typename CAR>
struct ConsLength<CAR, unit_t>
{
	static const unsigned int value = 1;
};

template<>
struct ConsLength<unit_t, unit_t>
{
	static const unsigned int value = 0;
};

template<typename Tuple, typename Functor>
void forall_types(Functor f)
{
	ConsIterator<Functor, typename Tuple::head_t, typename Tuple::tail_t>::TypeAction(f);
}

template<typename Tuple, typename Functor>
void forall_values(Tuple& tpl, Functor f)
{
	ConsIterator<Functor, typename Tuple::head_t, typename Tuple::tail_t>::ValueAction(f, tpl.car, tpl.cdr);
}

template<typename Tuple, typename Functor>
typename Functor::Accumulator fold_types(Functor f, typename Functor::Accumulator initial)
{
	return ConsFolder<Functor, typename Tuple::head_t, typename Tuple::tail_t>::TypeFold(f, initial);
}

template<template<typename> class Functor, typename Tuple>
struct map_types
	: public ConsMap<Functor, typename Tuple::head_t, typename Tuple::tail_t>::inherited
{
	typedef typename ConsMap<Functor, typename Tuple::head_t, typename Tuple::tail_t>::inherited inherited;
};

template<template<typename> class Functor, typename Tuple>
void map_values(map_types<Functor, Tuple>& out, const Tuple& tpl)
{
	ConsMap<Functor, typename Tuple::head_t, typename Tuple::tail_t>::map(out, tpl);
}

template<template<typename> class Functor, typename Tuple>
map_types<Functor, Tuple> map_values(const Tuple& tpl)
{
	map_types<Functor, Tuple> out;
	map_values(out, tpl);
	return out;
}

template<typename Tuple1, typename Tuple2>
struct tuple_join
	: public ConsAppend<typename Tuple1::head_t, typename Tuple1::tail_t, Tuple2>
{
};

template<typename Tuple1, typename Type>
struct tuple_find
	: public ConsFind<Type, typename Tuple1::head_t, typename Tuple1::tail_t>
{
};

template<typename Tuple>
struct tuple_length
	: public ConsLength<typename Tuple::head_t, typename Tuple::tail_t>
{
};

}

#endif //_USG_CORE_TUPLE_H_
