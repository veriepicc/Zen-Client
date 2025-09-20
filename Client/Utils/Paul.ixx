module;

#include <cmath>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <algorithm>
#include <array>
#include <random>
#include <type_traits>
#include <utility>

export module Paul;

// Paul: Modern single-file math library for Zen
// - Clean, constexpr-first API
// - Header-only module, no external deps
// - Friendly names: Paul::Vec3f, Paul::Mat4, Paul::Quat, Paul::Transform, Paul::Rect, Paul::BlockPos

export namespace Paul
{
	// ------------------------------------------------------------
	// Numeric constants and helpers
	// ------------------------------------------------------------
	template <typename T>
	[[nodiscard]] constexpr T PiV() noexcept { return static_cast<T>(3.141592653589793238462643383279502884L); }

	template <typename T>
	[[nodiscard]] constexpr T TauV() noexcept { return static_cast<T>(6.283185307179586476925286766559005768L); }

	template <typename T>
	[[nodiscard]] constexpr T HalfPiV() noexcept { return static_cast<T>(1.570796326794896619231321691639751442L); }

	inline constexpr float Pi = PiV<float>();
	inline constexpr float Tau = TauV<float>();
	inline constexpr float HalfPi = HalfPiV<float>();

	template <typename T>
	[[nodiscard]] constexpr T DegToRad(T degrees) noexcept { return degrees * (PiV<T>() / static_cast<T>(180)); }

	template <typename T>
	[[nodiscard]] constexpr T RadToDeg(T radians) noexcept { return radians * (static_cast<T>(180) / PiV<T>()); }

	template <typename T>
	[[nodiscard]] constexpr T Epsilon() noexcept { return std::numeric_limits<T>::epsilon() * static_cast<T>(8); }

	template <typename T>
	[[nodiscard]] constexpr T Abs(T v) noexcept { return v < static_cast<T>(0) ? -v : v; }

	template <typename T>
	[[nodiscard]] constexpr T Clamp(T v, T lo, T hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }

	template <typename T>
	[[nodiscard]] constexpr T Saturate(T v) noexcept { return Clamp<T>(v, static_cast<T>(0), static_cast<T>(1)); }

	template <typename T, typename U>
	[[nodiscard]] constexpr auto Lerp(const T& a, const T& b, U t) noexcept -> T { return static_cast<T>(a + (b - a) * static_cast<T>(t)); }

	template <typename T, typename U>
	[[nodiscard]] constexpr auto Remap(U x, U inMin, U inMax, T outMin, T outMax) noexcept -> T
	{
		const U t = (x - inMin) / (inMax - inMin);
		return static_cast<T>(outMin + (outMax - outMin) * t);
	}

	template <typename T>
	[[nodiscard]] constexpr bool NearlyEqual(T a, T b, T eps = Epsilon<T>()) noexcept { return Abs(a - b) <= eps; }

	// Smooth helpers
	template <typename T>
	[[nodiscard]] constexpr T SmoothStep(T edge0, T edge1, T x) noexcept
	{
		T t = Saturate((x - edge0) / (edge1 - edge0));
		return t * t * (static_cast<T>(3) - static_cast<T>(2) * t);
	}

	// ------------------------------------------------------------
	// Vector types: Vec2/3/4
	// ------------------------------------------------------------
	template <typename T>
	struct Vec2
	{
		T x, y;

		constexpr Vec2() noexcept : x(static_cast<T>(0)), y(static_cast<T>(0)) {}
		constexpr Vec2(T x_, T y_) noexcept : x(x_), y(y_) {}

		[[nodiscard]] static constexpr Vec2 Zero() noexcept { return Vec2(); }
		[[nodiscard]] static constexpr Vec2 One() noexcept { return Vec2(static_cast<T>(1), static_cast<T>(1)); }
		[[nodiscard]] static constexpr Vec2 UnitX() noexcept { return Vec2(static_cast<T>(1), static_cast<T>(0)); }
		[[nodiscard]] static constexpr Vec2 UnitY() noexcept { return Vec2(static_cast<T>(0), static_cast<T>(1)); }

