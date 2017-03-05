#pragma once
#include <boost/log/trivial.hpp>
#include <vector>
#include <iostream>

#include "macros.h"

namespace xhair
{
	static uint32_t stat1, stat2;
	const uint32_t INVALID_U32 = 0xffffffff;
	namespace traits
	{
		template<typename PointT, int D>
		struct access
		{
		};

		template<class PointT>
		struct access<PointT, 0>
		{
			static float get(const PointT& p)
			{
				return p.x;
			}
		};

		template<class PointT>
		struct access<PointT, 1>
		{
			static float get(const PointT& p)
			{
				return p.y;
			}
		};

		template<class PointT>
		struct access<PointT, 2>
		{
			static float get(const PointT& p)
			{
				return p.z;
			}
		};

	}

	/** convenience function for access of point coordinates **/
	template<int D, typename PointT>
	inline float get(const PointT& p)
	{
		return traits::access<PointT, D>::get(p);
	}


	template <class PointT, class ContainerT=std::vector<PointT>>
	class GridRaster
	{
		const int MASK26[26][3] = { {1,1,1},{ -1,1,1 },{ 1,-1,1 },{ -1,-1,1 },
		{ 1,0,1 },{ -1,0,1 },{ 0,-1,1 },{ 0,1,1 },{ 0,0,1 },
		{ 1,1,0 },{ -1,1,0 },{ 1,-1,0 },{ -1,-1,0 },
		{ 1,0,0 },{ -1,0,0 },{ 0,-1,0 },{ 0,1,0 },
		{ 1,1,-1 },{ -1,1,-1 },{ 1,-1,-1 },{ -1,-1,-1 },
		{ 1,0,-1 },{ -1,0,-1 },{ 0,-1,-1 },{ 0,1,-1 },{ 0,0,-1 } };

		typedef decltype(get<1>(PointT(0, 0, 0))) T;
		struct Cube
		{
			uint32_t start, end;    // start and end in succ_

			//uint32_t id[3];
			//uint32_t size = 0;    // number of points
		};

		class CubeIterator
		{
		public:
			CubeIterator(Cube& cube, const std::vector<uint32_t>& succ)
			{
				ptr = &cube;
				succ_ = &succ;
				cur = cube.start;
				//end = succ_->size()-1;
			}

			bool isValid() const { return ptr->start != INVALID_U32; }
			//bool validate(uint32_t id) const { return id != end; }

			uint32_t next()
			{
				uint32_t res = cur;
				cur = succ_->at(cur);
				return res;
			}

		private:
			uint32_t cur;// = INVALID_U32, end;
			const Cube* ptr;
			const std::vector<uint32_t>* succ_;
		};

		typedef uint8_t count_t;

	public:
		GridRaster(T dr)
		{
			const double rfactor = 1.01;
			r0 = dr;
			offset = rfactor * dr;
		}


		~GridRaster() { freeAlloc(); }

		void freeAlloc()
		{
			if (data_) data_ = nullptr;

			//if (size_) SAFE_DELETE_ARRAY(size_);
			//if (start_) SAFE_DELETE_ARRAY(start_);
			//if (end_) SAFE_DELETE_ARRAY(end_);

			if (grid_) SAFE_DELETE_ARRAY(grid_);
			successors_.clear();
			pointMap_.clear();
			flag_.clear();
		}

		void reset()
		{
			//memset(size_, 0, sizeof(count_t)*n*n*n);
			//memset(start_, 0xff, sizeof(uint32_t)*n*n*n);
			for (uint32_t i = 0; i < N; ++i)
			{
				// initially each element links simply to the following element.
				successors_[i] = i + 1;
				flag_[i] = -1;
			}
			successors_[N] = N;
			memset(grid_, 0xff, sizeof(Cube)*nTotal);
		}

