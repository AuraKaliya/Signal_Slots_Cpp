#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <string>
#include <functional>
#include <map>

//===============================================信号-槽 管理器==========================================
template<typename... Args>
class SignalManager
{
public:
    static SignalManager<Args...>*GetInstance()
    {
        if(m_instance==nullptr)
        {
            m_instance=new SignalManager();
        }
        return m_instance;
    }

    void RegisterSignal(const std::string & signalName,const std::string & emitObjGID,std::function<void(Args...)> slot)
    {

        if(m_signalToSlotDictionary.find(signalName)==m_signalToSlotDictionary.end())
        {
            m_signalToSlotDictionary.insert(std::make_pair(signalName,std::multimap<std::string,std::function<void(Args...)> >{}));
        }

        m_signalToSlotDictionary[signalName].insert(std::make_pair(emitObjGID,slot));
    }

    void SoluteSignal(const std::string& signalName,const std::string emitObjGID,Args... args)
    {
        auto it=m_signalToSlotDictionary.find(signalName);
        if(it!=m_signalToSlotDictionary.end())
        {
            auto range = it->second.equal_range(emitObjGID);
            for(auto function=range.first;function !=range.second;function ++)
            {
                function->second(std::forward<decltype(args)>(args)...);
            }
        }
    }

    auto GetSolutionBySignal(const std::string& signalName,const std::string emitObjGID)
    {
        auto it=m_signalToSlotDictionary.find(signalName);
        if(it!=m_signalToSlotDictionary.end())
        {
            auto range = it->second.equal_range(emitObjGID);
            auto lambda=[=](auto &&... args){
                for(auto function=range.first;function !=range.second;function ++)
                {
                    function->second(std::forward<decltype(args)>(args)...);
                }
            };
            return lambda;
        }
    }

private:
    static SignalManager<Args...>* m_instance;
    SignalManager()=default;

    std::map<std::string,std::multimap<std::string,std::function<void(Args...)> > > m_signalToSlotDictionary;
};

template<typename... Args>
SignalManager<Args...>* SignalManager<Args...>::m_instance=nullptr;

//===============================================信号-槽 管理器==========================================

//=============================register 相关   信号注册的宏  ==============================
#define REGISTER_SIGNAL_STATIC(signalObj,signalName,slotFunc,... ) \
do { \
   std::function<void(__VA_ARGS__)> func =[](auto&&... args)->decltype(auto) \
        {\
return (*slotFunc)(std::forward<decltype(args)>(args)...);\
};\
 SignalManager<__VA_ARGS__>::GetInstance()->RegisterSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)), func); \
} while (false)
//   ++++++++++++++++++++++++++++由于REGISTER_SIGNAL也能处理多参情况  所以合并使用 +++++++++++++++++++++++++++++++++++
#define REGISTER_SIGNAL(signalObj,slotObj,signalName,slotFunc,... ) \
do { \
   std::function<void(__VA_ARGS__)> func =[slotObj](auto&&... args)->decltype(auto) \
        {\
return (slotObj->*slotFunc)(std::forward<decltype(args)>(args)...);\
};\
 SignalManager<__VA_ARGS__>::GetInstance()->RegisterSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)), func); \
} while (false)
//=============================register==============================

//===========================emit  相关 信号发射宏=======================================

// 信号发射宏 
/*
    ！存在问题： EMIT_SIGNAL_ARGS在多参数时直接使用编译错误，而展开后又能正常运行。 目前EMIT_SIGNAL_ARGS也可以处理无参情况
*/

//#define EMIT_SIGNAL(signalObj, signalName) \
//    SignalManager<>::GetInstance()->SoluteSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)));

//#define EMIT_SIGNAL_ARGS(signalObj, signalName,...) \
//    SignalManager<FUNCTION_EXPAND(decltype,__VA_ARGS__)>::GetInstance()->SoluteSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)),__VA_ARGS__);

//#define EMIT_SIGNAL_ARGS(signalObj, signalName,...) \
//do { \
//    SignalManager<FUNCTION_EXPAND(decltype,__VA_ARGS__)>::GetInstance()->SoluteSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)),__VA_ARGS__);\
//} while (false)


//==============目前这个在MinGW中使用正常
#define EMIT_SIGNAL(signalObj, signalName,...) \
do { \
SignalManager<FUNCTION_EXPAND(decltype,__VA_ARGS__)>::GetInstance()->GetSolutionBySignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)))(__VA_ARGS__);\
} while (false)

