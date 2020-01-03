#ifndef UNIT_BUFF
#define UNIT_BUFF

#include <string.h>
#include <BWAPI.h>

#define UNITBUFF_INIT_LEN 30
#define UNITBUFF_RESIZE_CONST 20

class UnitBuff {

private:

    BWAPI::Unit *array = new BWAPI::Unit[UNITBUFF_INIT_LEN];
    int size = UNITBUFF_INIT_LEN;
    int len ;

    void resize() {
        BWAPI::Unit *temp = new BWAPI::Unit[size + UNITBUFF_RESIZE_CONST];
        memcpy(temp, array, sizeof(BWAPI::Unit) * size);
        array = temp;
        size += UNITBUFF_RESIZE_CONST;
    }

public:

    void add(const BWAPI::Unit b_unit) {
        array[len] = b_unit;
        len++;
        if (len == size) {
            resize();
        }
    }

    bool has(const BWAPI::Unit b_unit) {
        for (register int i = 0; i < len; i++) {
            if (array[i]->getID() == b_unit->getID()) {
                return true;
            }
        }
        return false;
    }
    
    int length() {
        return len;
    }

    void clear() {
        len = 0;
    }

    const BWAPI::Unit operator [](int i) {
        return array[i];
    }

    void free_data() {
        // only for use at end of program
        delete array;
    }
};

#endif