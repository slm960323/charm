/*****************************************************************************
 * $Source$
 * $Author$
 * $Date$
 * $Revision$
 *****************************************************************************/

#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <iostream.h>
#include <string.h>
#include <stdlib.h>
//  #include <string>
//  #include <map>
//  using std::map;//<-- may cause problems for pre-ISO C++ compilers

#include "xi-util.h"


/******************* Utilities ****************/

class Chare;//Forward declaration
class Message;
class TParamList;
extern int fortranMode;
extern const char *cur_file;
void die(const char *why,int line=-1);

static char *msg_prefix(void) {return (char *) "CMessage_";}
static char *chare_prefix(void) {return (char *) "CProxy_";}

class Value : public Printable {
  private:
    int factor;
    char *val;
  public:
    Value(char *s);
    void print(XStr& str) { str << val; }
    int getIntVal(void);
};

class ValueList : public Printable {
  private:
    Value *val;
    ValueList *next;
  public:
    ValueList(Value* v, ValueList* n=0) : val(v), next(n) {}
    void print(XStr& str) {
      if(val) {
        str << "["; val->print(str); str << "]";
      }
      if(next)
        next->print(str);
    }
};

class Construct : public Printable {
  protected:
    int external;
  public:
    int line;
    void setExtern(int e) { external = e; }
    virtual void genDecls(XStr& str) = 0;
    virtual void genDefs(XStr& str) = 0;
    virtual void genReg(XStr& str) = 0;
};

