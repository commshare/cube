#pragma once
struct TiXmlHelperSimple {};

template<class T, class U>
class Conversion
{
	typedef char Small;
	class Big { char dummy[2]; };
	static Small Test(U);
	static Big Test(...);
	static T MakeT();
public:
	enum { exists = sizeof(Test(MakeT())) == sizeof(Small) };
};
namespace Conversion2Helper
{
	typedef char Small;
	struct Big {char dummy[2];}; 

	template<typename Type,Type Ptr>
	struct MemberHelperClass;

	template<typename T,typename Type>
	Small MemberHelper_f(MemberHelperClass<Type,&T::TypeHelperFunc> *);
	template<typename T,typename Type>
	Small MemberHelper_f(MemberHelperClass<Type,&T::TypeHelperFunc1> *);
	template<typename T,typename Type>
	Small MemberHelper_f(MemberHelperClass<Type,&T::TypeHelperFunc2> *);
	template<typename T,typename Type>
	Big MemberHelper_f(...);
}

template<typename T, typename Type>
class Conversion2
{
public:
	enum { exists = sizeof(Conversion2Helper::MemberHelper_f<T,Type>(0)) == sizeof(Conversion2Helper::Small) };
};
#define SETBASECLASSFUNCTYPE(baseclasstype) public: static void TypeHelperFunc(baseclasstype)
#define SETBASECLASSFUNCTYPE1(baseclasstype) public: static void TypeHelperFunc1(baseclasstype)
#define SETBASECLASSFUNCTYPE2(baseclasstype) public: static void TypeHelperFunc2(baseclasstype)
#define GetConversion2_exists(classtype, baseclasstype) Conversion2<classtype,void(*)(baseclasstype)>::exists
