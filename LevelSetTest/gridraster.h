#pragma once
#include <macros.h>
#include <vector>
#include <iostream>

namespace XRwy
{
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
		//struct Cube
		//{
		//	//uint32_t id[3];

		//	uint32_t start, end;    // start and end in succ_
		//	//uint32_t size = 0;    // number of points
		//};

		typedef uint8_t count_t;

	public:
		GridRaster(const ContainerT& pts, float dr)
		{
			data_ = &pts;

			const uint32_t N = pts.size();
			successors_ = std::vector<uint32_t>(N);

			// determine axis-aligned bounding box.
			float min[3], max[3];
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

			//float ctr[3] = { 0 };
			for (int i = 0; i < 3; i++)
				ctr[i] = (min[i] + max[i]) / 2;

			float maxextent = 0.5f * (max[0] - min[0]);
			for (uint32_t i = 1; i < 3; ++i)
			{
				float extent = 0.5f * (max[i] - min[i]);
				if (extent > maxextent) maxextent = extent;
			}

			offset = 1.05 * dr;
			n = int(maxextent * 2 / offset) + 1;
			n2 = n*n;
			maxextent = n * offset / 2;

			//grid_ = new Cube**[n];
			//for (int i = 0; i < n; i++)
			//{
			//	grid_[i] = new Cube*[n];
			//	for (int j = 0; j < n; j++)
			//	{
			//		grid_[i][j] = new Cube[n];
			//		//for (int k = 0; k < n; k++)
			//		//{
			//		//	auto cptr = &grid_[i][j][k];
			//		//	cptr->id[0] = i;
			//		//	cptr->id[1] = j;
			//		//	cptr->id[2] = k;
			//		//	cptr->size = 0;
			//		//}
			//	}
			//}

			size_ = new count_t[n*n*n];
			memset(size_, 0, sizeof(count_t)*n*n*n);

			start_ = new uint32_t[n*n*n];
			memset(start_, 0xff, sizeof(uint32_t)*n*n*n);

			end_ = new uint32_t[n*n*n];
			//memset(end_, 0, sizeof(uint32_t)*n*n*n);

			//createGrid(ctr[0], ctr[1], ctr[2], maxextent);
			//root_ = createOctant(ctr[0], ctr[1], ctr[2], maxextent, 0, N - 1, N);
		}

		void free()
		{
			//if (grid_)
			//{
			//	for (int i = 0; i < n; i++)
			//	{
			//		for (int j = 0; j < n; j++)
			//			SAFE_DELETE_ARRAY(grid_[i][j]);
			//		SAFE_DELETE_ARRAY(grid_[i]);
			//	}
			//	SAFE_DELETE_ARRAY(grid_);
			//}

			if (data_) data_ = nullptr;
			if (size_) SAFE_DELETE_ARRAY(size_);
			if (start_) SAFE_DELETE_ARRAY(start_);
			if (end_) SAFE_DELETE_ARRAY(end_);

			successors_.clear();
		}

		void reset()
		{
			//memset(size_, 0, sizeof(count_t)*n*n*n);
			memset(start_, 0xff, sizeof(uint32_t)*n*n*n);

			//for (int i = 0; i < n; i++)
			//{
			//	for (int j = 0; j < n; j++)
			//	{
			//		for (int k = 0; k < n; k++)
			//			grid_[i][j][k].size = 0;
			//	}
			//}
		}

		void query(std::vector<uint32_t>& res0, std::vector<uint32_t>& res1)
		{
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
			return size_[id(x,y,z)];
		}

		uint32_t id(uint32_t x, uint32_t y, uint32_t z) const
		{
			return n2*x + n*y + z;
		}

	private:
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

				//Cube& cell = grid_[code[0]][code[1]][code[2]];

				// set child starts and update successors...
				auto tmpId = id(code[0],code[1], code[2]);
				if (start_[tmpId] == 0xffffffff)
					start_[tmpId] = i;
				else
					successors_[end_[tmpId]] = i;

				//size_[tmpId] ++;
				end_[tmpId] = i;
			}
		}

	private:

		std::vector<uint32_t> successors_;    // single connected list of next point indices...
		const ContainerT* data_ = nullptr;
		//Cube *** grid_ = nullptr;

		float offset,ctr[3];
		uint32_t n, n2;

		count_t *size_ = nullptr;
		uint32_t *start_ = nullptr, *end_ = nullptr;

		float min_[3], sl_;
	};
}