		void initialize(const ContainerT& pts)
		{
			N = pts.size();

			if (!b_init)
			{
				b_init = true;

				successors_.resize(N + 1);
				pointMap_.resize(N);
				flag_.resize(N);
			}

			assert(N == pointMap_.size());

			// bounding box
			T min[3], max[3];
			min[0] = get<0>(pts[0]);
			min[1] = get<1>(pts[0]);
			min[2] = get<2>(pts[0]);
			max[0] = min[0];
			max[1] = min[1];
			max[2] = min[2];

			for (uint32_t i = 0; i < N; ++i)
			{
				const PointT& p = pts[i];

				if (get<0>(p) < min[0]) min[0] = get<0>(p);
				if (get<1>(p) < min[1]) min[1] = get<1>(p);
				if (get<2>(p) < min[2]) min[2] = get<2>(p);
				if (get<0>(p) > max[0]) max[0] = get<0>(p);
				if (get<1>(p) > max[1]) max[1] = get<1>(p);
				if (get<2>(p) > max[2]) max[2] = get<2>(p);
			}

			for (int i = 0; i < 3; i++)
				center_[i] = (min[i] + max[i]) / 2;


			T extent[3];
			for (uint32_t i = 0; i < 3; ++i)
				extent[i] = 0.5f * (max[i] - min[i]);

			if (!grid_ || extent[0] > extent_[0] |
				extent[1] > extent_[1] || extent[2] > extent_[2])
			{
				const double boxfactor = 1.1;

				SAFE_DELETE_ARRAY(grid_);
				for (uint32_t i = 0; i < 3; i++)
				{
					n[i] = int(extent[i] * 2 * boxfactor / offset) + 1;
					extent_[i] = n[i] * offset / 2 ;
				}

				nRow = n[0];
				nLayer = n[1] * n[0];
				nTotal = nLayer * n[2];
				grid_ = new Cube[nTotal];

				BOOST_LOG_TRIVIAL(trace) << "Reallocate happened";
			}

			reset();
			assginPoints(pts, center_, extent_);
		}

		template<class ResContainerT>
		void query(ResContainerT& res0, ResContainerT& res1, bool filter, ResContainerT* old0, ResContainerT* old1,
			ResContainerT& p0, ResContainerT& p1, ResContainerT& n0, ResContainerT& n1)
		{
			size_t _count[5] = { 0 };

			p0.clear();
			p1.clear();
			n0.clear();
			n1.clear();

			//const int max_neighbor = 2;

			const uint32_t N = this->N;

			T dr2 = r0 * r0;
			std::vector<uint32_t> locals;
			size_t oldPtr = 0;
			for (uint32_t i = 0; i < N; i++)
			{
				auto c0 = pointMap_[i];
				Cube &cell = grid_[id(c0.data())];
				assert(cell.start != INVALID_U32);

				locals.clear();

				CubeIterator itr(cell, successors_);
				uint32_t id2;

				//int tmp = max_neighbor;
				//bool tmpFlag = true;

				while (oldPtr < old0->size() && old0->at(oldPtr) == i)
				{
					auto id2 = old1->at(oldPtr);
					flag_[id2] = i;

					if (closeEnough(i, id2, dr2))
					{
						res0.push_back(i);
						res1.push_back(id2);
						//tmp--;
					}
					else
					{
						n0.push_back(i);
						n1.push_back(id2);
						_count[0]++;
					}
					oldPtr++;
				}

				do
				{
					id2 = itr.next();
					if (id2 == N) break;
					locals.push_back(id2);
				} while (1);

				for (auto i1 = locals.begin(); i1 != locals.end(); i1++)
				{
					if ((!filter || validPair(i, *i1)) && flag_[*i1] != i && closeEnough(i, *i1, dr2*0.5))
					{
						res0.push_back(i);
						res1.push_back(*i1);

						p0.push_back(i);
						p1.push_back(*i1);

						//if (--tmp <= 0)
						//{
						//	tmpFlag = false;
						//	break;
						//}	
						_count[1]++;
					}
				}

				//if (!tmpFlag) continue;

				// 26 neighbor
				for (int j = 0; j < 26; j++)
				{
					uint32_t shift[3] = { c0[0] + MASK26[j][0],
						c0[1] + MASK26[j][1], c0[2] + MASK26[j][2] };
					if (!validId(shift)) continue;
					auto& cell2 = grid_[id(shift)];
					if (cell2.start != INVALID_U32)
					{
						_count[3]++;
						CubeIterator itr(cell2, successors_);
						do
						{
							_count[4]++;
							id2 = itr.next();
							if (id2 == N) break;
							if ((!filter || validPair(i, id2)) && flag_[id2] != i && closeEnough(i, id2, dr2*0.5))
							{
								_count[2]++;

								res0.push_back(i);
								res1.push_back(id2);

								p0.push_back(i);
								p1.push_back(id2);

								//if (--tmp <= 0)
								//{
								//	tmpFlag = false;
								//	break;
								//}
								_count[1]++;
							}
						} while (1);

						//if (!tmpFlag)
						//{
						//	break;
						//}
					}
				}
			}
			BOOST_LOG_TRIVIAL(info) << "\tUpdate: " << _count[0] << '/' << _count[1] << '/' << res0.size();
			BOOST_LOG_TRIVIAL(info) << "\t" << _count[2] << '/' << _count[3] << '/' << _count[4];

		}

