/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>
#include <string>
#include <functional>
#include <span>
#include <format>
#include <source_location>
#include <algorithm>
#include <iterator>

#include "../unicode.h"
#include "common.h"

namespace ghassanpl
{
	struct windows_message_box_result;
	struct windows_message_box_params;

	enum class windows_message_box_event;

	/// \addtogroup Windows
	
	/// \addtogroup msg Message Box
	/// Functions to display advanced message boxes
	/// \ingroup Windows
	///@{

	/// Type of icon in the message box. If not given, `Information` will be used by default.
	enum class windows_message_box_icon
	{
		Warning = -1, ///< An exclamation-point icon
		Error = -2, ///< A stop-sign icon
		Information = -3, ///< An icon consisting of a lowercase letter i in a circle
		Security = -4 ///< A shield icon
	};

	/// Holds the results of the windows_message_box invocation
	struct windows_message_box_result
	{
		/// Will be true if the message box was closed via the "X" button or failed to appear (perhaps due to argument errors)
		bool Failed = false;

		/// The number of the button that was clicked
		size_t ClickedButton = 0;

		/// Whether or not the checkbox was checked
		bool CheckboxValue = false;

		explicit operator bool() const noexcept { return !Failed; }
		operator size_t() const noexcept { return Failed ? -1 : ClickedButton; }
	};

	/// Helper functions and parameters
	namespace msg
	{
		/// Title of the message box window. This is "Message" by default.
		struct title { std::string_view str; };

		/// The primary instruction to the user (the main header of the message box)
		struct description { std::string_view str; };

		/// The longer description of the message (the smaller text of the message box).
		/// 
		/// This text can contain hyperlinks (`<A HREF="">asd</A>`) which will trigger the callback function, if given.
		struct long_description { std::string_view str; };

		/// If a non-empty checkbox_text is given, a checkbox will be present in the message box. The message box result will specify whether or not it was checked.
		/// 
		/// The callback, if present, will also be called when the checkbox state changes.
		struct checkbox_text { std::string_view str; };

		/// If a non-empty additional_info is given, a collapsible sub-section will be present in the message box, containg the text given in the additional info.
		/// 
		/// This text can contain hyperlinks (`<A HREF="">asd</A>`) which will trigger the callback function, if given.
		struct additional_info { std::string_view str; };

		/// If given, the button with this name will be selected by default
		struct default_button { std::string_view str; };

		/// If given, the message box will be modal to this window
		struct window_handle { win::HWND handle; };

		/// List of buttons: just OK
		static constexpr inline std::string_view ok_button[] = { "OK" };
		/// List of buttons: Yes, No
		static constexpr inline std::string_view yes_no_buttons[] = { "Yes", "No" };
		/// List of buttons: Yes, No, Cancel
		static constexpr inline std::string_view yes_no_cancel_buttons[] = { "Yes", "No", "Cancel" };
		/// List of buttons: Abort, Retry, Ignore
		static constexpr inline std::string_view abort_retry_ignore_buttons[] = { "Abort", "Retry", "Ignore" };
		/// List of buttons: Debug, Abort, Continue
		static constexpr inline std::string_view debug_abort_continue_buttons[] = { "Debug", "Abort", "Continue" };
	}

	/// The primary internal workhorse function; using the variadic version instead of this is preferable.
	/// 
	/// \param param Structure holding the parameters of the message box
	/// \returns The result of the user interacting with the message box
	windows_message_box_result windows_message_box(windows_message_box_params const& param);

	/// Primary function to display the message box
	/// 
	///	\param title Title of the message box window
	///	\param description The primary instruction to the user (the main header of the message box)
	///	\param args Additional parameters to tweak the message box behavior. They can be:
	/// - one of the types in namespace \ref ghassanpl::msg
	/// - a \ref windows_message_box_icon (`::Warning, ::Error, ::Information` or `::Security`) specifying the type of icon to display. This is `::Information` by default
	/// - a range convertible to a range of `string_view`s - these will be used as the names of the buttons to display in the message box. The default is a single "OK" button
	/// - a size_t 0-based number of the button that should be highlighted by default. If not given, the 0th button is highlighted
	/// - a function of signature `bool(` \ref windows_message_box_event `, uintptr_t, uintptr_t)` that will be used as a callback, and called when different events occur during the user interaction.
	/// \returns The result of the user interacting with the message box. 
	/// \sa windows_message_box_result
	/// \sa windows_message_box_event 
	template <typename... ARGS>
	windows_message_box_result windows_message_box(std::string_view title, std::string_view description, ARGS&&... args)
	{
		return ::ghassanpl::windows_message_box({ ::ghassanpl::msg::title{title}, ::ghassanpl::msg::description{description}, std::forward<ARGS>(args)... });
	}

	namespace msg
	{
		/// Helper to create a confirm box, with "Are you sure?" as text, and Yes and No buttons
		/// \returns true if Yes button was pressed
		template <typename... ARGS>
		bool confirm(std::string_view description, ARGS&&... args)
		{
			auto result = ::ghassanpl::windows_message_box({ ::ghassanpl::msg::title{"Are you sure?"}, ::ghassanpl::msg::description{description}, ::ghassanpl::msg::yes_no_buttons, 1, std::forward<ARGS>(args)... });
			return result && result == 0;
		}

