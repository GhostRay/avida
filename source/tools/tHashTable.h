/*
 *  tHashTable.h
 *  Avida
 *
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

#ifndef tHashTable_h
#define tHashTable_h

/*
 * This template is used to look up objects of the desired type by an integer
 * hash ID.  It is implemented through use of a linked list that contains all
 * of the individual entries stored in the table (in an arbitrary order). 
 * The hash table points to the first entry in the list that fits in its
 * cell.  If there are no entries that fit in the cell, the table contains a
 * NULL pointer at that location.
 *
 * The inputs are HASH_TYPE and DATA_TYPE
 *
 * INTERFACE:
 *    tHashTable(int in_hash_size=HASH_TABLE_SIZE_DEFAULT)  // Constructor
 *    ~tHashTable()                                   // Destructor
 *    int GetSize()                                   // Get num entries
 *    void Add(const HASH_TYPE & key, DATA_TYPE data) // Add new entry
 *    void SetValue(const HASH_TYPE & key, DATA_TYPE data)  // Add/modify entry
 *    bool HasEntry(const HASH_TYPE & key)            // Test if key exists
 *    bool Find(const HASH_TYPE & key, T & out_data)  // Find entry for key
 *    T Remove(const HASH_TYPE & key)                 // Remove entry
 *    void SetTableSize(int _hash)                    // Change hash table size
 *
 *
 * IMPLEMENTATION NOTES:
 *
 * On INSERT: If a cell already has at least one entry in it, the new entry
 * gets inserted into the linked list before the existing entry.  If the cell
 * is currently empty, the new entry gets placed at the end of the linked
 * list.  In either case, the cell is updated to point at the new entry.
 *
 * On DELETE: Start looking at the position in the list where the cell is
 * pointing and continue until the entry-to-be-deleted is found.  If the
 * entry to be deleted is the one being pointed at, be sure to update the
 * cell.
 *
 * On LOOKUP: If the cell has a NULL pointer, lookup fails.  Otherwise search
 * through list until either correct entry is found (lookup succeeds) or else
 * the lookup finds an entry not in the current cell (lookup fails).
 */

#ifndef cString_h
#include "cString.h"
#endif
#ifndef tArray_h
#include "tArray.h"
#endif
#ifndef tList_h
#include "tList.h"
#endif

#include <stdlib.h>

#if USE_tMemTrack
# ifndef tMemTrack_h
#  include "tMemTrack.h"
# endif
#endif


#define HASH_TABLE_SIZE_DEFAULT 23
#define HASH_TABLE_SIZE_MEDIUM  331
#define HASH_TABLE_SIZE_LARGE   2311

template <class DATA_TYPE> class tList; // access
template <class DATA_TYPE> class tListIterator; // aggregate