		void stat()
		{
			int count = 0, a=0, b=0;
			for (int i = 0; i < n[0]; i++)
			{
				for (int j = 0; j < n[1]; j++)
				{
					for (int k = 0; k < n[2]; k++)
					{
						auto sz = getSize(i, j, k);
						a += sz;
						if (b <sz) b = sz;
						if (sz) count++;
					}
				}
			}
			std::cout << "Number of occupied cell: " << count << std::endl;
			std::cout << "Number of points: " << a << std::endl;
			std::cout << "Maximum number of point in a cell: " << b << std::endl;
		}

		count_t getSize(uint32_t x, uint32_t y, uint32_t z) const
		{
			CubeIterator itr(grid_[id(x,y,z)], successors_);
			count_t count = 0;
			if (itr.isValid())
				while (itr.next() != N) count++;
			return count;
		}

		uint32_t id(uint32_t* c) const
		{
			return id(c[0], c[1], c[2]);
		}

		uint32_t id(uint32_t x, uint32_t y, uint32_t z) const
		{
			return nLayer*x + nRow*y + z;
		}

		template<class ResContainerT>
		uint32_t checkPairs(ResContainerT& res0, ResContainerT& res1)
		{
			T dr2 = r0*r0;
			uint32_t res = 0u;
			for (uint32_t i = 0; i < res0.size(); i++)
			{
				if (!closeEnough(res0[i], res1[i], dr2))
				{
					BOOST_LOG_TRIVIAL(debug) << get<0>(data_->at(res0[i])) << ", " << get<1>(data_->at(res0[i])) << ", " << get<2>(data_->at(res0[i])) << '\t||'
						<< get<0>(data_->at(res1[i])) << ", " << get<1>(data_->at(res1[i])) << ", " << get<2>(data_->at(res1[i])) << "\t|| "
						<< std::sqrt(squaredDist(data_->at(res0[i]), data_->at(res1[i]))) << std::endl;
					res++;
				}
			}
			return res;
		}
		
	private:

		T squaredDist(const PointT& pa, const PointT& pb) const
		{
			T tmp[3];
			tmp[0] = get<0>(pa) - get<0>(pb);
			tmp[1] = get<1>(pa) - get<1>(pb);
			tmp[2] = get<2>(pa) - get<2>(pb);
			return tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2];
		}

		bool closeEnough(uint32_t a, uint32_t b, float dr2) const
		{
			const PointT& pa = data_->at(a);
			const PointT& pb = data_->at(b);
			T tmp[3];
			tmp[0] = get<0>(pa) - get<0>(pb);

			dr2 -= tmp[0] * tmp[0];
			if (dr2 > 0)
			{
				tmp[1] = get<1>(pa) - get<1>(pb);
				tmp[2] = get<2>(pa) - get<2>(pb);
				dr2 -= tmp[1] * tmp[1] + tmp[2] * tmp[2];
				if (dr2 > 0)
					return true;
			}
			return false;
		}

		bool validPair(uint32_t a, uint32_t b) const
		{
			return a < b;
		}

		bool validId(uint32_t* a) const { return validId(a[0], a[1], a[2]); }

		bool validId(uint32_t x, uint32_t y, uint32_t z) const
		{
			return x < n[0] && y < n[1] && z < n[2];
		}

		void assginPoints(const ContainerT& pts, T* center, T* extent)
		{
			// @ extend is half length, radius
			for (int i = 0; i < 3; i++)
				min_[i] = center[i] - extent[i];

			assert(N == pts.size());
			data_ = &pts;
			const ContainerT& points = pts;
			for (uint32_t i = 0; i < N; ++i)
			{
				const PointT& p = points[i];

				// determine Morton code for each point...
				uint32_t code[3];
				code[0] = int((get<0>(p) - min_[0]) / offset);
				code[1] = int((get<1>(p) - min_[1]) / offset);
				code[2] = int((get<2>(p) - min_[2]) / offset);

				// set child starts and update successors...
				auto tmp = id(code);
				for (int i1 = 0; i1 < 3; i1++)
					pointMap_[i][i1] = code[i1];

				auto& node = grid_[tmp];
				if (node.start > 99999999)
					node.start = i;
				else
					successors_[node.end] = i;

				//size_[tmpId] ++;
				node.end = i;
				successors_[node.end] = N;
			}
		}

	private:
		std::vector<uint32_t> successors_;    // single connected list of next point indices...
		std::vector<std::array<uint32_t, 3>> pointMap_;
		std::vector<int> flag_;

		 const ContainerT* ExternPtr data_ = nullptr;
		Cube * grid_ = nullptr;

		T offset,center_[3], r0, extent_[3];
		uint32_t n[3], nTotal, nLayer, nRow, N;

		T min_[3];

		bool b_init = false;
	};
}