//#define EMIT_SIGNAL_LAMBDA(signalObj, signalName,...) \
//do{\
//    auto lambda=[signalObj](auto&&... args)->decltype(auto)\
//{ \
//    return (SignalManager<decltype(args)...>::GetInstance()->SoluteSignal)(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)),std::forward<decltype(args)>(args)...);\
//};\
//lambda(__VA_ARGS__);\
//}while(false)\

//#define EMIT_SIGNAL_ARGS(signalObj, signalName,...) \
//do{\
//    auto lambda=[signalObj](auto&&... args)->decltype(auto)\
//{ \
//    return (SignalManager<FUNCTION_EXPAND(decltype,__VA_ARGS__)>::GetInstance()->SoluteSignal)(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)),std::forward<decltype(args)>(args)...);\
//};\
//lambda(__VA_ARGS__);\
//}while(false)\

//===========================emit=======================================


//====================================ARGS  SOLUTION======================================================

//#define REGISTER_SIGNAL_STATIC_ARGS(signalObj,signalName,slotFunc,... ) \
//do { \
//auto boundFunction = std::bind(slotFunc,ARGS_COUNT_RECURSIVE_EXPAND(__VA_ARGS__));\
//        std::function<void(__VA_ARGS__)> func = boundFunction;\
//        SignalManager<__VA_ARGS__>::GetInstance()->RegisterSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)), func); \
//} while (false)

#define ARGS_COUNT_RECURSIVE_EXPAND(...) ARGS_RECURSIVE_EXPAND_(ARGS_COUNT(__VA_ARGS__))
#define ARGS_RECURSIVE_EXPAND_(N) ARGS_RECURSIVE_EXPAND(N)

//计算参数个数
#define ARGS_COUNT(...) INTERNAL_ARGS_COUNT_PRIVATE(\
    0, ##__VA_ARGS__,\
	64, 63, 62, 61, 60, \
	59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
	39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
	29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
	 9,  8,  7,  6,  5,  4,  3,  2,  1,  0)

#define INTERNAL_ARGS_COUNT_PRIVATE(\
	 _0,  _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, \
	_10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
	_20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
	_30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
	_40, _41, _42, _43, _44, _45, _46, _47, _48, _49, \
	_50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
	_60, _61, _62, _63, _64, N, ...) N

#define ARGS_RECURSIVE_EXPAND( N ) ARGS_PLACEHOLDERS_##N(N)

//======================================================
//  MSVC中统计参数个数的宏需要进行二次转换才能正常

//#define ARG_T(t) t

//#define ARG_N(_1,_2,_3,_4,_5,_6,N,...) N

//#define ARG_N_HELPER(...) ARG_T(ARG_N(__VA_ARGS__))

//#define ARGS_COUNT(...) ARGS_ADDARG(ARG_N_HELPER(__VA_ARGS__,6,5,4,3,2,1),__VA_ARGS__)


//#define ARGS_ADDARG(_1,_2,...) ARGS_COUNT_2(ARG##_2##_COUNT_ZERO)

//zero数量多于变量数
//
//#define ARG_COUNT_ZERO 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

//#define ARG_T_2(t) t

//#define ARG_N_2(_1,_2,_3,_4,_5,_6,N,...) N

//#define ARG_N_HELPER_2(...) ARG_T_2(ARG_N_2(__VA_ARGS__))

//#define ARGS_COUNT_2(...) ARG_N_HELPER_2(__VA_ARGS__,6,5,4,3,2,1)

//======================================================


//占位符展开  最初的打算用bind来关联，后面换成lambda 弃用了
#define ARGS_PLACEHOLDERS( N ) std::placeholders::_##N