class ConstructList : public Construct {
    Construct *construct;
    ConstructList *next;
  public:
    ConstructList(int l, Construct *c, ConstructList *n=0) :
      construct(c), next(n) {line = l;}
    void setExtern(int e);
    void print(XStr& str);
    void genDecls(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
};

/*********************** Type System **********************/
class Type : public Printable {
  public:
    virtual void print(XStr&) = 0;
    virtual int isVoid(void) const {return 0;}
    virtual int isBuiltin(void) const { return 0; }
    virtual int isMessage(void) const {return 0;}
    virtual int isTemplated(void) const { return 0; }
    virtual int isPointer(void) const {return 0;}
    virtual int isReference(void) const {return 0;}
    virtual int isConst(void) const {return 0;}
    virtual Type *deref(void) {return this;}
    virtual const char *getBaseName(void) = 0;
    virtual void genProxyName(XStr &str);
    virtual void genMsgProxyName(XStr& str);
    virtual void printVar(XStr &str, char *var) {print(str); str<<" "; str<<var;}
};

class BuiltinType : public Type {
  private:
    const char *name;
  public:
    BuiltinType(const char *n) : name(n) {}
    int isBuiltin(void) const {return 1;}
    void print(XStr& str) { str << name; }
    int isVoid(void) const { return !strcmp(name, "void"); }
    const char *getBaseName(void) { return name; }
};

class NamedType : public Type {
  private:
    const char *name;
    TParamList *tparams;
  public:
    NamedType(const char* n, TParamList* t=0)
       : name(n), tparams(t) {}
    int isTemplated(void) const { return (tparams!=0); }
    void print(XStr& str);
    const char *getBaseName(void) { return name; }
    virtual void genProxyName(XStr& str) { str << chare_prefix(); print(str);}
    virtual void genMsgProxyName(XStr& str) { str << msg_prefix(); print(str);}
};

class PtrType : public Type {
  private:
    Type *type;
    int numstars; // level of indirection
  public:
    PtrType(Type *t) : type(t), numstars(1) {}
    int isPointer(void) const {return 1;}
    int isMessage(void) const {return numstars==1 && !type->isBuiltin();}
    void indirect(void) { numstars++; }
    void print(XStr& str);
    const char *getBaseName(void) { return type->getBaseName(); }
    virtual void genMsgProxyName(XStr& str) { 
      if(numstars != 1) {
        die("too many stars-- entry parameter must have form 'MTYPE *msg'"); 
      } else {
        str << msg_prefix();
        type->print(str);
      }
    }
};

class ReferenceType : public Type {
  private:
    Type *referant;
  public:
    ReferenceType(Type *t) : referant(t) {}
    int isReference(void) const {return 1;}
    void print(XStr& str) {str<<referant<<" &";}
    virtual Type *deref(void) {return referant;}
    const char *getBaseName(void) { return referant->getBaseName(); }
};

class ConstType : public Type {
  private:
    Type *type;
  public:
    ConstType(Type *t) : type(t) {}
    int isConst(void) const {return 1;}
    void print(XStr& str) {str<<"const "<<type;}
    const char *getBaseName(void) { return type->getBaseName(); }
};

class TypeList : public Printable {
    Type *type;
    TypeList *next;
  public:
    TypeList(Type *t, TypeList *n=0) : type(t), next(n) {}
    void print(XStr& str);
    void genProxyNames(XStr& str, const char*, const char*, const char*);
    void genProxyNames2(XStr& str, const char*, const char*, 
                        const char*, const char*);
};

/**************** Parameter types & lists (for marshalling) ************/
class Parameter {
    Type *type;
    const char *name; /*The name of the variable, if any*/
    const char *given_name; /*The name of the msg in ci file, if any*/
    const char *arrLen; /*The expression for the length of the array;
    			 NULL if not an array*/
    Value *val; /*Initial value, if any*/
    int line;
    friend class ParamList;
    void pup(XStr &str);
    void marshallArraySizes(XStr &str);
    void marshallArrayData(XStr &str);
    void beginUnmarshall(XStr &str);
    void unmarshall(XStr &str);
    void unmarshallAddress(XStr &str);
    void endUnmarshall(XStr &str);
  public:
    Parameter(int Nline,Type *Ntype,const char *Nname=0,
    	const char *NarrLen=0,Value *Nvalue=0);
    void print(XStr &str,int withDefaultValues=0);
    void printAddress(XStr &str);
    void printValue(XStr &str);
    int isMessage(void) const {return type->isMessage();}
    int isVoid(void) const {return type->isVoid();}
    int isArray(void) const {return arrLen!=NULL;}
    Type *getType(void) {return type;}
    const char *getName(void) {return name;}
    void printMsg(XStr& str) {
      type->print(str);
      if(given_name!=0)
        str << given_name;
    }
};
class ParamList {
    Parameter *param;
    ParamList *next;
    typedef void (Parameter::*fn_t)(XStr &str);
    void callEach(fn_t f,XStr &str);
  public:
    ParamList(Parameter *Nparam,ParamList *Nnext=NULL)
    	:param(Nparam), next(Nnext) {}
    void print(XStr &str,int withDefaultValues=0);
    void printAddress(XStr &str);
    void printValue(XStr &str);
    int isMessage(void) const {
    	return (next==NULL) && param->isMessage();
    }
    int isVoid(void) const {
    	return (next==NULL) && param->isVoid();
    }
    int isMarshalled(void) const {
    	return !isVoid() && !isMessage();
    }
    const char *getBaseName(void) {
    	return param->type->getBaseName();
    }
    void genMsgProxyName(XStr &str) {
    	param->type->genMsgProxyName(str);
    }
    void printMsg(XStr& str) {
        param->printMsg(str);
    }
    void marshall(XStr &str,int orMakeVoid);
    void beginUnmarshall(XStr &str);
    void unmarshall(XStr &str);
    void unmarshallAddress(XStr &str);
    void endUnmarshall(XStr &str);
};

class FuncType : public Type {
  private:
    Type *rtype;
    const char *name;
    ParamList *params;
  public:
    FuncType(Type* r, const char* n, ParamList* p) 
    	:rtype(r),name(n),params(p) {}
    void print(XStr& str) { 
      rtype->print(str);
      str << "(*" << name << ")(";
      if(params)
        params->print(str);
    }
    const char *getBaseName(void) { return name; }
};

/****************** Template Support **************/
/* Template Instantiation Parameter */
class TParam : public Printable {
  public:
    virtual void genSpec(XStr& str)=0;
};

/* List of Template Instantiation parameters */
class TParamList : public Printable {
    TParam *tparam;
    TParamList *next;
  public:
    TParamList(TParam *t, TParamList *n=0) : tparam(t), next(n) {}
    void print(XStr& str);
    void genSpec(XStr& str);
};

/* A type instantiation parameter */
class TParamType : public TParam {
  Type *type;
  public:
    TParamType(Type *t) : type(t) {}
    void print(XStr& str) { type->print(str); }
    void genSpec(XStr& str) { type->print(str); }
};

/* A Value instantiation parameter */
class TParamVal : public TParam {
    char *val;
  public:
    TParamVal(char *v) : val(v) {}
    void print(XStr& str) { str << val; }
    void genSpec(XStr& str) { str << val; }
};
/* A template construct */
class TVarList;
class TEntity;

class Template : public Construct {
    TVarList *tspec;
    TEntity *entity;
  public:
    Template(TVarList *t, TEntity *e) : tspec(t), entity(e) {}
    void print(XStr& str);
    void genDecls(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
    void genSpec(XStr& str);
    void genVars(XStr& str);
};

/* An entity that could be templated, i.e. chare, group or a message */
class TEntity : public Construct {
  protected:
    Template *templat;
  public:
    void setTemplate(Template *t) { templat = t; }
    virtual void genTSpec(XStr& str) { if (templat) templat->genSpec(str); }
    virtual void genTVars(XStr& str) { if (templat) templat->genVars(str); }
};
/* A formal argument of a template */
class TVar : public Printable {
  public:
    virtual void genLong(XStr& str) = 0;
    virtual void genShort(XStr& str) = 0;
};

/* a formal type argument */
class TType : public TVar {
    Type *type;
    Type *init;
  public:
    TType(Type *t, Type *i=0) : type(t), init(i) {}
    void print(XStr& str);
    void genLong(XStr& str);
    void genShort(XStr& str);
};

/* a formal function argument */
class TFunc : public TVar {
    FuncType *type;
    char *init;
  public:
    TFunc(FuncType *t, char *v=0) : type(t), init(v) {}
    void print(XStr& str) { type->print(str); if(init) str << "=" << init; }
    void genLong(XStr& str){ type->print(str); if(init) str << "=" << init; }
    void genShort(XStr& str) {str << type->getBaseName(); }
};

/* A formal variable argument */
class TName : public TVar {
    Type *type;
    char *name;
    char *val;
  public:
    TName(Type *t, char *n, char *v=0) : type(t), name(n), val(v) {}
    void print(XStr& str);
    void genLong(XStr& str);
    void genShort(XStr& str);
};

/* A list of formal arguments to a template */
class TVarList : public Printable {
    TVar *tvar;
    TVarList *next;
  public:
    TVarList(TVar *v, TVarList *n=0) : tvar(v), next(n) {}
    void print(XStr& str);
    void genLong(XStr& str);
    void genShort(XStr& str);
};

/******************* Chares, Arrays, Groups ***********/

/* Member of a chare or group, i.e. entry, RO or ROM */
class Member : public Construct {
  protected:
    Chare *container;
  public:
    virtual void setChare(Chare *c) { container = c; }
    virtual int isPure(void) { return 0; }
    virtual int isSdag(void) { return 0; }
    virtual void collectSdagCode(XStr& str, int& sdagPresent) { return; }
    XStr makeDecl(const char *returnType);
};

/* List of members of a chare or group */
class MemberList : public Printable {
    Member *member;
    MemberList *next;
  public:
    MemberList(Member *m, MemberList *n=0) : member(m), next(n) {}
    void print(XStr& str);
    void setChare(Chare *c);
    int isPure(void);
    void genDecls(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
    void collectSdagCode(XStr& str, int& sdagPresent);
};

/* Chare or group is a templated entity */
class Chare : public TEntity {
  public:
    enum { //Set these attribute bits in "attrib"
    	CABSTRACT=1<<1, 
    	CMIGRATABLE=1<<2,
    	CMAINCHARE=1<<10,
    	CARRAY=1<<11,
    	CGROUP=1<<12,
    	CNODEGROUP=1<<13
    };
    typedef unsigned int attrib_t;
  protected:
    attrib_t attrib;
    	
