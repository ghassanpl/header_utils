namespace impl
{

#ifdef _WIN32
	static constexpr inline ::ghassanpl::platform::operating_system operating_system = ::ghassanpl::platform::operating_system::windows;
#ifdef _WIN64
	static constexpr inline unsigned int operating_system_flags = (1 << ::ghassanpl::platform::operating_system_flags::sixty_four_bits);
#else;
	static constexpr inline unsigned int operating_system_flags = 0;
#endif
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
	static constexpr inline ::ghassanpl::platform::operating_system operating_system = ::ghassanpl::platform::operating_system::linux;
	static constexpr inline unsigned int operating_system_flags = (1<<operating_system_flags::posix);
#endif

#if defined(__MACOSX__) || (defined(__APPLE__) && defined(__MACH__))
	static constexpr inline ::ghassanpl::platform::operating_system operating_system = ::ghassanpl::platform::operating_system::macos;
	static constexpr inline unsigned int operating_system_flags = (1<<operating_system_flags::posix);
#endif

#ifdef _MSC_VER
	static constexpr inline ::ghassanpl::platform::compiler_type compiler_type = ::ghassanpl::platform::compiler_type::visual_studio;
#endif

}
