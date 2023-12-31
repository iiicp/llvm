//===-- ConstantRange.cpp - ConstantRange implementation ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Represent a range of possible values that may occur when the program is run
// for an integral value.  This keeps track of a lower and upper bound for the
// constant, which MAY wrap around the end of the numeric range.  To do this, it
// keeps track of a [lower, upper) bound, which specifies an interval just like
// STL iterators.  When used with boolean values, the following are important
// ranges (other integral ranges use min/max values for special range values):
//
//  [F, F) = {}     = Empty set
//  [T, F) = {T}
//  [F, T) = {F}
//  [T, T) = {F, T} = Full set
//
//===----------------------------------------------------------------------===//

#include "llvm/InstrTypes.h"
#include "llvm/Support/ConstantRange.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

/// Initialize a full (the default) or empty set for the specified type.
///
ConstantRange::ConstantRange(uint32_t BitWidth, bool Full) {
  if (Full)
    Lower = Upper = APInt::getMaxValue(BitWidth);
  else
    Lower = Upper = APInt::getMinValue(BitWidth);
}

/// Initialize a range to hold the single specified value.
///
ConstantRange::ConstantRange(const APInt &V) : Lower(V), Upper(V + 1) {}

ConstantRange::ConstantRange(const APInt &L, const APInt &U) :
  Lower(L), Upper(U) {
  assert(L.getBitWidth() == U.getBitWidth() &&
         "ConstantRange with unequal bit widths");
  assert((L != U || (L.isMaxValue() || L.isMinValue())) &&
         "Lower == Upper, but they aren't min or max value!");
}

ConstantRange ConstantRange::makeICmpRegion(unsigned Pred,
                                            const ConstantRange &CR) {
  if (CR.isEmptySet())
    return CR;
#if 1
  uint32_t W = CR.getBitWidth();
  switch (Pred) {
    default: assert(0 && "Invalid ICmp predicate to makeICmpRegion()");
    case CmpInst::ICMP_EQ:
      return CR;
    case CmpInst::ICMP_NE:
      if (CR.isSingleElement())
        return ConstantRange(CR.getUpper(), CR.getLower());
      return ConstantRange(W);
    case CmpInst::ICMP_ULT: {
      APInt UMax(CR.getUnsignedMax());
      if (UMax.isMinValue())
        return ConstantRange(W, /* empty */ false);
      return ConstantRange(APInt::getMinValue(W), UMax);
    }
    case CmpInst::ICMP_SLT: {
      APInt SMax(CR.getSignedMax());
      if (SMax.isMinSignedValue())
        return ConstantRange(W, /* empty */ false);
      return ConstantRange(APInt::getSignedMinValue(W), SMax);
    }
    case CmpInst::ICMP_ULE: {
      APInt UMax(CR.getUnsignedMax());
      if (UMax.isMaxValue())
        return ConstantRange(W);
      return ConstantRange(APInt::getMinValue(W), UMax + 1);
    }
    case CmpInst::ICMP_SLE: {
      APInt SMax(CR.getSignedMax());
      if (SMax.isMaxSignedValue())
        return ConstantRange(W);
      return ConstantRange(APInt::getSignedMinValue(W), SMax + 1);
    }
    case CmpInst::ICMP_UGT: {
      APInt UMin(CR.getUnsignedMin());
      if (UMin.isMaxValue())
        return ConstantRange(W, /* empty */ false);
      return ConstantRange(UMin + 1, APInt::getNullValue(W));
    }
    case CmpInst::ICMP_SGT: {
      APInt SMin(CR.getSignedMin());
      if (SMin.isMaxSignedValue())
        return ConstantRange(W, /* empty */ false);
      return ConstantRange(SMin + 1, APInt::getSignedMinValue(W));
    }
    case CmpInst::ICMP_UGE: {
      APInt UMin(CR.getUnsignedMin());
      if (UMin.isMinValue())
        return ConstantRange(W);
      return ConstantRange(UMin, APInt::getNullValue(W));
    }
    case CmpInst::ICMP_SGE: {
      APInt SMin(CR.getSignedMin());
      if (SMin.isMinSignedValue())
        return ConstantRange(W);
      return ConstantRange(SMin, APInt::getSignedMinValue(W));
    }
  }
#endif
  return CR;
}

