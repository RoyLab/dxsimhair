#pragma once
#include "linmath.h"
#include <cmath>
#include <fstream>
#include <cstdint>

namespace {
	void trilinear_intp(const float p[3], float weights[8]) {
		float pc[3] = { 1 - p[0], 1 - p[1], 1 - p[2] };
		float w2[4] = { pc[0] * pc[1], p[0] * pc[1], pc[0] * p[1] , p[0] * p[1] };
		weights[0] = w2[0] * pc[2]; 
		weights[1] = w2[1] * pc[2];
		weights[2] = w2[2] * pc[2]; 
		weights[3] = w2[3] * pc[2];
		weights[4] = w2[0] * p[2]; 
		weights[5] = w2[1] * p[2]; 
		weights[6] = w2[2] * p[2]; 
		weights[7] = w2[3] * p[2]; 
	}
}

namespace XRwy {
	using Pos3 = vec3;
	using Vec3 = vec3;
	using Gradient3 = vec3;

	class ColliderFixer {
	public:
		//get the input distance and grad, and ouput the correction of the pos and vel
		virtual bool fix(float distance, Gradient3 grad, Pos3 pos, Vec3 vel) const = 0;
	};

	class DistanceQuerier {
	public:
		//query the distance of pos, and write the gradient to grad, return the distance
		virtual float query_pos(const Pos3 pos, Gradient3 grad) const = 0;
	};

	class Collider {
	public:
		Collider(ColliderFixer *fixer_, DistanceQuerier *querier_) : fixer(fixer_), querier(querier_) {}

		bool fix(Pos3 pos, Vec3 vel) const {
			Gradient3 grad;
			float distance = querier->query_pos(pos, grad);
			return fixer->fix(distance, grad, pos, vel);
		}

		~Collider() = default; //it just a wrapper, we don't free anything

		ColliderFixer *fixer;
		DistanceQuerier *querier;
	};

	class BasicColliderFixer : public ColliderFixer {
	public:
		BasicColliderFixer(float tolerance_, float push_time_) : e(tolerance_), b(e / exp(e)), push_time(push_time_), push_time_1(1.0 / push_time_) {}

		virtual bool fix(float distance, Gradient3 grad, Pos3 pos, Vec3 vel) const {
			if (distance >= e) return false;
			float target_distance = func(distance); //target_distance > e > distance

			float diff_distance = target_distance - distance;
			Pos3 diff; 
			vec3_scale(diff, grad, diff_distance);
			/*vec3_add(pos, pos, diff);*/

			Vec3 vel_n, vel_t;
			float proj_len = vec3_mul_inner(vel, grad);
			vec3_scale(vel_n, grad, proj_len);
			vec3_sub(vel_t, vel, vel_n);
			vec3_scale(vel_n, diff, push_time_1);
			vec3_add(vel, vel_n, vel_t);
		}

		void set_tolerance(float tolerance_) {
			e = tolerance_;
			b = e / exp(e);
		}

		void set_push_time(float push_time_) {
			push_time = push_time_;
			push_time_1 = 1.0 / push_time;
		}

	private:
		float e;
		float b; // b = e/exp(e)
		float push_time;
		float push_time_1;

		inline float func(float x) const {
			return b * std::exp(x);
		}
	};

	class SphereDistanceQuerier : public DistanceQuerier {
	public:
		SphereDistanceQuerier(float center_x, float center_y, float center_z, float r_) : center{center_x, center_y, center_z}, r(r_) {}
		SphereDistanceQuerier(Pos3 center_, float r_): SphereDistanceQuerier(center_[0], center_[1], center_[2], r_) {}

		virtual float query_pos(const Pos3 pos, Gradient3 grad) const {
			Pos3 diff_from_center;
			vec3_sub(diff_from_center, pos, center);
			
			float diff_len = vec3_len(diff_from_center);
			vec3_norm(grad, diff_from_center);
			return diff_len - r;
		}
	private:
		Pos3 center;
		float r;
	};