		enum { btn_debug = 0, btn_abort = 1, btn_continue = 2 };

		/// A helper function for \ref Assuming macros.
		/// 
		/// Will take all the arguments of the \ref ghassanpl::ReportAssumptionFailure function, as well as additional ones that will be forwarded to \ref windows_message_box
		template <typename... ARGS>
		auto assumption_failure(std::string_view expectation, std::span<std::pair<std::string_view, std::string> const> values, std::string data, std::source_location loc, ARGS&&... args)
		{
			std::string message_information, message_long;
			message_information += std::format("Assumed: {}", expectation);
			message_long += std::format("In function `{}` at file `{}`, line {}\n\n", loc.function_name(), loc.file_name(), loc.line());
			for (auto& kvp : values)
				message_long += std::format("'{}' = '{}'\n", kvp.first, kvp.second);
			if (!data.empty())
				message_long += std::format("Additional Information: {}", move(data));
			return ::ghassanpl::windows_message_box({ 
				::ghassanpl::msg::title{"Assumption Failed"}, 
				::ghassanpl::msg::description{message_information}, 
				::ghassanpl::msg::long_description{message_long}, 
				::ghassanpl::msg::debug_abort_continue_buttons,
				1, 
				std::forward<ARGS>(args)...
			});
		}
	}

	/// Type of event that caused the callback to be called
	enum class windows_message_box_event
	{
		DialogCreated = 0, ///< The dialog box has been created
		ButtonClicked = 2, ///< A button on the message box was clicked. `param1` holds the button id as `int`
		LinkClicked = 3, ///< A link in the message box text was clicked. `param1` holds the URL given, as a `const wchar_t*`
		DialogDestroyed = 5, ///< The dialog box has been destroyed
		CheckboxClicked = 8, ///< The checkbox in the message box was clicked. `param2` holds the value of the checkbox, as `bool`
		HelpRequested = 9, ///< A help button/link was clicked
	};

	/// Holds all the parameters for the message box. Prefer to use the variadic version of \ref windows_message_box to using this struct
	struct windows_message_box_params
	{
		std::string_view title = "Message";
		windows_message_box_icon icon = windows_message_box_icon::Information;
		std::string_view description{};
		std::span<std::string_view const> buttons = msg::ok_button;
		std::vector<std::string_view> buttons_storage;
		size_t default_button = 0;
		std::string_view default_button_str{};
		std::string_view long_description{};
		std::string_view checkbox_text{};
		std::string_view additional_info{};
		std::function<bool(windows_message_box_event, uintptr_t, uintptr_t)> callback{};
		void* window_handle = nullptr;

		template <typename... ARGS>
		windows_message_box_params(ARGS&&... args)
		{
			(Set(std::forward<ARGS>(args)), ...);
		}

		void Set(msg::title param) { title = param.str; }

		void Set(windows_message_box_icon param) { icon = param; }

		void Set(msg::description param) { description = param.str; }
		void Set(std::string_view desc) { description = desc; }

		void Set(std::span<std::string_view const> param) { buttons = param; }

		template <typename T>
		requires (std::constructible_from<decltype(buttons_storage), decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))> && !std::constructible_from<decltype(buttons), T>)
		void Set(T param) { buttons_storage = std::vector<std::string_view>{ std::begin(param), std::end(param) }; buttons = buttons_storage; }

		void Set(msg::default_button param) { default_button_str = param.str; }
		void Set(size_t param) { default_button = param; }

		void Set(msg::long_description param) { long_description = param.str; }

		void Set(msg::checkbox_text param) { checkbox_text = param.str; }

		void Set(msg::additional_info param) { additional_info = param.str; }

		void Set(std::function<bool(windows_message_box_event, uintptr_t, uintptr_t)> param) { callback = std::move(param); }
		// TODO: void Set(std::function<bool(windows_message_box_event, int, std::string_view)> param) { callback = std::move(param); }

		void Set(msg::window_handle param) { window_handle = param.handle; }
	};

	///@}
}

#if 1
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Comctl32.lib")
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

namespace ghassanpl::win
{
	extern "C"
	{
#pragma pack(push, 4)
		typedef struct _TASKDIALOG_BUTTON
		{
			int     nButtonID;
			PCWSTR  pszButtonText;
		} TASKDIALOG_BUTTON;

		typedef HRESULT(__stdcall* PFTASKDIALOGCALLBACK)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData);
		typedef int TASKDIALOG_FLAGS;
		typedef int TASKDIALOG_COMMON_BUTTON_FLAGS;