/// isFullSet - Return true if this set contains all of the elements possible
/// for this data-type
bool ConstantRange::isFullSet() const {
  return Lower == Upper && Lower.isMaxValue();
}

/// isEmptySet - Return true if this set contains no members.
///
bool ConstantRange::isEmptySet() const {
  return Lower == Upper && Lower.isMinValue();
}

/// isWrappedSet - Return true if this set wraps around the top of the range,
/// for example: [100, 8)
///
bool ConstantRange::isWrappedSet() const {
  return Lower.ugt(Upper);
}

/// isSignWrappedSet - Return true if this set wraps around the INT_MIN of
/// its bitwidth, for example: i8 [120, 140).
///
bool ConstantRange::isSignWrappedSet() const {
  return contains(APInt::getSignedMaxValue(getBitWidth())) &&
         contains(APInt::getSignedMinValue(getBitWidth()));
}

/// getSetSize - Return the number of elements in this set.
///
APInt ConstantRange::getSetSize() const {
  if (isEmptySet()) 
    return APInt(getBitWidth(), 0);
  if (getBitWidth() == 1) {
    if (Lower != Upper)  // One of T or F in the set...
      return APInt(2, 1);
    return APInt(2, 2);      // Must be full set...
  }

  // Simply subtract the bounds...
  return Upper - Lower;
}

/// getUnsignedMax - Return the largest unsigned value contained in the
/// ConstantRange.
///
APInt ConstantRange::getUnsignedMax() const {
  if (isFullSet() || isWrappedSet())
    return APInt::getMaxValue(getBitWidth());
  else
    return getUpper() - 1;
}

/// getUnsignedMin - Return the smallest unsigned value contained in the
/// ConstantRange.
///
APInt ConstantRange::getUnsignedMin() const {
  if (isFullSet() || (isWrappedSet() && getUpper() != 0))
    return APInt::getMinValue(getBitWidth());
  else
    return getLower();
}

/// getSignedMax - Return the largest signed value contained in the
/// ConstantRange.
///
APInt ConstantRange::getSignedMax() const {
  APInt SignedMax(APInt::getSignedMaxValue(getBitWidth()));
  if (!isWrappedSet()) {
    if (getLower().sle(getUpper() - 1))
      return getUpper() - 1;
    else
      return SignedMax;
  } else {
    if (getLower().isNegative() == getUpper().isNegative())
      return SignedMax;
    else
      return getUpper() - 1;
  }
}

/// getSignedMin - Return the smallest signed value contained in the
/// ConstantRange.
///
APInt ConstantRange::getSignedMin() const {
  APInt SignedMin(APInt::getSignedMinValue(getBitWidth()));
  if (!isWrappedSet()) {
    if (getLower().sle(getUpper() - 1))
      return getLower();
    else
      return SignedMin;
  } else {
    if ((getUpper() - 1).slt(getLower())) {
      if (getUpper() != SignedMin)
        return SignedMin;
      else
        return getLower();
    } else {
      return getLower();
    }
  }
}

/// contains - Return true if the specified value is in the set.
///
bool ConstantRange::contains(const APInt &V) const {
  if (Lower == Upper)
    return isFullSet();

  if (!isWrappedSet())
    return Lower.ule(V) && V.ult(Upper);
  else
    return Lower.ule(V) || V.ult(Upper);
}

/// contains - Return true if the argument is a subset of this range.
/// Two equal sets contain each other. The empty set contained by all other
/// sets.
///
bool ConstantRange::contains(const ConstantRange &Other) const {
  if (isFullSet() || Other.isEmptySet()) return true;
  if (isEmptySet() || Other.isFullSet()) return false;

  if (!isWrappedSet()) {
    if (Other.isWrappedSet())
      return false;

    return Lower.ule(Other.getLower()) && Other.getUpper().ule(Upper);
  }

  if (!Other.isWrappedSet())
    return Other.getUpper().ule(Upper) ||
           Lower.ule(Other.getLower());

  return Other.getUpper().ule(Upper) && Lower.ule(Other.getLower());
}

