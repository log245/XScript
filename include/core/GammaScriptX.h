﻿#ifndef __GAMMA_SCRIPTX_H__
#define __GAMMA_SCRIPTX_H__
#pragma warning(disable: 4624)
#pragma warning(disable: 4510)
#pragma warning(disable: 4610)

#include "common/Help.h"
#include "common/TList.h"
#include "core/GammaScriptDef.h"
#include "core/GammaScriptWrap.h"

//=====================================================================
// GammaScriptX.h 
// 定义脚本和C++接口的辅助函数和辅助宏
// 使用辅助宏可以快速注册c++类型以及函数
// 柯达昭
// 2007-10-21
//=====================================================================

//==========================================================================
// 注册类的辅助类和宏
//==========================================================================
#define NORMAL_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVTable ) \
class _class##Construct : public IObjectConstruct \
{\
	virtual void Assign( void* pDest, void* pSrc )	\
	{ *(_class*)pDest = *(_class*)pSrc; }\
	virtual void Construct( void* pObj ) \
	{ \
		_class* pNew = new( pObj )_class;\
		if( !funGetVirtualTable ) return; \
		( (SVirtualObj*)pNew )->m_pTable = pVTable; \
	}\
	virtual void Destruct( void* pObj )		\
	{ static_cast<_class*>( pObj )->~_class(); }\
};

#define UNDUPLICATION_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVTable ) \
class _class##Construct : public IObjectConstruct \
{\
	virtual void Assign( void* pDest, void* pSrc )	\
	{ throw( "Can not call construct on unduplication object" ); }\
	virtual void Construct( void* pObj ) \
	{ \
		_class* pNew = new( pObj )_class;\
		if( !funGetVirtualTable ) return; \
		( (SVirtualObj*)pNew )->m_pTable = pVTable; \
	}\
	virtual void Destruct( void* pObj )		\
	{ static_cast<_class*>( pObj )->~_class(); }\
};

#define DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual )	\
struct SGetVTable { SGetVTable() \
{ \
	if( !funGetVirtualTable || pVirtual ) return; \
	pVirtual = funGetVirtualTable( this );\
} };

class CScriptRegisterNode : public Gamma::TList<CScriptRegisterNode>::CListNode
{
	void(*m_funRegister)();
	typedef typename Gamma::TList<CScriptRegisterNode>::CListNode ParentType;
public:
	CScriptRegisterNode( Gamma::TList<CScriptRegisterNode>& list, void(*fun)() )
		: m_funRegister( fun )
	{
		list.PushBack(*this);
	}

	bool Register() 
	{ 
		m_funRegister(); 
		auto n = GetNext(); 
		Remove(); 
		return n ? n->Register() : true;
	}
};

typedef Gamma::TList<CScriptRegisterNode> CScriptRegisterList;
struct SGlobalExe { SGlobalExe( bool ) {} };

template<typename _Derive, typename ... _Base>
class TInheritInfo
{
public:
	enum { size = sizeof...(_Base) + 1 };
	template<typename...Param> struct TOffset {};
	template<> struct TOffset<> { static void Get(ptrdiff_t* ary) {} };

	template<typename First, typename...Param>
	struct TOffset<First, Param...> { static void Get(ptrdiff_t* ary)
	{ *ary = ((ptrdiff_t)(First*)(_Derive*)0x40000000) - 0x40000000;
	  TOffset<Param...>::Get(++ary); } };

	static const ptrdiff_t* Values()
	{
		static ptrdiff_t result[size] = { sizeof(_Derive) };
		TOffset<_Base...>::Get(&result[1]);
		return result;
	}

	static const char** Types()
	{
		static const char* result[size] = 
		{ typeid(_Derive).name(), typeid(_Base...).name()... };
		return result;
	}
};


