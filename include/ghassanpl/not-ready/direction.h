#pragma once

namespace ghassanpl
{

#if 0

	enum class Direction
	{
		Right,
		RightDown,
		Down,
		LeftDown,
		Left,
		LeftUp,
		Up,
		RightUp,

		All
	};

	struct DirectionProperties
	{
		Point Vector;
		double Angle;
		int Opposite;
		int Next;
		int Previous;
		Align Alignment;
	};

	extern const DirectionProperties Directions[8];

	constexpr EnumFlags<Direction> AllCardinalDirections = EnumFlags<Direction>{ Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	constexpr EnumFlags<Direction> AllDiagonalDirections = EnumFlags<Direction>{ Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };

	inline const DirectionProperties& Dir(Direction d) { return Directions[int(d)]; }

	constexpr inline bool IsCardinal(Direction dir) { return (int(dir) & 1) == 0; }
	constexpr inline bool IsDiagonal(Direction dir) { return (int(dir) & 1) != 0; }

	inline Direction AngleToDirection(radians_t angle) {
		return Direction(int(EnsurePositiveAngle(angle + DegreesToRadians(45.0 / 2.0))
			/ DegreesToRadians(45.0)));
	}

	enum class Direction
	{
		None = -1,

		Right = 0,
		RightDown,
		Down,
		LeftDown,
		Left,
		LeftUp,
		Up,
		RightUp,

		East = 0,
		SouthEast,
		South,
		SouthWest,
		West,
		NorthWest,
		North,
		NorthEast
	};

	/// TODO: Tests

	constexpr inline Direction operator+(Direction dir, int d) { return (Direction)((int(dir) + d) % 8); }
	constexpr inline Direction operator-(Direction dir, int d) { return (Direction)((int(dir) + (8 + (d % 8))) % 8); }

	constexpr inline Direction& operator++(Direction& dir) { return dir = dir + 1; }
	constexpr inline Direction& operator--(Direction& dir) { return dir = dir + 7; }

	constexpr inline Direction operator++(Direction& dir, int) { auto res = dir; ++dir; return res; }
	constexpr inline Direction operator--(Direction& dir, int) { auto res = dir; --dir; return res; }

	constexpr inline Direction Opposite(Direction dir) { return dir + 4; }
	constexpr inline Direction NextCardinal(Direction dir) { return dir + 2; }

	namespace {
		static constexpr const int DirectionValue[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	}

	constexpr inline int HorizontalDirection(Direction dir) { return DirectionValue[(int)dir]; }
	constexpr inline int VerticalDirection(Direction dir) { return DirectionValue[int(dir + 6)]; }

	constexpr inline bool IsCardinal(Direction dir) { return (int(dir) & 1) == 0; }
	constexpr inline bool IsDiagonal(Direction dir) { return (int(dir) & 1) != 0; }

	constexpr inline gh::EnumFlags<Direction> AllCardinalDirections = gh::EnumFlags<Direction>{ Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	constexpr inline gh::EnumFlags<Direction> AllDiagonalDirections = gh::EnumFlags<Direction>{ Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };

	/*
	template <>
	inline Direction value_cast<radians_t, Direction>(radians_t radians)
	{
		constexpr auto pi = 3.14159265358979323846264338327950288;
		auto angle = pi * 2 - radians.Value;
		angle = angle + pi / 8;
		angle = std::fmod(angle + pi * 4, pi * 2);
		return Direction(int(((8 * angle) / (pi * 2))));
	}
	*/
	template <>
	inline Direction value_cast<radians_t, Direction>(radians_t angle)
	{
		return Direction(int(glm::mod(glm::degrees(angle.Value - glm::radians(45.0 / 2.0)), 360.0) / 45.0) % 8);
	}

	template <>
	inline radians_t value_cast<Direction, radians_t>(Direction dir)
	{
		return radians_t{ (float)glm::radians(int(dir) * 45.0) };
	}

	template <>
	constexpr inline degrees_t value_cast<Direction, degrees_t>(Direction dir)
	{
		return degrees_t{ float(int(dir) * 45.0) };
	}

	template <>
	constexpr inline ivec2 value_cast<Direction, ivec2>(Direction dir)
	{
		return ivec2{ HorizontalDirection(dir), VerticalDirection(dir) };
	}

	template <>
	constexpr inline Align value_cast<Direction, Align>(Direction dir); /// TODO

	enum class Direction
	{
		None = -1,

		Right = 0,
		RightDown,
		Down,
		LeftDown,
		Left,
		LeftUp,
		Up,
		RightUp,

		East = 0,
		SouthEast,
		South,
		SouthWest,
		West,
		NorthWest,
		North,
		NorthEast
	};

	struct DirectionProperties
	{
		vec2 Vector = {};
		double Angle = 0;
		int Opposite = 0;
		int Next = 0;
		int Previous = 0;
		Alignment Alignment = {};
	};

	extern const DirectionProperties Directions[8];

	constexpr inline EnumFlags<Direction> AllCardinalDirections = EnumFlags<Direction>{ Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	constexpr inline EnumFlags<Direction> AllDiagonalDirections = EnumFlags<Direction>{ Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };

	inline const DirectionProperties& Dir(Direction d) { return Directions[int(d)]; }

	constexpr inline bool IsCardinal(Direction dir) { return (int(dir) & 1) == 0; }
	constexpr inline bool IsDiagonal(Direction dir) { return (int(dir) & 1) != 0; }

	inline Direction AngleToDirection(double angle)
	{
		return Direction(int(EnsurePositiveAngle(angle + DegreesToRadians(45.0 / 2.0)) / DegreesToRadians(45.0)));
	}

	constexpr inline Direction operator+(Direction dir, int d) { return (Direction)((int(dir) + d) % 8); }
	constexpr inline Direction operator-(Direction dir, int d) { return (Direction)((int(dir) + (8 + (d % 8))) % 8); }

	constexpr inline Direction& operator++(Direction& dir) { return dir = dir + 1; }
	constexpr inline Direction& operator--(Direction& dir) { return dir = dir + 7; }

	constexpr inline Direction operator++(Direction& dir, int) { auto res = dir; ++dir; return res; }
	constexpr inline Direction operator--(Direction& dir, int) { auto res = dir; --dir; return res; }

	constexpr inline Direction Opposite(Direction dir) { return dir + 4; }
	constexpr inline Direction NextCardinal(Direction dir) { return dir + 2; }

	namespace {
		static constexpr const int DirectionValue[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	}

	constexpr inline int HorizontalDirection(Direction dir) { return DirectionValue[(int)dir]; }
	constexpr inline int VerticalDirection(Direction dir) { return DirectionValue[int(dir + 6)]; }

	/*
	template <>
	inline Direction value_cast<radians_t, Direction>(radians_t radians)
	{
		constexpr auto pi = 3.14159265358979323846264338327950288;
		auto angle = pi * 2 - radians.Value;
		angle = angle + pi / 8;
		angle = std::fmod(angle + pi * 4, pi * 2);
		return Direction(int(((8 * angle) / (pi * 2))));
	}
	*/
	template <>
	inline Direction value_cast<radians_t, Direction>(radians_t angle)
	{
		return Direction(int(glm::mod(glm::degrees(angle.Value - glm::radians(45.0 / 2.0)), 360.0) / 45.0) % 8);
	}

	template <>
	inline radians_t value_cast<Direction, radians_t>(Direction dir)
	{
		return radians_t{ (float)glm::radians(int(dir) * 45.0) };
	}

	template <>
	constexpr inline degrees_t value_cast<Direction, degrees_t>(Direction dir)
	{
		return degrees_t{ float(int(dir) * 45.0) };
	}

	template <>
	constexpr inline ivec2 value_cast<Direction, ivec2>(Direction dir)
	{
		return ivec2{ HorizontalDirection(dir), VerticalDirection(dir) };
	}

	template <>
	constexpr inline Align value_cast<Direction, Align>(Direction dir); /// TODO

	inline Direction DirFromAngle(float radians)
	{
		constexpr auto pi = 3.14159265358979323846264338327950288;
		auto angle = pi * 2 - radians;
		angle = angle + pi / 8;
		angle = std::fmod(angle + pi * 4, pi * 2);
		return Direction(int(((8 * angle) / (pi * 2))));
	}

	inline Direction DirFromPoint(Point p)
	{
		static constexpr Direction dirs[] = {
			DIR_NORTHWEST,/// -1, -1
			DIR_WEST,     /// -1,  0
			DIR_SOUTHWEST,/// -1,  1
			DIR_NORTH,    ///  0, -1
			(Direction)-1,
			DIR_SOUTH,    ///  0,  1
			DIR_NORTHEAST,///  1, -1
			DIR_EAST,     ///  1,  0
			DIR_SOUTHEAST ///  1,  1
		};
		return dirs[(p.y + 1) + ((p.x + 1) * 3)];
	}

	inline Point DirToPoint(Direction p)
	{
		static constexpr Point dirs[] = {
			{ 1,  0},
			{ 1, -1},
			{ 0, -1},
			{-1, -1},
			{-1,  0},
			{-1,  1},
			{ 0,  1},
			{ 1,  1},
		};
		return dirs[p];
	}


	inline bool IsSurrounding(ivec2 const a, ivec2 const b) { return std::abs(a.x - b.x) < 2 && std::abs(a.y - b.y) < 2; }
	inline bool IsNeighbor(ivec2 const a, ivec2 const b) { return IsSurrounding(a, b) && std::abs(a.y - b.y) != std::abs(a.x - b.x); }
	inline bool IsDiagonalNeighbor(ivec2 const a, ivec2 const b) { return IsSurrounding(a, b) && std::abs(a.y - b.y) == std::abs(a.x - b.x); }

	enum class Direction
	{
		None = -1,

		Right = 0,
		RightDown,
		Down,
		LeftDown,
		Left,
		LeftUp,
		Up,
		RightUp,

		/// Aliases
		East = 0,
		SouthEast,
		South,
		SouthWest,
		West,
		NorthWest,
		North,
		NorthEast
	};

	using DirectionBitmap = enum_flags<Direction, uint8_t>;

	constexpr inline Direction operator+(Direction dir, int d) { return (Direction)((int(dir) + d) % 8); }
	constexpr inline Direction operator-(Direction dir, int d) { return (Direction)((int(dir) + (8 + (d % 8))) % 8); }

	constexpr inline Direction& operator++(Direction& dir) { return dir = dir + 1; }
	constexpr inline Direction& operator--(Direction& dir) { return dir = dir + 7; }

	constexpr inline Direction operator++(Direction& dir, int) { auto res = dir; ++dir; return res; }
	constexpr inline Direction operator--(Direction& dir, int) { auto res = dir; --dir; return res; }

	constexpr inline Direction Opposite(Direction dir) { return dir + 4; }
	constexpr inline Direction NextCardinal(Direction dir) { return dir + 2; }
	constexpr inline Direction PrevCardinal(Direction dir) { return dir + 6; }

	namespace {
		static constexpr const int DirectionToOffset[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
		static constexpr const int OffsetToDirection[] = { 5, 6, 7, 4, -1, 0, 3, 2, 1 };
	}

	constexpr inline int HorizontalOffset(Direction dir) { return DirectionToOffset[(int)dir]; }
	constexpr inline int VerticalOffset(Direction dir) { return DirectionToOffset[int(dir + 6)]; }

	constexpr inline bool IsCardinal(Direction dir) { return (int(dir) & 1) == 0; }
	constexpr inline bool IsDiagonal(Direction dir) { return (int(dir) & 1) != 0; }

	//constexpr inline DirectionBitmap AllCardinalDirections = { Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	//constexpr inline DirectionBitmap AllDiagonalDirections = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };
	//constexpr inline DirectionBitmap AllDirections = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown, Direction::Left, Direction::Right, Direction::Up, Direction::Down };

	constexpr inline Direction AllCardinalDirections[] = { Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	constexpr inline Direction AllDiagonalDirections[] = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };
	constexpr inline Direction AllDirections[] = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown, Direction::Left, Direction::Right, Direction::Up, Direction::Down };

	inline Direction ToDirection(radians_t angle)
	{
		return Direction(int(glm::mod(glm::degrees(angle.Value), 360.0f) / 45.0) % 8);
	}

	constexpr inline Direction ToDirection(ivec2 vec)
	{
		return (Direction)OffsetToDirection[vec.x + vec.y * 3 + 4];
	}

	constexpr inline radians_t ToRadians(Direction dir)
	{
		return radians_t{ (float)glm::radians(int(dir) * 45.0) };
	}

	constexpr inline degrees_t ToDegrees(Direction dir)
	{
		return degrees_t{ float(int(dir) * 45.0) };
	}

	inline GLM_CONSTEXPR ivec2 ToVector(Direction dir)
	{
		return ivec2{ HorizontalOffset(dir), VerticalOffset(dir) };
	}

	inline GLM_CONSTEXPR const char* ToName(Direction dir)
	{
		constexpr const char* names[] = { "Right", "Lower Right", "Down", "Lower Left", "Left", "Upper Left", "Up", "Upper Right" };
		return names[int(dir)];
	}

	inline GLM_CONSTEXPR const char* ToCompassName(Direction dir)
	{
		constexpr const char* names[] = { "East", "Southeast", "South", "Southwest", "West", "Northwest", "North", "Northeast" };
		return names[int(dir)];
	}

	inline bool IsSurrounding(ivec2 const a, ivec2 const b) { return std::abs(a.x - b.x) < 2 && std::abs(a.y - b.y) < 2; }
	inline bool IsNeighbor(ivec2 const a, ivec2 const b) { return IsSurrounding(a, b) && std::abs(a.y - b.y) != std::abs(a.x - b.x); }
	inline bool IsDiagonalNeighbor(ivec2 const a, ivec2 const b) { return IsSurrounding(a, b) && std::abs(a.y - b.y) == std::abs(a.x - b.x); }

	enum class Direction
	{
		None = -1,

		Right = 0,
		RightDown,
		Down,
		LeftDown,
		Left,
		LeftUp,
		Up,
		RightUp,

		/// Aliases
		East = 0,
		SouthEast,
		South,
		SouthWest,
		West,
		NorthWest,
		North,
		NorthEast
	};

	using DirectionBitmap = enum_flags<Direction, uint8_t>;

	constexpr inline Direction operator+(Direction dir, int d) { return (Direction)((int(dir) + d) % 8); }
	constexpr inline Direction operator-(Direction dir, int d) { return (Direction)((int(dir) + (8 + (d % 8))) % 8); }

	constexpr inline Direction& operator++(Direction& dir) { return dir = dir + 1; }
	constexpr inline Direction& operator--(Direction& dir) { return dir = dir + 7; }

	constexpr inline Direction operator++(Direction& dir, int) { auto res = dir; ++dir; return res; }
	constexpr inline Direction operator--(Direction& dir, int) { auto res = dir; --dir; return res; }

	constexpr inline Direction Opposite(Direction dir) { return dir + 4; }
	constexpr inline Direction NextCardinal(Direction dir) { return dir + 2; }
	constexpr inline Direction PrevCardinal(Direction dir) { return dir + 6; }

	namespace {
		static constexpr const int DirectionToOffset[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
		static constexpr const int OffsetToDirection[] = { 5, 6, 7, 4, -1, 0, 3, 2, 1 };
	}

	constexpr inline int HorizontalOffset(Direction dir) { return DirectionToOffset[(int)dir]; }
	constexpr inline int VerticalOffset(Direction dir) { return DirectionToOffset[int(dir + 6)]; }

	constexpr inline bool IsCardinal(Direction dir) { return (int(dir) & 1) == 0; }
	constexpr inline bool IsDiagonal(Direction dir) { return (int(dir) & 1) != 0; }

	//constexpr inline DirectionBitmap AllCardinalDirections = { Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	//constexpr inline DirectionBitmap AllDiagonalDirections = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };
	//constexpr inline DirectionBitmap AllDirections = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown, Direction::Left, Direction::Right, Direction::Up, Direction::Down };

	constexpr inline Direction AllCardinalDirections[] = { Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	constexpr inline Direction AllDiagonalDirections[] = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };
	constexpr inline Direction AllDirections[] = { Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown, Direction::Left, Direction::Right, Direction::Up, Direction::Down };

	inline Direction ToDirection(radians_t angle)
	{
		return Direction(int(glm::mod(glm::degrees(angle.Value), 360.0f) / 45.0) % 8);
	}

	constexpr inline Direction ToDirection(ivec2 vec)
	{
		return (Direction)OffsetToDirection[vec.x + vec.y * 3 + 4];
	}

	constexpr inline radians_t ToRadians(Direction dir)
	{
		return radians_t{ (float)glm::radians(int(dir) * 45.0) };
	}

	constexpr inline degrees_t ToDegrees(Direction dir)
	{
		return degrees_t{ float(int(dir) * 45.0) };
	}

	inline GLM_CONSTEXPR ivec2 ToVector(Direction dir)
	{
		return ivec2{ HorizontalOffset(dir), VerticalOffset(dir) };
	}

	inline GLM_CONSTEXPR const char* ToName(Direction dir)
	{
		constexpr const char* names[] = { "Right", "Lower Right", "Down", "Lower Left", "Left", "Upper Left", "Up", "Upper Right" };
		return names[int(dir)];
	}

	inline GLM_CONSTEXPR const char* ToCompassName(Direction dir)
	{
		constexpr const char* names[] = { "East", "Southeast", "South", "Southwest", "West", "Northwest", "North", "Northeast" };
		return names[int(dir)];
	}

	typedef enum
	{
		DIR_None = -1,

		DIR_Right = 0,
		DIR_RightDown,
		DIR_Down,
		DIR_LeftDown,
		DIR_Left,
		DIR_LeftUp,
		DIR_Up,
		DIR_RightUp,
	} direction_t;

	static const int DirectionToOffset[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	static const int OffsetToDirection[] = { 5, 6, 7, 4, -1, 0, 3, 2, 1 };
#define DIR_V_OFFSET 6

#define DIR_Add(d, n) (((d) + (n))%8)
#define DIR_Sub(d, n) (((d) + 8 + ((n)%8))%8)
#define DIR_Next(d) DIR_Add(d, 1)
#define DIR_Prev(d) DIR_Add(d, 7)

#define DIR_H(dir) (DirectionToOffset[(int)dir])
#define DIR_V(dir) (DirectionToOffset[(int)DIR_Add(dir, DIR_V_OFFSET))

#define DIR_IsCardinal(dir) ((((int)dir) & 1) == 0)
#define DIR_IsDiagonal(dir) ((((int)dir) & 1) != 0)

#define DIR_ToDirection(x, y) ((direction_t)OffsetToDirection[(x) + (y) * 3 + 4])

#endif

}


	enum class Direction
	{
		None = -1,

		Right = 0,
		RightDown,
		Down,
		LeftDown,
		Left,
		LeftUp,
		Up,
		RightUp,

		East = 0,
		SouthEast,
		South,
		SouthWest,
		West,
		NorthWest,
		North,
		NorthEast
	};

	/// TODO: Tests

	constexpr inline Direction operator+(Direction dir, int d) { return (Direction)((int(dir) + d) % 8); }
	constexpr inline Direction operator-(Direction dir, int d) { return (Direction)((int(dir) + (8 + (d % 8))) % 8); }

	constexpr inline Direction& operator++(Direction& dir) { return dir = dir + 1; }
	constexpr inline Direction& operator--(Direction& dir) { return dir = dir + 7; }

	constexpr inline Direction operator++(Direction& dir, int) { auto res = dir; ++dir; return res; }
	constexpr inline Direction operator--(Direction& dir, int) { auto res = dir; --dir; return res; }

	constexpr inline Direction Opposite(Direction dir) { return dir + 4; }
	constexpr inline Direction NextCardinal(Direction dir) { return dir + 2; }

	namespace {
		static constexpr const int DirectionValue[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	}

	constexpr inline int HorizontalDirection(Direction dir) { return DirectionValue[(int)dir]; }
	constexpr inline int VerticalDirection(Direction dir) { return DirectionValue[int(dir + 6)]; }

	constexpr inline bool IsCardinal(Direction dir) { return (int(dir) & 1) == 0; }
	constexpr inline bool IsDiagonal(Direction dir) { return (int(dir) & 1) != 0; }

	constexpr inline gh::EnumFlags<Direction> AllCardinalDirections = gh::EnumFlags<Direction>{ Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	constexpr inline gh::EnumFlags<Direction> AllDiagonalDirections = gh::EnumFlags<Direction>{ Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };

	/*
	template <>
	inline Direction value_cast<radians_t, Direction>(radians_t radians)
	{
		constexpr auto pi = 3.14159265358979323846264338327950288;
		auto angle = pi * 2 - radians.Value;
		angle = angle + pi / 8;
		angle = std::fmod(angle + pi * 4, pi * 2);
		return Direction(int(((8 * angle) / (pi * 2))));
	}
	*/
	template <>
	inline Direction value_cast<radians_t, Direction>(radians_t angle)
	{
		return Direction(int(glm::mod(glm::degrees(angle.Value - glm::radians(45.0 / 2.0)), 360.0) / 45.0) % 8);
	}

	template <>
	inline radians_t value_cast<Direction, radians_t>(Direction dir)
	{
		return radians_t{ (float)glm::radians(int(dir) * 45.0) };
	}

	template <>
	constexpr inline degrees_t value_cast<Direction, degrees_t>(Direction dir)
	{
		return degrees_t{ float(int(dir) * 45.0) };
	}

	template <>
	constexpr inline ivec2 value_cast<Direction, ivec2>(Direction dir)
	{
		return ivec2{ HorizontalDirection(dir), VerticalDirection(dir) };
	}

	template <>
	constexpr inline Align value_cast<Direction, Align>(Direction dir); /// TODO
	
	
	enum class Direction
	{
		None = -1,

		Right = 0,
		RightDown,
		Down,
		LeftDown,
		Left,
		LeftUp,
		Up,
		RightUp,

		East = 0,
		SouthEast,
		South,
		SouthWest,
		West,
		NorthWest,
		North,
		NorthEast
	};

	struct DirectionProperties
	{
		vec2 Vector = {};
		double Angle = 0;
		int Opposite = 0;
		int Next = 0;
		int Previous = 0;
		Alignment Alignment = {};
	};

	extern const DirectionProperties Directions[8];

	constexpr inline EnumFlags<Direction> AllCardinalDirections = EnumFlags<Direction>{ Direction::Left, Direction::Right, Direction::Up, Direction::Down };
	constexpr inline EnumFlags<Direction> AllDiagonalDirections = EnumFlags<Direction>{ Direction::LeftUp, Direction::RightUp, Direction::RightDown, Direction::LeftDown };

	inline const DirectionProperties& Dir(Direction d) { return Directions[int(d)]; }

	constexpr inline bool IsCardinal(Direction dir) { return (int(dir) & 1) == 0; }
	constexpr inline bool IsDiagonal(Direction dir) { return (int(dir) & 1) != 0; }

	inline Direction AngleToDirection(double angle)
	{
		return Direction(int(EnsurePositiveAngle(angle + DegreesToRadians(45.0 / 2.0)) / DegreesToRadians(45.0)));
	}
	
	constexpr inline Direction operator+(Direction dir, int d) { return (Direction)((int(dir) + d) % 8); }
	constexpr inline Direction operator-(Direction dir, int d) { return (Direction)((int(dir) + (8 + (d % 8))) % 8); }

	constexpr inline Direction& operator++(Direction& dir) { return dir = dir + 1; }
	constexpr inline Direction& operator--(Direction& dir) { return dir = dir + 7; }

	constexpr inline Direction operator++(Direction& dir, int) { auto res = dir; ++dir; return res; }
	constexpr inline Direction operator--(Direction& dir, int) { auto res = dir; --dir; return res; }

	constexpr inline Direction Opposite(Direction dir) { return dir + 4; }
	constexpr inline Direction NextCardinal(Direction dir) { return dir + 2; }

	namespace {
		static constexpr const int DirectionValue[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	}

	constexpr inline int HorizontalDirection(Direction dir) { return DirectionValue[(int)dir]; }
	constexpr inline int VerticalDirection(Direction dir) { return DirectionValue[int(dir + 6)]; }

	/*
	template <>
	inline Direction value_cast<radians_t, Direction>(radians_t radians)
	{
		constexpr auto pi = 3.14159265358979323846264338327950288;
		auto angle = pi * 2 - radians.Value;
		angle = angle + pi / 8;
		angle = std::fmod(angle + pi * 4, pi * 2);
		return Direction(int(((8 * angle) / (pi * 2))));
	}
	*/
	template <>
	inline Direction value_cast<radians_t, Direction>(radians_t angle)
	{
		return Direction(int(glm::mod(glm::degrees(angle.Value - glm::radians(45.0 / 2.0)), 360.0) / 45.0) % 8);
	}

	template <>
	inline radians_t value_cast<Direction, radians_t>(Direction dir)
	{
		return radians_t{ (float)glm::radians(int(dir) * 45.0) };
	}

	template <>
	constexpr inline degrees_t value_cast<Direction, degrees_t>(Direction dir)
	{
		return degrees_t{ float(int(dir) * 45.0) };
	}

	template <>
	constexpr inline ivec2 value_cast<Direction, ivec2>(Direction dir)
	{
		return ivec2{ HorizontalDirection(dir), VerticalDirection(dir) };
	}

	template <>
	constexpr inline Align value_cast<Direction, Align>(Direction dir); /// TODO
	