    NamedType *type;
    MemberList *list;
    TypeList *bases;
    int entryCount;

    void genRegisterMethodDecl(XStr& str);
    void genRegisterMethodDef(XStr& str);
  public:
    Chare(int ln, attrib_t Nattr,
    	NamedType *t, TypeList *b=0, MemberList *l=0);
    void genProxyBases(XStr& str,const char* p,const char* s,const char* sep) {
      bases->genProxyNames(str, p, s, sep);
    }
    XStr proxyName(int withTemplates=1) 
    {
    	XStr str;
    	str<<proxyPrefix()<<type;
    	if (withTemplates) genTVars(str);
    	return str;
    }
    XStr baseName(int withTemplates=1) 
    {
    	XStr str;
    	str<<type->getBaseName();
    	if (withTemplates) genTVars(str);
    	return str;
    }
    int  isTemplated(void) { return (templat!=0); }
    int  isDerived(void) { return (bases!=0); }
    int  isAbstract(void) { return attrib&CABSTRACT; }
    int  isMigratable(void) { return attrib&CMIGRATABLE; }
    int  isMainChare(void) {return attrib&CMAINCHARE;}
    int  isArray(void) {return attrib&CARRAY;}
    int  isGroup(void) {return attrib&CGROUP;}
    int  isNodeGroup(void) {return attrib&CNODEGROUP;}
    void setAbstract(int a) { if (a) attrib|=CABSTRACT; else attrib&=~CABSTRACT; }
    void print(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
    void genDecls(XStr &str);
    int nextEntry(void) {return entryCount++;}
    virtual void genSubDecls(XStr& str);
    virtual char *chareTypeName(void) {return (char *)"chare";}
    virtual char *proxyPrefix(void) {return (char *) "CProxy_";}
};

class MainChare : public Chare {
  public:
    MainChare(int ln, attrib_t Nattr, 
    	NamedType *t, TypeList *b=0, MemberList *l=0):
	    Chare(ln, Nattr|CMAINCHARE, t,b,l) {}
    virtual char *chareTypeName(void) {return (char *) "mainchare";}
};

class Array : public Chare {
  protected:
    XStr indexSuffix;
    XStr indexType;//"CkArrayIndex"+indexSuffix;
  public:
    Array(int ln, attrib_t Nattr, NamedType *index,
    	NamedType *t, TypeList *b=0, MemberList *l=0);
    virtual int is1D(void) {return indexSuffix==(const char*)"1D";}
    virtual void genSubDecls(XStr& str);
    virtual char *chareTypeName(void) {return (char *) "array";}
};

class Group : public Chare {
  public:
    Group(int ln, attrib_t Nattr,
    	NamedType *t, TypeList *b=0, MemberList *l=0):
	    Chare(ln,Nattr|CGROUP,t,b,l) {}
    virtual void genSubDecls(XStr& str);
    virtual char *chareTypeName(void) {return (char *) "group";}
};

class NodeGroup : public Group {
  public:
    NodeGroup(int ln, attrib_t Nattr,
    	NamedType *t, TypeList *b=0, MemberList *l=0):
	    Group(ln,Nattr|CNODEGROUP,t,b,l) {}
    virtual char *chareTypeName(void) {return (char *) "nodegroup";}
};


/****************** Messages ***************/
class Message; // forward declaration

class MsgVar {
 public:
  Type *type;
  char *name;
  MsgVar(Type *t, char *n) : type(t), name(n) {}
  Type *getType() { return type; }
  char *getName() { return name; }
  void print(XStr &str) {type->print(str);str<<" "<<name<<"[];";}
};

class MsgVarList : public Printable {
 public:
  MsgVar *msg_var;
  MsgVarList *next;
  MsgVarList(MsgVar *mv, MsgVarList *n=0) : msg_var(mv), next(n) {}
  void print(XStr &str) {
    msg_var->print(str);
    str<<"\n";
    if(next) next->print(str);
  }
  int len(void) { return (next==0)?1:(next->len()+1); }
};

class Message : public TEntity {
    NamedType *type;
    MsgVarList *mvlist;
    void printVars(XStr& str) {
      if(mvlist!=0) {
        str << "{\n";
        mvlist->print(str);
        str << "}\n";
      }
    }
  public:
    Message(int l, NamedType *t, MsgVarList *mv=0)
      : type(t), mvlist(mv) 
      { line=l; setTemplate(0); }
    void print(XStr& str);
    void genDecls(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
    virtual char *proxyPrefix(void) {return msg_prefix();}
    void genAllocDecl(XStr& str);
    int numVars(void) { return ((mvlist==0) ? 0 : mvlist->len()); }
};

/******************* Entry Point ****************/
// Entry attributes
#define STHREADED 0x01
#define SSYNC     0x02
#define SLOCKED   0x04
#define SVIRTUAL  0x08
#define SPURE     0x10
#define SMIGRATE  0x20 //<- is magic migration constructor
#define SCREATEHERE   0x40 //<- is a create-here-if-nonexistant
#define SCREATEHOME   0x80 //<- is a create-at-home-if-nonexistant

/* An entry construct */
class Entry : public Member {
  private:
    int line,entryCount;
    int attribs;
    Type *retType;
    char *name;
    ParamList *param;
    Value *stacksize;
    XStr *sdagCode;