//====================================================================
// 普通类注册
//====================================================================
#define REGIST_CLASS_FUNCTION_BEGIN( _class, ... ) \
	namespace _class##_namespace { \
	static SGlobalExe _class##_register( \
	CScriptBase::RegistClass( #_class, \
	TInheritInfo<_class, ##__VA_ARGS__>::size, \
	TInheritInfo<_class, ##__VA_ARGS__>::Types(), \
	TInheritInfo<_class, ##__VA_ARGS__>::Values() ) ); \
	typedef _class org_class; CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	struct _first : public _class, public SGetVTable{};\
	typedef _first 


#define REGIST_CLASS_FUNCTION_BEGIN_UNDUPLICATION( _class, ... ) \
	namespace _class##_namespace { \
	static SGlobalExe _class##_register( \
	CScriptBase::RegistClass( #_class, \
	TInheritInfo<_class, ##__VA_ARGS__>::size, \
	TInheritInfo<_class, ##__VA_ARGS__>::Types(), \
	TInheritInfo<_class, ##__VA_ARGS__>::Values() ) ); \
	typedef _class org_class; CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	struct _first : public _class, public SGetVTable{};\
	typedef _first 


#define REGIST_CLASS_FUNCTION_BEGIN_ABSTRACT( _class, ... ) \
	namespace _class##_namespace { \
	static SGlobalExe _class##_register( \
	CScriptBase::RegistClass( #_class, \
	TInheritInfo<_class, ##__VA_ARGS__>::size, \
	TInheritInfo<_class, ##__VA_ARGS__>::Types(), \
	TInheritInfo<_class, ##__VA_ARGS__>::Values() ) ); \
	typedef _class org_class; CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	typedef struct _first {}


#define REGIST_CLASS_FUNCTION_END() _last;\
	struct _class : public _last {}; \
	static SGlobalExe _class_fun_register( listRegister.GetFirst()->Register() ); \
	NORMAL_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVirtual ); \
	static _class##Construct s_Instance; \
	static SGlobalExe _class_construct_register( \
	CScriptBase::RegistConstruct( &s_Instance, typeid( org_class ).name() ) ); }


#define REGIST_CLASS_FUNCTION_END_UNDUPLICATION() _last;\
	struct _class : public _last {}; \
	static SGlobalExe _class_fun_register( listRegister.GetFirst()->Register() ); \
	UNDUPLICATION_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVirtual ); \
	static _class##Construct s_Instance; \
	static SGlobalExe _class_construct_register( \
	CScriptBase::RegistConstruct( &s_Instance, typeid( org_class ).name() ) ); }


#define REGIST_CLASS_FUNCTION_END_ABSTRACT() \
	_class; static SGlobalExe _class_fun_register( listRegister.GetFirst()->Register() ); \
	static SGlobalExe _class_construct_register( \
	CScriptBase::RegistConstruct( NULL, typeid( org_class ).name() ) ); }


#define REGIST_CLASSFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class \
	{ \
		static void Register()\
		{ \
			CScriptBase::RegistClassFunction( Gamma::MakeClassFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function );\
		} \
	};  \
	static CScriptRegisterNode _function##RegisterNode( listRegister, &_function##_Impl_Class::Register ); \
	typedef _function##_Base_Class 


#define REGIST_CLASSFUNCTION_WITHNAME( _function, _function_name ) \
	_function_name##_Base_Class; struct _function_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			CScriptBase::RegistClassFunction( Gamma::MakeClassFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function_name );\
		} \
	};  \
	static CScriptRegisterNode _function_name##RegisterNode( listRegister, &_function_name##_Impl_Class::Register ); \
	typedef _function_name##_Base_Class 


#define REGIST_CLASSFUNCTION_OVERLOAD( _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_fun_name_lua##_Base_Class; struct _fun_name_lua##_Impl_Class \
	{ \
		static void Register()\
		{ \
			CScriptBase::RegistClassFunction( Gamma::MakeClassFunArg( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), \
			Gamma::CreateFunWrap( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), typeid( org_class ).name(), #_fun_name_lua );\
		} \
	};  \
	static CScriptRegisterNode _fun_name_lua##RegisterNode( listRegister, &_fun_name_lua##_Impl_Class::Register ); \
	typedef _fun_name_lua##_Base_Class 


#define REGIST_STATICFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class \
	{ \
		static void Register()\
		{ \
			CScriptBase::RegistClassStaticFunction( Gamma::MakeFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function );\
		} \
	};  \
	static CScriptRegisterNode _function##RegisterNode( listRegister, &_function##_Impl_Class::Register ); \
	typedef _function##_Base_Class 


#define REGIST_STATICFUNCTION_WITHNAME( _function, _function_name ) \
	_function_name##_Base_Class; struct _function_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			CScriptBase::RegistClassStaticFunction( Gamma::MakeFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function_name );\
		} \
	};  \
	static CScriptRegisterNode _function_name##RegisterNode( listRegister, &_function_name##_Impl_Class::Register ); \
	typedef _function_name##_Base_Class 


