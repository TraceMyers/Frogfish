#pragma once

#include "../../data/FrogBase.h"
#include <string.h>

#define FBARRAY_INIT_SIZE 15 
#define FBARRAY_RESIZE_CONST 5

class FBArray {

private:

    FBase *array = new FBase[FBARRAY_INIT_SIZE];
    int size = FBARRAY_INIT_SIZE;
    int len = 0;

    void resize() {
        FBase *temp = new FBase[size + FBARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(FBase) * size);
        delete array;
        array = temp;
        size += FBARRAY_RESIZE_CONST;
    }

public:

    void add(FBase f_base) {
        array[len] = f_base;
        len++;
        if (len == size) {
            resize();
        }
    }

    FBase remove(FBase f_base) {
        FBase rm_f_base = nullptr;
        for (register int i = 0; i < len; i++) {
            if (array[i] == f_base) {
                rm_f_base = array[i];
                array[i] = array[len - 1];
                len--;
                break;
            }    
        }
        return rm_f_base;
    }

    FBase remove_at(int i) {
        FBase rm_f_base = nullptr;
        if (i < len && len > 0) {
            rm_f_base = array[i];
            array[i] = array[len - 1];
            len--;
        }
        return rm_f_base;
    }

    int length() const {
        return len;
    }

    const FBase operator [](int i) const {
        return array[i];
    }

    int find(FBase f_base) const {
        for (register int i = 0; i < len; i++) {
            if (array[i] == f_base) {
                return i;
            }    
        }
        return -1;
    }

    // ONLY for use when this storage site is not primary storage
    // for the base (eg BaseStorage.self_newly_stored)
    void clear() {
        len = 0;
    }

    void free_data() {
        // only for use at end of program
        // Being that this is the only storage used for FrogBase pointers,
        // one should use a loop that does:
        //      FBase f_base = fb_array.remove_at(0);
        //      f_base.free_data();
        //      delete f_base;
        // ... until this FBArray is empty before calling free_data() here
        delete array;
    }
};