    XStr epIdx(int include__idx_=1);
    void genEpIdxDecl(XStr& str);
    void genEpIdxDef(XStr& str);
    
    void genChareDecl(XStr& str);
    void genChareStaticConstructorDecl(XStr& str);
    void genChareStaticConstructorDefs(XStr& str);
    void genChareDefs(XStr& str);
    
    void genArrayDefs(XStr& str);
    void genArrayStaticConstructorDecl(XStr& str);
    void genArrayStaticConstructorDefs(XStr& str);
    void genArrayDecl(XStr& str);
    
    void genGroupDecl(XStr& str);
    void genGroupStaticConstructorDecl(XStr& str);
    void genGroupStaticConstructorDefs(XStr& str);
    void genGroupDefs(XStr& str);
    
    XStr paramType(int withDefaultVals);
    XStr paramComma(int withDefaultVals);
    XStr marshallMsg(int orMakeVoid=1);
    XStr callThread(const XStr &procName,int prependEntryName=0);
  public:
    Entry(int l, int a, Type *r, char *n, ParamList *p, Value *sz=0);
    void setChare(Chare *c);
    int getStackSize(void) { return (stacksize ? stacksize->getIntVal() : 0); }
    int isThreaded(void) { return (attribs & STHREADED); }
    int isSync(void) { return (attribs & SSYNC); }
    int isConstructor(void) { return !strcmp(name, container->baseName(0).get_string());}
    int isExclusive(void) { return (attribs & SLOCKED); }
    int isVirtual(void) { return (attribs & SVIRTUAL); }
    int isCreate(void) { return (attribs & SCREATEHERE)||(attribs & SCREATEHOME); }
    int isCreateHome(void) { return (attribs & SCREATEHOME); }
    int isCreateHere(void) { return (attribs & SCREATEHERE); }
    const char *Virtual(void) {return isVirtual()?"virtual ":"";}
    int isPure(void) { return (attribs & SPURE); }
    int isSdag(void) { return (sdagCode!=0); }
    void print(XStr& str);
    void genDecls(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
    void setSdagCode(char *str) {
      if(str!=0) {
        if(!param->isMessage()) 
          die("Marshalling or void methods unsupported in sdag yet", line);
        sdagCode = new XStr("sdagentry ");
	*sdagCode << name << "(";
        param->printMsg(*sdagCode);
        *sdagCode << ") ";
        *sdagCode << "{\n" << str << "\n}\n";
      }
    }
    void collectSdagCode(XStr& str, int& sdagPresent) {
       if(isSdag()) {
         str << *sdagCode;
         sdagPresent = 1;
       }
    }
};


/****************** Modules, etc. ****************/
class Module : public Construct {
    int _isMain;
    char *name;
    ConstructList *clist;
  public:
    Module(int l, char *n, ConstructList *c) : name(n), clist(c) { 
	    line = l;
	    _isMain=0; 
    }
    void print(XStr& str);
    void generate();
    void genDecls(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
    void setMain(void) { _isMain = 1; }
    int isMain(void) { return _isMain; }
};

class ModuleList : public Printable {
    Module *module;
    ModuleList *next;
  public:
    int line;
    ModuleList(int l, Module *m, ModuleList *n=0) : line(l),module(m),next(n) {}
    void print(XStr& str);
    void generate();
};

class Readonly : public Member {
    int msg; // is it a readonly var(0) or msg(1) ?
    Type *type;
    char *name;
    ValueList *dims;
  public:
    Readonly(int l, Type *t, char *n, ValueList* d, int m=0) 
	    : msg(m), type(t), name(n)
            { line=l; dims=d; setChare(0); }
    void print(XStr& str);
    void genDecls(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
};

class InitCall : public Member {
    const char *name;
public:
    InitCall(int l, const char *n);
    void print(XStr& str);
    void genDecls(XStr& str);
    void genDefs(XStr& str);
    void genReg(XStr& str);
};

#endif
