#pragma once
#include "linmath.h"
#include <cmath>
#include <fstream>
#include <cstdint>
#include "Eigen\src\Core\Array.h"

namespace {
	inline void trilinear_intp(const float p[3], float weights[8]) {
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

	inline void mat4x4_mul_vec3(vec3 ret, const float mat[16], const vec3 vec) {
		vec3 vec_{ vec[0], vec[1], vec[2] };
		ret[0] = mat[0] * vec_[0] + mat[1] * vec_[1] + mat[2] * vec_[2] + mat[3];
		ret[1] = mat[4] * vec_[0] + mat[5] * vec_[1] + mat[6] * vec_[2] + mat[7];
		ret[2] = mat[8] * vec_[0] + mat[9] * vec_[1] + mat[10] * vec_[2] + mat[11];
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
		class Helper {
		public:
			static void get_inverse_mat_array(float inverse_mat[16], const float mat[16]) {
				Eigen::Matrix4f mat4, mat4_inverse;
				for (int i = 0; i < 4; ++i)
					for (int j = 0; j < 4; ++j)
						mat4(i, j) = mat[i * 4 + j];

				mat4_inverse = mat4.inverse();

				for (int i = 0; i < 4; ++i)
					for (int j = 0; j < 4; ++j)
						inverse_mat[i * 4 + j] = mat4_inverse(i, j);
			}
		};
	public:
		Collider(ColliderFixer *fixer_, DistanceQuerier *querier_) : fixer(fixer_), querier(querier_) {}

		bool fix(Pos3 pos, Vec3 vel, const float world_mat[16], const float world_inverse_mat[16]) const {
			Pos3 rel_pos; 
			mat4x4_mul_vec3(rel_pos, world_inverse_mat, pos);

			Gradient3 rel_grad, rel_diff;
			float rel_distance;
			rel_distance = querier->query_pos(rel_pos, rel_grad);
			vec3_scale(rel_diff, rel_grad, rel_distance);

			Gradient3 diff;
			mat4x4_mul_vec3(diff, world_mat, rel_diff);

			Gradient3 grad;
			float distance = vec3_len(diff);
			vec3_scale(grad, diff, 1.0 / distance);
			if (rel_distance < 0) {
				distance = -distance;
				grad[0] = -grad[0]; grad[1] = -grad[1]; grad[2] = -grad[2];
			}

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
			vec3_add(pos, pos, diff);

			if (vel) {
				Vec3 vel_n, vel_t;
				float proj_len = vec3_mul_inner(vel, grad);
				vec3_scale(vel_n, grad, proj_len);
				vec3_sub(vel_t, vel, vel_n);
				vec3_scale(vel_n, diff, push_time_1);
				vec3_add(vel, vel_n, vel_t);
			}
		}

		void set_tolerance(float tolerance_) {
			e = tolerance_;
			b = e / exp(e);
		}

		//void set_tolerance(float tolerance_, const float collider_world2local_mat[16]) {
		//	vec3 vx{ collider_world2local_mat[0], collider_world2local_mat[4], collider_world2local_mat[8] };
		//	vec3 vy{ collider_world2local_mat[1], collider_world2local_mat[5], collider_world2local_mat[9] };
		//	vec3 vz{ collider_world2local_mat[2], collider_world2local_mat[6], collider_world2local_mat[10] };
		//	float sx = vec3_len(vx), sy = vec3_len(vy), sz = vec3_len(vz);
		//	float avg_s = (sx + sy + sz) / 3.0;
		//	set_tolerance(avg_s * tolerance_);
		//}

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

			float diff_from_center_len = vec3_len(diff_from_center);
			vec3_scale(grad, diff_from_center, 1.0 / diff_from_center_len);

			return diff_from_center_len - r;
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

			if ((ci >= 0 && cj >= 0 && ck >= 0 && ci < nx - 1 && cj < ny - 1 && ck < nz - 1) == false) {
				return 1e20;
			}

			const auto o = *this;
			float localcoord[] = { i - ci, j - cj, k - ck };
			float weights[8];
			trilinear_intp(localcoord, weights);

			CellData* cell_handles[] = {
				&o(ci, cj, ck), &o(ci + 1, cj, ck),
				&o(ci, cj + 1, ck), &o(ci + 1, cj + 1, ck),
				&o(ci, cj, ck + 1), &o(ci + 1, cj, ck + 1),
				&o(ci, cj + 1, ck + 1), &o(ci + 1, cj + 1, ck + 1)
			};
			//for (int it = 0; it < 8; ++it) {
			//	if (cell_handles[it] == nullptr) {
			//		//dump all data
			//		using namespace std;
			//		ofstream fout("C:\\Users\\vivid\\Desktop\\IllegalIndex.txt", ios::out);
			//		fout << setprecision(10);
			//		fout << "it=" << it << endl;
			//		fout << "i=" << i << ", ci=" << ci << endl;
			//		fout << "j=" << j << ", cj=" << cj << endl;
			//		fout << "k=" << k << ", ck=" << ck << endl;

			//		fout << "x=" << nx << ", y=" << ny << ", z=" << nz << ", yz=" << nyz << ", size=" << data_size << endl;
			//		fout << "bbox(x)=(" << bbox[0] << ',' << bbox[3] << "), bbox(y)=(" << bbox[1] << ',' << bbox[4] << "), bbox(z)=(" << bbox[2] << ',' << bbox[5] << ')' << endl;
			//		fout << "diag(x)=" << diag[0] << ", diag(y)=" << diag[1] << ", diag(z)=" << diag[2] << endl;
			//		fout << "step(x)=" << step[0] << ", step(y)=" << step[1] << ", step(z)=" << step[2] << endl;
			//		fout << "max(x) by step=" << step[0] * (nx - 1) + bbox[0]
			//			<< ", max(y) by step=" << step[1] * (ny - 1) + bbox[1]
			//			<< ", max(z) by step=" << step[2] * (nz - 1) + bbox[2] << endl;

			//		fout.close();
			//		assert(0);
			//	}
			//}

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
			const int offset = i * nyz + j * nz + k;
			//if (offset < 0 || offset >= data_size) {
			//	CellData *null = 0;
			//	return *null;
			//}
			return data[offset];
		}
	};
}