template <class HASH_TYPE, class DATA_TYPE> class tHashTable {
#if USE_tMemTrack
  tMemTrack<tHashTable<HASH_TYPE, DATA_TYPE> > mt;
#endif
  
  // We create a structure with full information about each entry stored in
  // this dictionary.
  template <class E_HASH_TYPE, class E_DATA_TYPE> struct tHashEntry {
  #if USE_tMemTrack
    tMemTrack<tHashEntry<E_HASH_TYPE, E_DATA_TYPE> > mt;
  #endif
    E_HASH_TYPE key;
    int id;
    E_DATA_TYPE data;

    template<class Archive>
    void serialize(Archive & a, const unsigned int version){
      a.ArkvObj("key", key);
      a.ArkvObj("id", id);
      a.ArkvObj("data", data);
    }
  };
  
private:
  int entry_count;  // How many entries are we storing?
  int table_size;  // What size hash table are we using?
  
  // Create a linked list of all hash entries in the table, as well as a
  // companion array with pointers into the list that will give the start of
  // each hash entry.
  tList< tHashEntry<HASH_TYPE, DATA_TYPE> > entry_list;
  tArray< tListNode< tHashEntry<HASH_TYPE, DATA_TYPE> > * > cell_array;
  
  // Create an iterator for entry_list
  mutable tListIterator< tHashEntry<HASH_TYPE, DATA_TYPE> > list_it;
  
  // Create a set of HashKey methods for each of the basic data types that
  // we allow:
  
  // HASH_TYPE = int
  // Simply mod the into by the size of the hash table and hope for the best
  int HashKey(const int& key) const
  {
    return abs(key % table_size);
  }

  // HASH_TYPE = void*
  // Casts the pointer to an int, shift right last two bit positions, mod by
  // the size of the hash table and hope for the best.  The shift is to account
  // for typical 4-byte alignment of pointer values.  Depending on architecture
  // this may not be true and could result in suboptimal hashing at higher
  // order alignments.
  int HashKey(const void* const& key) const
  {
    return abs(((int)key >> 2) % table_size);
  }
  
  // HASH_TYPE = cString
  // We hash a string simply by adding up the individual character values in
  // that string and modding by the hash size.  For most applications this
  // will work fine (and reasonably fast!) but some patterns will cause all
  // strings to go into the same cell.  For example, "ABC"=="CBA"=="BBB".
  int HashKey(const cString& key) const {
    unsigned int out_hash = 0;
    for (int i = 0; i < key.GetSize(); i++)
      out_hash += (unsigned int) key[i];
    return out_hash % table_size;
  }
  
  // Function to find the appropriate tHashEntry for a key that is passed
  // in and return it.
  tHashEntry<HASH_TYPE, DATA_TYPE> * FindEntry(const HASH_TYPE& key) const {
    const int bin = HashKey(key);
    if (cell_array[bin] == NULL) return NULL;
    
    // Set the list iterator to the first entry of this bin.
    list_it.Set(cell_array[bin]);
    
    // Loop through all entries in this bin to see if any are a perfect match.
    while (list_it.Get() != NULL && list_it.Get()->id == bin) {
      if (list_it.Get()->key == key) return list_it.Get();
      list_it.Next();
    }
    
    // No matches found.
    return NULL;
  }
private:
    // disabled copy constructor.
    tHashTable(const tHashTable &);
public:
    tHashTable(int in_table_size=HASH_TABLE_SIZE_DEFAULT)
    : entry_count(0)
    , table_size(in_table_size)
    , cell_array(in_table_size)
    , list_it(entry_list)
  {
      cell_array.SetAll(NULL);
  }
  
  ~tHashTable() {
    while (entry_list.GetSize()) delete entry_list.Pop();
  }
  
  void ClearAll() {
    list_it.Reset();
    while (list_it.Next() != NULL) {
      delete list_it.Remove();
    }
    entry_count = 0;
    cell_array.SetAll(NULL);
  }
  
  
  bool OK() const {
    std::cout << "ENTRY_COUNT = " << entry_count << std::endl;
    std::cout << "TABLE_SIZE = " << table_size << std::endl;
    int count = 0;
    std::cout << "LIST ELEMENTS:" << std::endl;
    list_it.Reset();
    while (list_it.Next() != NULL) {
      tHashEntry<HASH_TYPE, DATA_TYPE> * cur_entry = list_it.Get();
      std::cout << "  " << count << " : "
      << cur_entry->id << " "
      << cur_entry->key << " "
      << cur_entry->data << " "
      << std::endl;
    }
    std::cout << std::endl;
    std::cout << "ARRAY CELLS: "
      << cell_array.GetSize()
      << std::endl;
    for (int i = 0; i < table_size; i++) {
      tListNode< tHashEntry<HASH_TYPE, DATA_TYPE> > * cur_list_node = cell_array[i];
      if (cur_list_node == NULL) {
        std::cout << "  NULL" << std::endl;
      } else {
        std::cout << "  " << cur_list_node->data->id << " " << cur_list_node->data->key << std::endl;
      }
    }
    
    return true;
  }
  
  int GetSize() const { return entry_count; }
  
  // This function is used to add a new entry...
  void Add(const HASH_TYPE & key, DATA_TYPE data) {
    // Build the new entry...
    tHashEntry<HASH_TYPE, DATA_TYPE> * new_entry = new tHashEntry<HASH_TYPE, DATA_TYPE>;
    new_entry->key = key;
    new_entry->data = data;
    const int bin = HashKey(key);
    new_entry->id = bin;
    
    
    // Determine where this new entry should go; either at the end of
    // the list (if there are no others in the bin) or following another
    // entry in the bin.
    if (cell_array[bin] == NULL) { list_it.Reset(); } // Reset to list start
    else { list_it.Set(cell_array[bin]); }            // Else find insert point
    
    entry_list.Insert(list_it, new_entry); // Place new entry in the list
    list_it.Prev();                        // Back up to new entry
    cell_array[bin] = list_it.GetPos();    // Record position
    
    // Update our entry count...
    entry_count++;
  }
  
  
  // This function will change the value of an entry that exists, or add it
  // if it doesn't exist.
  void SetValue(const HASH_TYPE & key, DATA_TYPE data) {
    tHashEntry<HASH_TYPE, DATA_TYPE> * cur_entry = FindEntry(key);
    if (cur_entry == NULL) {
      Add(key, data);
      return;
    }
    cur_entry->data = data;
  }
  
  
  bool HasEntry(const HASH_TYPE & key) const {
    return FindEntry(key) != NULL;
  }
  
  bool Find(const HASH_TYPE & key, DATA_TYPE & out_data) const {
    tHashEntry<HASH_TYPE, DATA_TYPE> * found_entry = FindEntry(key);
    if (found_entry != NULL) {
      out_data = found_entry->data;
      return true;
    }
    return false;
  }
  
  DATA_TYPE Remove(const HASH_TYPE & key) {
    // Determine the bin that we are going to be using.
    const int bin = HashKey(key);
    
    DATA_TYPE out_data;
    assert(cell_array[bin] != NULL);
    list_it.Set(cell_array[bin]);
    
    // If we are deleting the first entry in this bin we must clean up...
    if (list_it.Get()->key == key) {
      out_data = list_it.Get()->data;
      delete list_it.Remove();
      list_it.Next();
      entry_count--;
      // See if the next entry is still part of this cell.
      if (list_it.AtRoot() == false && list_it.Get()->id == bin) {
        cell_array[bin] = list_it.GetPos();
      } else {
        cell_array[bin] = NULL;
      }
    }
    
    // If it was not the first entry in this cell, keep looking!
    else {
      while (list_it.Next() != NULL && list_it.Get()->id == bin) {
        if (list_it.Get()->key == key) {
          out_data = list_it.Get()->data;
          delete list_it.Remove();
          entry_count--;
          break;
        }
      }
    }
    
    return out_data;
  }
  
  void SetTableSize(int _hash) {
    // Create the new table...
    table_size = _hash;
    cell_array.ResizeClear(table_size);
    cell_array.SetAll(NULL);
    
    // Backup all of the entries in the list and re-insert them one-by-one.
    tList< tHashEntry<HASH_TYPE, DATA_TYPE> > backup_list;
    backup_list.Transfer(entry_list);
    
    while (backup_list.GetSize() > 0) {
      tHashEntry<HASH_TYPE, DATA_TYPE> * cur_entry = backup_list.Pop();
      
      // determine the new bin for this entry.
      int bin = HashKey(cur_entry->key);
      cur_entry->id = bin;
      
      if (cell_array[bin] == NULL) { list_it.Reset(); } // Reset to list start
      else { list_it.Set(cell_array[bin]); }            // Else find insert point
      
      entry_list.Insert(list_it, cur_entry); // Place new entry in the list
      list_it.Prev();                        // Back up to new entry
      cell_array[bin] = list_it.GetPos();    // Record position
    }
  }
  
  // The following method allows the user to convert the dictionary contents
  // into lists.  Empty lists show be passed in as arguments and the method
  // will fill in their contents.
  void AsLists(tList<HASH_TYPE>& key_list, tList<DATA_TYPE>& value_list) const
  {
    // Setup the lists to fill in.
    assert(key_list.GetSize() == 0);
    assert(value_list.GetSize() == 0);
    tListIterator<HASH_TYPE> key_it(key_list);
    tListIterator<DATA_TYPE> value_it(value_list);
    
    // Loop through the current entries and included them into the output
    // list one at a time.
    list_it.Reset();
    while (list_it.Next() != NULL) {
      // Grab the info about the current entry.
      HASH_TYPE& cur_key = list_it.Get()->key;
      DATA_TYPE& cur_value = list_it.Get()->data;
      
      // Find the position to place this in the lists.
      key_it.Reset();
      value_it.Reset();
      value_it.Next();
      while (key_it.Next() != NULL && cur_key > *(key_it.Get())) {
        value_it.Next();
      }
      key_list.Insert(key_it, &cur_key);
      value_list.Insert(value_it, &cur_value);
    }
  }
  
  void GetValues(tList<DATA_TYPE>& value_list) const
  {
    list_it.Reset();
    while (list_it.Next() != NULL) value_list.Push(&list_it.Get()->data);
  }

  void GetValues(tArray<DATA_TYPE>& value_array) const
  {
    value_array.Resize(entry_count);
    int idx = 0;

    list_it.Reset();
    while (list_it.Next() != NULL) value_array[idx++] = list_it.Get()->data;
  }
  
  
  
  template<class Archive> 
  void serialize(Archive & a, const unsigned int version){
    a.ArkvObj("entry_count", entry_count);
    a.ArkvObj("table_size", table_size);
    a.ArkvObj("entry_list", entry_list);
    a.ArkvObj("cell_array", cell_array);
  }
};

#endif
