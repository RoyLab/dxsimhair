#pragma once
#include <vector>
#include <cassert>
#include <algorithm>

namespace XR
{
	template <class T, bool Owner = false>
	class ArrayWrapper
	{
	public:
		ArrayWrapper(T* arr, size_t sz) :
			m_data(arr), m_size(sz) {}

        virtual ~ArrayWrapper()
        {
            if (Owner && m_data)
            {
                delete[]m_data;
                m_data = nullptr;
            }
        }

		size_t size() const { return m_size; }
		T& operator[](int idx) { return m_data[idx]; }
		const T& operator[](int idx) const { return m_data[idx]; }

		T& at(int idx) { return m_data[idx]; }
		const T& at(int idx) const { return m_data[idx]; }
	private:
		const size_t m_size;
		T* m_data;
	};

    template <class T>
    class RecursiveVector:
        public std::vector<T>
    {
        typedef std::vector<T> Base;
    public:
        RecursiveVector() {}
        virtual ~RecursiveVector() {}

        const_reference operator[](size_type _Pos) const { return Base::operator[](_Pos % size()); }
        reference operator[](size_type _Pos) { return Base::operator[](_Pos % size()); }
    };

    template <class T, size_t BlockSize = 4>
    class BlockDeque
    {
        struct Block
        {
            T data[BlockSize];
            Block* next;
        };

        typedef size_t size_type;

    public:
        BlockDeque() : head_(nullptr) {}
        virtual ~BlockDeque()
        {
            Block* cur_block = head_;
            while (cur_block)
            {
                Block* del_block = cur_block;
                cur_block = cur_block->next;
                delete del_block;
            }
        }

        size_type size() const { return size_; }
        T& at(size_type idx)
        {
#ifdef _DEBUG
            assert(idx < size_);
#endif
            lldiv_t loc = std::lldiv(idx, BlockSize);
            return indices_[loc.quot]->data[loc.rem];
        }
        //TODO


    protected:
        std::vector<Block*> indices_;
        Block *head_, *tail;
        size_type capacity_, size_;
    };

    // invalidate the iterator of the last element
    // if a set of elements need to be removed, sort should be conducted and remove from the back
    template<class Container>
    void vec_quick_delete(typename Container::iterator itr, Container& vec)
    {
        std::swap(vec.back(), *itr);
        vec.pop_back();
    }

    template<class Container>
    void vec_quick_delete(typename Container::size_type i, Container& vec)
    {
        std::swap(vec.back(), vec[i]);
        vec.pop_back();
    }

    template <class OuterItr, class InnerItr, void (*Assign)(OuterItr, InnerItr&, InnerItr&)>
    class DoubleIterator
    {
    public:
        DoubleIterator(const OuterItr& begin, const OuterItr& end)
        {
            invalid = false;
            outer_cur_ = begin;
            outer_end_ = end;
            assignInnerPtr();
        }

        DoubleIterator& operator++()
        {
            ++inner_cur_;
            if (inner_cur_ == inner_end_)
            {
                ++outer_cur_;
                assignInnerPtr();
            }
            return *this;
        }

        typename InnerItr::value_type* operator->() const
        {
            return inner_cur_.operator->();
        }

        typename InnerItr::value_type* pointer() const
        {
            return &*inner_cur_;
        }
        
        operator bool() const
        {
            return outer_cur_ != outer_end_;
        }

    private:
        void assignInnerPtr()
        {
            while (outer_cur_ != outer_end_)
            {
                Assign(outer_cur_, inner_cur_, inner_end_);
                if (inner_cur_ != inner_end_)
                {
                    break;
                }
                ++outer_cur_;
            }
        }

        OuterItr outer_end_;
        OuterItr outer_cur_;

        InnerItr inner_end_;
        InnerItr inner_cur_;
        bool     invalid;
    };
}

namespace std
{
    template<class _RanIt,
        class _Pr> inline
        pair<_RanIt, _RanIt>
        my_guess(_RanIt _First, _RanIt _Last, _Pr& _Pred)
    {
        _RanIt Mid = _First;
        --_Last;
        while (Mid != _Last)
        {
            _RanIt Comp = Mid; ++Comp;
            if (!_Pred(*Mid, *Comp))
            {
                std::swap(*Mid, *Comp);
                std::swap(Mid, Comp);
            }
            else
            {
                std::swap(*Comp, *_Last);
                --_Last;
            }
        }
        _RanIt Mid_1 = Mid; ++Mid_1;
        return pair<_RanIt, _RanIt>(Mid, Mid_1);
    }

    template<class _RanIt,
        class _Diff,
        class _Pr> inline
        void _Sort_unchecked2(_RanIt _First, _RanIt _Last, _Diff _Ideal, _Pr& _Pred)
    {	// order [_First, _Last), using _Pred
        _Diff _Count;
        while (1 < (_Count = _Last - _First))
        {	// divide and conquer by quicksort
            pair<_RanIt, _RanIt> _Mid =
                my_guess(_First, _Last, _Pred);

            if (_Mid.first - _First < _Last - _Mid.second)
            {	// loop on second half
                _Sort_unchecked2(_First, _Mid.first, _Ideal, _Pred);
                _First = _Mid.second;
            }
            else
            {	// loop on first half
                _Sort_unchecked2(_Mid.second, _Last, _Ideal, _Pred);
                _Last = _Mid.first;
            }
        }
    }

    template<class _RanIt,
        class _Pr> inline
        void _Sort_unchecked22(_RanIt _First, _RanIt _Last, _Pr& _Pred)
    {	// order [_First, _Last), using _Pred
        _Sort_unchecked2(_First, _Last, _Last - _First, _Pred);
    }

    template<class _RanIt,
        class _Pr> inline
        void quicksort(_RanIt _First, _RanIt _Last, _Pr _Pred)
    {	// order [_First, _Last), using _Pred
        _DEBUG_RANGE(_First, _Last);
        _Sort_unchecked22(_Unchecked(_First), _Unchecked(_Last), _Pred);
    }
}