#define ARGS_PLACEHOLDERS_0(N) 
#define ARGS_PLACEHOLDERS_1(N) ARGS_PLACEHOLDERS( N)
#define ARGS_PLACEHOLDERS_2(N) ARGS_PLACEHOLDERS_1(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_3(N) ARGS_PLACEHOLDERS_2(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_4(N) ARGS_PLACEHOLDERS_3(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_5(N) ARGS_PLACEHOLDERS_4(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_6(N) ARGS_PLACEHOLDERS_5(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_7(N) ARGS_PLACEHOLDERS_6(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_8(N) ARGS_PLACEHOLDERS_7(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_9(N) ARGS_PLACEHOLDERS_8(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_10(N) ARGS_PLACEHOLDERS_9(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_11(N) ARGS_PLACEHOLDERS_10(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_12(N) ARGS_PLACEHOLDERS_11(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_13(N) ARGS_PLACEHOLDERS_12(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_14(N) ARGS_PLACEHOLDERS_13(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_15(N) ARGS_PLACEHOLDERS_14(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_16(N) ARGS_PLACEHOLDERS_15(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_17(N) ARGS_PLACEHOLDERS_16(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_18(N) ARGS_PLACEHOLDERS_17(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_19(N) ARGS_PLACEHOLDERS_18(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_20(N) ARGS_PLACEHOLDERS_19(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_21(N) ARGS_PLACEHOLDERS_20(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_22(N) ARGS_PLACEHOLDERS_21(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_23(N) ARGS_PLACEHOLDERS_22(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_24(N) ARGS_PLACEHOLDERS_23(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_25(N) ARGS_PLACEHOLDERS_24(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_26(N) ARGS_PLACEHOLDERS_25(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_27(N) ARGS_PLACEHOLDERS_26(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_28(N) ARGS_PLACEHOLDERS_27(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_29(N) ARGS_PLACEHOLDERS_28(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_30(N) ARGS_PLACEHOLDERS_29(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_31(N) ARGS_PLACEHOLDERS_30(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_32(N) ARGS_PLACEHOLDERS_31(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_33(N) ARGS_PLACEHOLDERS_32(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_34(N) ARGS_PLACEHOLDERS_33(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_35(N) ARGS_PLACEHOLDERS_34(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_36(N) ARGS_PLACEHOLDERS_35(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_37(N) ARGS_PLACEHOLDERS_36(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_38(N) ARGS_PLACEHOLDERS_37(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_39(N) ARGS_PLACEHOLDERS_38(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_40(N) ARGS_PLACEHOLDERS_39(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_41(N) ARGS_PLACEHOLDERS_40(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_42(N) ARGS_PLACEHOLDERS_41(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_43(N) ARGS_PLACEHOLDERS_42(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_44(N) ARGS_PLACEHOLDERS_43(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_45(N) ARGS_PLACEHOLDERS_44(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_46(N) ARGS_PLACEHOLDERS_45(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_47(N) ARGS_PLACEHOLDERS_46(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_48(N) ARGS_PLACEHOLDERS_47(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_49(N) ARGS_PLACEHOLDERS_48(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_50(N) ARGS_PLACEHOLDERS_49(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_51(N) ARGS_PLACEHOLDERS_50(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_52(N) ARGS_PLACEHOLDERS_51(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_53(N) ARGS_PLACEHOLDERS_52(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_54(N) ARGS_PLACEHOLDERS_53(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_55(N) ARGS_PLACEHOLDERS_54(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_56(N) ARGS_PLACEHOLDERS_55(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_57(N) ARGS_PLACEHOLDERS_56(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_58(N) ARGS_PLACEHOLDERS_57(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_59(N) ARGS_PLACEHOLDERS_58(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_60(N) ARGS_PLACEHOLDERS_59(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_61(N) ARGS_PLACEHOLDERS_60(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_62(N) ARGS_PLACEHOLDERS_61(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )
#define ARGS_PLACEHOLDERS_63(N) ARGS_PLACEHOLDERS_62(ARGS_DEV(N)),ARGS_PLACEHOLDERS( N )

#define ARGS_DEV(N) ARGS_##N##_Dev

#define ARGS_1_Dev 
#define ARGS_2_Dev 1
#define ARGS_3_Dev 2
#define ARGS_4_Dev 3
#define ARGS_5_Dev 4
#define ARGS_6_Dev 5
#define ARGS_7_Dev 6
#define ARGS_8_Dev 7
#define ARGS_9_Dev 8
#define ARGS_10_Dev 9
#define ARGS_11_Dev 10
#define ARGS_12_Dev 11
#define ARGS_13_Dev 12
#define ARGS_14_Dev 13
#define ARGS_15_Dev 14
#define ARGS_16_Dev 15
#define ARGS_17_Dev 16
#define ARGS_18_Dev 17
#define ARGS_19_Dev 18
#define ARGS_20_Dev 19
#define ARGS_21_Dev 20
#define ARGS_22_Dev 21
#define ARGS_23_Dev 22
#define ARGS_24_Dev 23
#define ARGS_25_Dev 24
#define ARGS_26_Dev 25
#define ARGS_27_Dev 26
#define ARGS_28_Dev 27
#define ARGS_29_Dev 28
#define ARGS_30_Dev 29
#define ARGS_31_Dev 30
#define ARGS_32_Dev 31
#define ARGS_33_Dev 32
#define ARGS_34_Dev 33
#define ARGS_35_Dev 34
#define ARGS_36_Dev 35
#define ARGS_37_Dev 36
#define ARGS_38_Dev 37
#define ARGS_39_Dev 38
#define ARGS_40_Dev 39
#define ARGS_41_Dev 40
#define ARGS_42_Dev 41
#define ARGS_43_Dev 42
#define ARGS_44_Dev 43
#define ARGS_45_Dev 44
#define ARGS_46_Dev 45
#define ARGS_47_Dev 46
#define ARGS_48_Dev 47
#define ARGS_49_Dev 48
#define ARGS_50_Dev 49
#define ARGS_51_Dev 50
#define ARGS_52_Dev 51
#define ARGS_53_Dev 52
#define ARGS_54_Dev 53
#define ARGS_55_Dev 54
#define ARGS_56_Dev 55
#define ARGS_57_Dev 56
#define ARGS_58_Dev 57
#define ARGS_59_Dev 58
#define ARGS_60_Dev 59
#define ARGS_61_Dev 60
#define ARGS_62_Dev 61
#define ARGS_63_Dev 62



