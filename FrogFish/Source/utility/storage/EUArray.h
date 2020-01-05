#pragma once

#include "../../data/EnemyUnit.h"
#include <string.h>

#define EUARRAY_INIT_SIZE 50
#define EUARRAY_RESIZE_CONST 25

class EUArray {

private:

    EUnit *array = new EUnit[EUARRAY_INIT_SIZE];
    int size = EUARRAY_INIT_SIZE;
    int len = 0;

    void resize() {
        EUnit *temp = new EUnit[size + EUARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(EUnit) * size);
        delete array;
        array = temp;
        size += EUARRAY_RESIZE_CONST;
    }

public:

    void add(EUnit e_unit) {
        array[len] = e_unit;
        len++;
        if (len == size) {
            resize();
        }
    }

    void remove(int ID) {
        for (register int i = 0; i < len; i++) {
            if ((array[i])->get_ID() == ID) {
                array[i] = array[len - 1];
                len--;
                break;
            }    
        }
    }

    void clear() {
        len = 0;
    }

    int length() const {
        return len;
    }

    const EUnit operator [] (int i) const {
        return array[i];
    }

    EUnit find(int ID) const {
        for (register int i = 0; i < len; i++) {
            if ((array[i])->get_ID() == ID) {
                return array[i];
            }    
        }
        return nullptr;
    }

    void free_data() {
        // only for use at end of program
        delete array;
    }
};