/// subtract - Subtract the specified constant from the endpoints of this
/// constant range.
ConstantRange ConstantRange::subtract(const APInt &Val) const {
  assert(Val.getBitWidth() == getBitWidth() && "Wrong bit width");
  // If the set is empty or full, don't modify the endpoints.
  if (Lower == Upper) 
    return *this;
  return ConstantRange(Lower - Val, Upper - Val);
}

/// intersectWith - Return the range that results from the intersection of this
/// range with another range.  The resultant range is guaranteed to include all
/// elements contained in both input ranges, and to have the smallest possible
/// set size that does so.  Because there may be two intersections with the
/// same set size, A.intersectWith(B) might not be equal to B.intersectWith(A).
ConstantRange ConstantRange::intersectWith(const ConstantRange &CR) const {
  assert(getBitWidth() == CR.getBitWidth() && 
         "ConstantRange types don't agree!");

  // Handle common cases.
  if (   isEmptySet() || CR.isFullSet()) return *this;
  if (CR.isEmptySet() ||    isFullSet()) return CR;

  if (!isWrappedSet() && CR.isWrappedSet())
    return CR.intersectWith(*this);

  if (!isWrappedSet() && !CR.isWrappedSet()) {
    if (Lower.ult(CR.Lower)) {
      if (Upper.ule(CR.Lower))
        return ConstantRange(getBitWidth(), false);

      if (Upper.ult(CR.Upper))
        return ConstantRange(CR.Lower, Upper);

      return CR;
    } else {
      if (Upper.ult(CR.Upper))
        return *this;

      if (Lower.ult(CR.Upper))
        return ConstantRange(Lower, CR.Upper);

      return ConstantRange(getBitWidth(), false);
    }
  }

  if (isWrappedSet() && !CR.isWrappedSet()) {
    if (CR.Lower.ult(Upper)) {
      if (CR.Upper.ult(Upper))
        return CR;

      if (CR.Upper.ult(Lower))
        return ConstantRange(CR.Lower, Upper);

      if (getSetSize().ult(CR.getSetSize()))
        return *this;
      else
        return CR;
    } else if (CR.Lower.ult(Lower)) {
      if (CR.Upper.ule(Lower))
        return ConstantRange(getBitWidth(), false);

      return ConstantRange(Lower, CR.Upper);
    }
    return CR;
  }

  if (CR.Upper.ult(Upper)) {
    if (CR.Lower.ult(Upper)) {
      if (getSetSize().ult(CR.getSetSize()))
        return *this;
      else
        return CR;
    }

    if (CR.Lower.ult(Lower))
      return ConstantRange(Lower, CR.Upper);

    return CR;
  } else if (CR.Upper.ult(Lower)) {
    if (CR.Lower.ult(Lower))
      return *this;

    return ConstantRange(CR.Lower, Upper);
  }
  if (getSetSize().ult(CR.getSetSize()))
    return *this;
  else
    return CR;
}


