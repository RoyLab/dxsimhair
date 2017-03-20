#pragma once
#include <array>
#include <cassert>

namespace XR
{
	template <int N, class T, class PointT>
	inline T get(const PointT& pt)
	{
		return reinterpret_cast<const T*>(&pt)[N];
	}

	class BoundingBox
	{
	public:
        typedef double Scalar;
        BoundingBox() {}
        BoundingBox(Scalar a, Scalar b, Scalar c, Scalar d, Scalar e, Scalar f):
            rep{ a, b, c, d, e, f } {}

		template <class PointT>
		BoundingBox(const PointT& pt) { init(pt); }

		template <class ForwardIterator>
		BoundingBox(ForwardIterator a, ForwardIterator b);

        template <class ForwardIterator>
        BoundingBox(ForwardIterator a, ForwardIterator b, std::vector<uint32_t>& vec);

		template <class PointT>
		inline void init(const PointT& pt);

		template <class PointT>
		inline void include(const PointT& pt);

        inline void include_bbox(const BoundingBox& pt);

        template <class PointT>
        inline bool has_on_unbounded_side(const PointT& pt);

		double xmin() const { return rep[0]; }
		double ymin() const { return rep[1]; }
		double zmin() const { return rep[2]; }
		double xmax() const { return rep[3]; }
		double ymax() const { return rep[4]; }
		double zmax() const { return rep[5]; }

		double minVal(int id) const { assert(id < 3 && id >= 0); return rep[id]; }
        double maxVal(int id) const { assert(id < 3 && id >= 0); return rep[id + 3]; }
        template <class PointT> PointT min() const { return PointT(rep[0], rep[1], rep[2]); }
        template <class PointT> PointT max() const { return PointT(rep[3], rep[4], rep[5]); }

        template <class PointT> PointT center() const { return PointT((rep[3]+rep[0])/2, (rep[4]+rep[1])/2, (rep[5]+rep[2])/2); }
        template <class PointT> PointT diagonal() const { return PointT(rep[3] - rep[0], rep[4] - rep[1], rep[5] - rep[2]); }
		const Scalar* data() const { return rep.data(); }

        template <class PointT> bool isInclude_left_open_right_close(const PointT&) const;

	protected:
		std::array<Scalar, 6> rep;
	};


	template <class ForwardIterator>
	BoundingBox::BoundingBox(ForwardIterator a, ForwardIterator b)
	{
		assert(a != b);
		init(*a);
		while (++a != b)
			include(*a);
	}

    template <class ForwardIterator>
    BoundingBox::BoundingBox(ForwardIterator a, ForwardIterator b, std::vector<uint32_t>& vec)
    {
        assert(a != b);
        assert(vec.empty());
        uint32_t count = 0;
        init(*a);
        typedef typename std::remove_reference<decltype(*a)>::type::Scalar T;
        while (++a != b)
        {
            ++count;
            auto &pt = *a;
            if (xmin() >= get<0, T>(pt))
            {
                if (xmin() > get<0, T>(pt))
                    vec.clear();

                vec.push_back(count);
                rep[0] = get<0, T>(pt);
            }
            else if (xmax() < get<0, T>(pt))
                rep[3] = get<0, T>(pt);

            if (ymin() > get<1, T>(pt))
                rep[1] = get<1, T>(pt);
            else if (ymax() < get<1, T>(pt))
                rep[4] = get<1, T>(pt);

            if (zmin() > get<2, T>(pt))
                rep[2] = get<2, T>(pt);
            else if (zmax() < get<2, T>(pt))
                rep[5] = get<2, T>(pt);
        }
    }

	template <class PointT>
	void BoundingBox::init(const PointT& pt)
	{
		typedef typename PointT::Scalar T;
		rep[0] = get<0, T>(pt);
		rep[1] = get<1, T>(pt);
		rep[2] = get<2, T>(pt);
		rep[3] = get<0, T>(pt);
		rep[4] = get<1, T>(pt);
		rep[5] = get<2, T>(pt);
	}

	template <class PointT>
	void BoundingBox::include(const PointT& pt)
	{
		typedef typename PointT::Scalar T;
		if (xmin() > get<0, T>(pt))
			rep[0] = get<0, T>(pt);
		else if (xmax() < get<0, T>(pt))
			rep[3] = get<0, T>(pt);

		if (ymin() > get<1, T>(pt))
			rep[1] = get<1, T>(pt);
		else if (ymax() < get<1, T>(pt))
			rep[4] = get<1, T>(pt);

		if (zmin() > get<2, T>(pt))
			rep[2] = get<2, T>(pt);
		else if (zmax() < get<2, T>(pt))
			rep[5] = get<2, T>(pt);
	}

    template<class PointT>
    bool BoundingBox::isInclude_left_open_right_close(const PointT& pt) const
    {
        typedef typename PointT::Scalar T;

        if ((get<0, T>(pt) >= rep[0]) && (get<0, T>(pt) < rep[3])
            && (get<1, T>(pt) >= rep[1]) && (get<1, T>(pt) < rep[4])
            && (get<2, T>(pt) >= rep[2]) && (get<2, T>(pt) < rep[5]))
            return true;

        else return false;
    }

    void BoundingBox::include_bbox(const BoundingBox& bbox)
    {
        typedef BoundingBox::Scalar T;
        if (xmin() > bbox.xmin())
            rep[0] = bbox.xmin();

        if (xmax() < bbox.xmax())
            rep[3] = bbox.xmax();

        if (ymin() > bbox.ymin())
            rep[1] = bbox.ymin();

        if (ymax() < bbox.ymax())
            rep[4] = bbox.ymax();

        if (zmin() > bbox.zmin())
            rep[2] = bbox.zmin();

        if (zmax() < bbox.zmax())
            rep[5] = bbox.zmax();
    }


    template <class PointT>
    bool BoundingBox::has_on_unbounded_side(const PointT& pt)
    {
        typedef typename PointT::Scalar T;

        if ((get<0, T>(pt) < rep[0]) && (get<0, T>(pt) > rep[3])
            && (get<1, T>(pt) < rep[1]) && (get<1, T>(pt) > rep[4])
            && (get<2, T>(pt) < rep[2]) && (get<2, T>(pt) > rep[5]))
            return true;

        return false;
    }

	template <class PointT>
	void normalizeCoords(const PointT& c, const PointT& d, PointT& pt)
	{
		pt -= c;
		pt /= d / 2;
	}


    template <class PointT>
    void invCoords(const PointT& c, const PointT& d, PointT& pt)
    {
        pt *= d / 2;
        pt += c;
    }

}