#define REGIST_STATICFUNCTION_OVERLOAD( _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_fun_name_lua##_Base_Class; struct _fun_name_lua##_Impl_Class \
	{ \
		static void Register()\
		{ \
			CScriptBase::RegistClassStaticFunction( Gamma::MakeFunArg( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), \
			Gamma::CreateFunWrap( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), typeid( org_class ).name(), #_fun_name_lua );\
		} \
	};  \
	static CScriptRegisterNode _fun_name_lua##RegisterNode( listRegister, &_fun_name_lua##_Impl_Class::Register ); \
	typedef _fun_name_lua##_Base_Class 


#define REGIST_CLASSMEMBER_GETSET( _member, get, set ) \
	_member##_Base_Class; struct _member##_Impl_Class \
	{ \
		static void Register()\
		{ \
			org_class* c = (org_class*)0x4000000; IFunctionWrap* funGetSet[2];\
			funGetSet[0] = get ? Gamma::CreateMemberGetWrap( c, &c->_member ) : NULL;\
			funGetSet[1] = set ? Gamma::CreateMemberSetWrap( c, &c->_member ) : NULL;\
			CScriptBase::RegistClassMember( Gamma::MakeMemberArg( c, &c->_member ),\
			funGetSet, typeid( org_class ).name(), #_member );\
		} \
	};  \
	static CScriptRegisterNode _member##_get_RegisterNode( listRegister, &_member##_Impl_Class::Register ); \
	typedef _member##_Base_Class 


#define REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, get, set ) \
	_new_name##_Base_Class; struct _new_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			org_class* c = (org_class*)0x4000000; IFunctionWrap* funGetSet[2];\
			funGetSet[0] = get ? Gamma::CreateMemberGetWrap( c, &c->_member ) : NULL;\
			funGetSet[1] = set ? Gamma::CreateMemberSetWrap( c, &c->_member ) : NULL;\
			CScriptBase::RegistClassMember( Gamma::MakeMemberArg( c, &c->_member ),\
			funGetSet, typeid( org_class ).name(), #_new_name );\
		} \
	};  \
	static CScriptRegisterNode _new_name##_get_RegisterNode( listRegister, &_new_name##_Impl_Class::Register ); \
	typedef _new_name##_Base_Class 


#define REGIST_CLASSMEMBER_SET( _member ) \
	REGIST_CLASSMEMBER_GETSET( _member, false, true )


#define REGIST_CLASSMEMBER_SET_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, false, true )


#define REGIST_CLASSMEMBER_GET( _member ) \
	REGIST_CLASSMEMBER_GETSET( _member, true, false )


#define REGIST_CLASSMEMBER_GET_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, true, false )


#define REGIST_CLASSMEMBER( _member ) \
	REGIST_CLASSMEMBER_GETSET( _member, true, true )


#define REGIST_CLASSMEMBER_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, true, true )


#define REGIST_DESTRUCTOR() \
	destructor_Base_Class; struct destructor_Impl_Class \
	{ \
		static void Register()\
		{ \
			Gamma::BindDestructorWrap<org_class>( CScriptBase::RegistDestructor( typeid( org_class ).name(), \
			Gamma::CreateDestructorWrap<org_class>( Gamma::GetDestructorFunIndex<org_class>() ) ) );\
		} \
	};  \
	static CScriptRegisterNode destructor##_register_node( listRegister, &destructor_Impl_Class::Register ); \
	typedef destructor_Base_Class


#define REGIST_GLOBALFUNCTION( _function ) \
	SGlobalExe _function##_register( CScriptBase::RegistFunction( \
	Gamma::MakeFunArg( &_function ), Gamma::CreateFunWrap( &_function ), typeid( _function ).name(), #_function ) ); 


#define REGIST_GLOBALFUNCTION_WITHNAME( _function, _function_name ) \
	SGlobalExe _function_name##_register( CScriptBase::RegistFunction( \
	Gamma::MakeFunArg( &_function ), Gamma::CreateFunWrap( &_function ), NULL, #_function_name ) ); 


#define REGIST_GLOBALFUNCTION_OVERLOAD(  _fun_type, _fun_name_cpp, _fun_name_lua ) \
    SGlobalExe _fun_name_lua##_register( CScriptBase::RegistFunction( \
	Gamma::MakeFunArg( static_cast<_fun_type>(&_fun_name_cpp) ), \
	Gamma::CreateFunWrap( static_cast<_fun_type>(&_fun_name_cpp) ), NULL, #_fun_name_lua ) ); 	


#define REGIST_CALLBACKFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class : public _function##_Base_Class \
	{	\
		struct __class : public org_class { \
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, false, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register(){ __class::Bind( CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	static CScriptRegisterNode _function##_register_node( listRegister, &_function##_Impl_Class::Register ); \
	static SGlobalExe _function##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable );\
	typedef _function##_Impl_Class 


#define REGIST_CALLBACKFUNCTION_WITHNAME( _function, _function_name ) \
	_function##_Base_Class; struct _function_name##_Impl_Class : public _function##_Base_Class \
	{	\
		struct __class : public org_class { typedef _function##_Impl_Class T; \
		static void* GetVirtualTable( SGetVTable* p ){ return ((SVirtualObj*)(T*)( p ))->m_pTable; }\
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, false, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register(){ __class::Bind( CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function_name ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	static CScriptRegisterNode _function_name##_register_node( listRegister, &_function_name##_Impl_Class::Register ); \
	static SGlobalExe _function_name##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable );\
	typedef _function_name##_Impl_Class 


#define REGIST_CALLBACKFUNCTION_OVERLOAD( _function, _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_function##_Base_Class; struct _fun_name_lua##_Impl_Class : public _function##_Base_Class \
	{	\
		struct __class : public org_class { typedef _function##_Impl_Class T; \
		static void* GetVirtualTable( SGetVTable* p ){ return ((SVirtualObj*)(T*)( p ))->m_pTable; }\
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, false, static_cast<_fun_type>(&__class::_fun_name_cpp) ) ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( static_cast<_fun_type>(&__class::_fun_name_cpp) ); }\
		static STypeInfoArray MakeFunArg() { return Gamma::MakeClassFunArg( static_cast<_fun_type>(&__class::_fun_name_cpp) ); } }; \
		static void Register(){ __class::Bind( CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_fun_name_lua ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	static CScriptRegisterNode _fun_name_lua##_register_node( listRegister, &_fun_name_lua##_Impl_Class::Register ); \
	static SGlobalExe _fun_name_lua##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_fun_name_lua##_Impl_Class::GetVirtualTable );\
	typedef _fun_name_lua##_Impl_Class 	


#define REGIST_PUREVIRTUALFUNCTION( _function ) \
	_function##_Base_Class; \
	auto _function##_Type = &org_class::_function;\
	typedef decltype( GetFunTypeExplain( _function##_Type ) ) _function##_TypeExplain;\
	struct _function##_Impl_Class : public _function##_Base_Class \
	{	\
		typedef _function##_TypeExplain E;\
		DEFINE_PUREVIRTUAL_IMPLEMENT( _function, _function##_Base_Class );\
		struct __class : public org_class { \
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register(){ __class::Bind( CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	static CScriptRegisterNode _function##_register_node( listRegister, &_function##_Impl_Class::Register ); \
	static SGlobalExe _function##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable );\
	typedef _function##_Impl_Class 


#define REGIST_PUREVIRTUALFUNCTION_WITHNAME( _function, _function_name ) \
	_function##_Base_Class; \
	auto _function_name##_Type = &org_class::_function;\
	typedef decltype( GetFunTypeExplain( _function##_Type ) ) _function_name##_TypeExplain;\
	struct _function_name##_Impl_Class : public _function##_Base_Class \
	{	\
		typedef _function_name##_TypeExplain E;\
		DEFINE_PUREVIRTUAL_IMPLEMENT( _function, _function##_Base_Class );\
		struct __class : public org_class { typedef _function##_Impl_Class T; \
		static void* GetVirtualTable( SGetVTable* p ){ return ((SVirtualObj*)(T*)( p ))->m_pTable; }\
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register(){ __class::Bind( CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function_name ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	static CScriptRegisterNode _function_name##_register_node( listRegister, &_function_name##_Impl_Class::Register ); \
	static SGlobalExe _function_name##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable );\
	typedef _function_name##_Impl_Class 


#define REGIST_PUREVIRTUALFUNCTION_OVERLOAD( _function, _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_function##_Base_Class; \
	auto _function_name##_Type = &org_class::_function;\
	typedef decltype( GetFunTypeExplain( _function##_Type ) ) _function_name##_TypeExplain;\
	struct _fun_name_lua##_Impl_Class : public _function##_Base_Class \
	{	\
		DEFINE_PUREVIRTUAL_IMPLEMENT( _function, _function##_Base_Class );\
		struct __class : public org_class { typedef _function##_Impl_Class T; \
		static void* GetVirtualTable( SGetVTable* p ){ return ((SVirtualObj*)(T*)( p ))->m_pTable; }\
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, static_cast<_fun_type>(&__class::_fun_name_cpp) ) ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( static_cast<_fun_type>(&__class::_fun_name_cpp) ); }\
		static STypeInfoArray MakeFunArg() { return Gamma::MakeClassFunArg( static_cast<_fun_type>(&__class::_fun_name_cpp) ); } }; \
		static void Register(){ __class::Bind( CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_fun_name_lua ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	static CScriptRegisterNode _fun_name_lua##_register_node( listRegister, &_fun_name_lua##_Impl_Class::Register ); \
	static SGlobalExe _fun_name_lua##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_fun_name_lua##_Impl_Class::GetVirtualTable );\
	typedef _fun_name_lua##_Impl_Class 


#define REGIST_ENUMTYPE( EnumType ) \
    static SGlobalExe EnumType##_register( \
    CScriptBase::RegistEnum( typeid( EnumType ).name(), #EnumType, (int32)sizeof(EnumType) ) );


#endif