/// unionWith - Return the range that results from the union of this range with
/// another range.  The resultant range is guaranteed to include the elements of
/// both sets, but may contain more.  For example, [3, 9) union [12,15) is
/// [3, 15), which includes 9, 10, and 11, which were not included in either
/// set before.
///
ConstantRange ConstantRange::unionWith(const ConstantRange &CR) const {
  assert(getBitWidth() == CR.getBitWidth() && 
         "ConstantRange types don't agree!");

  if (   isFullSet() || CR.isEmptySet()) return *this;
  if (CR.isFullSet() ||    isEmptySet()) return CR;

  if (!isWrappedSet() && CR.isWrappedSet()) return CR.unionWith(*this);

  if (!isWrappedSet() && !CR.isWrappedSet()) {
    if (CR.Upper.ult(Lower) || Upper.ult(CR.Lower)) {
      // If the two ranges are disjoint, find the smaller gap and bridge it.
      APInt d1 = CR.Lower - Upper, d2 = Lower - CR.Upper;
      if (d1.ult(d2))
        return ConstantRange(Lower, CR.Upper);
      else
        return ConstantRange(CR.Lower, Upper);
    }

    APInt L = Lower, U = Upper;
    if (CR.Lower.ult(L))
      L = CR.Lower;
    if ((CR.Upper - 1).ugt(U - 1))
      U = CR.Upper;

    if (L == 0 && U == 0)
      return ConstantRange(getBitWidth());

    return ConstantRange(L, U);
  }

  if (!CR.isWrappedSet()) {
    // ------U   L-----  and  ------U   L----- : this
    //   L--U                            L--U  : CR
    if (CR.Upper.ule(Upper) || CR.Lower.uge(Lower))
      return *this;

    // ------U   L----- : this
    //    L---------U   : CR
    if (CR.Lower.ule(Upper) && Lower.ule(CR.Upper))
      return ConstantRange(getBitWidth());

    // ----U       L---- : this
    //       L---U       : CR
    //    <d1>  <d2>
    if (Upper.ule(CR.Lower) && CR.Upper.ule(Lower)) {
      APInt d1 = CR.Lower - Upper, d2 = Lower - CR.Upper;
      if (d1.ult(d2))
        return ConstantRange(Lower, CR.Upper);
      else
        return ConstantRange(CR.Lower, Upper);
    }

    // ----U     L----- : this
    //        L----U    : CR
    if (Upper.ult(CR.Lower) && Lower.ult(CR.Upper))
      return ConstantRange(CR.Lower, Upper);

    // ------U    L---- : this
    //    L-----U       : CR
    if (CR.Lower.ult(Upper) && CR.Upper.ult(Lower))
      return ConstantRange(Lower, CR.Upper);
  }

  assert(isWrappedSet() && CR.isWrappedSet() &&
         "ConstantRange::unionWith missed wrapped union unwrapped case");

  // ------U    L----  and  ------U    L---- : this
  // -U  L-----------  and  ------------U  L : CR
  if (CR.Lower.ule(Upper) || Lower.ule(CR.Upper))
    return ConstantRange(getBitWidth());

  APInt L = Lower, U = Upper;
  if (CR.Upper.ugt(U))
    U = CR.Upper;
  if (CR.Lower.ult(L))
    L = CR.Lower;

  return ConstantRange(L, U);
}

/// zeroExtend - Return a new range in the specified integer type, which must
/// be strictly larger than the current type.  The returned range will
/// correspond to the possible range of values as if the source range had been
/// zero extended.
ConstantRange ConstantRange::zeroExtend(uint32_t DstTySize) const {
  if (isEmptySet()) return ConstantRange(DstTySize, /*isFullSet=*/false);

  unsigned SrcTySize = getBitWidth();
  assert(SrcTySize < DstTySize && "Not a value extension");
  if (isFullSet() || isWrappedSet())
    // Change into [0, 1 << src bit width)
    return ConstantRange(APInt(DstTySize,0), APInt(DstTySize,1).shl(SrcTySize));

  return ConstantRange(Lower.zext(DstTySize), Upper.zext(DstTySize));
}

/// signExtend - Return a new range in the specified integer type, which must
/// be strictly larger than the current type.  The returned range will
/// correspond to the possible range of values as if the source range had been
/// sign extended.
ConstantRange ConstantRange::signExtend(uint32_t DstTySize) const {
  if (isEmptySet()) return ConstantRange(DstTySize, /*isFullSet=*/false);

  unsigned SrcTySize = getBitWidth();
  assert(SrcTySize < DstTySize && "Not a value extension");
  if (isFullSet() || isSignWrappedSet()) {
    return ConstantRange(APInt::getHighBitsSet(DstTySize,DstTySize-SrcTySize+1),
                         APInt::getLowBitsSet(DstTySize, SrcTySize-1) + 1);
  }

  return ConstantRange(Lower.sext(DstTySize), Upper.sext(DstTySize));
}