	class GridCollisionQuerier : public DistanceQuerier {
		struct CellData {
			float dist;
			Gradient3 grad;
		};
	public:
		GridCollisionQuerier(const char *file_path) {
			using namespace std;

			ifstream fgrid(file_path, ios::binary);
			if (!fgrid.is_open()) throw std::exception("file not found");

			int32_t int32_buffer;
			ReadNBytes(fgrid, &int32_buffer, 4); nx = static_cast<int>(int32_buffer);
			ReadNBytes(fgrid, &int32_buffer, 4); ny = static_cast<int>(int32_buffer);
			ReadNBytes(fgrid, &int32_buffer, 4); nz = static_cast<int>(int32_buffer);
			nyz = ny * nz;
			data_size = nx * nyz;

			ReadNBytes(fgrid, step, sizeof(float) * 3);
			ReadNBytes(fgrid, bbox, sizeof(float) * 6);
			for (int i = 0; i < 3; ++i)
				diag[i] = bbox[i + 3] - bbox[i];

			data = new CellData[data_size];
			ReadNBytes(fgrid, data, sizeof(CellData) * data_size);

			cout << "Grid Info: ..." << endl;
			cout << "reading file path=" << file_path << endl;
			cout << "x=" << nx << ", y=" << ny << ", z=" << nz << ", yz=" << nyz << ", size=" << data_size << endl;
			cout << "bbox(x)=(" << bbox[0] << ',' << bbox[3] << "), bbox(y)=(" << bbox[1] << ',' << bbox[4] << "), bbox(z)=(" << bbox[2] << ',' << bbox[5] << ')' << endl;
			cout << "diag(x)=" << diag[0] << ", diag(y)=" << diag[1] << ", diag(z)=" << diag[2] << endl;
			cout << "step(x)=" << step[0] << ", step(y)=" << step[1] << ", step(z)=" << step[2] << endl;
			cout << "max(x) by step=" << step[0] * (nx - 1) + bbox[0]
				<< ", max(y) by step=" << step[1] * (ny - 1) + bbox[1]
				<< ", max(z) by step=" << step[2] * (nz - 1) + bbox[2] << endl;

			auto min_dist_it = min_element(data, data + data_size, [](const CellData &lhs, const CellData &rhs) { return lhs.dist < rhs.dist; });
			auto max_dist_it = max_element(data, data + data_size, [](const CellData &lhs, const CellData &rhs) { return lhs.dist < rhs.dist; });
			auto min_grad_it = min_element(data, data + data_size, [](const CellData &lhs, const CellData &rhs) { return vec3_len(lhs.grad) < vec3_len(rhs.grad); });
			auto max_grad_it = max_element(data, data + data_size, [](const CellData &lhs, const CellData &rhs) { return vec3_len(lhs.grad) < vec3_len(rhs.grad); });

			cout << "min dist=" << min_dist_it->dist << ", max_dist=" << max_dist_it->dist << ", min_grad=" << vec3_len(min_grad_it->grad) << ", max_grad=" << vec3_len(max_grad_it->grad) << endl;

			int negative_count = 0;
			for (int i = 0; i < data_size; ++i)
				if (data[i].dist < 0)
					++negative_count;
			cout << "negative " << negative_count << " in total " << data_size << endl;
		}

		virtual float query_pos(const Pos3 pos, Gradient3 grad) const {
			float i = (pos[0] - bbox[0]) / step[0];
			float j = (pos[1] - bbox[1]) / step[1];
			float k = (pos[2] - bbox[2]) / step[2];
			int ci = static_cast<int>(i);
			int cj = static_cast<int>(j);
			int ck = static_cast<int>(k);

			if ((i >= 0 && j >= 0 && k >= 0 && i < nx - 1 && j < ny - 1 && k < nz - 1) == false)
				return 1e20; //very large

			const auto o = *this;
			float localcoord[] = { i - ci, j - cj, k - ck };
			float weights[8];
			trilinear_intp(localcoord, weights);

			CellData* cell_handles[] = {
				&o(i, j, k), &o(i + 1, j, k),
				&o(i, j + 1, k), &o(i + 1, j + 1, k),
				&o(i, j, k + 1), &o(i + 1, j, k + 1),
				&o(i, j + 1, k + 1), &o(i + 1, j + 1, k + 1)
			};

			float ret_dist = 0.0;
			Gradient3 grad_tmp;
			vec3_zero(grad);
			for (int i = 0; i < 8; ++i) {
				ret_dist += cell_handles[i]->dist * weights[i];
				vec3_scale(grad_tmp, cell_handles[i]->grad, weights[i]);
				vec3_add(grad, grad, grad_tmp);
			}
			vec3_norm(grad, grad);

			return ret_dist;
		}
	protected:
		CellData* data;
		int nx, ny, nz, nyz, data_size;
		float bbox[6];
		float diag[3];
		Pos3 step;

		CellData &operator() (const int i, const int j, const int k) const {
			return *(data + i * nyz + j * nz + k);
		}
	};
}