#pragma once
#include <boost/log/trivial.hpp>
#include <vector>
#include <iostream>

#include "macros.h"

namespace XRwy
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
		GridRaster(const ContainerT& pts, T dr, double factor=1.05)
		{
			data_ = &pts;

			const uint32_t N = pts.size();
			this->N = N;
			successors_ = std::vector<uint32_t>(N+1);
			pointMap_ = std::vector<uint32_t[3]>(N);

			// determine axis-aligned bounding box.
			T min[3], max[3];
			min[0] = get<0>(pts[0]);
			min[1] = get<1>(pts[0]);
			min[2] = get<2>(pts[0]);
			max[0] = min[0];
			max[1] = min[1];
			max[2] = min[2];

			for (uint32_t i = 0; i < N; ++i)
			{
				// initially each element links simply to the following element.
				successors_[i] = i + 1;
				const PointT& p = pts[i];

				if (get<0>(p) < min[0]) min[0] = get<0>(p);
				if (get<1>(p) < min[1]) min[1] = get<1>(p);
				if (get<2>(p) < min[2]) min[2] = get<2>(p);
				if (get<0>(p) > max[0]) max[0] = get<0>(p);
				if (get<1>(p) > max[1]) max[1] = get<1>(p);
				if (get<2>(p) > max[2]) max[2] = get<2>(p);
			}
			successors_[N] = N;

			//float ctr[3] = { 0 };
			for (int i = 0; i < 3; i++)
				ctr[i] = (min[i] + max[i]) / 2;

			T maxextent = 0.5f * (max[0] - min[0]);
			for (uint32_t i = 1; i < 3; ++i)
			{
				T extent = 0.5f * (max[i] - min[i]);
				if (extent > maxextent) maxextent = extent;
			}

			offset = factor * dr;
			r0 = dr;
			n = int(maxextent * 2 / offset) + 1;
			n2 = n*n;
			maxextent = n * offset / 2;

			grid_ = new Cube[n*n*n];
			memset(grid_, 0xff, sizeof(Cube)*n*n*n);

			//size_ = new count_t[n*n*n];
			//memset(size_, 0, sizeof(count_t)*n*n*n);

			//start_ = new uint32_t[n*n*n];
			//memset(start_, 0xff, sizeof(uint32_t)*n*n*n);

			//end_ = new uint32_t[n*n*n];
			//memset(end_, 0, sizeof(uint32_t)*n*n*n);
		}

		~GridRaster() { free(); }

		void free()
		{
			if (data_) data_ = nullptr;

			//if (size_) SAFE_DELETE_ARRAY(size_);
			//if (start_) SAFE_DELETE_ARRAY(start_);
			//if (end_) SAFE_DELETE_ARRAY(end_);

			if (grid_) SAFE_DELETE_ARRAY(grid_);
			successors_.clear();
			pointMap_.clear();
		}

		void reset()
		{
			//memset(size_, 0, sizeof(count_t)*n*n*n);
			//memset(start_, 0xff, sizeof(uint32_t)*n*n*n);
			memset(grid_, 0xff, sizeof(Cube)*n*n*n);
		}

		template<class ResContainerT>
		void query(ResContainerT& res0, ResContainerT& res1, bool filter = false)
		{
			const uint32_t N = this->N;
			std::vector<bool> flag(N, false);

			T dr2 = r0 * r0;
			std::vector<uint32_t> locals;
			for (uint32_t i = 0; i < N; i++)
			{
				//if (flag[i]) continue;

				auto c0 = pointMap_[i];
				auto &cell = grid_[id(c0)];
				assert(cell.start != INVALID_U32);

				locals.clear();

				CubeIterator itr(cell, successors_);
				uint32_t id2;
				do
				{
					id2 = itr.next();
					if (id2 == N) break;
					locals.push_back(id2);
					//flag[id2] = true;
				} while (1);


				for (auto i1 = locals.begin(); i1 != locals.end(); i1++)
				{
					if ((!filter || validPair(i, *i1)) && closeEnough(i, *i1, dr2))
					{
						res0.push_back(i);
						res1.push_back(*i1);
					}
				}

				// 26 neighbor
				for (int j = 0; j < 26; j++)
				{
					uint32_t shift[3] = { c0[0] + MASK26[j][0],
						c0[1] + MASK26[j][1], c0[2] + MASK26[j][2] };
					if (!validId(shift)) continue;
					auto& cell2 = grid_[id(shift)];
					if (cell2.start != INVALID_U32)
					{
						CubeIterator itr(cell2, successors_);
						do
						{
							id2 = itr.next();
							if (id2 == N) break;
							if ((!filter || validPair(i, id2)) && closeEnough(i, id2, dr2))
							{
								res0.push_back(i);
								res1.push_back(id2);
							}
						} while (1);
					}
				}
			}
		}

		void createGrid()
		{
			createGrid(ctr[0], ctr[1], ctr[2], n * offset / 2);
		}

		void stat()
		{
			int count = 0, a=0, b=0;
			for (int i = 0; i < n; i++)
			{
				for (int j = 0; j < n; j++)
				{
					for (int k = 0; k < n; k++)
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
			return n2*x + n*y + z;
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
			bool res = squaredDist(pa, pb) <= dr2;
			//if (res) stat1++;
			//else stat2++;
			return res;
		}

		bool validPair(uint32_t a, uint32_t b) const
		{
			return a < b;
		}

		bool validId(uint32_t* a) const { return validId(a[0], a[1], a[2]); }

		bool validId(uint32_t x, uint32_t y, uint32_t z) const
		{
			return x < n && y < n && z < n;
		}

		void createGrid(float x, float y, float z, float extent)
		{
			// @ extend is half length, radius
			sl_ = extent * 2;
			min_[0] = x - extent;
			min_[1] = y - extent;
			min_[2] = z - extent;

			static const float factor[] = { -0.5f, 0.5f };
			const uint32_t size = data_->size();

			const ContainerT& points = *data_;
			for (uint32_t i = 0; i < size; ++i)
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
				successors_[node.end] = size;
			}
		}

	private:
		std::vector<uint32_t> successors_;    // single connected list of next point indices...
		std::vector<uint32_t[3]> pointMap_;

		const ContainerT* data_ = nullptr;
		Cube * grid_ = nullptr;

		T offset,ctr[3], r0;
		uint32_t n, n2, N;

		//count_t *size_ = nullptr;
		//uint32_t *start_ = nullptr, *end_ = nullptr;

		T min_[3], sl_;
	};
}