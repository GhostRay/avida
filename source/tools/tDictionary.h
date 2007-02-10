/*
 *  tDictionary.h
 *  Avida
 *
 *  Called "tDictionary.hh" prior to 10/11/05.
 *  Copyright 1999-2007 Michigan State University. All rights reserved.
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

#ifndef tDictionary_h
#define tDictionary_h

/*
 * This template is used to look up objects of the desired type by name.
 * It is essentially a wrapper around tHashTable<cString, DATA_TYPE>, with
 * the addition of NearMatch().
 *
 * For details about the encapsulated methods, see tHashTable.
 */

#ifndef cString_h
#include "cString.h"
#endif
#ifndef cStringUtil_h
#include "cStringUtil.h"
#endif
#ifndef tHashTable_h
#include "tHashTable.h"
#endif

#if USE_tMemTrack
# ifndef tMemTrack_h
#  include "tMemTrack.h"
# endif
#endif


template <class T> class tDictionary {
#if USE_tMemTrack
  tMemTrack<tDictionary<T> > mt;
#endif
private:
  tHashTable<cString, T> m_hash;

  // disabled copy constructor.
  tDictionary(const tDictionary &);
public:
  tDictionary() { ; }
  tDictionary(int in_hash_size) : m_hash(in_hash_size) { ; }

  // The following methods just call the encapsulated tHashTable
  bool OK() { return m_hash.OK(); }
  int GetSize() { return m_hash.GetSize(); }
  void Add(const cString& name, T data) { m_hash.Add(name, data); }
  void SetValue(const cString& name, T data) { m_hash.SetValue(name, data); }
  bool HasEntry(const cString& name) const { return m_hash.HasEntry(name); }
  bool Find(const cString& name, T& out_data) const { return m_hash.Find(name, out_data); }
  T Remove(const cString& name) { return m_hash.Remove(name); }
  void SetHash(int _hash) { m_hash.SetTableSize(_hash); }
  void AsLists(tList<cString>& name_list, tList<T>& value_list) const {
    m_hash.AsLists(name_list, value_list);
  }

  // This function will take an input string and load its value into the
  // dictionary; it will only work for types that cStringUtil can convert to.
  void Load(cString load_string, char assign='=') {
    // Break the string into two based on the assignment character.
    cString key(load_string.Pop(assign));
 
    // Convert the value to the correct type.
    T value;
    value = cStringUtil::Convert(load_string, value);

    SetValue(key, value);
  }
  
  // This function has no direct implementation in tHashTable
  // Grabs key/value lists, and processes the keys.
  cString NearMatch(const cString name) {
    tList<cString> keys;
    tList<T> values;
    m_hash.AsLists(keys, values);

    cString best_match("");
    int best_dist = name.GetSize();
    tListIterator<cString> list_it(keys);
    while (list_it.Next() != NULL) {
      int dist = cStringUtil::EditDistance(name, *list_it.Get());
      if (dist < best_dist) {
        best_dist = dist;
        best_match = *list_it.Get();
      }
    }
    return best_match;
  }

  template<class Archive>
  void serialize(Archive& a, const unsigned int version){
    a.ArkvObj("m_hash", m_hash);
  }
};

#endif