//输入一个函数名，参数，根据参数个数展开函数传入单个参数的结果   (f,x,y)  --> f(x),f(y)
// 但是！MSVC中预处理阶段会把 decltype 处理成这样 (f,x,y)  --> f(x,y)

#define FUNCTION_FUCK_MSVC

#define FUNCTION_EXPAND(Function,...)  FUNCTION_EXPAND_(Function,ARGS_COUNT(__VA_ARGS__),__VA_ARGS__)

#define FUNCTION_EXPAND_(F,N,...) FUNCTION_RECURSIVE_EXPAND(F,N,__VA_ARGS__)

#define FUNCTION_RECURSIVE_EXPAND(F,N,...)  FUNCTION_##N(F,__VA_ARGS__)

//二级宏展开
#define FUNCTION_I(X) FUNCTION_II(X)
#define FUNCTION_II(X) FUNCTION_##X


#define FUNCTION_I_WRAPPER(F,N,...) (N>0 ? (FUNCTION_I(N)(F,__VA_ARGS__)) :FUNCTION_I(N)() )

#define FUNCTION_END(...)
#define FUNCTION_0(F,...)  FUNCTION_END(__VA_ARGS__)

//=====
//#define FUNCTION_1(F,_1)     F(_1)
//#define FUNCTION_2(F,_1,_2,...)     F(_1),F(_2)
//#define FUNCTION_3(F,_1,_2,_3,...)     F(_1),F(_2),F(_3)
//#define FUNCTION_4(F,_1,_2,_3,_4,...)     F(_1),F(_2),F(_3), F(_4)
//#define FUNCTION_5(F,_1,_2,_3,_4,_5,...)     F(_1),F(_2),F(_3), F(_4), F(_5)
//#define FUNCTION_6(F,_1,_2,_3,_4,_5,_6...)     F(_1),F(_2),F(_3), F(_4), F(_5),F(_6)
//=====

#define FUNCTION_1(F,_1,...)   F(_1)
#define FUNCTION_2(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_3(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_4(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_5(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_6(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_7(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_8(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_9(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_10(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_11(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_12(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_13(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_14(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_15(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_16(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_17(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_18(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_19(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_20(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_21(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_22(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_23(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_24(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_25(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_26(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_27(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_28(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_29(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_30(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_31(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_32(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_33(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_34(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_35(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_36(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_37(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_38(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_39(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_40(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_41(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_42(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_43(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_44(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_45(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_46(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_47(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_48(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_49(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_50(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_51(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_52(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_53(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_54(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_55(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_56(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_57(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_58(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_59(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_60(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_61(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_62(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)
#define FUNCTION_63(F,_1,...)  F(_1) ,FUNCTION_I(ARGS_COUNT(__VA_ARGS__))(F,__VA_ARGS__)

#define DECLTYPE(x)  decltype(x)
//================ARGS TYPE




//====================================ARGS  SOLUTION======================================================


//======================BOOST 的方法 网上找的 不完整=============================
#define ARGS_CAT(A,B) ARGS_CAT_(A,B)
#define ARGS_CAT_(A,B) A##B

//以下是条件执行的宏
#define ARGS_IF(cond,t,f) ARGS_IIF( ARGS_BOOL(cond),t,f)

#define ARGS_IIF(bit,t,f) ARGS_IIF_I(bit,t,f)
#define ARGS_IIF_I(bit,t,f) ARGS_IIF_ ## bit(t,f)

#define ARGS_IIF_0(t,f) f
#define ARGS_IIF_1(t,f) t

#define ARGS_BOOL(x) ARGS_BOOL_I(x)
#define ARGS_BOOL_I(x) ARGS_BOOL_ ## x

#define ARGS_BOOL_0 0
#define ARGS_BOOL_1 1
#define ARGS_BOOL_2 1

//======================BOOST=============================

#endif // SIGNALMANAGER_H
