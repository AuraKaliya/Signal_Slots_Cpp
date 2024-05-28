#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <string>
#include <functional>
#include <map>

//===============================================信号-槽 管理器==========================================

class SignalManager
{
public:

    template<typename... Args>
    auto RegisterSignal(const std::string & signalName,const std::string & emitObjGID,std::function<void(Args...)> slot)
    {

        if(m_signalToSlotDictionary<Args...>.find(signalName)==m_signalToSlotDictionary<Args...>.end())
        {
            m_signalToSlotDictionary<Args...>.insert(std::make_pair(signalName,std::multimap<std::string,std::function<void(Args...)> >{}));
        }

        m_signalToSlotDictionary<Args...>[signalName].insert(std::make_pair(emitObjGID,slot));
    }

    template<typename... Args>
    auto SoluteSignal(const std::string& signalName,const std::string emitObjGID,std::tuple<Args...> args)
    {
        auto it=m_signalToSlotDictionary<Args...>.find(signalName);
        if(it!=m_signalToSlotDictionary<Args...>.end())
        {
            auto range = it->second.equal_range(emitObjGID);
            for(auto function=range.first;function !=range.second;function ++)
            {
                std::apply(function->second,args);
            }
        }
    }
    template<typename... Args>
    SignalManager(Args&&... args) {};

private:
    template<typename... Args>
    static std::map<std::string,std::multimap<std::string,std::function<void(Args...)> > > m_signalToSlotDictionary;
};

template<typename... Args>
std::map<std::string,std::multimap<std::string,std::function<void(Args...)> > > SignalManager::m_signalToSlotDictionary{};

//===============================================信号-槽 管理器==========================================

//=============================register 相关   信号注册的宏  ==============================

#define REGISTER_SIGNAL_STATIC(signalObj,signalName,slotFunc,... ) \
do { \
        std::function<void(__VA_ARGS__)> func =[](auto&&... args)->decltype(auto) \
    {\
            return (*slotFunc)(std::forward<decltype(args)>(args)...);\
    };\
        auto tmpManager=std::apply([](auto&&...args){\
                  return SignalManager{args...};\
          },std::tuple<__VA_ARGS__>{});\
        tmpManager.RegisterSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)), func); \
} while (false)
//   ++++++++++++++++++++++++++++由于REGISTER_SIGNAL也能处理多参情况  所以合并使用 +++++++++++++++++++++++++++++++++++

#define REGISTER_SIGNAL(signalObj,slotObj,signalName,slotFunc,... ) \
do { \
        std::function<void(__VA_ARGS__)> func =[slotObj](auto&&... args)->decltype(auto) \
    {\
            return (slotObj->*slotFunc)(std::forward<decltype(args)>(args)...);\
    };\
        auto tmpManager=std::apply([](auto&&...args){\
                    return SignalManager{args...};\
                },std::tuple<__VA_ARGS__>{});\
        tmpManager.RegisterSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)), func); \
} while (false)
//=============================register==============================

//===========================emit  相关 信号发射宏=======================================
#define EMIT_SIGNAL(signalObj, signalName,...) \
do { \
auto tmpManager=std::apply([](auto&&...args){\
        return SignalManager{args...};\
},std::tuple{__VA_ARGS__});\
tmpManager.SoluteSignal(signalName, std::to_string(reinterpret_cast<uintptr_t>(signalObj)), std::tuple{__VA_ARGS__});\
} while (false)
//===========================emit=======================================


#endif // SIGNALMANAGER_H
