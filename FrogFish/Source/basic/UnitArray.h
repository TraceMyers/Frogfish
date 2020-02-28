#pragma once
#pragma message("including UnitArray")

#include <BWAPI.h> 

namespace Basic {

    class UnitArray {

        /* PUBLIC */
        public:

        UnitArray();
        void              add(const BWAPI::Unit u);
        bool              remove(const BWAPI::Unit u);
        bool              remove_at(int i);
        int               size() const;
        int               find(const BWAPI::Unit u) const;
        const BWAPI::Unit get_by_ID(int ID) const;
        bool              remove_by_ID(int ID);
        void              clear();
        const BWAPI::Unit operator [] (int i) const;

        /* PRIVATE */
        private:
        
        static const int UNITARRAY_INIT_SIZE = 50;
        static const int UNITARRAY_RESIZE_CONST = 100;
        BWAPI::Unit *    array;
        int              internal_size;
        int              _size;

        void resize();
    };

}