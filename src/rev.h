/*
 * This file is part of FreeRCT.
 * FreeRCT is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * FreeRCT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with FreeRCT. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file rev.h Declaration of FreeRCT revision dependent variables. */

#ifndef REV_H
#define REV_H

#include <string>

extern const char _freerct_revision[];
extern const char _freerct_build_date[];

const std::string &freerct_install_prefix();
const std::string &freerct_userdata_prefix();
const std::string &freerct_executable_prefix();

void OverrideInstallPrefix(const char *dir);
void OverrideUserdataPrefix(const char *dir);

#endif /* REV_H */
