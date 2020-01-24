#include <BWAPI.h>

namespace UnitData {

    class UnitArray {

    public:

        void add(const BWAPI::UnitInterface *u) {
            array[_size] = u;
        }

    private:
        
        const int UNITARRAY_INIT_SIZE = 50;
        const int UNITARRAY_RESIZE_CONST = 100;

        const BWAPI::UnitInterface **array = 
            new const BWAPI::UnitInterface *[UNITARRAY_INIT_SIZE];
        int internal_size = UNITARRAY_INIT_SIZE;
        int _size = 0;

        void resize() {
            const BWAPI::UnitInterface **temp 
                = new const BWAPI::UnitInterface *[internal_size + UNITARRAY_RESIZE_CONST];
            memcpy((void *)temp, array, sizeof(const BWAPI::Unit) * internal_size);
            delete array; 
            array = temp;
            internal_size += UNITARRAY_RESIZE_CONST;
        }
    };

}