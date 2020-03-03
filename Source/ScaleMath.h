/*
  ==============================================================================

    ScaleMath.h
    Created: 22 Feb 2020 7:13:35pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

const int CF_MAXDEPTH = 20;
const double CF_ROUND0THRESH = 10e-8;

/* 
	Returns the Euclidian version of modulo, where all results are positive
	ie. -7 % 5 = 3 
*/
static int modulo(int numIn, int mod)
{
	if (mod != 0)
		return ((numIn % mod) + mod) % mod;

	return 0;
}

static Array<int> getGeneratorChain(int generator, int period)
{
	Array<int> chain;

	for (int i = 0; i < period; i++)
		chain.add(modulo(generator * i, period));

	return chain;
}

static Array<int> getCoprimes(int numIn, bool onlyFirstHalf=false)
{
	Array<int> coprimes = { 1 };
	int div, mod;
	int max = onlyFirstHalf ? (numIn / 2 + 1) : numIn;
	for (int i = 2; i < max; i++)
	{
		div = numIn;
		mod = i;
		while (mod > 1)
		{
			div %= mod;
			div += mod;
			mod = div - mod;
			div = div - mod;
		}

		if (mod > 0)
		{
			coprimes.add(i);
		}
	}

	return coprimes;
}

/*
	Returns an array of digits representing the continued fraction expansion of a given proportion
*/
static Array<int> getContinuedFraction(double fractionIn, int maxDepth = CF_MAXDEPTH, double round0Thresh = CF_ROUND0THRESH)
{
	Array<int> cf;
	double f = fractionIn;

	for (int i = 0; i < maxDepth; i++)
	{
		cf.add((int)f);
		f = f - cf[i];
		if (f > round0Thresh)
			f = 1.0 / f;
		else
			break;
	}

	return cf;
}

// TODO: make with template class
/*
	Takes an array and tries to arrange it symmetrically, such that the array can be wrapped into a symmetric circle
	If array cannot be arranged symmetrically, function will return an empty array
	This algorithm prefers higher values closer to the center (the "top" of the circle).
	ex. [7, 7, 7, 5, 5] => [7, 7, 5, 5, 7]
*/
static Array<int> arrangeSymmetrically(Array<int> arrayOfValues, int centerIndex = 0)
{
	Array<int> symmetricArray = { arrayOfValues[centerIndex] };
	arrayOfValues.remove(centerIndex);

	int arrangeSize = arrayOfValues.size();
	bool isOdd = arrangeSize % 2;
	Array<Point<int>> groupings; // [(unique value, number of appearances),...]
	Array<int> values;

	// find unique values and how many times they appear
	for (auto num : arrayOfValues)
	{
		int valInd = values.indexOf(num);

		if (valInd < 0)
		{
			groupings.add(Point<int>(num, 1));
			values.add(num);
		}
		else
		{
			Point<int>& pt = groupings.getReference(valInd);
			pt.setY(pt.y + 1);
		}
	}

	// already symmetric if only one type of value
	if (groupings.size() == 1)
	{
		symmetricArray.addArray(arrayOfValues);
		return symmetricArray;
	}

	Array<int> oddGroupIdx;
	for (int i = 0; i < values.size(); i++)
	{
		if (groupings[i].y % 2 == 1)
			oddGroupIdx.add(i);
	}

	// determine if symmetry is possible
	// odd needs ONE odd group, even needs ZERO odd groups
	if ((isOdd && oddGroupIdx.size() != 1) || (!isOdd && oddGroupIdx.size() > 0))
	{
		return Array<int>();
	}

	int oddGroupValue;
	if (isOdd)
		oddGroupValue = values[oddGroupIdx[0]];

	// sort by increasing value, but we will use this in reverse
	Array<int> sortedValues = values;
	sortedValues.sort();

	// move odd group to first index
	if (isOdd)
	{
		sortedValues.removeFirstMatchingValue(oddGroupValue);
		sortedValues.insert(0, oddGroupValue);
	}

	// rebuild groupings with sorted order
	Array<Point<int>> sortedGroupings;
	for (auto val : sortedValues)
	{
		sortedGroupings.add(groupings.getReference(values.indexOf(val)));
	}

	// populate symmetric array
	int i = 0;
	int valInd = values.size() - 1;
	bool increment = false;
	while (i < arrayOfValues.size())
	{
		int num = sortedGroupings.getReference(valInd).y;
		if (valInd != 0)
			num = num / 2;
		else
			increment = true;

		for (int n = 0; n < num; n++)
		{
			symmetricArray.add(sortedValues[valInd]);
			i++;
		}

		if (increment)
			valInd++;
		else
			valInd--;
	}

	return symmetricArray;
}

template <class T>
static bool isSymmetric(Array<T> arrayIn)
{
	int indNext, indPrev, symCount;
	int halfSize = arrayIn.size();

	for (int i = 0; i < arrayIn.size(); i++)
	{
		symCount = 0;
		for (int j = 1; j <= halfSize; j++)
		{
			indNext = (i + j) % arrayIn.size();
			indPrev = (i - j);
			indPrev += indPrev < 0 ? arrayIn.size() : 0;

			if (arrayIn[indNext] == arrayIn[indPrev])
			{
				symCount++;
			}
		}

		if (symCount == halfSize)
			return true;
	}
	return false;
}