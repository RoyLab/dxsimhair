#pragma once

namespace XRwy
{
template<class T>
class SparseLinkVector
{
public:
	class Iterator
	{
	public:
		Iterator(SparseLinkVector& c);
		~Iterator();

		size_t index() const;
		Iterator& operator++();
		Iterator& operator++(int) { ++(*this); }
	};

public:
	SparseLinkVector(size_t n);
	~SparseLinkVector();



private:
	T *data = 0;
	size_t *jump = 0;
	size_t first;

};
}