/// truncate - Return a new range in the specified integer type, which must be
/// strictly smaller than the current type.  The returned range will
/// correspond to the possible range of values as if the source range had been
/// truncated to the specified type.
ConstantRange ConstantRange::truncate(uint32_t DstTySize) const {
  unsigned SrcTySize = getBitWidth();
  assert(SrcTySize > DstTySize && "Not a value truncation");
  APInt Size(APInt::getLowBitsSet(SrcTySize, DstTySize));
  if (isFullSet() || getSetSize().ugt(Size))
    return ConstantRange(DstTySize, /*isFullSet=*/true);

  return ConstantRange(Lower.trunc(DstTySize), Upper.trunc(DstTySize));
}

/// zextOrTrunc - make this range have the bit width given by \p DstTySize. The
/// value is zero extended, truncated, or left alone to make it that width.
ConstantRange ConstantRange::zextOrTrunc(uint32_t DstTySize) const {
  unsigned SrcTySize = getBitWidth();
  if (SrcTySize > DstTySize)
    return truncate(DstTySize);
  else if (SrcTySize < DstTySize)
    return zeroExtend(DstTySize);
  else
    return *this;
}

/// sextOrTrunc - make this range have the bit width given by \p DstTySize. The
/// value is sign extended, truncated, or left alone to make it that width.
ConstantRange ConstantRange::sextOrTrunc(uint32_t DstTySize) const {
  unsigned SrcTySize = getBitWidth();
  if (SrcTySize > DstTySize)
    return truncate(DstTySize);
  else if (SrcTySize < DstTySize)
    return signExtend(DstTySize);
  else
    return *this;
}

ConstantRange
ConstantRange::add(const ConstantRange &Other) const {
  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);
  if (isFullSet() || Other.isFullSet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  APInt Spread_X = getSetSize(), Spread_Y = Other.getSetSize();
  APInt NewLower = getLower() + Other.getLower();
  APInt NewUpper = getUpper() + Other.getUpper() - 1;
  if (NewLower == NewUpper)
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  ConstantRange X = ConstantRange(NewLower, NewUpper);
  if (X.getSetSize().ult(Spread_X) || X.getSetSize().ult(Spread_Y))
    // We've wrapped, therefore, full set.
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  return X;
}

ConstantRange
ConstantRange::sub(const ConstantRange &Other) const {
  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);
  if (isFullSet() || Other.isFullSet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  APInt Spread_X = getSetSize(), Spread_Y = Other.getSetSize();
  APInt NewLower = getLower() - Other.getUpper() + 1;
  APInt NewUpper = getUpper() - Other.getLower();
  if (NewLower == NewUpper)
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  ConstantRange X = ConstantRange(NewLower, NewUpper);
  if (X.getSetSize().ult(Spread_X) || X.getSetSize().ult(Spread_Y))
    // We've wrapped, therefore, full set.
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  return X;
}

ConstantRange
ConstantRange::multiply(const ConstantRange &Other) const {
  // TODO: If either operand is a single element and the multiply is known to
  // be non-wrapping, round the result min and max value to the appropriate
  // multiple of that element. If wrapping is possible, at least adjust the
  // range according to the greatest power-of-two factor of the single element.

  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);
  if (isFullSet() || Other.isFullSet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  APInt this_min = getUnsignedMin().zext(getBitWidth() * 2);
  APInt this_max = getUnsignedMax().zext(getBitWidth() * 2);
  APInt Other_min = Other.getUnsignedMin().zext(getBitWidth() * 2);
  APInt Other_max = Other.getUnsignedMax().zext(getBitWidth() * 2);

  ConstantRange Result_zext = ConstantRange(this_min * Other_min,
                                            this_max * Other_max + 1);
  return Result_zext.truncate(getBitWidth());
}

ConstantRange
ConstantRange::smax(const ConstantRange &Other) const {
  // X smax Y is: range(smax(X_smin, Y_smin),
  //                    smax(X_smax, Y_smax))
  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);
  APInt NewL = APIntOps::smax(getSignedMin(), Other.getSignedMin());
  APInt NewU = APIntOps::smax(getSignedMax(), Other.getSignedMax()) + 1;
  if (NewU == NewL)
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);
  return ConstantRange(NewL, NewU);
}

