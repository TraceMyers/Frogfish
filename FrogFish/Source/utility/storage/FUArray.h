#ifndef FU_ARRAY
#define FU_ARRAY

#include "../data/FrogUnit.h"
#include <string.h>

#define FUNIT_INIT_LEN 30
#define FUNIT_RESIZE_CONST 20

class FUArray {

private:

    FUnit *array = new FUnit[FUNIT_INIT_LEN];
    int size = FUNIT_INIT_LEN;
    int len = 0;

    void resize() {
        FUnit *temp = new FUnit[size + FUNIT_RESIZE_CONST];
        memcpy(temp, array, sizeof(FUnit) * size);
        array = temp;
        size += FUNIT_RESIZE_CONST;
    }

public:

    void add(FUnit f_unit) {
        array[len] = f_unit;
        len++;
        if (len == size) {
            resize();
        }
    }

    void remove(int ID) {
        const register int last_val_i = len - 1;
        for (register int i = 0; i < len; i++) {
            if ((array[i])->get_ID() == ID) {
                if (i != last_val_i) {
                    array[i] = array[last_val_i];
                }
                if (len > 0) {
                    len--;
                }
                break;
            }    
        }
    }

    void clear() {
        len = 0;
    }

    int length() {
        return len;
    }

    FUnit operator [](int i) {
        return array[i];
    }

    FUnit find(int ID) {
        const register int last_val_i = len - 1;
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

#endif