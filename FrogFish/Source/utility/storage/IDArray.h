#pragma once

#include <string.h>

#define IDARRAY_INIT_SIZE 800
#define IDARRAY_RESIZE_CONST 100

class IDArray {

private:

    int *array = new int[IDARRAY_INIT_SIZE];
    int size = IDARRAY_INIT_SIZE;
    int len = 0;

    void resize() {
        int *temp = new int[size + IDARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(int) * size);
        delete array;
        array = temp;
        size += IDARRAY_RESIZE_CONST;
    }

public:

    void add(int f_unit) {
        array[len] = f_unit;
        len++;
        if (len == size) {
            resize();
        }
    }

    void remove(int ID) {
        for (register int i = 0; i < len; i++) {
            if (array[i] == ID) {
                array[i] = array[len - 1];
                len--;
                break;
            }    
        }
    }

    void clear() {
        len = 0;
    }

    bool has(int ID) {
        for (register int i = 0; i < len; i++) {
            if (array[i] == ID) {
                return true;
            }    
        }
        return false;
    }

    void free_data() {
        // only for use at end of program
        // called by owner
        delete array;
    }
};