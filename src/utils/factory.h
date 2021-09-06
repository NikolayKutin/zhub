// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

    template <class ID, class Base, class... ConstuctorArgs> // TODO Key
    class GenericObjectFactory
    {
    private:
        typedef std::shared_ptr<Base>(*fInstantiator)(ConstuctorArgs...);

        template <class Derived>
        static std::shared_ptr<Base> instantiator(ConstuctorArgs... args)
        {
            return std::make_shared<Derived>(args...);
        }

        static std::shared_ptr<Base> empty_instantiator(ConstuctorArgs... args)
        {
            return std::shared_ptr<Base>();
        }

        std::map<ID, fInstantiator> classes;

    public:
        GenericObjectFactory() {}
        template <class Derived>
        void add(ID id)
        {
            classes[id] = &instantiator<Derived>;
        }
        fInstantiator get(ID id)
        {
            if(classes.count(id))
                return classes[id];
            else
                return &empty_instantiator;       
        }
    };