		typedef struct _TASKDIALOGCONFIG
		{
			UINT        cbSize;
			HWND        hwndParent;
			HINSTANCE   hInstance;
			TASKDIALOG_FLAGS                dwFlags;
			TASKDIALOG_COMMON_BUTTON_FLAGS  dwCommonButtons;
			PCWSTR      pszWindowTitle;
			union
			{
				HICON   hMainIcon;
				PCWSTR  pszMainIcon;
			};
			PCWSTR      pszMainInstruction;
			PCWSTR      pszContent;
			UINT        cButtons;
			const TASKDIALOG_BUTTON* pButtons;
			int         nDefaultButton;
			UINT        cRadioButtons;
			const TASKDIALOG_BUTTON* pRadioButtons;
			int         nDefaultRadioButton;
			PCWSTR      pszVerificationText;
			PCWSTR      pszExpandedInformation;
			PCWSTR      pszExpandedControlText;
			PCWSTR      pszCollapsedControlText;
			union
			{
				HICON   hFooterIcon;
				PCWSTR  pszFooterIcon;
			};
			PCWSTR      pszFooter;
			PFTASKDIALOGCALLBACK pfCallback;
			LONG_PTR    lpCallbackData;
			UINT        cxWidth;
		} TASKDIALOGCONFIG;
#pragma pack(pop)
	
		void __stdcall InitCommonControls(void);
		HRESULT __stdcall TaskDialogIndirect(const TASKDIALOGCONFIG* pTaskConfig, int* pnButton, int* pnRadioButton, int* pfVerificationFlagChecked);
	}
}

namespace ghassanpl
{

	inline windows_message_box_result windows_message_box(windows_message_box_params const& param)
	{
		using namespace win;
		/// Set the common parameters
		TASKDIALOGCONFIG task_config = {};
		task_config.cbSize = sizeof(TASKDIALOGCONFIG);
		task_config.hwndParent = param.window_handle ? (HWND)param.window_handle : GetActiveWindow();
		task_config.hInstance = GetModuleHandleW(nullptr);
		task_config.dwFlags =
			/*TDF_ENABLE_HYPERLINKS*/ 0x0001
			| /*TDF_USE_COMMAND_LINKS*/ 0x0010
			| /*TDF_SIZE_TO_CONTENT*/ 0x01000000;
		task_config.dwCommonButtons = 0;

		/// No radiobuttons, no other platforms use them
		task_config.cRadioButtons = 0;
		task_config.pRadioButtons = nullptr;
		task_config.nDefaultRadioButton = 0;

		task_config.pszCollapsedControlText = nullptr;
		task_config.hFooterIcon = nullptr;
		task_config.pszFooter = nullptr;
		task_config.cxWidth = 0;

		/// Titles and descriptions
		auto title_u16 = string_ops::to_wstring(param.title);
		auto description_u16 = string_ops::to_wstring(param.description);
		auto long_description_u16 = string_ops::to_wstring(param.long_description);
		auto checkbox_text_u16 = string_ops::to_wstring(param.checkbox_text);
		auto additional_info_u16 = string_ops::to_wstring(param.additional_info);

		task_config.pszWindowTitle = title_u16.c_str();
		//AssumeBetween(int(icon), int(MessageBoxIcon::Warning), int(MessageBoxIcon::Security));
#define MAKEINTRESOURCEW(i) ((LPWSTR)((ULONG_PTR)((WORD)(i))))
		task_config.pszMainIcon = MAKEINTRESOURCEW(int(param.icon));
#undef MAKEINTRESOURCEW
		task_config.pszMainInstruction = description_u16.c_str();
		if (!param.long_description.empty())
			task_config.pszContent = long_description_u16.c_str();

		if (!param.checkbox_text.empty())
			task_config.pszVerificationText = checkbox_text_u16.c_str();
		if (!param.additional_info.empty())
		{
			task_config.pszExpandedControlText = L"Additional Information";
			task_config.pszExpandedInformation = additional_info_u16.c_str();
		}

		/// Callback
		if (param.callback)
		{
			task_config.pfCallback = [](HWND hwnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData) {
				auto ptr = (decltype(param.callback)*)dwRefData;
				return ptr->operator()((windows_message_box_event)uNotification, wParam, lParam) ? S_OK : S_FALSE;
			};
			task_config.lpCallbackData = (LONG_PTR)&param.callback;
		}

		/// Buttons
		std::vector<TASKDIALOG_BUTTON> button_vector;
		std::vector<std::wstring> button_texts;
		std::transform(param.buttons.begin(), param.buttons.end(), std::back_inserter(button_texts), string_ops::to_wstring);
		std::transform(button_texts.begin(), button_texts.end(), std::back_inserter(button_vector), [id = 0](auto& wstr) mutable { return TASKDIALOG_BUTTON{ id++, wstr.c_str() }; });
		
		task_config.cButtons = (UINT)button_vector.size();
		task_config.pButtons = button_vector.data();
		if (param.default_button_str.empty())
			task_config.nDefaultButton = (int)param.default_button;
		else
			task_config.nDefaultButton = int(std::find(param.buttons.begin(), param.buttons.end(), param.default_button_str) - param.buttons.begin());

		int clicked_id = 0;
		int checkbox_value = false;
		InitCommonControls();
		auto result = TaskDialogIndirect(&task_config, &clicked_id, nullptr, &checkbox_value);
		if (result != ((HRESULT)0L))
		{
			return { true };
		}

		return{ false, size_t(clicked_id), checkbox_value != 0 };
	}

}
#endif