/******************************************************************************
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Author:
 *     Hui Peng <peng124@purdue.edu>
 * Date:
 *     2021-04-26
 *****************************************************************************/

#ifndef INCLUDE_UTIL_CASTING_H_
#define INCLUDE_UTIL_CASTING_H_

#include <llvm/Support/Casting.h>
#include <llvm/Support/type_traits.h>
#include <memory>
#include <type_traits>

namespace llvm {
// support for dyn_cast on shared_ptr

template <typename To, typename From>
struct isa_impl_cl<To, const std::shared_ptr<From>> {
    static inline bool doit(const std::shared_ptr<From> &Val) {
        assert(Val && "isa<> used on a null pointer");
        return isa_impl_cl<To, From>::doit(*Val);
    }
};

template <class To, class From>
struct cast_retty_impl<To, std::shared_ptr<From>> {
  private:
    using PointerType = typename cast_retty_impl<To, From *>::ret_type;
    using ResultType = typename std::remove_pointer<PointerType>::type;

  public:
    using ret_type = std::shared_ptr<ResultType>;
};

template <class To, class FromTy>
struct cast_convert_val<To, std::shared_ptr<FromTy>, std::shared_ptr<FromTy>> {
    using FromSPtr = std::shared_ptr<FromTy>;
    using RetTypeWConst = typename cast_retty<To, FromSPtr>::ret_type;
    using CastTo = typename std::remove_const<To>::type;
    using RetType = typename std::remove_const<RetTypeWConst>::type;
    static RetType doit(const FromSPtr &Val) {
        RetType Res2 = std::dynamic_pointer_cast<CastTo>(Val);
        return Res2;
    }
};

template <class X, class Y>
inline typename cast_retty<X, std::shared_ptr<Y>>::ret_type
cast(std::shared_ptr<Y> &Val) {
    assert(isa<X>(Val.get()) && "cast<Ty>() argument of incompatible type!");
    using YSPtr = std::shared_ptr<Y>;
    using ret_type = typename cast_retty<X, YSPtr>::ret_type;
    return ret_type(
        cast_convert_val<X, YSPtr,
                         typename simplify_type<YSPtr>::SimpleType>::doit(Val));
}

template <class X, class Y>
inline typename cast_retty<X, std::shared_ptr<Y>>::ret_type
cast(std::shared_ptr<Y> &&Val) {
    assert(isa<X>(Val.get()) && "cast<Ty>() argument of incompatible type!");
    using YSPtr = std::shared_ptr<Y>;
    using ret_type = typename cast_retty<X, YSPtr>::ret_type;
    return ret_type(
        cast_convert_val<X, YSPtr,
                         typename simplify_type<YSPtr>::SimpleType>::doit(Val));
}

template <class X, class Y>
LLVM_NODISCARD inline auto dyn_cast(std::unique_ptr<Y> &Val)
    -> decltype(cast<X>(Val)) {
    if (!Val)
        return nullptr;
    return unique_dyn_cast<X, Y>(Val);
}

template <class X, class Y>
LLVM_NODISCARD inline auto dyn_cast(std::unique_ptr<Y> &&Val)
    -> decltype(cast<X>(Val)) {
    return unique_dyn_cast<X, Y>(Val);
}

// dyn_cast_or_null<X> - Functionally identical to unique_dyn_cast, except that
// a null value is accepted.
template <class X, class Y>
LLVM_NODISCARD inline auto dyn_cast_or_null(std::unique_ptr<Y> &Val)
    -> decltype(cast<X>(Val)) {
    if (!Val)
        return nullptr;
    return unique_dyn_cast<X, Y>(Val);
}

template <class X, class Y>
LLVM_NODISCARD inline auto dyn_cast_or_null(std::unique_ptr<Y> &&Val)
    -> decltype(cast<X>(Val)) {
    return unique_dyn_cast_or_null<X, Y>(Val);
}

} // End namespace llvm

#endif /* INCLUDE_UTIL_CASTING_H_ */
