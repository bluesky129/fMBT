/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2011, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 */

#include "aalang_java.hh"
#include "helper.hh"

aalang_java::aalang_java(): aalang(),action_cnt(1), tag_cnt(1), name_cnt(0),
                            istate(NULL),ainit(NULL), name(NULL), tag(false)
{
  default_guard="return true;";
  default_body="";
  default_adapter="";
  std::vector<std::string> t;
  aname.push_back(t);
  tname.push_back(t);
  amap.push_back(0);
  tmap.push_back(0);
  anames.push_back(default_adapter);
}

aalang_java::~aalang_java()
{
  delete istate;
  delete name;
}


void aalang_java::set_starter(std::string* st,const char* file,int line,int col)
{
  s+=*st;
  delete st;
}

void aalang_java::set_name(std::string* name,bool first,ANAMETYPE t)
{
  s+="\n//action"+to_string(action_cnt)+": \""+*name+"\"\n";
  anames.push_back(*name);
  aname.back().push_back(*name);
  delete name;
  amap.push_back(action_cnt);
  name_cnt++;
}

void aalang_java::set_namestr(std::string* _name)
{
  name=_name;
  s+="public class "+*name+"{\n\t";
}

void aalang_java::parallel(bool start) {

}

void aalang_java::serial(bool start) {

}

void aalang_java::set_variables(std::string* var,const char* file,int line,int col)
{
  s+="//variables\n"+*var+"\n";
  delete var;
}

void aalang_java::set_istate(std::string* ist,const char* file,int line,int col)
{
  istate=ist;
}

void aalang_java::set_ainit(std::string* iai,const char* file,int line,int col)
{
  ainit=iai;
}


void aalang_java::set_push(std::string* p,const char* file,int line,int col)
{
  push=*p;
  delete p;
}

void aalang_java::set_pop(std::string* p,const char* file,int line,int col)
{
  pop=*p;
  delete p;
}

void aalang_java::set_tagname(std::string* name,bool first)
{
  s+="\n//tag"+to_string(tag_cnt)+": \""+*name+"\"\n";
  tname.back().push_back(*name);
  delete name;
  tmap.push_back(tag_cnt);
  name_cnt++;
  tag=true;
}

void aalang_java::next_tag()
{
  std::vector<std::string> t;
  tname.push_back(t);
  tag_cnt+=name_cnt;
  name_cnt=0;
  tag=false;
}

void aalang_java::set_guard(std::string* gua,const char* file,int line,int col)
{
  if (tag) {
    s+="boolean tag"+to_string(tag_cnt)+"_guard() {\n"+
      *gua+"}\n";
  } else {
    s+="boolean action"+to_string(action_cnt)+"_guard() {\n"+
      *gua+"}\n";
  }
  if (gua!=&default_guard)
    delete gua;
}

void aalang_java::set_body(std::string* bod,const char* file,int line,int col)
{
  s+="void action"+to_string(action_cnt)+"_body() {\n"+*bod+"}\n";
  if (bod!=&default_body)
    delete bod;
}

void aalang_java::set_adapter(std::string* ada,const char* file,int line,int col)
{
  s+="int action" + to_string(action_cnt) + "_adapter() {\n" +
    "\tif (true) {\n"+
      *ada + "\n" +
    "\t}\n" +
    "\treturn " + to_string(action_cnt) + ";\n"
    "}\n";
  if (ada!=&default_adapter)
    delete ada;
}

void aalang_java::next_action()
{
  std::vector<std::string> t;
  aname.push_back(t);
  action_cnt+=name_cnt;
  name_cnt=0;
}

