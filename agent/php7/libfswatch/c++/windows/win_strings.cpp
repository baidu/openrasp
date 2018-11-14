/*
 * Copyright (c) 2015 Enrico M. Crisostomo
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "libfswatch_config.h"

#ifdef HAVE_WINDOWS

#include "win_strings.hpp"
#include <windows.h>
#include <locale>
#include <codecvt>

namespace fsw
{
  namespace win_strings
  {
    using namespace std;

    string wstring_to_string(wchar_t * s)
    {
	  using convert_typeX = std::codecvt_utf8<wchar_t>;
	  std::wstring_convert<convert_typeX, wchar_t> converterX;

	  return converterX.to_bytes(s);
    }

    string wstring_to_string(const wstring & s)
    {
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes(s);
    }
  }
}

#else
void empty_symbol_for_win_strings() {}
#endif /* HAVE_WINDOWS */