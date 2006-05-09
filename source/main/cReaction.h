/*
 *  cReaction.h
 *  Avida
 *
 *  Created by David on 12/5/05.
 *  Copyright 2005-2006 Michigan State University. All rights reserved.
 *  Copyright 1993-2004 California Institute of Technology.
 *
 */

#ifndef cReaction_h
#define cReaction_h

#ifndef cString_h
#include "cString.h"
#endif
#ifndef tList_h
#include "tList.h"
#endif

class cTaskEntry;
class cReactionProcess;
class cReactionRequisite;

class cReaction
{
private:
  cString name;
  int id;
  cTaskEntry* task;
  tList<cReactionProcess> process_list;
  tList<cReactionRequisite> requisite_list;
  bool active;


  cReaction(); // @not_implemented
  cReaction(const cReaction&); // @not_implemented
  cReaction& operator=(const cReaction&); // @not_implemented

public:
  cReaction(const cString& _name, int _id);
  ~cReaction();

  const cString & GetName() const { return name; }
  int GetID() const { return id; }
  cTaskEntry* GetTask() { return task; }
  const tList<cReactionProcess>& GetProcesses() { return process_list; }
  const tList<cReactionRequisite>& GetRequisites() { return requisite_list; }
  bool GetActive() const { return active; }

  void SetTask(cTaskEntry* _task) { task = _task; }
  cReactionProcess* AddProcess();
  cReactionRequisite* AddRequisite();
  void SetActive(bool in_active = true) { active = in_active; }

  // These methods will modify the value of the process listed.
  bool ModifyValue(double new_value, int process_num = 0);
  bool MultiplyValue(double value_mult, int process_num = 0); 

  // This method will modify the instruction triggered by this process
  bool ModifyInst(int inst_id, int process_num = 0); 

  double GetValue(int process_num = 0);
};


#ifdef ENABLE_UNIT_TESTS
namespace nReaction {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif

#endif
