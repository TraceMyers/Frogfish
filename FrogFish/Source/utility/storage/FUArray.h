#pragma once

#include "../../data/FrogUnit.h"
#include <string>

#define FUARRAY_INIT_SIZE 30
#define FUARRAY_RESIZE_CONST 20

class FUArray {

private:

    FUnit *array = new FUnit[FUARRAY_INIT_SIZE];
    int size = FUARRAY_INIT_SIZE;
    int len = 0;

    void resize() {
        FUnit *temp = new FUnit[size + FUARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(FUnit) * size);
        delete array;
        array = temp;
        size += FUARRAY_RESIZE_CONST;
    }

public:

    void add(FUnit f_unit) {
        array[len] = f_unit;
        len++;
        if (len == size) {
            resize();
        }
    }

    bool remove(int ID) {
        for (register int i = 0; i < len; i++) {
            if ((array[i])->get_ID() == ID) {
                array[i] = array[len - 1];
                len--;
                return true;
            }    
        }
        return false;
    }

    void clear() {
        len = 0;
    }

    int length() const {
        return len;
    }

    const FUnit operator [](int i) const {
        return array[i];
    }

    FUnit find(int ID) {
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