ConstantRange
ConstantRange::umax(const ConstantRange &Other) const {
  // X umax Y is: range(umax(X_umin, Y_umin),
  //                    umax(X_umax, Y_umax))
  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);
  APInt NewL = APIntOps::umax(getUnsignedMin(), Other.getUnsignedMin());
  APInt NewU = APIntOps::umax(getUnsignedMax(), Other.getUnsignedMax()) + 1;
  if (NewU == NewL)
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);
  return ConstantRange(NewL, NewU);
}

ConstantRange
ConstantRange::udiv(const ConstantRange &RHS) const {
  if (isEmptySet() || RHS.isEmptySet() || RHS.getUnsignedMax() == 0)
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);
  if (RHS.isFullSet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  APInt Lower = getUnsignedMin().udiv(RHS.getUnsignedMax());

  APInt RHS_umin = RHS.getUnsignedMin();
  if (RHS_umin == 0) {
    // We want the lowest value in RHS excluding zero. Usually that would be 1
    // except for a range in the form of [X, 1) in which case it would be X.
    if (RHS.getUpper() == 1)
      RHS_umin = RHS.getLower();
    else
      RHS_umin = APInt(getBitWidth(), 1);
  }

  APInt Upper = getUnsignedMax().udiv(RHS_umin) + 1;

  // If the LHS is Full and the RHS is a wrapped interval containing 1 then
  // this could occur.
  if (Lower == Upper)
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  return ConstantRange(Lower, Upper);
}

ConstantRange
ConstantRange::binaryAnd(const ConstantRange &Other) const {
  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);

  // TODO: replace this with something less conservative

  APInt umin = APIntOps::umin(Other.getUnsignedMax(), getUnsignedMax());
  if (umin.isAllOnesValue())
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);
  return ConstantRange(APInt::getNullValue(getBitWidth()), umin + 1);
}

ConstantRange
ConstantRange::binaryOr(const ConstantRange &Other) const {
  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);

  // TODO: replace this with something less conservative

  APInt umax = APIntOps::umax(getUnsignedMin(), Other.getUnsignedMin());
  if (umax.isMinValue())
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);
  return ConstantRange(umax, APInt::getNullValue(getBitWidth()));
}

ConstantRange
ConstantRange::shl(const ConstantRange &Other) const {
  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);

  APInt min = getUnsignedMin().shl(Other.getUnsignedMin());
  APInt max = getUnsignedMax().shl(Other.getUnsignedMax());

  // there's no overflow!
  APInt Zeros(getBitWidth(), getUnsignedMax().countLeadingZeros());
  if (Zeros.ugt(Other.getUnsignedMax()))
    return ConstantRange(min, max + 1);

  // FIXME: implement the other tricky cases
  return ConstantRange(getBitWidth(), /*isFullSet=*/true);
}

ConstantRange
ConstantRange::lshr(const ConstantRange &Other) const {
  if (isEmptySet() || Other.isEmptySet())
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);
  
  APInt max = getUnsignedMax().lshr(Other.getUnsignedMin());
  APInt min = getUnsignedMin().lshr(Other.getUnsignedMax());
  if (min == max + 1)
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);

  return ConstantRange(min, max + 1);
}

ConstantRange ConstantRange::inverse() const {
  if (isFullSet()) {
    return ConstantRange(getBitWidth(), /*isFullSet=*/false);
  } else if (isEmptySet()) {
    return ConstantRange(getBitWidth(), /*isFullSet=*/true);
  }
  return ConstantRange(Upper, Lower);
}

/// print - Print out the bounds to a stream...
///
void ConstantRange::print(raw_ostream &OS) const {
  if (isFullSet())
    OS << "full-set";
  else if (isEmptySet())
    OS << "empty-set";
  else
    OS << "[" << Lower << "," << Upper << ")";
}

/// dump - Allow printing from a debugger easily...
///
void ConstantRange::dump() const {
  print(dbgs());
}
