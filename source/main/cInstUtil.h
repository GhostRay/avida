/*
 *  cInstUtil.h
 *  Avida
 *
 *  Called "inst_util.hh" prior to 12/5/05.
 *  Copyright 1999-2007 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; version 2
 *  of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef cInstUtil_h
#define cInstUtil_h

#include <iostream>

#ifndef cGenome_h
#include "cGenome.h"
#endif

class cAvidaContext;
class cInitFile;
class cInstSet;
class cString;

class cInstUtil
{
private:
  cInstUtil(); // @not_implemented
  
public:

  // ========= Genome-File Interaction =========
  // Saving and loading of files.  These functions assume that the genome is
  // the only thing in the file unless 'Internal' is in the function name
  // (Internal genomes must begin with a number that indicates genome length)

  static bool LoadGenome(const cString &filename, const cInstSet &inst_set, cGenome& out_genome);
  static cGenome LoadGenome(const cString &filename, const cInstSet &inst_set);
  static cGenome LoadInternalGenome(std::istream & fp, const cInstSet &inst_set);
  static void SaveGenome(std::ostream& fp, const cInstSet & inst_set,
			 const cGenome &gen);
  static void SaveInternalGenome(std::ostream& fp, const cInstSet &inst_set,
				 const cGenome &gen);

  // ========= Genome Construction =========
  static cGenome RandomGenome(cAvidaContext& ctx, int length, const cInstSet & inst_set);
};


#ifdef ENABLE_UNIT_TESTS
namespace nInstUtil {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif  

#endif
