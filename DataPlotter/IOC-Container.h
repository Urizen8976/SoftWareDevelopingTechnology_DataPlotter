#ifndef IOCCONTAINER_H
#define IOCCONTAINER_H


#include <functional>
#include <iostream>
#include <memory>
#include <map>          //  Библ. с шаблонным классом для реализации фссоциативного контейнера (коллекции фабрик определенных типов)
#include <string>


using namespace std;


class IOCContainer
{
    static int s_nextTypeId; // Значение следующего идентификатора, статическая переменная внутри класса по умолчанию хранит 0
    template<typename T> static int GetTypeID() // Шаблонная функция для определенного типа T, возвращает значение идентефикатора типа
    {
        static int typeId = s_nextTypeId++;
        return typeId;
    }

public:
    class FactoryRoot // Абстрактный базовый класс для всех фабрик, которые предоставляют способы создания объектов
    {
    public: virtual ~FactoryRoot() {}
    };

    // Коллекция фабрик для определенных типов объектов в формате: ключ - значение
    std::map<int, std::shared_ptr<FactoryRoot>> m_factories;

    //-----------------------------------------------------------------------------------------------------------

    // Фабрика определенного типа T
    template<typename T> class CFactory : public FactoryRoot
    {
        std::function<std::shared_ptr<T>()> m_functor;  // Функтор для создания и возврата объекта - объект, вызываемый как функция

    public:
        ~CFactory() {}
        CFactory(std::function<std::shared_ptr<T>()> functor) : m_functor(functor) {}
        std::shared_ptr<T> GetObject() {return m_functor();} // Создает объект
    };

    //-----------------------------------------------------------------------------------------------------------

    //Получить экземпляр объекта из контейнера
    template<typename T> std::shared_ptr<T> GetObject()
    {
        auto typeId = GetTypeID<T>();                                       // Получаем typeId фабрики
        auto factoryBase = m_factories[typeId];                             // Получаем необходимую нам фабрику
        auto factory = std::static_pointer_cast<CFactory<T>>(factoryBase);  // Приводим указатель к типу конкретной фабрики
        return factory->GetObject();                                        // Возвращаем объект созданный с использованием заданного функтора фабрики
    }

    //-----------------------------------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------------------------------

    //Регистрация одного экземпляра объекта
    template<typename TInterface> void RegisterInstance(std::shared_ptr<TInterface> t)
    {
        // Аналогичная работа функции, однако в данном случае регистрируем только лишь один экземпляр объекта
        m_factories[GetTypeID<TInterface>()] = std::make_shared<CFactory<TInterface>>([ = ] { return t; });
    }

    //-----------------------------------------------------------------------------------------------------------

    // Перегрузка функции RegisterFunctor, вызов по указателю на functor
    template<typename TInterface, typename... TS> void RegisterFunctor(std::shared_ptr<TInterface> (*functor)(std::shared_ptr<TS>... ts))
    {
        RegisterFunctor(std::function<std::shared_ptr<TInterface>(std::shared_ptr<TS>... ts)>(functor));
    }

    //-----------------------------------------------------------------------------------------------------------

    //Фабрика, которая будет вызывать конструктор, для каждого экземпляра
    //Регистрация фабрики для создания объектов, где TInterface - интерфейс,
    //TConcrete  - конкретная реализация интерфейса, TArguments - набор параметор передаваемых в конструктор TConcrete
    template<typename TInterface, typename TConcrete, typename... TArguments> void RegisterFactory()
    {
        RegisterFunctor(
            std::function<std::shared_ptr<TInterface>(std::shared_ptr<TArguments>... ts)>(
                [](std::shared_ptr<TArguments>... arguments) -> std::shared_ptr<TInterface> {
                    return std::make_shared<TConcrete>(
                        std::forward<std::shared_ptr<TArguments>>(arguments)...);
                }));
    }

    //-----------------------------------------------------------------------------------------------------------

    // Перегрузка функции RegisterInstance, классы нижнего уровня (и аргументы) передаются по другому
    template<typename TInterface, typename TConcrete, typename... TArguments> void RegisterInstance()
    {
        RegisterInstance<TInterface>(std::make_shared<TConcrete>(GetObject<TArguments>()...));
    }
};


#endif // IOCCONTAINER_H
