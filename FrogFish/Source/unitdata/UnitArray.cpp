#include <BWAPI.h>

namespace UnitData {

    class UnitArray {

    public:

        void add(const BWAPI::Unit *u) {
            array[_size] = u;
        }

    private:
        
        const int UNITARRAY_INIT_SIZE = 50;
        const int UNITARRAY_RESIZE_CONST = 100;

        const BWAPI::Unit *array = 
            new const BWAPI::Unit [UNITARRAY_INIT_SIZE];
        int internal_size = UNITARRAY_INIT_SIZE;
        int _size = 0;

        void resize() {
            const BWAPI::Unit *temp 
                = new const BWAPI::Unit [internal_size + UNITARRAY_RESIZE_CONST];
            memcpy((void *)temp, array, sizeof(const BWAPI::Unit) * internal_size);
            delete array; 
            array = temp;
            internal_size += UNITARRAY_RESIZE_CONST;
        }
    };

}