		[[nodiscard]] constexpr T& operator[](std::size_t i) noexcept { return i == 0 ? x : y; }
		[[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept { return i == 0 ? x : y; }

		[[nodiscard]] constexpr Vec2 operator+(const Vec2& r) const noexcept { return { x + r.x, y + r.y }; }
		[[nodiscard]] constexpr Vec2 operator-(const Vec2& r) const noexcept { return { x - r.x, y - r.y }; }
		[[nodiscard]] constexpr Vec2 operator*(T s) const noexcept { return { x * s, y * s }; }
		[[nodiscard]] constexpr Vec2 operator/(T s) const noexcept { return { x / s, y / s }; }

		constexpr Vec2& operator+=(const Vec2& r) noexcept { x += r.x; y += r.y; return *this; }
		constexpr Vec2& operator-=(const Vec2& r) noexcept { x -= r.x; y -= r.y; return *this; }
		constexpr Vec2& operator*=(T s) noexcept { x *= s; y *= s; return *this; }
		constexpr Vec2& operator/=(T s) noexcept { x /= s; y /= s; return *this; }

		[[nodiscard]] constexpr T dot(const Vec2& r) const noexcept { return x * r.x + y * r.y; }
		[[nodiscard]] constexpr T lengthSq() const noexcept { return dot(*this); }
		[[nodiscard]] T length() const noexcept { if constexpr (std::is_floating_point_v<T>) return std::sqrt(lengthSq()); else return static_cast<T>(std::sqrt(static_cast<double>(lengthSq()))); }
		[[nodiscard]] Vec2 normalized() const noexcept { if (auto len = length(); len != static_cast<T>(0)) return *this / len; else return Vec2(); }
	};

	template <typename T>
	[[nodiscard]] constexpr Vec2<T> operator*(T s, const Vec2<T>& v) noexcept { return v * s; }

	template <typename T>
	struct Vec3
	{
		T x, y, z;

		constexpr Vec3() noexcept : x(static_cast<T>(0)), y(static_cast<T>(0)), z(static_cast<T>(0)) {}
		constexpr Vec3(T x_, T y_, T z_) noexcept : x(x_), y(y_), z(z_) {}

		[[nodiscard]] static constexpr Vec3 Zero() noexcept { return Vec3(); }
		[[nodiscard]] static constexpr Vec3 One() noexcept { return Vec3(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1)); }
		[[nodiscard]] static constexpr Vec3 UnitX() noexcept { return Vec3(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)); }
		[[nodiscard]] static constexpr Vec3 UnitY() noexcept { return Vec3(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)); }
		[[nodiscard]] static constexpr Vec3 UnitZ() noexcept { return Vec3(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)); }

		[[nodiscard]] constexpr T& operator[](std::size_t i) noexcept { return i == 0 ? x : (i == 1 ? y : z); }
		[[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept { return i == 0 ? x : (i == 1 ? y : z); }

		[[nodiscard]] constexpr Vec3 operator+(const Vec3& r) const noexcept { return { x + r.x, y + r.y, z + r.z }; }
		[[nodiscard]] constexpr Vec3 operator-(const Vec3& r) const noexcept { return { x - r.x, y - r.y, z - r.z }; }
		[[nodiscard]] constexpr Vec3 operator*(T s) const noexcept { return { x * s, y * s, z * s }; }
		[[nodiscard]] constexpr Vec3 operator/(T s) const noexcept { return { x / s, y / s, z / s }; }

		constexpr Vec3& operator+=(const Vec3& r) noexcept { x += r.x; y += r.y; z += r.z; return *this; }
		constexpr Vec3& operator-=(const Vec3& r) noexcept { x -= r.x; y -= r.y; z -= r.z; return *this; }
		constexpr Vec3& operator*=(T s) noexcept { x *= s; y *= s; z *= s; return *this; }
		constexpr Vec3& operator/=(T s) noexcept { x /= s; y /= s; z /= s; return *this; }

		[[nodiscard]] constexpr T dot(const Vec3& r) const noexcept { return x * r.x + y * r.y + z * r.z; }
		[[nodiscard]] constexpr Vec3 cross(const Vec3& r) const noexcept { return { y * r.z - z * r.y, z * r.x - x * r.z, x * r.y - y * r.x }; }
		[[nodiscard]] constexpr T lengthSq() const noexcept { return dot(*this); }
		[[nodiscard]] T length() const noexcept { if constexpr (std::is_floating_point_v<T>) return std::sqrt(lengthSq()); else return static_cast<T>(std::sqrt(static_cast<double>(lengthSq()))); }
		[[nodiscard]] Vec3 normalized() const noexcept { if (auto len = length(); len != static_cast<T>(0)) return *this / len; else return Vec3(); }

		template <typename U>
		[[nodiscard]] constexpr Vec3<U> cast() const noexcept { return { static_cast<U>(x), static_cast<U>(y), static_cast<U>(z) }; }
	};

	template <typename T>
	[[nodiscard]] constexpr Vec3<T> operator*(T s, const Vec3<T>& v) noexcept { return v * s; }

	template <typename T>
	struct Vec4
	{
		T x, y, z, w;

		constexpr Vec4() noexcept : x(static_cast<T>(0)), y(static_cast<T>(0)), z(static_cast<T>(0)), w(static_cast<T>(0)) {}
		constexpr Vec4(T x_, T y_, T z_, T w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

		[[nodiscard]] static constexpr Vec4 Zero() noexcept { return Vec4(); }
		[[nodiscard]] static constexpr Vec4 One() noexcept { return Vec4(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1), static_cast<T>(1)); }

		[[nodiscard]] constexpr T& operator[](std::size_t i) noexcept { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
		[[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }

		[[nodiscard]] constexpr Vec4 operator+(const Vec4& r) const noexcept { return { x + r.x, y + r.y, z + r.z, w + r.w }; }
		[[nodiscard]] constexpr Vec4 operator-(const Vec4& r) const noexcept { return { x - r.x, y - r.y, z - r.z, w - r.w }; }
		[[nodiscard]] constexpr Vec4 operator*(T s) const noexcept { return { x * s, y * s, z * s, w * s }; }
		[[nodiscard]] constexpr Vec4 operator/(T s) const noexcept { return { x / s, y / s, z / s, w / s }; }

		constexpr Vec4& operator+=(const Vec4& r) noexcept { x += r.x; y += r.y; z += r.z; w += r.w; return *this; }
		constexpr Vec4& operator-=(const Vec4& r) noexcept { x -= r.x; y -= r.y; z -= r.z; w -= r.w; return *this; }
		constexpr Vec4& operator*=(T s) noexcept { x *= s; y *= s; z *= s; w *= s; return *this; }
		constexpr Vec4& operator/=(T s) noexcept { x /= s; y /= s; z /= s; w /= s; return *this; }

		[[nodiscard]] constexpr T dot(const Vec4& r) const noexcept { return x * r.x + y * r.y + z * r.z + w * r.w; }
		[[nodiscard]] constexpr T lengthSq() const noexcept { return dot(*this); }
		[[nodiscard]] T length() const noexcept { if constexpr (std::is_floating_point_v<T>) return std::sqrt(lengthSq()); else return static_cast<T>(std::sqrt(static_cast<double>(lengthSq()))); }
		[[nodiscard]] Vec4 normalized() const noexcept { if (auto len = length(); len != static_cast<T>(0)) return *this / len; else return Vec4(); }
	};

	template <typename T>
	[[nodiscard]] constexpr Vec4<T> operator*(T s, const Vec4<T>& v) noexcept { return v * s; }

	using Vec2f = Vec2<float>;
	using Vec3f = Vec3<float>;
	using Vec4f = Vec4<float>;
	using Vec2i = Vec2<int>;
	using Vec3i = Vec3<int>;
	using Vec4i = Vec4<int>;

	// Vector helpers (component-wise) — now that Vec2/3/4 are declared
	template <typename T>
	[[nodiscard]] constexpr Vec2<T> Clamp(const Vec2<T>& v, T lo, T hi) noexcept { return { Clamp(v.x, lo, hi), Clamp(v.y, lo, hi) }; }
	template <typename T>
	[[nodiscard]] constexpr Vec3<T> Clamp(const Vec3<T>& v, T lo, T hi) noexcept { return { Clamp(v.x, lo, hi), Clamp(v.y, lo, hi), Clamp(v.z, lo, hi) }; }
	template <typename T>
	[[nodiscard]] constexpr Vec4<T> Clamp(const Vec4<T>& v, T lo, T hi) noexcept { return { Clamp(v.x, lo, hi), Clamp(v.y, lo, hi), Clamp(v.z, lo, hi), Clamp(v.w, lo, hi) }; }

	template <typename T>
	[[nodiscard]] constexpr bool NearlyEqual(const Vec2<T>& a, const Vec2<T>& b, T eps = Epsilon<T>()) noexcept { return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps); }
	template <typename T>
	[[nodiscard]] constexpr bool NearlyEqual(const Vec3<T>& a, const Vec3<T>& b, T eps = Epsilon<T>()) noexcept { return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps) && NearlyEqual(a.z, b.z, eps); }
	template <typename T>
	[[nodiscard]] constexpr bool NearlyEqual(const Vec4<T>& a, const Vec4<T>& b, T eps = Epsilon<T>()) noexcept { return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps) && NearlyEqual(a.z, b.z, eps) && NearlyEqual(a.w, b.w, eps); }

	// ------------------------------------------------------------
	// Rect and AABB
	// ------------------------------------------------------------
	template <typename T>
	struct RectT
	{
		Vec2<T> min;
		Vec2<T> max;

		constexpr RectT() noexcept : min(), max() {}
		constexpr RectT(Vec2<T> min_, Vec2<T> max_) noexcept : min(min_), max(max_) {}

		[[nodiscard]] constexpr T width() const noexcept { return max.x - min.x; }
		[[nodiscard]] constexpr T height() const noexcept { return max.y - min.y; }
		[[nodiscard]] constexpr Vec2<T> size() const noexcept { return { width(), height() }; }
		[[nodiscard]] constexpr Vec2<T> center() const noexcept { return { (min.x + max.x) / static_cast<T>(2), (min.y + max.y) / static_cast<T>(2) }; }
		[[nodiscard]] constexpr bool contains(const Vec2<T>& p) const noexcept { return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y; }
		[[nodiscard]] constexpr bool intersects(const RectT& r) const noexcept { return !(r.max.x < min.x || r.min.x > max.x || r.max.y < min.y || r.min.y > max.y); }

		constexpr RectT& expand(T by) noexcept { min.x -= by; min.y -= by; max.x += by; max.y += by; return *this; }
		[[nodiscard]] constexpr RectT expanded(T by) const noexcept { auto t = *this; t.expand(by); return t; }

		[[nodiscard]] static constexpr RectT unionOf(const RectT& a, const RectT& b) noexcept
		{
			return RectT({ std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y) }, { std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y) });
		}

		[[nodiscard]] static constexpr RectT intersectionOf(const RectT& a, const RectT& b) noexcept
		{
			return RectT({ std::max(a.min.x, b.min.x), std::max(a.min.y, b.min.y) }, { std::min(a.max.x, b.max.x), std::min(a.max.y, b.max.y) });
		}
	};

	struct AABB
	{
		Vec3f min;
		Vec3f max;

		constexpr AABB() noexcept : min(), max() {}
		constexpr AABB(const Vec3f& min_, const Vec3f& max_) noexcept : min(min_), max(max_) {}

		[[nodiscard]] constexpr Vec3f center() const noexcept { return { (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f }; }
		[[nodiscard]] constexpr Vec3f extent() const noexcept { return { (max.x - min.x) * 0.5f, (max.y - min.y) * 0.5f, (max.z - min.z) * 0.5f }; }
		[[nodiscard]] constexpr Vec3f size() const noexcept { return { max.x - min.x, max.y - min.y, max.z - min.z }; }
		[[nodiscard]] constexpr bool contains(const Vec3f& p) const noexcept { return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y && p.z >= min.z && p.z <= max.z; }
		[[nodiscard]] constexpr bool intersects(const AABB& b) const noexcept { return !(b.max.x < min.x || b.min.x > max.x || b.max.y < min.y || b.min.y > max.y || b.max.z < min.z || b.min.z > max.z); }
	};

	// ------------------------------------------------------------
	// Ray and intersections
	// ------------------------------------------------------------
	struct Ray
	{
		Vec3f origin;
		Vec3f direction; // assumed normalized

		constexpr Ray() noexcept : origin(), direction(Vec3f::UnitZ()) {}
		constexpr Ray(const Vec3f& o, const Vec3f& d) noexcept : origin(o), direction(d) {}

		[[nodiscard]] constexpr Vec3f at(float t) const noexcept { return origin + direction * t; }
	};

	[[nodiscard]] inline bool IntersectRayAABB(const Ray& ray, const AABB& box, float& tMinOut, float& tMaxOut) noexcept
	{
		// Slab method
		float tmin = -std::numeric_limits<float>::infinity();
		float tmax =  std::numeric_limits<float>::infinity();

		for (int i = 0; i < 3; ++i)
		{
			float ro = i == 0 ? ray.origin.x : (i == 1 ? ray.origin.y : ray.origin.z);
			float rd = i == 0 ? ray.direction.x : (i == 1 ? ray.direction.y : ray.direction.z);
			float mn = i == 0 ? box.min.x : (i == 1 ? box.min.y : box.min.z);
			float mx = i == 0 ? box.max.x : (i == 1 ? box.max.y : box.max.z);

			if (NearlyEqual(rd, 0.0f))
			{
				if (ro < mn || ro > mx) return false; // parallel and outside
			}
			else
			{
				float inv = 1.0f / rd;
				float t1 = (mn - ro) * inv;
				float t2 = (mx - ro) * inv;
				if (t1 > t2) std::swap(t1, t2);
				tmin = std::max(tmin, t1);
				tmax = std::min(tmax, t2);
				if (tmin > tmax) return false;
			}
		}

		tMinOut = tmin;
		tMaxOut = tmax;
		return true;
	}

	struct Sphere
	{
		Vec3f center{0.0f, 0.0f, 0.0f};
		float radius{1.0f};

		constexpr Sphere() noexcept = default;
		constexpr Sphere(const Vec3f& c, float r) noexcept : center(c), radius(r) {}
	};

	[[nodiscard]] inline bool IntersectRaySphere(const Ray& ray, const Sphere& s, float& tOut) noexcept
	{
		Vec3f oc = { ray.origin.x - s.center.x, ray.origin.y - s.center.y, ray.origin.z - s.center.z };
		float b = oc.dot(ray.direction);
		float c = oc.dot(oc) - s.radius * s.radius;
		float disc = b * b - c;
		if (disc < 0.0f) return false;
		float t = -b - std::sqrt(disc);
		if (t < 0.0f) t = -b + std::sqrt(disc);
		if (t < 0.0f) return false;
		tOut = t;
		return true;
	}

	[[nodiscard]] inline bool IntersectRayTriangle(const Ray& ray, const Vec3f& a, const Vec3f& b, const Vec3f& c, float& tOut, float& uOut, float& vOut) noexcept
	{
		// Möller–Trumbore
		const float eps = 1e-6f;
		Vec3f ab{ b.x - a.x, b.y - a.y, b.z - a.z };
		Vec3f ac{ c.x - a.x, c.y - a.y, c.z - a.z };
		Vec3f p = ray.direction.cross(ac);
		float det = ab.dot(p);
		if (NearlyEqual(det, 0.0f, eps)) return false;
		float invDet = 1.0f / det;
		Vec3f tvec{ ray.origin.x - a.x, ray.origin.y - a.y, ray.origin.z - a.z };
		float u = tvec.dot(p) * invDet; if (u < 0.0f || u > 1.0f) return false;
		Vec3f q = tvec.cross(ab);
		float v = ray.direction.dot(q) * invDet; if (v < 0.0f || u + v > 1.0f) return false;
		float t = ac.dot(q) * invDet; if (t < 0.0f) return false;
		tOut = t; uOut = u; vOut = v; return true;
	}

	// ------------------------------------------------------------
	// Quaternion (w, x, y, z)
	// ------------------------------------------------------------
	struct Quat
	{
		float w, x, y, z;

		constexpr Quat() noexcept : w(1.0f), x(0), y(0), z(0) {}
		constexpr Quat(float w_, float x_, float y_, float z_) noexcept : w(w_), x(x_), y(y_), z(z_) {}

		[[nodiscard]] static constexpr Quat Identity() noexcept { return Quat(); }

		[[nodiscard]] static Quat FromAxisAngle(const Vec3f& axis, float radians) noexcept
		{
			Vec3f n = axis.normalized();
			float s = std::sin(radians * 0.5f);
			return Quat(std::cos(radians * 0.5f), n.x * s, n.y * s, n.z * s);
		}

		[[nodiscard]] static Quat FromEulerXYZ(float pitchRadians, float yawRadians, float rollRadians) noexcept
		{
			// Rotation order: X (pitch), Y (yaw), Z (roll)
			float cx = std::cos(pitchRadians * 0.5f); float sx = std::sin(pitchRadians * 0.5f);
			float cy = std::cos(yawRadians   * 0.5f); float sy = std::sin(yawRadians   * 0.5f);
			float cz = std::cos(rollRadians  * 0.5f); float sz = std::sin(rollRadians  * 0.5f);
			return Quat(
				cx * cy * cz + sx * sy * sz,
				sx * cy * cz - cx * sy * sz,
				cx * sy * cz + sx * cy * sz,
				cx * cy * sz - sx * sy * cz
			);
		}

		[[nodiscard]] float length() const noexcept { return std::sqrt(w*w + x*x + y*y + z*z); }
		[[nodiscard]] float lengthSq() const noexcept { return w*w + x*x + y*y + z*z; }
		[[nodiscard]] Quat normalized() const noexcept { float len = length(); return len > 0.0f ? Quat(w/len, x/len, y/len, z/len) : Quat(); }

		[[nodiscard]] Quat operator*(const Quat& r) const noexcept
		{
			return Quat(
				w*r.w - x*r.x - y*r.y - z*r.z,
				w*r.x + x*r.w + y*r.z - z*r.y,
				w*r.y - x*r.z + y*r.w + z*r.x,
				w*r.z + x*r.y - y*r.x + z*r.w
			);
		}

		[[nodiscard]] Vec3f rotate(const Vec3f& v) const noexcept
		{
			// v' = q * (0, v) * conj(q)
			Quat qv(0.0f, v.x, v.y, v.z);
			Quat inv(w, -x, -y, -z);
			Quat res = (*this) * qv * inv;
			return { res.x, res.y, res.z };
		}

		[[nodiscard]] static Quat Slerp(Quat a, Quat b, float t) noexcept
		{
			// Ensure shortest path
			float dot = a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z;
			if (dot < 0.0f) { b.w = -b.w; b.x = -b.x; b.y = -b.y; b.z = -b.z; dot = -dot; }
			const float DOT_THRESHOLD = 0.9995f;
			if (dot > DOT_THRESHOLD)
			{
				// Linear interpolation for close quats
				Quat result(
					a.w + t*(b.w - a.w),
					a.x + t*(b.x - a.x),
					a.y + t*(b.y - a.y),
					a.z + t*(b.z - a.z)
				);
				return result.normalized();
			}
			float theta_0 = std::acos(dot);
			float theta = theta_0 * t;
			float sin_theta = std::sin(theta);
			float sin_theta_0 = std::sin(theta_0);
			float s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
			float s1 = sin_theta / sin_theta_0;
			return Quat(
				s0*a.w + s1*b.w,
				s0*a.x + s1*b.x,
				s0*a.y + s1*b.y,
				s0*a.z + s1*b.z
			);
		}
	};

	// ------------------------------------------------------------
	// 4x4 Matrix (row-major) suitable for DirectX-style left-handed transforms
	// ------------------------------------------------------------
	struct Mat4
	{
		// Row-major storage: m[row][col]
		float m[4][4];

		constexpr Mat4() noexcept : m{ {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} } {}
		explicit constexpr Mat4(float diag) noexcept : m{ {diag,0,0,0}, {0,diag,0,0}, {0,0,diag,0}, {0,0,0,diag} } {}
		constexpr Mat4(std::array<float,16> a) noexcept : m{ {a[0],a[1],a[2],a[3]}, {a[4],a[5],a[6],a[7]}, {a[8],a[9],a[10],a[11]}, {a[12],a[13],a[14],a[15]} } {}

		[[nodiscard]] static constexpr Mat4 Identity() noexcept { return Mat4(); }

		[[nodiscard]] static constexpr Mat4 Translation(const Vec3f& t) noexcept
		{
			Mat4 r = Mat4::Identity();
			r.m[3][0] = t.x; r.m[3][1] = t.y; r.m[3][2] = t.z;
			return r;
		}

		[[nodiscard]] static constexpr Mat4 Scale(const Vec3f& s) noexcept
		{
			Mat4 r{};
			r.m[0][0] = s.x; r.m[1][1] = s.y; r.m[2][2] = s.z; r.m[3][3] = 1.0f;
			return r;
		}

		[[nodiscard]] static Mat4 RotationAxisAngle(const Vec3f& axis, float radians) noexcept
		{
			Vec3f a = axis.normalized();
			float c = std::cos(radians);
			float s = std::sin(radians);
			float t = 1.0f - c;

			Mat4 R{};
			R.m[0][0] = t*a.x*a.x + c;     R.m[0][1] = t*a.x*a.y + s*a.z; R.m[0][2] = t*a.x*a.z - s*a.y; R.m[0][3] = 0.0f;
			R.m[1][0] = t*a.x*a.y - s*a.z; R.m[1][1] = t*a.y*a.y + c;     R.m[1][2] = t*a.y*a.z + s*a.x; R.m[1][3] = 0.0f;
			R.m[2][0] = t*a.x*a.z + s*a.y; R.m[2][1] = t*a.y*a.z - s*a.x; R.m[2][2] = t*a.z*a.z + c;     R.m[2][3] = 0.0f;
			R.m[3][0] = 0.0f;               R.m[3][1] = 0.0f;               R.m[3][2] = 0.0f;               R.m[3][3] = 1.0f;
			return R;
		}

		[[nodiscard]] static Mat4 FromQuat(const Quat& q) noexcept
		{
			Quat n = q.normalized();
			float xx = n.x * n.x, yy = n.y * n.y, zz = n.z * n.z;
			float xy = n.x * n.y, xz = n.x * n.z, yz = n.y * n.z;
			float wx = n.w * n.x, wy = n.w * n.y, wz = n.w * n.z;

			Mat4 R{};
			R.m[0][0] = 1.0f - 2.0f * (yy + zz); R.m[0][1] = 2.0f * (xy + wz);       R.m[0][2] = 2.0f * (xz - wy);       R.m[0][3] = 0.0f;
			R.m[1][0] = 2.0f * (xy - wz);       R.m[1][1] = 1.0f - 2.0f * (xx + zz); R.m[1][2] = 2.0f * (yz + wx);       R.m[1][3] = 0.0f;
			R.m[2][0] = 2.0f * (xz + wy);       R.m[2][1] = 2.0f * (yz - wx);       R.m[2][2] = 1.0f - 2.0f * (xx + yy); R.m[2][3] = 0.0f;
			R.m[3][0] = 0.0f;                    R.m[3][1] = 0.0f;                    R.m[3][2] = 0.0f;                    R.m[3][3] = 1.0f;
			return R;
		}

		[[nodiscard]] static Mat4 PerspectiveLH(float fovY, float aspect, float zn, float zf) noexcept
		{
			float f = 1.0f / std::tan(fovY * 0.5f);
			Mat4 P{};
			P.m[0][0] = f / aspect; P.m[0][1] = 0; P.m[0][2] = 0;                          P.m[0][3] = 0;
			P.m[1][0] = 0;           P.m[1][1] = f; P.m[1][2] = 0;                          P.m[1][3] = 0;
			P.m[2][0] = 0;           P.m[2][1] = 0; P.m[2][2] = zf / (zf - zn);             P.m[2][3] = 1.0f;
			P.m[3][0] = 0;           P.m[3][1] = 0; P.m[3][2] = (-zn * zf) / (zf - zn);      P.m[3][3] = 0;
			return P;
		}

		[[nodiscard]] static Mat4 OrthoLH(float left, float right, float bottom, float top, float zn, float zf) noexcept
		{
			Mat4 O{};
			O.m[0][0] = 2.0f / (right - left); O.m[0][1] = 0;                     O.m[0][2] = 0;                O.m[0][3] = 0;
			O.m[1][0] = 0;                     O.m[1][1] = 2.0f / (top - bottom); O.m[1][2] = 0;                O.m[1][3] = 0;
			O.m[2][0] = 0;                     O.m[2][1] = 0;                     O.m[2][2] = 1.0f/(zf-zn);     O.m[2][3] = 0;
			O.m[3][0] = -(right + left)/(right - left);
			O.m[3][1] = -(top + bottom)/(top - bottom);
			O.m[3][2] = -zn/(zf - zn);
			O.m[3][3] = 1.0f;
			return O;
		}

		[[nodiscard]] static Mat4 RotateX(float radians) noexcept
		{
			float c = std::cos(radians), s = std::sin(radians);
			Mat4 R{}; R.m[1][1]=c; R.m[1][2]=s; R.m[2][1]=-s; R.m[2][2]=c; return R;
		}

		[[nodiscard]] static Mat4 RotateY(float radians) noexcept
		{
			float c = std::cos(radians), s = std::sin(radians);
			Mat4 R{}; R.m[0][0]=c; R.m[0][2]=-s; R.m[2][0]=s; R.m[2][2]=c; return R;
		}

		[[nodiscard]] static Mat4 RotateZ(float radians) noexcept
		{
			float c = std::cos(radians), s = std::sin(radians);
			Mat4 R{}; R.m[0][0]=c; R.m[0][1]=s; R.m[1][0]=-s; R.m[1][1]=c; return R;
		}

		[[nodiscard]] static Mat4 RotationXYZ(float pitch, float yaw, float roll) noexcept
		{
			// Order X (pitch), Y (yaw), Z (roll)
			return RotateX(pitch) * RotateY(yaw) * RotateZ(roll);
		}

		[[nodiscard]] static Mat4 LookAtLH(const Vec3f& eye, const Vec3f& target, const Vec3f& up) noexcept
		{
			Vec3f z = (target - eye).normalized();
			Vec3f x = up.cross(z).normalized();
			Vec3f y = z.cross(x);

			Mat4 V{};
			V.m[0][0] = x.x; V.m[0][1] = y.x; V.m[0][2] = z.x; V.m[0][3] = 0.0f;
			V.m[1][0] = x.y; V.m[1][1] = y.y; V.m[1][2] = z.y; V.m[1][3] = 0.0f;
			V.m[2][0] = x.z; V.m[2][1] = y.z; V.m[2][2] = z.z; V.m[2][3] = 0.0f;
			V.m[3][0] = -x.dot(eye); V.m[3][1] = -y.dot(eye); V.m[3][2] = -z.dot(eye); V.m[3][3] = 1.0f;
			return V;
		}

		[[nodiscard]] constexpr Vec4f multiply(const Vec4f& v) const noexcept
		{
			return Vec4f(
				m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0]*v.w,
				m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1]*v.w,
				m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2]*v.w,
				m[0][3]*v.x + m[1][3]*v.y + m[2][3]*v.z + m[3][3]*v.w
			);
		}

		[[nodiscard]] constexpr Mat4 transposed() const noexcept
		{
			Mat4 t{};
			for (int r = 0; r < 4; ++r) for (int c = 0; c < 4; ++c) t.m[r][c] = m[c][r];
			return t;
		}

		[[nodiscard]] Vec3f transformPoint(const Vec3f& p) const noexcept
		{
			Vec4f v = multiply(Vec4f(p.x, p.y, p.z, 1.0f));
			float iw = (NearlyEqual(v.w, 0.0f) ? 1.0f : (1.0f / v.w));
			return { v.x * iw, v.y * iw, v.z * iw };
		}

		[[nodiscard]] Vec3f transformVector(const Vec3f& v3) const noexcept
		{
			Vec4f v = multiply(Vec4f(v3.x, v3.y, v3.z, 0.0f));
			return { v.x, v.y, v.z };
		}

		[[nodiscard]] constexpr Mat4 operator*(const Mat4& r) const noexcept
		{
			Mat4 o{};
			for (int row = 0; row < 4; ++row)
			{
				for (int col = 0; col < 4; ++col)
				{
					o.m[row][col] = m[row][0] * r.m[0][col]
						+ m[row][1] * r.m[1][col]
						+ m[row][2] * r.m[2][col]
						+ m[row][3] * r.m[3][col];
				}
			}
			return o;
		}
	};

	// ------------------------------------------------------------
	// Transform (T * R * S)
	// ------------------------------------------------------------
	struct Transform
	{
		Vec3f position { 0.0f, 0.0f, 0.0f };
		Quat  rotation { 1.0f, 0.0f, 0.0f, 0.0f };
		Vec3f scale    { 1.0f, 1.0f, 1.0f };

		[[nodiscard]] Mat4 matrix() const noexcept
		{
			return Mat4::Scale(scale) * Mat4::FromQuat(rotation) * Mat4::Translation(position);
		}

		[[nodiscard]] Transform combinedWith(const Transform& parent) const noexcept
		{
			Transform out;
			out.scale = { parent.scale.x * scale.x, parent.scale.y * scale.y, parent.scale.z * scale.z };
			out.rotation = parent.rotation * rotation;
			out.position = parent.position + parent.rotation.rotate({ parent.scale.x * position.x, parent.scale.y * position.y, parent.scale.z * position.z });
			return out;
		}
	};

	// ------------------------------------------------------------
	// Color utilities
	// ------------------------------------------------------------
	struct ColorRGBA8
	{
		std::uint8_t r { 0 }, g { 0 }, b { 0 }, a { 255 };

		constexpr ColorRGBA8() noexcept = default;
		constexpr ColorRGBA8(std::uint8_t r_, std::uint8_t g_, std::uint8_t b_, std::uint8_t a_ = 255) noexcept : r(r_), g(g_), b(b_), a(a_) {}

		[[nodiscard]] static constexpr ColorRGBA8 FromARGB(std::uint32_t argb) noexcept
		{
			return ColorRGBA8(
				static_cast<std::uint8_t>((argb >> 16) & 0xFF),
				static_cast<std::uint8_t>((argb >> 8)  & 0xFF),
				static_cast<std::uint8_t>((argb >> 0)  & 0xFF),
				static_cast<std::uint8_t>((argb >> 24) & 0xFF)
			);
		}

		[[nodiscard]] constexpr std::uint32_t toARGB() const noexcept
		{
			return (static_cast<std::uint32_t>(a) << 24)
				 | (static_cast<std::uint32_t>(r) << 16)
				 | (static_cast<std::uint32_t>(g) << 8)
				 | (static_cast<std::uint32_t>(b));
		}
	};

	struct ColorF
	{
		float r { 0 }, g { 0 }, b { 0 }, a { 1 };

		constexpr ColorF() noexcept = default;
		constexpr ColorF(float r_, float g_, float b_, float a_ = 1.0f) noexcept : r(r_), g(g_), b(b_), a(a_) {}

		[[nodiscard]] static constexpr ColorF FromBytes(const ColorRGBA8& c) noexcept
		{
			return { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
		}

		[[nodiscard]] constexpr ColorRGBA8 toBytes() const noexcept
		{
			return {
				static_cast<std::uint8_t>(Clamp(r, 0.0f, 1.0f) * 255.0f + 0.5f),
				static_cast<std::uint8_t>(Clamp(g, 0.0f, 1.0f) * 255.0f + 0.5f),
				static_cast<std::uint8_t>(Clamp(b, 0.0f, 1.0f) * 255.0f + 0.5f),
				static_cast<std::uint8_t>(Clamp(a, 0.0f, 1.0f) * 255.0f + 0.5f)
			};
		}

		[[nodiscard]] static ColorF FromHSV(float h, float s, float v, float a_ = 1.0f) noexcept
		{
			// h in [0,1), s,v in [0,1]
			h = h - std::floor(h);
			float c = v * s;
			float x = c * (1.0f - std::fabs(std::fmod(h * 6.0f, 2.0f) - 1.0f));
			float m = v - c;
			float r1, g1, b1;
			int hi = static_cast<int>(h * 6.0f);
			switch (hi % 6) {
				case 0: r1 = c; g1 = x; b1 = 0; break;
				case 1: r1 = x; g1 = c; b1 = 0; break;
				case 2: r1 = 0; g1 = c; b1 = x; break;
				case 3: r1 = 0; g1 = x; b1 = c; break;
				case 4: r1 = x; g1 = 0; b1 = c; break;
				default: r1 = c; g1 = 0; b1 = x; break;
			}
			return { r1 + m, g1 + m, b1 + m, a_ };
		}

		[[nodiscard]] Vec3f toHSV() const noexcept
		{
			float maxc = std::max(r, std::max(g, b));
			float minc = std::min(r, std::min(g, b));
			float delta = maxc - minc;
			float h = 0.0f;
			if (delta > 0.0f)
			{
				if (maxc == r) h = std::fmod(((g - b) / delta), 6.0f);
				else if (maxc == g) h = ((b - r) / delta) + 2.0f;
				else h = ((r - g) / delta) + 4.0f;
				h /= 6.0f; if (h < 0.0f) h += 1.0f;
			}
			float s = (maxc <= 0.0f) ? 0.0f : (delta / maxc);
			float v = maxc;
			return { h, s, v };
		}
	};

	// ------------------------------------------------------------
	// Random utilities
	// ------------------------------------------------------------
	class RNG
	{
	public:
		RNG() : engine(std::random_device{}()) {}
		explicit RNG(std::uint32_t seed) : engine(seed) {}

		[[nodiscard]] int nextInt(int loInclusive, int hiInclusive)
		{
			std::uniform_int_distribution<int> dist(loInclusive, hiInclusive);
			return dist(engine);
		}

		[[nodiscard]] float nextFloat(float loInclusive, float hiInclusive)
		{
			std::uniform_real_distribution<float> dist(loInclusive, hiInclusive);
			return dist(engine);
		}

		[[nodiscard]] Vec3f nextUnitVector3()
		{
			// Marsaglia method
			float x1, x2, s;
			do { x1 = nextFloat(-1.0f, 1.0f); x2 = nextFloat(-1.0f, 1.0f); s = x1*x1 + x2*x2; } while (s >= 1.0f || NearlyEqual(s, 0.0f));
			float z = 1.0f - 2.0f * s;
			float t = 2.0f * std::sqrt(1.0f - s);
			return { x1 * t, x2 * t, z };
		}

	private:
		std::mt19937 engine;
	};

	// ------------------------------------------------------------
	// Yaw/Pitch helpers (left-handed, +Z forward, yaw around +Y)
	// ------------------------------------------------------------
	[[nodiscard]] inline Vec3f DirectionFromYawPitch(float yawRadians, float pitchRadians) noexcept
	{
		float cy = std::cos(yawRadians), sy = std::sin(yawRadians);
		float cp = std::cos(pitchRadians), sp = std::sin(pitchRadians);
		return { sy * cp, -sp, cy * cp };
	}

	inline void YawPitchFromDirection(const Vec3f& dir, float& yawOut, float& pitchOut) noexcept
	{
		Vec3f n = dir.normalized();
		yawOut = std::atan2(n.x, n.z);
		pitchOut = -std::asin(Clamp(n.y, -1.0f, 1.0f));
	}

	// ------------------------------------------------------------
	// Project-related aliases kept for convenience
	// ------------------------------------------------------------
	using Rect = RectT<float>;
	using BlockPos = Vec3<int>;
}


