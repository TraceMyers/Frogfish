#pragma once

#include "../../data/EnemyBase.h"
#include <string>

#define EBARRAY_INIT_SIZE 15 
#define EBARRAY_RESIZE_CONST 5

class EBArray {

private:

    EBase *array = new EBase[EBARRAY_INIT_SIZE];
    int size = EBARRAY_INIT_SIZE;
    int len = 0;

    void resize() {
        EBase *temp = new EBase[size + EBARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(EBase) * size);
        delete array;
        array = temp;
        size += EBARRAY_RESIZE_CONST;
    }

public:

    void add(EBase e_base) {
        array[len] = e_base;
        len++;
        if (len == size) {
            resize();
        }
    }

    EBase remove(EBase e_base) {
        EBase rm_e_base = nullptr;
        for (register int i = 0; i < len; i++) {
            if (array[i] == e_base) {
                rm_e_base = array[i];
                array[i] = array[len - 1];
                len--;
                break;
            }    
        }
        return rm_e_base;
    }

    EBase remove_at(int i) {
        EBase rm_e_base = nullptr;
        if (i < len && len > 0) {
            rm_e_base = array[i];
            array[i] = array[len - 1];
            len--;
        }
        return rm_e_base;
    }

    int length() {
        return len;
    }

    const EBase operator [](int i) {
        return array[i];
    }

    int find(EBase e_base) {
        for (register int i = 0; i < len; i++) {
            if (array[i] == e_base) {
                return i;
            }    
        }
        return -1;
    }

    // ONLY for use when this storage site is not primary storage
    // for the base (eg BaseStorage.enemy_newly_stored)
    void clear() {
        len = 0;
    }

    void free_data() {
        // Only for use at end of program.
        // Being that this is the only storage used for EnemyBase pointers,
        // one should use a loop that does:
        //      EBase e_base = eb_array.remove_at(0);
        //      e_base.free_data();
        //      delete e_base;
        // ... until this EBArray is empty before calling free_data() here
        delete array;
    }
};