#pragma once
#include "types.hpp"
#include <cstdlib>
#include <immintrin.h>

void init_spheres();

// Returns hit t values or 0 depending on if this ray hit this sphere or not
[[nodiscard]] __m256 sphere_hit(const RayCluster& rays, const Sphere& sphere, const float t_max);

void set_face_normal(const RayCluster& rays, HitRecords& hit_rec, const Vec3_256& outward_norm);

void create_hit_record(
	HitRecords& hit_rec,
	const RayCluster& rays,
	SphereCluster& sphere_cluster,
	const __m256 t_vals
);

// updates a sphere cluster with a sphere given a mask of where to insert the new sphere's values
void update_sphere_cluster(SphereCluster& curr_cluster, const Sphere& curr_sphere, const __m256 update_mask);

void find_sphere_hits(HitRecords& hit_rec, const RayCluster& rays, const float t_max);
