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

#include "win_paths.hpp"
#include "../libfswatch_exception.hpp"
#include "../../gettext_defs.h"
#include <locale>
#include <codecvt>

using namespace std;

namespace fsw
{
  namespace win_paths
  {
    wstring posix_to_win_w(string path)
    {
	  using convert_typeX = std::codecvt_utf8<wchar_t>;
	  std::wstring_convert<convert_typeX, wchar_t> converterX;

	  return converterX.from_bytes(path);
    }

    string win_w_to_posix(wstring path)
    {
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes(path);
    }
  }
}

#else
void empty_symbol_for_win_paths() {}
#endif /* HAVE_WINDOWS */