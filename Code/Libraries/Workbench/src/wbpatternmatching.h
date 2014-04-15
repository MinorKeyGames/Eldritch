#ifndef WBPATTERNMATCHING_H
#define WBPATTERNMATCHING_H

#include "array.h"
#include "wbparamevaluator.h"

class WBRule;
class WBEvent;

namespace WBPatternMatching
{
	// Returns true if a match is found. OutIndex is the index into Rules of the best matching rule.
	bool	SingleCompare( const Array<WBRule>& Rules, const WBEvent& Event, const WBParamEvaluator::SPEContext& PEContext, uint& OutIndex );

	// Returns true if any matches are found. OutIndices is indices into Rules of the matching rules.
	bool	MultiCompare( const Array<WBRule>& Rules, const WBEvent& Event, const WBParamEvaluator::SPEContext& PEContext, Array<uint>& OutIndices );

	// Returns true if the rule matches.
	bool	Compare( const WBRule& Rule, const WBEvent& Event, const WBParamEvaluator::SPEContext& PEContext );
}

#endif // WBPATTERNMATCHING_H