std::string aalang_java::stringify()
{
  s=s+"String [] action_names = { \"\",";
  for(std::list<std::vector<std::string> >::iterator i=aname.begin();i!=aname.end();i++) {
    for(std::vector<std::string>::iterator j=i->begin();j!=i->end();j++) {
      s+="\t\""+ *j +"\"\n";
      s+=",";
    }
  }
  s+="\t};\n";

  s=s+"\tString [] tag_names = {\"\",";

  for(std::list<std::vector<std::string> >::iterator i=tname.begin();i!=tname.end();i++) {
    for(std::vector<std::string>::iterator j=i->begin();j!=i->end();j++) {
      s+="\t\""+*j+"\"\n";
      s+=",";
    }
  }
  s+="};\n";

  s=s+"public " + *name + "() {\n" "\t};\n";

  if (istate) {
    s+="boolean reset() {\n"
      "if (true) {\n" +
      *istate+
      "}\n"
      "\nreturn true;\n}\n\n";
  }

  if (ainit) {
    s+="boolean init() {\n"
      "if (true) {\n" +
      *ainit+
      "}\n"
      "\nreturn true;\n}\n\n";
  }


  if (pop!="") {
    s=s+"\n void pop(){\n"+pop+"\n}\n";
    s=s+"\n void push(){\n"+push+"\n}\n";
  }

  s+="public int adapter_observe() {\n";

  int outputcount=0;
  for(int i=1;i<action_cnt;i++) {
    if (anames[i][0]=='o') {
      outputcount++;
    }
  }
  int preva=-1;
  if (outputcount) {
    s=s+"\tint tmp;\n";
    for(int i=1;i<action_cnt;i++) {
      if (anames[i][0]=='o' && preva!=amap[i]) {
        preva=amap[i];
        s+="\ttmp=action"+to_string(amap[i])+"_adapter();\n"
          "\tif (tmp!=0) { return tmp; }\n";
      }
    }
  }

  s+="\treturn 0;\n"
    "}\n"

   " int adapter_execute(int action) {\n"
    "\tswitch(action) {\n";

  for(int i=1;i<action_cnt;i++) {
    if (anames[i][0]=='i') {
      s+="\t\tcase "+to_string(i)+":\n"
        "\t\treturn action"+to_string(amap[i])+
        "_adapter();\n";
    }
  }
  s=s+"\t\tdefault:\n"
    "\t\treturn 0;\n"
    "\t}\n"
    "}\n"
    " int model_execute(int action) {\n"
    "\tswitch(action) {\n";

  for(int i=1;i<action_cnt;i++) {
    s+="\t\tcase "+to_string(i)+":\n"
      "\t\taction"+to_string(amap[i])+"_body();\n\t\treturn "+
      to_string(i)+";\n";
  }
  s=s+"\t\tdefault:\n"
    "\t\treturn 0;\n"
    "\t}\n"
    "}\n"
    " int [] getActions() {\n"
    "\tint tmp[] = new int [action_names.length];\n"
    "\tint len=0;\n";

  for(int i=1;i<action_cnt;i++) {
    s+="\tif (action"+to_string(amap[i])+"_guard()) {\n"
      "\t\ttmp[len]="+to_string(i)+";\n"
      "\t\tlen++;\n"
      "\t}\n";
  }
  s+="\tint ret[] = new int [len];\n"
    "\tSystem.arraycopy(tmp,0,ret,0,len);\n"
    "\treturn ret;\n"
    "}\n";

  s=s+" int[] getprops() {\n"
    "\tint tmp[] = new int [tag_names.length];\n"
    "\tint len=0;\n";

  for(int i=1;i<tag_cnt;i++) {
    s+="\tif (tag"+to_string(tmap[i])+"_guard()) {\n"
      "\t\ttmp[len]="+to_string(i)+";\n"
      "\t\tlen++;\n"
      "\t}\n";
  }

  s+="\tint ret[] = new int [len];\n"
    "\tSystem.arraycopy(tmp,0,ret,0,len);\n"
    "\treturn ret;\n"
    "}\n"
    "};\n";

  factory_register();

  return s;
}

void aalang_java::factory_register()
{
}
